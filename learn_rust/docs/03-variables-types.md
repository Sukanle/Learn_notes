# 03 · 变量、类型与字符串

> 对应 The Book 第 3.1–3.2 节
> 示例 crate：[`learn_var`](../learn_var)
> 运行：`cargo run`、`cargo run --example scalars`、`cargo run --example strings`

---

## 1. 变量默认不可变

```rust
let x = 5;
// x = 6;             // error[E0384]: cannot assign twice to immutable variable
let mut y = 5;
y = 6;                // OK
```

C++ 的 `const` 是**可选**的，而且有 `const`/`constexpr`/`mutable` 等多种变体；
Rust 把“不可变”设为默认，需要修改时必须显式写 `mut`。

```cpp
int x = 5;            // C++ 默认可变
const int y = 5;      // 需要 const 才不可变
```

> `mut` 是类型的一部分（`&mut T` 与 `&T` 不同），不是可随意丢弃的修饰符。
> 函数签名会告诉调用者它是否会修改参数。

### 1.1 Shadowing（遮蔽）

```rust
let spaces = "   ";       // &str
let spaces = spaces.len(); // usize：同名变量，类型可以改变
```

C++ 不允许在同一作用域重复定义同名变量（会报 redefinition）；Rust 允许 shadowing，
新变量会遮蔽旧变量，旧变量仍然存活到作用域结束（但不再可访问）。

与 `mut` 的区别：

- `mut` 是同一个变量，类型不能变；
- shadowing 是**新变量**，可以改变类型，也可以改变可变性。

---

## 2. 常量与静态变量

```rust
const MAX_POINTS: u32 = 100_000;       // 编译期常量，内联，无固定地址
static GREETING: &str = "hello";       // 静态变量，有固定地址
static mut COUNTER: u32 = 0;           // 可变静态：访问需要 unsafe
```

| Rust | C++ | 说明 |
| --- | --- | --- |
| `const` | `constexpr` | 编译期求值，可内联 |
| `static` | `static` / 全局变量 | 整个程序一份，有地址 |
| `static mut` | 全局可变变量 | Rust 中访问是 `unsafe`（数据竞争风险） |
| `const fn` | `consteval` / `constexpr` 函数 | 可在编译期调用 |

命名约定：常量 `SCREAMING_SNAKE_CASE`，类型 `UpperCamelCase`，变量/函数 `snake_case`。

---

## 3. 标量类型

### 3.1 整数

| Rust | 宽度 | C++ 对应 |
| --- | --- | --- |
| `i8` / `u8` | 8 bit | `int8_t` / `uint8_t` |
| `i16` / `u16` | 16 bit | `int16_t` / `uint16_t` |
| `i32` / `u32` | 32 bit | `int32_t` / `uint32_t` |
| `i64` / `u64` | 64 bit | `int64_t` / `uint64_t` |
| `i128` / `u128` | 128 bit | `__int128`（非标准） |
| `isize` / `usize` | 指针宽度 | `ptrdiff_t` / `size_t` |

- 默认整数类型是 **`i32`**；
- 字面量可以加后缀：`42u8`、`1_000i64`；
- 下划线只是可读性分隔符；
- 没有隐式整数提升/截断：`i32` 不会自动变成 `i64`。

```rust
let x: i32 = 1;
let y: i64 = 2;
// let z = x + y;        // error[E0308]: mismatched types
let z = i64::from(x) + y; // 显式转换
let w = x as i64;         // 也可以 cast，但可能丢失检查
```

`as` 与 `static_cast` 的差异：

- 整数窄化：`300i32 as u8` 得到 `44`（截断，不是 UB）；
- 浮点转整数：Rust 1.45+ 饱和（超出范围得到 `MIN`/`MAX`），C++ 是 UB；
- 指针与整数互转需要 `as`，但解引用仍需 `unsafe`。

### 3.2 溢出

```rust
let mut n: u8 = 255;
// n += 1;                 // debug: panic "attempt to add with overflow"
n = n.wrapping_add(1);     // 0
```

