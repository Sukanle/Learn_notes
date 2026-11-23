# 15 · 并发：线程、Channel 与共享状态

> 对应 The Book 第 16 章
> 示例 crate：[`concurrency`](../concurrency)
> 运行：`cargo run`、`cargo run --example thread_scope`

Rust 的目标不是“让并发变简单”，而是**让数据竞争在编译期不可能**（safe code 内）。
代价是你需要显式选择共享方式：消息传递、`Mutex`、原子操作或无共享。

---

## 1. 线程基础

```rust
use std::thread;
use std::time::Duration;

let handle = thread::spawn(|| {
    for i in 1..=3 {
        println!("子线程 {i}");
        thread::sleep(Duration::from_millis(10));
    }
});

handle.join().unwrap();   // 等待并取得返回值
```

| Rust | C++ |
| --- | --- |
| `thread::spawn` | `std::thread` / `std::jthread` |
| `JoinHandle::join` | `thread::join` |
| 不 join 则 detach（drop handle） | `std::thread` 析构时 terminate；`jthread` 自动 join |
| `thread::sleep` | `std::this_thread::sleep_for` |
| `thread::scope` | 无直接对应（类似 `jthread` + 手动 join） |
| `thread::available_parallelism` | `std::thread::hardware_concurrency` |

注意：Rust 的 `JoinHandle` 被 drop 时**不会**自动 join（线程继续 detach 运行）。
C++ 的 `std::thread` 析构会 `std::terminate`，`std::jthread` 会 join；
Rust 与 `jthread` 更接近，但需要显式 `join()` 或使用 `thread::scope`。

### 1.1 `move` 闭包

```rust
let data = vec![1, 2, 3, 4, 5];
let handle = thread::spawn(move || data.iter().sum::<i32>());
println!("{}", handle.join().unwrap());
// println!("{data:?}");   // error: data 已被 move
```

线程可能比创建它的函数活得久，因此不能借用栈上的局部变量。
`move` 把所有权交给闭包；返回值/错误通过 `join()` 传回。

### 1.2 `thread::scope`：借用栈数据

```rust
let data: Vec<u64> = (1..=1_000_000).collect();

let sums: Vec<u64> = thread::scope(|scope| {
    let handles: Vec<_> = data
        .chunks(250_000)
        .map(|chunk| scope.spawn(move || chunk.iter().map(|x| x * x).sum::<u64>()))
        .collect();

    handles.into_iter().map(|h| h.join().unwrap()).collect()
});
```

- scope 内的线程可以借用外部的 `data`，不需要 `Arc`；
- scope 结束前会 join 所有线程，保证不会悬垂；
- `chunks` 给每个线程一个不重叠的 `&[u64]`，借用检查器能证明安全；
- 可以配合 `chunks_mut` / `iter_mut` 并行修改不同元素。

这类似 C++ 中用 `std::span` + 手动 join，但安全由编译器保证。

---

## 2. 消息传递：`mpsc::channel`

```rust
use std::sync::mpsc;
use std::thread;

let (tx, rx) = mpsc::channel();

thread::spawn(move || {
    tx.send(String::from("hello")).unwrap();
});

let received = rx.recv().unwrap();
println!("{received}");
```

- `mpsc` = multi-producer, single-consumer；
- `send` 取得值的所有权（避免悬垂）；
- `recv` 阻塞，返回 `Result<T, RecvError>`；
- 所有 sender 被 drop 后，`recv` 返回 `Err`，`for msg in rx` 结束。

```rust
let (tx, rx) = mpsc::channel();
let tx1 = tx.clone();

thread::spawn(move || tx.send(1).unwrap());
thread::spawn(move || tx1.send(2).unwrap());

for msg in rx {
    println!("{msg}");
}
```

C++ 对照：

```cpp
// 需要自己组合 mutex + condition_variable + queue
std::queue<int> q;
std::mutex m;
std::condition_variable cv;
```

Rust 的 channel 内部就是这个模式，但你不需要手写锁和条件变量。

### 2.1 有界 channel

```rust
let (tx, rx) = mpsc::sync_channel(10);   // 缓冲区 10
tx.send(1).unwrap();                     // 满时阻塞
```

`sync_channel(0)` 是 rendezvous（发送方阻塞到接收方取走），类似同步握手。

---

## 3. 共享状态：`Mutex<T>`

