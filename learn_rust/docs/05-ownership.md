# 05 · Ownership、Borrowing 与 Slices

> 对应 The Book 第 4 章 —— **全教程最重要的一章**
> 示例 crate：[`ownership`](../ownership)
> 运行：`cargo run`、`cargo run --example move_semantics`、`cargo run --example borrow_rules`、`cargo run --example slices`

C++ 开发者学 Rust，90% 的挫败感来自这一章。
好消息是：一旦把“所有权”当成**类型系统的一部分**而不是“运行时引用计数”，后面会顺很多。

---

## 1. 栈、堆与三种字符串/容器

```rust
let x = 5;                          // 栈上的 i32
let s = String::from("hello");      // 栈上 24 字节的“胖指针”，堆上是字符数据
let v = vec![1, 2, 3];              // 同理：ptr + len + capacity
```

`String` 的逻辑结构：

```text
栈: [ ptr | len | capacity ]  ──►  堆: "hello"
```

这和 `std::string` 类似（不过 libstdc++ 的 `std::string` 有 SSO，Rust 的 `String` 没有）。
`Vec<T>` 和 `std::vector<T>` 也一样：`ptr + len + capacity` 三元组。

C++ 的 `std::string` 拷贝是深拷贝，移动是“偷指针”；Rust 默认只有**移动**，
想复制必须显式 `.clone()`。

| 类型 | 位置 | 所有权 |
| --- | --- | --- |
| `i32`、`bool`、`char`、`[u8; N]` | 栈（或内联） | `Copy`，赋值即复制 |
| `String`、`Vec<T>`、`Box<T>` | 栈上指针 + 堆数据 | 独占，move 转移 |
| `&T`、`&mut T`、`&str`、`&[T]` | 栈上指针（`&str`/slice 是胖指针） | 借用，不拥有 |
| `Rc<T>`、`Arc<T>` | 栈上指针 + 堆计数 | 共享所有权（第 15 章） |

---

## 2. Move：赋值即转移

```rust
let s1 = String::from("hello");
let s2 = s1;            // 所有权从 s1 移到 s2
// println!("{s1}");    // error[E0382]: borrow of moved value: `s1`
```

编译器在 move 之后把 `s1` 标记为“未初始化”，任何读取都会在编译期被拒绝。
这与 C++ 的“moved-from 仍然有效但状态未指定”完全不同：

```cpp
std::string s1 = "hello";
std::string s2 = std::move(s1);
std::cout << s1;        // 合法（但内容未指定），容易埋雷
```

### 2.1 没有 move constructor

Rust 的 move 是按位复制（bitwise copy）+ 禁止访问源对象，**不会调用任何用户代码**。
因此：

- 不能自定义 move 构造；
- 不能依赖“move 后源对象自动清空”；
- 如果需要资源释放，实现 `Drop`；它会在 owner 离开作用域时调用；
- move 一个含 `Drop` 的类型只是转移释放责任，不会提前析构。

```rust
struct Buffer(Vec<u8>);

impl Drop for Buffer {
    fn drop(&mut self) {
        println!("release {}", self.0.len());
    }
}

let a = Buffer(vec![0; 1024]);
let b = a;              // move；a 不再可用
// 只有 b 离开作用域时 drop 一次
```

这从语言层面杜绝了 double free。

### 2.2 函数调用也会 move

```rust
fn consume(s: String) { /* s 在此函数结束时 drop */ }

let s = String::from("hello");
consume(s);
// println!("{s}");     // error: s 已被 move
```

如果不想交出所有权，传引用：

```rust
fn print_len(s: &str) { println!("{}", s.len()); }

let s = String::from("hello");
print_len(&s);          // 借用
println!("{s}");        // 仍然可用
```

> 经验法则：**函数参数优先用借用**（`&str`、`&[T]`、`&T`）；
> 需要保存或返回时才取得所有权。

### 2.3 返回所有权

```rust
fn make_greeting(name: &str) -> String {
    format!("Hello, {name}!")   // 新 String 的所有权交给调用者
}
```

返回引用则必须能证明它来自某个输入（见第 10 章 lifetime）：

```rust
fn first_word(s: &str) -> &str { ... }   // 返回值借用 s
```

---

## 3. `Copy` 与 `Clone`

```rust
let x = 5;
let y = x;
println!("{x} {y}");    // 都可用：i32 实现了 Copy

#[derive(Debug, Clone, Copy)]
struct Point { x: i32, y: i32 }

let p = Point { x: 1, y: 2 };
let q = p;              // 复制，不是 move
println!("{p:?} {q:?}");
```

| trait | 语义 | C++ 对应 |
| --- | --- | --- |
| `Copy` | 按位复制，赋值后源仍可用 | 平凡可拷贝类型 |
| `Clone` | 显式 `.clone()`，可深拷贝 | 拷贝构造函数 |
| `Drop` | 离开作用域自动清理 | 析构函数 |

