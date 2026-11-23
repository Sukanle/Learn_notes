# 14 · 智能指针：Box、Rc、RefCell、Deref、Drop

> 对应 The Book 第 15 章
> 示例 crate：[`smart_pointers`](../smart_pointers)
> 运行：`cargo run`、`cargo run --example deref_drop`、`cargo run --example interior_mutability`

C++ 的智能指针解决“谁负责 delete”的问题；Rust 的智能指针在此基础上把
**所有权、可变性、线程安全**编码进类型。理解每种指针的适用边界，比记住语法更重要。

---

## 1. 总览

| Rust | C++ | 所有权 | 可变性 | 线程安全 |
| --- | --- | --- | --- | --- |
| `Box<T>` | `unique_ptr<T>` | 独占 | 可 `&mut` | `T: Send` 时 `Send` |
| `Rc<T>` | `shared_ptr<T>` | 共享 | **只读** | `!Send` / `!Sync` |
| `Arc<T>` | `shared_ptr<T>`（原子计数） | 共享 | 只读 | `T: Send + Sync` 时可用 |
| `Weak<T>` | `weak_ptr<T>` | 弱引用 | 只读 | 同上 |
| `RefCell<T>` | `mutable` + 运行时检查 | 独占 | 运行时可借 `&mut` | `!Sync` |
| `Cell<T>` | `mutable`（仅 Copy） | 独占 | 整体替换 | `!Sync` |
| `Mutex<T>` | `mutex` + 保护数据 | 独占锁 | 加锁后可写 | 可跨线程 |
| `Cow<'a, T>` | 写时复制 | 借用/拥有 | 需要时 clone | 取决于 T |

---

## 2. `Box<T>`：最简单的堆分配

```rust
let b = Box::new(5);
println!("{}", *b);       // 解引用
println!("{b}");          // Display 通过 Deref coercion
```

用途：

1. **递归类型**：`enum List { Cons(i32, Box<List>), Nil }`
   没有 `Box` 时类型大小无限；
2. **trait object**：`Box<dyn Error>`、`Box<dyn Fn()>`；
3. **大对象转移**：避免在栈上复制；
4. **`Box::leak`**：故意泄漏，得到 `&'static mut T`（谨慎使用）。

```rust
#[derive(Debug)]
enum List {
    Cons(i32, Box<List>),
    Nil,
}

let list = List::Cons(1, Box::new(List::Cons(2, Box::new(List::Nil))));
```

C++ 对照：

```cpp
struct List {
    int value;
    std::unique_ptr<List> next;
};
```

`Box<T>` 是零开销的：大小就是一个指针，`Box::new` 分配，drop 时释放。

### 2.1 `Box<dyn Trait>` 与 `&dyn Trait`

```rust
let items: Vec<Box<dyn std::fmt::Display>> = vec![
    Box::new(1),
    Box::new("two"),
    Box::new(3.0),
];
```

- `Box<dyn Trait>` 拥有 trait object，类似 `unique_ptr<Base>`；
- `&dyn Trait` 借用，类似 `const Base&`；
- vtable 指针与数据指针组成胖指针（16 字节 on 64-bit）；
- 默认 `Box<dyn Trait + 'static>`，借用时要写 `Box<dyn Trait + 'a>`。

---

## 3. `Deref` 与 `Drop`

### 3.1 `Deref`

```rust
use std::ops::Deref;

struct MyBox<T>(T);

impl<T> Deref for MyBox<T> {
    type Target = T;
    fn deref(&self) -> &T { &self.0 }
}

let b = MyBox::new(String::from("Rust"));
println!("{}", *b);      // String
greet(&b);               // &MyBox<String> -> &String -> &str
```

`Deref` 让自定义类型获得 `*` 和自动解引用能力，类似 C++ 的 `operator*` / `operator->`。
**Deref coercion** 会在函数调用、字段访问等位置自动多次解引用：

```rust
fn greet(name: &str) { ... }

let s = String::from("Rust");
greet(&s);               // &String -> &str
```

> 不要滥用 `Deref` 来“模拟继承”。它主要用于智能指针和透明的 wrapper；
> 滥用会让 API 难以预测（Rust API Guidelines 有专门讨论）。

### 3.2 `Drop`

```rust
struct Logger(&'static str);

impl Drop for Logger {
    fn drop(&mut self) {
        println!("drop {}", self.0);
    }
}

{
    let _inner = Logger("inner");
}   // 此处调用 drop
```