```rust
use std::sync::{Arc, Mutex};

let counter = Arc::new(Mutex::new(0));
let mut handles = Vec::new();

for _ in 0..10 {
    let counter = Arc::clone(&counter);
    handles.push(std::thread::spawn(move || {
        let mut guard = counter.lock().unwrap();
        *guard += 1;
    }));   // guard 在此 drop，自动解锁
}

for h in handles { h.join().unwrap(); }
println!("{}", *counter.lock().unwrap());
```

要点：

- `Mutex<T>` 把**锁和数据绑定**，不可能忘记加锁；
- `lock()` 返回 `MutexGuard<T>`，RAII 自动解锁（类似 `lock_guard`）；
- 守卫实现了 `Deref`/`DerefMut`，可以像 `T` 一样使用；
- 如果持锁线程 panic，锁会 **poisoned**，`lock()` 返回 `Err`；
- `try_lock()` 不阻塞，返回 `TryLockError`。

```rust
let guard = counter.lock().unwrap();
// 锁在 guard 离开作用域时释放
```

C++ 对照：

```cpp
std::mutex m;
int counter = 0;

{
    std::lock_guard<std::mutex> lock(m);
    ++counter;
}
```

差异：

- Rust 的锁和数据是**同一个对象**，不存在“锁了 A 忘了锁 B”；
- 守卫离开作用域自动释放，提前 return / panic 也不会死锁（该线程的锁会 poison）；
- 没有 `std::recursive_mutex` 的直接对应（可用 `parking_lot::ReentrantMutex`）。

### 3.1 死锁与锁顺序

Rust 不能静态防止死锁，只能防止数据竞争。经典死锁：

```rust
// 线程 1：lock(a) 然后 lock(b)
// 线程 2：lock(b) 然后 lock(a)
```

避免方式：

- 固定锁顺序；
- 缩小临界区；
- 用 channel 代替共享状态；
- 用 `try_lock` + 超时；
- 用无锁结构（`crossbeam`、`arc-swap`）。

### 3.2 `RwLock`

```rust
use std::sync::RwLock;

let config = RwLock::new(vec![1, 2, 3]);

let read = config.read().unwrap();       // 多读者
drop(read);

let mut write = config.write().unwrap(); // 单写者
write.push(4);
```

读多写少时比 `Mutex` 更好；但实现依赖平台，写者饥饿策略不同。

### 3.3 原子类型

```rust
use std::sync::atomic::{AtomicUsize, Ordering};

let counter = AtomicUsize::new(0);
counter.fetch_add(1, Ordering::Relaxed);
println!("{}", counter.load(Ordering::Relaxed));
```

- 类似 `std::atomic<T>`；
- `Ordering` 对应 `memory_order_*`；
- 只适合简单类型（整数、指针、bool）；
- 复杂不变量仍需 `Mutex`。

---

## 4. `Send` 与 `Sync`

| trait | 含义 |
| --- | --- |
| `Send` | 所有权可以跨线程转移 |
| `Sync` | 引用 `&T` 可以跨线程共享 |

典型情况：

| 类型 | `Send` | `Sync` | 原因 |
| --- | --- | --- | --- |
| `i32`、`String` | ✅ | ✅ | 无共享可变状态 |
| `Rc<T>` | ❌ | ❌ | 引用计数非原子 |
| `RefCell<T>` | ✅ | ❌ | 运行时借用非线程安全 |
| `Arc<T>` | ✅（T: Send+Sync） | ✅（T: Send+Sync） | 原子计数 |
| `Mutex<T>` | ✅（T: Send） | ✅（T: Send） | 内部同步 |
| `Cell<T>` | ✅ | ❌ | 非原子 |
| `*const T` | ❌ | ❌ | 裸指针 |

编译器会拒绝：

```rust
let rc = std::rc::Rc::new(1);
std::thread::spawn(move || {
    println!("{rc}");
});
// error: `Rc<i32>` cannot be sent between threads safely
```

修复：换成 `Arc`。

> `Send`/`Sync` 是 `unsafe trait`，手写实现需要自己保证线程安全；
> 绝大多数情况下让编译器自动推导。

---

## 5. 数据竞争 vs 竞态条件

- **数据竞争（data race）**：两个线程同时访问同一内存位置，至少一个写，且没有同步。
  Rust 的 safe code 在编译期排除它。