规则：

- `Copy` 要求所有字段都是 `Copy`；
- 实现 `Drop` 的类型**不能**实现 `Copy`（否则会 double free）；
- `String`、`Vec<T>`、`Box<T>` 不是 `Copy`，但可以是 `Clone`；
- 数组 `[T; N]` 在 `T: Copy` 时是 `Copy`。

```rust
let v1 = vec![1, 2, 3];
let v2 = v1.clone();    // 深拷贝，堆上数据复制一份
```

`.clone()` 的代价是可见的：读代码的人立刻知道这里发生了复制。
不要为了绕过借用检查器而无脑 clone；先想清楚所有权关系。

---

## 4. Borrowing：借用而不是复制

### 4.1 不可变借用

```rust
fn calculate_length(s: &str) -> usize { s.len() }

let s = String::from("hello");
let len = calculate_length(&s);
println!("{s} {len}");  // s 仍可用
```

`&s` 创建不可变借用，类似 `const std::string&` 或 `std::string_view`。

### 4.2 可变借用

```rust
fn change(s: &mut String) {
    s.push_str(", world");
}

let mut s = String::from("hello");
change(&mut s);
println!("{s}");        // "hello, world"
```

`&mut String` 类似 `std::string&`，但额外保证：**借出期间，不能有其他任何访问**。

### 4.3 借用规则

> 对同一份数据，在任意时刻：
> - 要么有任意多个 `&T`；
> - 要么有且仅有一个 `&mut T`。

```rust
let mut s = String::from("hello");

let r1 = &s;
let r2 = &s;            // OK：多个不可变借用
println!("{r1} {r2}");

// let r3 = &mut s;     // error[E0502]: 同时存在不可变借用
```

```rust
let mut s = String::from("hello");
let r1 = &mut s;
// let r2 = &mut s;     // error[E0499]: 不能同时有两个可变借用
r1.push('!');
```

这条规则消除了：

- 迭代器失效（C++ 中 `vector::push_back` 后旧迭代器悬垂）；
- 数据竞争（两个线程同时写）；
- 大多数 use-after-free。

### 4.4 NLL：借用在使用结束后立即释放

Rust 2018+ 使用 **Non-Lexical Lifetimes**：借用的生命周期到“最后一次使用”为止，
而不是到代码块结束。

```rust
let mut s = String::from("abc");
let r1 = &s;
println!("{r1}");       // r1 最后一次使用
let r2 = &mut s;        // OK：r1 的借用已经结束
r2.push_str("def");
println!("{r2}");
```

C++ 中你可能需要手动加 `{}` 控制锁/引用作用域；Rust 由编译器做活跃性分析。

### 4.5 重借用（reborrow）

```rust
fn print_len(s: &str) { println!("{}", s.len()); }

fn append(s: &mut String) {
    print_len(&*s);     // &mut String -> &String -> &str
    s.push('!');
}
```

`&mut T` 可以临时重借用为 `&T`，类似 `T*` 隐式转 `const T*`。
重借用结束后，原来的可变借用继续有效。

---

## 5. Slices：连续区间的借用视图

```rust
let arr = [1, 2, 3, 4, 5];
let all: &[i32] = &arr;
let part: &[i32] = &arr[1..4];   // [2, 3, 4]

fn sum(xs: &[i32]) -> i32 { xs.iter().sum() }
println!("{}", sum(&arr));
println!("{}", sum(part));
```

| Rust | C++23 | 说明 |
| --- | --- | --- |
| `&[T]` | `std::span<const T>` | 只读视图 |
| `&mut [T]` | `std::span<T>` | 可写视图 |
| `&str` | `std::string_view` | UTF-8 只读视图 |
| `&[u8]` | `std::span<const std::byte>` | 原始字节 |

区别：`span` 不检查生命周期，Rust 的借用检查器会保证视图不超出所有者。

```rust
fn first_word(s: &str) -> &str {
    for (i, byte) in s.bytes().enumerate() {
        if byte == b' ' {
            return &s[..i];
        }
    }
    s
}
```

字符串切片：

```rust
let s = String::from("hello world");
let hello = &s[0..5];
let world = &s[6..11];
```

注意：切片边界必须落在 UTF-8 字符边界上，否则运行时 panic（见第 3 章）。

---

## 6. 用 C++ 的 bug 理解 Rust 的拒绝

### 6.1 悬垂引用

```cpp
int& bad() {
    int x = 42;
    return x;        // 返回栈变量引用，UB
}
```

Rust：

```rust
// fn bad() -> &i32 {
//     let x = 42;
//     &x              // error[E0106]/[E0515]: 返回局部变量引用
// }
```

正确写法：返回值（转移所有权）`fn good() -> i32 { 42 }`。

### 6.2 迭代器失效

```cpp
std::vector<int> v{1, 2, 3};
int* p = v.data();
v.push_back(4);      // 可能重新分配
*p = 10;             // UB
```

