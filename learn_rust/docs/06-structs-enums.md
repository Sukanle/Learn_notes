# 06 · Struct、Enum 与模式匹配

> 对应 The Book 第 5–6 章
> 示例 crate：[`structs_enums`](../structs_enums)
> 运行：`cargo run`、`cargo run --example structs`、`cargo run --example option`

---

## 1. Struct：数据 + 行为

### 1.1 三种形态

```rust
struct Rectangle { width: u32, height: u32 }   // named fields
struct Color(u8, u8, u8);                      // tuple struct
struct AlwaysEqual;                            // unit struct（零大小）
```

| Rust | C++ |
| --- | --- |
| named struct | `struct` / `class` |
| tuple struct | 轻量 wrapper（如 `struct Meters { double v; }`） |
| unit struct | 无状态 tag / marker 类型 |

Rust 没有“class”关键字；`struct` 负责数据，`impl` 负责行为。
字段默认**私有**（只在定义它的模块内可见），需要 `pub` 才对外暴露。
方法默认也是私有的，需要 `pub`（第 7 章）。

### 1.2 `impl` 与 self

```rust
impl Rectangle {
    fn area(&self) -> u32 { self.width * self.height }       // ≈ const 成员函数
    fn scale(&mut self, factor: u32) { ... }                  // ≈ 非 const 成员函数
    fn consume(self) -> u32 { self.width * self.height }      // 取得所有权
    fn square(size: u32) -> Self { Self { width: size, height: size } } // static
}
```

| 接收者 | C++ 对应 | 用途 |
| --- | --- | --- |
| `&self` | `const T&` / const 成员函数 | 只读 |
| `&mut self` | `T&` / 非 const 成员函数 | 修改 |
| `self` | `T`（按值） | 消耗对象 |
| 无 self（关联函数） | `static` 成员函数 | 构造函数、工具函数 |

`Self` 是当前类型的别名，类似 C++ 的注入类名。
一个类型可以有多个 `impl` 块（本仓库 `generics` 用到了这一特性）。

### 1.3 初始化与更新语法

```rust
let rect = Rectangle { width: 30, height: 50 };

let rect2 = Rectangle {
    width: 10,
    ..rect          // 其余字段从 rect 复制/移动
};
```

`..rect` 类似 C++ 的指定初始化器 + 逐字段赋值，但要求 `rect` 的剩余字段可以 move/copy。
如果 `rect` 有 `Drop`，部分 move 会导致后续不能整体使用。

### 1.4 `Debug` 与 `dbg!`

```rust
#[derive(Debug)]
struct Rectangle { width: u32, height: u32 }

let r = Rectangle { width: dbg!(30 * 2), height: 50 };
println!("{r:?}");      // Rectangle { width: 60, height: 50 }
println!("{r:#?}");     // 多行缩进格式
```

`#[derive(Debug)]` 类似自动生成 `operator<<` 的调试版本。
`dbg!(expr)` 会打印文件名、行号、表达式和值到 **stderr**，并返回表达式的值。

### 1.5 内存布局

默认布局是编译器优化的（字段可能重排），**不要**假设字段顺序与 C ABI 一致。
需要与 C 交互时使用：

```rust
#[repr(C)]
struct Header {
    magic: u32,
    version: u16,
    flags: u16,
}
```

`#[repr(C)]` 类似 C++ 的 standard-layout 约定，保证字段顺序和 padding 与 C 一致。
其余 `repr`：`repr(transparent)`、`repr(u8)`（enum 的底层整数类型）、`repr(packed)`。

---

## 2. Enum：带标签的联合体

```rust
enum Message {
    Quit,
    Move { x: i32, y: i32 },
    Write(String),
    ChangeColor(i32, i32, i32),
}
```

这比 C++ 的 `std::variant` 更自然：

```cpp
using Message = std::variant<
    QuitTag,
    Move,
    std::string,
    Color
>;
```

Rust 的每个 variant 可以直接带命名字段或位置字段，`match` 时可以解构。
内存布局 = 一个 tag + 所有 variant 中最大的 payload（可能还有 niche 优化）。

### 2.1 `match` 解构

```rust
fn handle(msg: &Message) {
    match msg {
        Message::Quit => println!("quit"),
        Message::Move { x, y } => println!("({x}, {y})"),
        Message::Write(text) => println!("{text}"),
        Message::ChangeColor(r, g, b) => println!("#{r:02X}{g:02X}{b:02X}"),
    }
}
```

`match` 强制穷尽：新增一个 variant，所有未更新的 `match` 都会编译失败。
这是 Rust 重构友好的重要原因（C++ 的 `switch` 不会提醒你漏了枚举值）。

### 2.2 `Option<T>`：没有空指针

```rust
enum Option<T> {
    None,
    Some(T),
}
```

| C++23 | Rust |
| --- | --- |
| `std::optional<T>` | `Option<T>` |
| `opt.has_value()` | `opt.is_some()` |
| `*opt`（未检查） | `opt.unwrap()`（panic）或 `if let` |
| `opt.value_or(x)` | `opt.unwrap_or(x)` |
| `opt.and_then(f)` | 同名 `and_then` |
| `T*` 可能为 null | `Option<&T>` 或 `Option<Box<T>>` |

区别：`std::optional` 有隐式 `bool` 转换和 `operator*` 未检查访问；Rust 必须显式处理。