- `Drop` 类似析构函数，离开作用域自动调用；
- 不能手动调用 `x.drop()`（会 double drop），用 `std::mem::drop(x)`；
- 局部变量按**声明的逆序**析构（与 C++ 一致）；
- 结构体字段按**声明顺序**析构；
- 实现 `Drop` 的类型不能实现 `Copy`。

```rust
let early = Logger("early");
drop(early);             // 提前释放，类似 reset()
```

### 3.3 `mem::take` / `mem::replace`

```rust
let mut v = vec![1, 2, 3];
let taken = std::mem::take(&mut v);   // v 变成空 Vec，返回原值
let old = std::mem::replace(&mut v, vec![9]);
```

这是 Rust 中“移动出借用内容”的标准手法（`E0507` 的常见修复）。

---

## 4. `Rc<T>`：单线程共享所有权

```rust
use std::rc::Rc;

let a = Rc::new(String::from("shared"));
println!("{}", Rc::strong_count(&a));   // 1

let b = Rc::clone(&a);                  // 计数器 +1，不复制 String
let c = Rc::clone(&a);
println!("{}", Rc::strong_count(&a));   // 3

drop(c);
println!("{}", Rc::strong_count(&a));   // 2
```

- `Rc::clone` 只增加引用计数，O(1)；
- 所有 `Rc<T>` 共享**不可变**访问；
- 最后一个 `Rc` 销毁时，内部 `T` 才 drop；
- 引用计数是**非原子**的，因此 `Rc<T>` 不是 `Send`/`Sync`，不能跨线程；
- 多线程共享用 `Arc<T>`。

C++ 的 `shared_ptr` 引用计数是原子的（线程安全），因此更重；
Rust 把单线程/多线程的选择显式化：`Rc` 更轻，`Arc` 更安全。

---

## 5. `RefCell<T>`：运行时借用检查

```rust
use std::cell::RefCell;

let cell = RefCell::new(vec![1, 2, 3]);

cell.borrow_mut().push(4);        // 运行时可变借用
println!("{:?}", cell.borrow());  // 运行时不可变借用
```

规则与编译期借用相同，但检查发生在运行时：

- `borrow()` 返回 `Ref<T>`；
- `borrow_mut()` 返回 `RefMut<T>`；
- 同时存在 `borrow` 和 `borrow_mut` → **panic**（不是 UB）；
- 借用守卫离开作用域自动释放。

```rust
let cell = RefCell::new(1);
let _a = cell.borrow();
// let _b = cell.borrow_mut();   // panic: already borrowed
```

`Cell<T>` 是更轻的版本，适用于 `Copy` 类型：`get()` / `set()` / `replace()`，
没有借用守卫。

### 5.1 `Rc<RefCell<T>>`：单线程共享可变

```rust
use std::cell::RefCell;
use std::rc::Rc;

let shared = Rc::new(RefCell::new(vec![1, 2, 3]));

let a = Rc::clone(&shared);
let b = Rc::clone(&shared);

a.borrow_mut().push(4);
b.borrow_mut().push(5);
println!("{:?}", shared.borrow());   // [1, 2, 3, 4, 5]
```

这是 Rust 中“多个所有者 + 可变”的标准组合，但要注意：

- 只能在单线程使用；
- 运行时 panic 可能破坏程序；
- 不要长期持有 `Ref`/`RefMut` 跨越复杂逻辑；
- 能通过重新设计所有权避免时，优先避免。

---

## 6. 循环引用与 `Weak<T>`

`Rc` 的循环引用会导致引用计数永远不归零，造成内存泄漏（Rust 允许泄漏，不保证不泄漏）。

```rust
use std::cell::RefCell;
use std::rc::{Rc, Weak};

struct Node {
    name: &'static str,
    parent: RefCell<Weak<Node>>,     // 父 -> 子用强引用，子 -> 父用弱引用
    children: RefCell<Vec<Rc<Node>>>,
}
```

```rust
*child.parent.borrow_mut() = Rc::downgrade(parent);
parent.children.borrow_mut().push(Rc::clone(child));
```

- `Weak<T>` 不计入 `strong_count`；
- `weak.upgrade()` 返回 `Option<Rc<T>>`，对象已释放时为 `None`；
- C++ 的 `weak_ptr::lock()` 完全对应；
- 打破环的关键是：**确定一个方向为强引用，反方向用弱引用**。

```rust
let weak = Rc::downgrade(&a);
drop(a);
assert!(weak.upgrade().is_none());
```