Rust：

```rust
let mut v = vec![1, 2, 3];
let p = &mut v[0];
v.push(4);           // error[E0499]/[E0502]: 已借出
*p = 10;
```

### 6.3 数据竞争

```cpp
int counter = 0;
std::thread t1([&] { for (int i = 0; i < 1000; ++i) ++counter; });
std::thread t2([&] { for (int i = 0; i < 1000; ++i) ++counter; });
// 数据竞争，UB
```

Rust 需要在编译期通过 `Send`/`Sync` 检查，并用 `Arc<Mutex<T>>` 或 channel
显式共享（见第 15 章）。裸 `&mut` 不能跨线程共享。

---

## 7. 常见错误与修复

| 错误 | 代码形态 | 修复 |
| --- | --- | --- |
| `E0382` borrow of moved value | 传值后继续用 | 传 `&`，或 `.clone()`，或调整作用域 |
| `E0505` cannot move out ... borrowed | 有借用时 move | 先结束借用，或返回所有权 |
| `E0499` two mutable borrows | 同时两个 `&mut` | 用块/`drop()` 结束前一个，或拆分字段 |
| `E0502` mutable while immutable | 读写同时 | 先读完再改；或用 `split_at_mut` |
| `E0515` returning reference to local | 返回局部引用 | 返回拥有值 |
| `E0597` does not live long enough | 借用短命对象 | 延长所有者生命周期，或 clone |
| `E0507` cannot move out of `&T` | 从借用中搬字段 | 用 `Clone`/`Copy`，或用 `Option::take` |
| `E0716` temporary dropped | 引用临时值 | 先绑定到变量 |

示例：`E0507`

```rust
struct User { name: String }

fn take_name(u: &User) -> String {
    u.name          // error[E0507]: cannot move out of borrowed content
}
// 修复一：u.name.clone()
// 修复二：fn take_name(u: User) -> String { u.name }
```

---

## 8. 设计建议

1. **先决定 owner**：数据结构里谁负责释放？通常是一个明确的所有者。
2. **借用优先**：只读用 `&T`/`&str`/`&[T]`，只写在必要时用 `&mut T`。
3. **不要用父指针建模树**：用 `Rc<RefCell<>>`/`Weak`（第 15 章），或 arena + 索引。
4. **需要“多个部分同时可变”**：用 `split_at_mut`、`iter_mut`、`chunks_mut`，
   或把结构体字段拆开（借用检查器能识别不同字段）。
5. **想 clone 时先停一下**：如果只是为了让编译器闭嘴，通常说明所有权设计有问题。
6. **返回引用要谨慎**：能返回拥有值就返回拥有值，接口更简单。
7. **不要让借用跨越 await / 锁**：会引入生命周期和死锁问题，缩小作用域。

```rust
// 借用检查器能理解“不同字段”的拆分
struct Pair { a: Vec<i32>, b: Vec<i32> }

let mut p = Pair { a: vec![], b: vec![] };
let a = &mut p.a;
let b = &mut p.b;   // OK：不同字段
a.push(1);
b.push(2);
```

---

## 9. 代码地图

| 文件 | 内容 |
| --- | --- |
| `ownership/src/main.rs` | move / Copy / clone / 借用 / NLL / slice 主线 |
| `examples/move_semantics.rs` | 含 `Drop` 的类型，观察所有权转移与析构顺序 |
| `examples/borrow_rules.rs` | 多数借用、单可变借用、防悬垂、重借用 |
| `examples/slices.rs` | `&[T]` / `&mut [T]` / `&str`、`get()` 与越界 |

```bash
cd ownership
cargo run
cargo run --example move_semantics
cargo run --example borrow_rules
cargo run --example slices
```

---

## 10. 练习

1. 写 `fn longest_word(s: &str) -> &str`，返回最长单词；再写一个返回 `String` 的版本，比较接口差异。
2. 实现 `fn split_at_mut` 的安全用法：把 `Vec<i32>` 从中间切开，同时修改两半。
3. 复现 `E0382`、`E0499`、`E0502`、`E0507` 各一次，并分别修复。
4. 设计一个 `struct Document { title: String, body: String }`，实现只读摘要函数和追加正文函数，
   要求不产生不必要的 clone。
5. 解释为什么 `let r = &mut v[0]; v.push(1);` 不能编译，并给出两种修复。

---

## 延伸阅读

- The Book 第 4 章 — <https://doc.rust-lang.org/book/ch04-00-understanding-ownership.html>
- Rustonomicon：Ownership — <https://doc.rust-lang.org/nomicon/ownership.html>
- `std::borrow` — <https://doc.rust-lang.org/std/borrow/index.html>
- C++ Core Guidelines：所有权 — <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-ownership>

上一章：[04 · 函数与控制流](04-functions-control-flow.md) ·
下一章：[06 · Struct、Enum 与模式匹配](06-structs-enums.md)
