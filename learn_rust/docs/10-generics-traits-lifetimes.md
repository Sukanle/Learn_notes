# 10 · 泛型、Trait 与 Lifetime

> 对应 The Book 第 10 章
> 示例 crate：[`generics`](../generics)、[`traits`](../traits)、[`lifetime`](../lifetime)
> 运行：三个目录分别 `cargo run`，以及各自的 `--example`

这三项是 Rust 抽象能力的支柱：

- **Generics**：一份代码适配多种类型，编译期单态化；
- **Traits**：定义类型必须提供的能力，静态或动态派发；
- **Lifetimes**：证明引用不会悬垂，不产生运行时开销。

---

## 1. 泛型（Generics）

### 1.1 泛型函数

```rust
fn largest<T: PartialOrd>(list: &[T]) -> &T {
    let mut largest = &list[0];
    for item in list {
        if item > largest {
            largest = item;
        }
    }
    largest
}
```

- `T` 是类型参数；
- `T: PartialOrd` 是 **trait bound**，表示 `T` 支持 `<` / `>`；
- 返回 `&T`，借用输入列表中的元素。

C++：

```cpp
template <typename T>
requires std::totally_ordered<T>
const T& largest(const std::vector<T>& list) { ... }
```

差异：

| 维度 | C++ 模板 | Rust 泛型 |
| --- | --- | --- |
| 检查时机 | 实例化时（concepts 改善） | 定义处检查 bound |
| 代码生成 | 每个实例化一份 | 单态化，也是每类型一份 |
| 约束语法 | `requires` / `enable_if` | `T: Trait` / `where` |
| 编译错误 | 模板回溯长 | 直接指出 bound 不满足 |
| 特化 | 支持全特化/偏特化 | 不支持特化（用 trait 分派） |
| 编译期计算 | `constexpr` / 模板元编程 | `const fn` / 宏 / const generics |

### 1.2 泛型 struct / enum / impl

```rust
struct Point<T> { x: T, y: T }

impl<T> Point<T> {
    fn new(x: T, y: T) -> Self { Self { x, y } }
}

impl<T: Copy> Point<T> {
    fn move_to(&mut self, other: &Point<T>) {
        self.x = other.x;
        self.y = other.y;
    }
}

impl Point<f64> {
    fn distance_to(&self, other: &Point<f64>) -> f64 { ... }
}
```

- `impl<T> Point<T>` 对所有 `T` 有效；
- `impl<T: Copy>` 只对满足 bound 的 `T` 有效；
- `impl Point<f64>` 是**具体类型**的 impl（类似模板特化）；
- 多个类型参数：`struct Pair<T, U> { first: T, second: U }`。

### 1.3 const generics

```rust
fn sum<const N: usize>(arr: [i32; N]) -> i32 {
    arr.iter().sum()
}

let a = [1, 2, 3];
println!("{}", sum(a));       // N 自动推导为 3
```

`const N: usize` 让长度成为类型的一部分，类似 C++ 的 `std::array<T, N>` 模板参数。
常见于固定大小缓冲区、矩阵库。

### 1.4 单态化的代价

```rust
fn id<T>(x: T) -> T { x }

let a = id(1);       // 生成 id::<i32>
let b = id("hi");    // 生成 id::<&str>
```

每个具体类型都生成一份机器码，因此：

- 运行时没有虚表/类型检查开销（zero-cost）；
- 编译时间与二进制体积可能增加；
- 需要异构集合时用 `dyn Trait` 避免爆炸。

---

## 2. Trait：能力与契约

### 2.1 定义与实现

```rust
trait Summary {
    fn author(&self) -> String;

    // 默认方法：可以调用其他必需方法
    fn summarize(&self) -> String {
        format!("(Read more from {}...)", self.author())
    }
}

struct Article { author: String }

impl Summary for Article {
    fn author(&self) -> String { self.author.clone() }
}
```

- trait ≈ 抽象基类 / interface / concept；
- 一个类型可以实现多个 trait；
- trait 可以为已有类型实现（只要满足 orphan rule）；
- 默认方法类似基类的虚函数默认实现，但**没有继承状态**。