| 方法 | 行为 | C++ 对应 |
| --- | --- | --- |
| `+` / `*` | debug panic，release 回绕 | 有符号溢出是 UB |
| `wrapping_add` | 回绕 | 无符号自然回绕 |
| `checked_add` | 返回 `Option` | 手动检查 |
| `saturating_add` | 饱和到边界 | `std::clamp` 思路 |
| `overflowing_add` | 返回值 + 溢出标志 | 手动检查 |

> 这是 Rust “安全默认 + 显式选择”的典型例子。生产代码不要依赖 debug 的 panic。

### 3.3 浮点

- `f32`（≈ `float`）、`f64`（≈ `double`），默认 `f64`；
- 没有隐式整数 ↔ 浮点转换；
- `f64::NAN`、`INFINITY`，比较 NaN 时 `==` 为 false（与 IEEE 754 一致）；
- `partial_cmp` 返回 `Option<Ordering>`，因为 NaN 无法排序；这就是浮点只实现 `PartialOrd` 而不是 `Ord` 的原因。

```rust
let x = 0.1_f64 + 0.2;
println!("{x:.17}");            // 0.30000000000000004
println!("{}", 1.0_f64 / 0.0);  // inf
println!("{}", 0.0_f64 / 0.0);  // NaN
```

### 3.4 bool 与 char

```rust
let t: bool = true;   // 1 byte
let c: char = '中';   // 4 bytes，Unicode Scalar Value
```

- `bool` 不能隐式转整数（C++ 可以）；需要 `u8::from(b)`；
- `char` 是 Unicode 标量值，范围 `U+0000..=U+D7FF`、`U+E000..=U+10FFFF`；
- 因此 `char` 不是“一个字节”，字符串也不能按 `char` 索引。

### 3.5 unit 与 never

```rust
fn f() {}          // 返回 ()
fn g() -> ! { panic!() }  // never type，类似 [[noreturn]]
```

`()` 是零大小类型（ZST），可以出现在泛型中，例如 `Result<(), Error>`。

---

## 4. 复合类型：Tuple 与 Array

### 4.1 Tuple

```rust
let tup: (i32, f64, char) = (500, 6.4, 'x');
let five_hundred = tup.0;       // 按索引访问
let (a, b, c) = tup;            // 解构，类似 C++ structured bindings
```

| Rust | C++ |
| --- | --- |
| `(i32, f64, char)` | `std::tuple<int, double, char>` |
| `tup.0` | `std::get<0>(tup)` |
| `let (a, b, c) = tup;` | `auto [a, b, c] = tup;` |
| `()` | `std::tuple<>` / `void` |

Rust 的 tuple 是语言内置的，字段没有名字；字段过多时应定义 struct。

### 4.2 Array

```rust
let a = [1, 2, 3, 4, 5];        // [i32; 5]
let b: [i32; 5] = [1, 2, 3, 4, 5];
let c = [3; 5];                 // [3, 3, 3, 3, 3]
println!("{}", a[0]);
```

| Rust | C++ |
| --- | --- |
| `[T; N]` | `std::array<T, N>` |
| `[0; 16]` | `std::array<T, 16>{}`（值初始化） |
| 长度是类型的一部分 | 长度是类型的一部分 |
| 索引越界 panic（安全） | `operator[]` 是 UB，`at()` 抛异常 |

数组在栈上（或在结构体内部），长度编译期已知。需要动态长度时用 `Vec<T>`。

> Rust 的数组**没有退化（decay）为指针**。传参时用切片：
> `fn sum(xs: &[i32])` 可以接收 `&[i32; 5]` 或 `&Vec<i32>`。

---

## 5. `String` 与 `&str`

这是 C++ 开发者最容易“看起来懂了”的地方。

```rust
let literal: &str = "hello";          // 静态字符串切片，&'static str
let owned: String = String::from("hello"); // 拥有堆内存
let view: &str = &owned;              // 借用 String 的内容
```

| Rust | C++ | 所有权 |
| --- | --- | --- |
| `String` | `std::string` | 拥有，可增长 |
| `&str` | `std::string_view` | 借用，只读 |
| `&'static str` | 字符串字面量 | 程序生命周期 |
| `String::with_capacity(n)` | `string::reserve(n)` | 预分配 |
| `.to_string()` | `std::string(sv)` | 复制为拥有者 |
| `.as_str()` | `std::string_view(s)` | 借用视图 |