---

## 7. `Arc<Mutex<T>>`：多线程共享可变

```rust
use std::sync::{Arc, Mutex};

let counter = Arc::new(Mutex::new(0));
let c = Arc::clone(&counter);

std::thread::spawn(move || {
    let mut guard = c.lock().unwrap();
    *guard += 1;
});
```

| 组合 | 线程安全 | 用途 |
| --- | --- | --- |
| `Rc<RefCell<T>>` | 否 | 单线程图/树/观察者 |
| `Arc<Mutex<T>>` | 是 | 多线程共享状态 |
| `Arc<RwLock<T>>` | 是 | 读多写少 |
| `Arc<AtomicUsize>` | 是 | 简单计数 |
| `Arc<T>`（T: Sync） | 是 | 共享只读 |

详见第 15 章。

---

## 8. C++ 对照总表

| C++23 | Rust | 备注 |
| --- | --- | --- |
| `unique_ptr<T>` | `Box<T>` | move-only |
| `shared_ptr<T>` | `Rc<T>` / `Arc<T>` | 单线程/多线程 |
| `weak_ptr<T>` | `Weak<T>` | `upgrade()` ↔ `lock()` |
| `make_unique` | `Box::new` | |
| `make_shared` | `Rc::new` / `Arc::new` | 控制块与数据可能分开分配 |
| `operator*` / `->` | `Deref` | 自动解引用 |
| 析构函数 | `Drop` | 不可手动调用 |
| `mutable` 成员 | `Cell` / `RefCell` | 运行时检查 |
| `shared_ptr` + `mutex` | `Arc<Mutex<T>>` | 锁与数据绑定 |
| `enable_shared_from_this` | `Weak` / 显式传 `Rc` | Rust 没有隐式自引用 |

---

## 9. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | `Box` 递归 List、`Rc` 计数、`Rc<RefCell<>>` 共享可变 |
| `examples/deref_drop.rs` | 自定义 `Deref`、deref coercion、`Drop` 顺序、`drop()` |
| `examples/interior_mutability.rs` | 父子树、`Weak` 打破循环、`upgrade()` |

```bash
cd smart_pointers
cargo run
cargo run --example deref_drop
cargo run --example interior_mutability
```

---

## 10. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `E0507` cannot move out of `Rc` | 试图从共享引用搬出值 | `(*rc).clone()` 或 `Rc::try_unwrap` |
| `already borrowed: BorrowMutError` | `RefCell` 借用冲突 | 缩小借用作用域 / 重构 |
| `Rc cannot be sent between threads` | `Rc` 不是 `Send` | 用 `Arc` |
| `RefCell cannot be shared between threads` | `RefCell` 不是 `Sync` | 用 `Mutex`/`RwLock` |
| 内存泄漏 / 计数不归零 | `Rc` 循环引用 | 反方向用 `Weak` |
| `Drop` 中再次 drop | 手动调用 `.drop()` | `std::mem::drop` |
| `cannot borrow as mutable` | `Rc` 只提供不可变 | 加 `RefCell`（单线程） |

---

## 11. 练习

1. 用 `Box` 实现一个二叉搜索树，插入/查找/中序遍历，不用 `Rc`。
2. 用 `Rc<RefCell<TreeNode>>` 实现树，并给每个节点加 `Weak` 父指针，验证无循环泄漏。
3. 实现一个观察者模式：`Rc<RefCell<Vec<Box<dyn Fn(i32)>>>>`，注册/触发回调。
4. 用 `Arc<Mutex<Vec<i32>>>` 启动 4 个线程并发 push，最后排序输出。
5. 比较 `Box<dyn Trait>` 与 `&dyn Trait` 的大小、所有权和适用场景。
6. 解释为什么 `RefCell<Vec<T>>` 的 `borrow_mut()` 不能跨线程，而 `Mutex<Vec<T>>` 可以。

---

## 延伸阅读

- The Book 第 15 章 — <https://doc.rust-lang.org/book/ch15-00-smart-pointers.html>
- `std::rc` / `std::sync::Arc` — <https://doc.rust-lang.org/std/rc/>
- `std::cell` — <https://doc.rust-lang.org/std/cell/>
- Rust API Guidelines：Deref — <https://rust-lang.github.io/api-guidelines/predictability.html>

上一章：[13 · 闭包与迭代器](13-iterators-closures.md) ·
下一章：[15 · 并发](15-concurrency.md)