### 2.2 作为参数：静态派发

```rust
fn notify<T: Summary>(item: &T) {
    println!("{}", item.summarize());
}

// 等价写法
fn notify(item: &impl Summary) {
    println!("{}", item.summarize());
}
```

`impl Trait` 是语法糖，仍然是泛型 + 单态化。多个参数要注意：

```rust
fn two<T: Summary>(a: &T, b: &T) { }      // a、b 必须同一类型
fn two(a: &impl Summary, b: &impl Summary) { } // 可以是不同类型
```

### 2.3 作为返回值

```rust
fn make_summary() -> impl Summary {
    Article { author: String::from("Alice") }
}
```

- 返回一个**单一具体类型**（调用者不知道是什么）；
- 不能根据条件返回不同类型；
- 需要多类型时返回 `Box<dyn Summary>`。

### 2.4 动态派发：`dyn Trait`

```rust
fn notify(item: &dyn Summary) {
    println!("{}", item.summarize());
}

let summaries: Vec<Box<dyn Summary>> = vec![
    Box::new(Article { author: String::from("A") }),
    Box::new(Tweet { username: String::from("B") }),
];
```

| 维度 | `impl Trait` / 泛型 | `dyn Trait` |
| --- | --- | --- |
| 派发 | 静态（编译期确定） | 动态（vtable） |
| 代码体积 | 每类型一份 | 一份 |
| 性能 | 可内联，通常更快 | 一次间接调用 |
| 异构集合 | 不支持 | 支持 |
| 对象安全要求 | 无 | 必须 object-safe |

对象安全（object safety）的常见限制：

- 方法不能返回 `Self`（除非 `where Self: Sized`）；
- 不能有泛型方法；
- 不能有带 `Self: Sized` 以外约束的关联常量；
- `Clone` 不是 object-safe（返回 `Self`），因此不能 `Box<dyn Clone>`。

### 2.5 关联类型 vs 泛型参数

```rust
trait Iterator {
    type Item;                       // 关联类型
    fn next(&mut self) -> Option<Self::Item>;
}

trait Convert<T> {                   // 泛型参数
    fn convert(self) -> T;
}
```

区别：

- 一个类型对 `Iterator` 只能有一个 `Item`；
- 一个类型可以对 `Convert<T>` 实现多次（`Convert<i32>`、`Convert<String>`）；
- 选择标准：概念上“唯一确定”的用关联类型，否则用泛型参数。

`Iterator::Item` 让 `collect::<Vec<_>>()` 能推断元素类型；`From` 使用了泛型参数。

### 2.6 运算符重载

```rust
use std::ops::Add;

#[derive(Debug, Copy, Clone)]
struct Meters(f64);

impl Add for Meters {
    type Output = Meters;
    fn add(self, rhs: Self) -> Self::Output {
        Meters(self.0 + rhs.0)
    }
}

println!("{}", (Meters(1.0) + Meters(2.0)).0);
```

运算符是普通 trait：`Add`、`Sub`、`Mul`、`Index`、`PartialEq`、`Ord`、`Deref` 等。
C++ 的 `operator+` 是语言内置机制；Rust 通过 trait 实现，且可以在泛型中约束。