```rust
fn plus_one(x: Option<i32>) -> Option<i32> {
    match x {
        None => None,
        Some(i) => Some(i + 1),
    }
}

println!("{:?}", plus_one(None));      // None
println!("{:?}", plus_one(Some(41)));  // Some(42)
```

**Niche optimization**：`Option<Box<T>>`、`Option<&T>`、`Option<NonZeroU32>`
等可以用无效位模式表示 `None`，因此大小与 `T` 相同。`Option<&T>` 就是 8 字节。

### 2.3 `if let`、`while let`、`let-else`

```rust
// 只关心一个分支
if let Some(max) = config_max {
    println!("max = {max}");
}

// 循环解构
let mut stack = vec![1, 2, 3];
while let Some(top) = stack.pop() {
    println!("{top}");
}

// 绑定失败时提前返回（Rust 1.65+）
let Some(max) = config_max else {
    return;
};
```

`let-else` 是处理“必须有值”的场景的好工具，类似：

```cpp
auto opt = get_config();
if (!opt) return;
auto max = *opt;
```

---

## 3. 方法与枚举的组合

```rust
impl Message {
    fn call(&self) {
        match self { ... }
    }
}

impl Coin {
    fn value_in_cents(&self) -> u8 {
        match self {
            Coin::Penny => 1,
            Coin::Nickel => 5,
            Coin::Dime => 10,
            Coin::Quarter(state) => {
                println!("Quarter from {state:?}");
                25
            }
        }
    }
}
```

这相当于把 `std::visit` 的 visitor 写进类型本身，调用侧更自然：

```rust
let coin = Coin::Quarter(UsState::Alaska);
println!("{}", coin.value_in_cents());
```

### 3.1 状态用 enum 表达

不要写一堆互斥的 bool：

```rust
// 不推荐
struct Connection {
    is_connecting: bool,
    is_connected: bool,
    is_closed: bool,
}

// 推荐
enum Connection {
    Connecting { attempt: u32 },
    Connected { since: std::time::Instant },
    Closed { reason: String },
}
```

enum 让非法状态**无法表示**（make illegal states unrepresentable），这是 Rust 建模的核心技巧。

---

## 4. 常见 C++ 对照

| C++23 | Rust | 注意 |
| --- | --- | --- |
| `struct` / `class` | `struct` + `impl` | 无继承，无虚函数（用 trait） |
| `std::optional<T>` | `Option<T>` | 必须显式处理 |
| `std::variant<...>` | `enum { ... }` | `match` 穷尽，字段直接命名 |
| `std::visit` | `match` | 不需要 visitor 重载集 |
| `enum class` | `enum` | Rust enum 可带数据 |
| `std::holds_alternative` | `matches!(v, Variant { .. })` | |
| `std::get<T>(v)` | `if let Variant(x) = v` | 失败不抛异常 |
| 成员函数 | `impl` 中的 `fn` | 可分散多个 impl |
| `static` 成员 | 关联函数 | 无 `self` |

---

## 5. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | Rectangle、Message enum、Coin + state、`Option`、`if let`/`let-else`/`while let` |
| `examples/option.rs` | `map`/`and_then`/`ok_or`、多个 Option 的组合 |
| `examples/structs.rs` | tuple struct / unit struct、`mem::take`、Copy 语义 |

```bash
cd structs_enums
cargo run
cargo run --example option
cargo run --example structs
```

---

## 6. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `missing field 'x' in initializer` | 少写字段 | 补字段或 `..base` |
| `field 'x' is private` | 跨模块访问私有字段 | 提供 `pub` 方法或 `pub` 字段 |
| `cannot move out of borrowed content` | 从 `&self` 搬字段 | `.clone()` 或改 `self` |
| `non-exhaustive patterns` | match 漏 variant | 补全或 `_` |
| `no method named ...` | 方法定义在别的 impl / 未导入 trait | 检查 trait 是否 `use` |
| `cannot borrow as mutable` | 方法需要 `&mut self`，变量不是 `mut` | `let mut x` |
| `PartialMove` | 部分字段被 move 后整体使用 | 避免部分 move，或重构 |

---

## 7. 练习

1. 给 `Rectangle` 增加 `fn can_hold(&self, other: &Rectangle) -> bool` 和 `fn scale(&mut self, f: u32)`。
2. 定义 `enum Shape { Circle(f64), Rect { w: f64, h: f64 }, Triangle(f64, f64, f64) }`，
   实现 `area()` 和 `perimeter()`，非法输入返回 `Result`。
3. 用 enum 重写 `branches` 的菜单状态，消除 `unreachable!()`。
4. 实现一个 `Stack<T>`，提供 `push`/`pop`/`peek`/`is_empty`，并解释每个方法的接收者。
5. 把 `struct User { name: String, age: u8 }` 的 `name` 改成 `Option<String>`，
   写一个函数返回显示名（缺省为 `"anonymous"`），比较 `match` 与组合子的写法。

---

## 延伸阅读

- The Book 第 5 章 — <https://doc.rust-lang.org/book/ch05-00-structs.html>
- The Book 第 6 章 — <https://doc.rust-lang.org/book/ch06-00-enums.html>
- `std::option` — <https://doc.rust-lang.org/std/option/>
- `#[repr]` 参考 — <https://doc.rust-lang.org/reference/type-layout.html>

上一章：[05 · Ownership](05-ownership.md) ·
下一章：[07 · Package、Crate 与 Module](07-modules.md)