- **竞态条件（race condition）**：结果依赖调度顺序，即使没有数据竞争也可能发生。
  Rust **不能**阻止，例如：

```rust
// 两个线程分别检查余额再取款，中间可能被调度
if balance >= amount {
    balance -= amount;   // 需要事务/原子操作
}
```

所以“Rust 无数据竞争”不等于“并发逻辑自动正确”。

---

## 6. C++ 并发对照

| C++23 | Rust |
| --- | --- |
| `std::thread` | `std::thread::spawn` |
| `std::jthread` | `std::thread` + 显式 join / `scope` |
| `std::mutex` + `lock_guard` | `Mutex<T>` + `MutexGuard` |
| `std::recursive_mutex` | 无直接对应 |
| `std::shared_mutex` | `RwLock` |
| `std::condition_variable` | `mpsc` / `Condvar` |
| `std::atomic<T>` | `AtomicT` + `Ordering` |
| `std::async` / `future` | `thread` + channel / `rayon` / async |
| C++17 并行算法 | `rayon` / `thread::scope` |
| `std::latch` / `barrier` | `std::sync::Barrier` / `OnceLock` |
| `thread_local` | `thread_local!` |
| `std::execution`（C++26） | `tokio` / `async-std` / `smol` |

Rust 标准库**没有**内置线程池或 async runtime；生产项目通常使用：

- `rayon`：数据并行（类似并行算法）；
- `tokio` / `async-std`：异步 I/O；
- `crossbeam`：高级并发数据结构；
- `parking_lot`：更快的锁。

---

## 7. `concurrency` crate 走读

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | `spawn`/`join`、`move` 闭包、`mpsc` 多生产者、`Arc<Mutex>` 计数 |
| `examples/thread_scope.rs` | `thread::scope` 借用分块、`chunks_mut` 并行写回 |

```bash
cd concurrency
cargo run
cargo run --example thread_scope
```

`examples/thread_scope.rs` 的核心：

```rust
let mut buffer = vec![0u64; 4];

thread::scope(|scope| {
    for (slot, value) in buffer.iter_mut().zip(1..=4u64) {
        scope.spawn(move || *slot = value * value);
    }
});   // 自动 join

println!("{buffer:?}");   // [1, 4, 9, 16]
```

每个线程拿到一个不重叠的 `&mut u64`，借用检查器允许这种“分段可变借用”。

---

## 8. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `Rc cannot be sent between threads` | `Rc` 非原子计数 | 用 `Arc` |
| `RefCell cannot be shared between threads` | 非线程安全 | `Mutex` / `RwLock` |
| `closure may outlive` | 线程借用局部变量 | `move` / `thread::scope` |
| `cannot move out of borrowed content` | 闭包没取得所有权 | `move` + clone / Arc |
| `PoisonError` | 持锁线程 panic | `.unwrap_or_else(\|e\| e.into_inner())` |
| 死锁 | 锁顺序不一致 | 固定顺序 / 缩小临界区 |
| `future cannot be sent`（async） | 跨 await 持有非 Send 类型 | 缩小作用域 / 换类型 |

---

## 9. 练习

1. 启动 8 个线程，每个计算 `1..=N` 的一段和，最后汇总（分别用 `Arc<Mutex>`、channel、`thread::scope`）。
2. 用 `mpsc` 实现生产者-消费者：2 个生产者、1 个消费者，消费者统计总数。
3. 用 `RwLock` 实现一个并发配置缓存：多个读者、偶尔写者。
4. 用 `AtomicUsize` 实现无锁计数器，并与 `Mutex` 版本做基准对比。
5. 复现一个死锁（两个 `Mutex`，不同顺序），然后用固定顺序修复。
6. 解释为什么 `RefCell` 是 `Send` 但不是 `Sync`，并写一个编译失败的例子。

---

## 延伸阅读

- The Book 第 16 章 — <https://doc.rust-lang.org/book/ch16-00-concurrency.html>
- `std::sync` — <https://doc.rust-lang.org/std/sync/>
- `std::thread` — <https://doc.rust-lang.org/std/thread/>
- Rustonomicon：Send and Sync — <https://doc.rust-lang.org/nomicon/send-and-sync.html>
- `rayon` — <https://docs.rs/rayon/>

上一章：[14 · 智能指针](14-smart-pointers.md) ·
下一章：[16 · OOP 与模式匹配](16-oop-patterns.md)