### 2.7 derive 与常用标准 trait

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
struct Version { major: u32, minor: u32, patch: u32 }
```

| trait | 作用 | C++ 对应 |
| --- | --- | --- |
| `Debug` | `{:?}` 调试输出 | 调试 `operator<<` |
| `Display` | `{}` 用户输出 | `operator<<` |
| `Clone` | `.clone()` | 拷贝构造 |
| `Copy` | 赋值即复制 | 平凡拷贝 |
| `PartialEq` / `Eq` | `==` / 等价关系 | `operator==` |
| `PartialOrd` / `Ord` | `<` / 全序 | `operator<=>` |
| `Hash` | 可作 HashMap key | `std::hash` |
| `Default` | 默认值 | 默认构造 |
| `From` / `Into` | 转换 | 转换构造 / `operator T` |

注意：`derive` 只能用于支持的 trait；复杂逻辑需要手写 impl。

### 2.8 Coherence 与 orphan rule

```rust
impl Summary for String { ... }        // 允许：trait 是本地定义的
impl From<MyType> for String { ... }   // 允许：MyType 是本地类型
// impl Display for String { ... }     // 禁止：trait 和类型都来自外部
```

规则：**trait 或类型至少有一个定义在当前 crate**。
这避免了“两个库各自给 `String` 实现 `Display`”导致全局冲突。
类似 C++ 的 ADL 冲突，但 Rust 在编译期直接禁止。

### 2.9 Blanket impl

```rust
trait Printable {
    fn print(&self);
}

impl<T: std::fmt::Display> Printable for T {
    fn print(&self) {
        println!("{self}");
    }
}
```

为所有满足 `Display` 的类型实现 `Printable`。标准库大量使用这种技巧，
例如 `impl<T: Display> ToString for T`。

---

## 3. Lifetime：引用的有效期

### 3.1 为什么需要

```rust
fn longest(x: &str, y: &str) -> &str {
    if x.len() > y.len() { x } else { y }
}
```

编译错误：`missing lifetime specifier`。
编译器不知道返回的引用来自 `x` 还是 `y`，因此无法判断调用者能使用多久。

```rust
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() { x } else { y }
}
```

含义：

- `x` 和 `y` 至少活得一样久（取较短者）；
- 返回值的有效范围不超过 `'a`；
- 不改变实际生命周期，只是**接口契约**。

### 3.2 省略规则（elision）

大多数情况不需要手写：

1. 每个输入引用各自获得一个 lifetime 参数；
2. 如果只有一个输入引用，输出 lifetime 等于它；
3. 如果有 `&self` / `&mut self`，输出 lifetime 等于 `self`。

```rust
fn first_word(s: &str) -> &str { ... }        // 规则 2
impl<'a> Parser<'a> {
    fn input(&self) -> &str { self.input }    // 规则 3
}
```

两个输入引用 + 返回引用，必须显式标注（或返回拥有值）。

### 3.3 结构体中的引用

```rust
struct ImportantExcerpt<'a> {
    part: &'a str,
}

fn main() {
    let novel = String::from("Call me Ishmael. Some years ago...");
    let first = novel.split('.').next().unwrap();
    let excerpt = ImportantExcerpt { part: first };
    println!("{}", excerpt.part);
}
```

`novel` 必须比 `excerpt` 活得久，否则编译失败。
结构体带引用时，整个结构体都带上了 lifetime 参数，会“传染”到使用它的类型。

### 3.4 方法中的 lifetime

```rust
impl<'a> ImportantExcerpt<'a> {
    fn level(&self) -> i32 { 3 }                       // 无引用返回

    fn announce_and_return_part(&self, announcement: &str) -> &'a str {
        println!("{announcement}");
        self.part                                      // 返回字段的 'a
    }
}
```

如果写 `-> &str`，省略规则会让返回值绑定到 `&self`，而不是 `'a`；
需要返回字段本身时，显式写 `'a`。

### 3.5 `'static`

```rust
let s: &'static str = "hello";       // 字符串字面量
static GREETING: &str = "hi";        // 静态变量
```

- `'static` 表示整个程序生命周期；
- 不要为了“让编译器闭嘴”到处加 `'static`，通常意味着设计有问题；
- `Box<dyn Trait>` 默认是 `Box<dyn Trait + 'static>`；
- 线程 spawn 的闭包要求 `'static`，除非使用 `thread::scope`。

### 3.6 常见 lifetime 错误