### 5.1 UTF-8 与索引

Rust 的 `String` 内部是 `Vec<u8>`，但保证是合法 UTF-8。因此：

```rust
let s = String::from("你好");
// let c = s[0];          // error: String cannot be indexed by integer
let c = s.chars().next();           // Option<char>
let bytes = s.as_bytes();           // &[u8]
let slice = &s[0..6];               // "你好"：6 个字节
// let bad = &s[0..1];              // panic: byte index 1 is not a char boundary
```

C++ 的 `std::string` 是字节序列，`s[0]` 就是一个字节；Rust 强制你区分
**字节（u8）**、**Unicode 标量（char）**、**字素簇（grapheme）**。
需要按“用户感知字符”切分时使用 `unicode-segmentation` crate。

### 5.2 拼接

```rust
let mut s = String::from("hello");
s.push_str(", world");  // 追加 &str，不消耗参数
s.push('!');            // 追加 char

let a = String::from("tic");
let b = String::from("tac");
let c = format!("{a}-{b}");    // 类似 std::format，不消耗 a/b
let d = a + &b;                // + 会消耗 a（类似 operator+ 的右值重载）
// println!("{a}");            // error: a 已被 move
```

`format!` 是最常用的拼接方式：返回新 `String`，参数按引用处理，不会 move。

### 5.3 选择建议

- 函数参数：`&str`（能接受 `String`、字面量、切片）；
- 返回值：需要新字符串时 `String`；只是视图时 `&str`（必须有明确的生命周期来源）；
- 结构体字段：需要拥有时 `String`，需要借用时带 lifetime 的 `&'a str`；
- 不要为了“省 clone”而在结构体里存 `&str`，除非生命周期关系确实清晰。

---

## 6. 本 crate 代码地图

`learn_var/src/main.rs`：

- tuple 定义、索引、解构；
- 数组的三种初始化；
- 数组索引与 `Debug` 打印。

`examples/scalars.rs`：

- 所有整数类型、字面量写法；
- 溢出三种处理方式；
- 浮点、bool、char、shadowing、const/static。

`examples/strings.rs`：

- `String` / `&str` 互相转换；
- UTF-8 遍历、字节切片、非字符边界；
- 查找、替换、分割、trim。

```bash
cd learn_var
cargo run
cargo run --example scalars
cargo run --example strings
```

---

## 7. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `cannot assign twice to immutable variable` | 没写 `mut` | `let mut x` |
| `mismatched types` | 整数宽度/有无符号混用 | `i64::from(x)` / `as` |
| `cannot index into a value of type String` | 按整数索引 String | `.chars()` / `.bytes()` / 切片 |
| `byte index is not a char boundary` | 切片落在多字节字符中间 | 用 `char_indices()` 找边界 |
| `borrow of moved value` | `+` 或传参消耗了 String | 用 `format!` 或传 `&s` |
| `attempt to add with overflow` | debug 溢出 | `checked_add` 等 |

---

## 8. 练习

1. 打印 `i32::MIN`、`i32::MAX`、`u64::MAX`，并尝试用 `checked_add` 处理边界。
2. 写一个 `fn count_chars(s: &str) -> (usize, usize)`，返回（字节数，字符数）。
3. 把 `"hello world"` 反转（按 char，而不是按字节），再按字节反转对比结果。
4. 用 tuple 表示 `(name, age, score)`，写函数找出分数最高的人。
5. 实现 `fn truncate_at_char_boundary(s: &mut String, max_bytes: usize)`，保证不 panic。

---

## 延伸阅读

- The Book 3.1–3.2 — <https://doc.rust-lang.org/book/ch03-01-variables-and-mutability.html>
- `std::primitive` 数值类型 — <https://doc.rust-lang.org/std/primitive.i32.html>
- UTF-8 与 Rust 字符串 — <https://doc.rust-lang.org/book/ch08-02-strings.html>

上一章：[02 · Guessing Game](02-guessing-game.md) ·
下一章：[04 · 函数与控制流](04-functions-control-flow.md)