| 错误 | 含义 | 修复 |
| --- | --- | --- |
| `missing lifetime specifier` | 返回引用的来源不明确 | 加 `'a` 或返回拥有值 |
| `does not live long enough` | 借用对象提前销毁 | 延长所有者生命周期 / clone |
| `returns a value referencing data owned by the current function` | 返回局部变量引用 | 返回 `String` / `Vec` |
| `cannot infer an appropriate lifetime` | 约束矛盾 | 检查 struct 字段与方法的 lifetime |
| `borrowed value does not live long enough` | 临时值/局部值 | 先绑定到变量 |
| `lifetime may not live long enough` | 泛型 bound 不足 | 加 `T: 'a` |

---

## 4. C++ 对照总表

| C++23 | Rust | 说明 |
| --- | --- | --- |
| `template<typename T>` | `<T>` | 单态化 |
| `concept C = ...` | `trait C` + bound | 定义处检查 |
| `requires` 子句 | `where T: Trait` | 约束 |
| 抽象基类 | trait | 无数据继承 |
| `virtual` 函数 | `dyn Trait` 方法 | vtable |
| CRTP / type erasure | 泛型 + trait / `dyn` | |
| 模板特化 | 具体类型 impl / trait 分派 | 无偏特化 |
| `[[clang::lifetimebound]]` | `'a` 标注 | 强制检查 |
| `std::string_view` 生命周期约定 | `&'a str` | 编译期验证 |
| `constexpr` / TMP | `const fn` / const generics / 宏 | |
| `operator+` | `impl Add` | trait |
| `std::hash` 特化 | `impl Hash` | orphan rule |
| `std::is_same` | `TypeId` / trait | 运行期/编译期 |

---

## 5. 代码地图

### 5.1 `generics`

- `src/main.rs`：`Point<T>`、`impl<T>`、`impl<T: Copy>`、`impl Point<f64>`；
- `examples/multiple_types.rs`：`Pair<T, U>`、`where` 子句、返回 `Option<&T>`。

```bash
cd generics && cargo run
cargo run --example multiple_types
```

### 5.2 `traits`

- `src/main.rs`：`Pair<T>` 的条件 impl（`T: Display + PartialOrd`）、`largest<T: PartialOrd>`；
- `examples/std_traits.rs`：`Display`、`From`、`FromStr`、`Default`、`Ord`、`Add`。

```bash
cd traits && cargo run
cargo run --example std_traits
```

### 5.3 `lifetime`

- `src/main.rs`：`longest<'a>`、带 lifetime 的 struct、方法返回 `'a`、`'static`；
- `examples/lifetime_rules.rs`：elision 三条规则、方法返回字段。

```bash
cd lifetime && cargo run
cargo run --example lifetime_rules
```

---

## 6. 练习

1. 实现 `fn largest<T: PartialOrd + Copy>(list: &[T]) -> T`，与返回 `&T` 的版本比较。
2. 定义 `trait Shape { fn area(&self) -> f64; }`，为 `Circle`、`Rect` 实现，
   然后分别用泛型函数和 `&dyn Shape` 求和。
3. 定义 `trait Stack<T>`，实现 `VecStack<T>`；解释为什么这里用泛型参数而不是关联类型。
4. 写一个带 lifetime 的 `struct Tokenizer<'a> { input: &'a str, pos: usize }`，
   实现 `next_token(&mut self) -> Option<&'a str>`。
5. 解释为什么 `fn f<'a>(x: &'a str, y: &str) -> &'a str` 可以编译，
   而 `fn g<'a>(x: &'a str, y: &'a str) -> &'a str` 的约束更强。
6. 用 `impl Trait` 和 `Box<dyn Trait>` 各写一个工厂函数，比较调用处与二进制体积。

---

## 延伸阅读

- The Book 第 10 章 — <https://doc.rust-lang.org/book/ch10-00-generics.html>
- Rust Reference：Trait bounds — <https://doc.rust-lang.org/reference/trait-bounds.html>
- Rustonomicon：Lifetimes — <https://doc.rust-lang.org/nomicon/lifetimes.html>
- C++20 concepts vs Rust traits — 设计取舍：名义类型 vs 结构化约束

上一章：[09 · 错误处理](09-error-handling.md) ·
下一章：[11 · 自动化测试](11-testing.md)
