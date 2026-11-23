# 09 · 错误处理：panic、Result 与 `?`

> 对应 The Book 第 9 章
> 示例 crate：[`panic`](../panic)、[`error`](../error)
> 运行：`cd panic && cargo run`（会 panic）、`cargo run --example panic_forms`；
> `cd error && cargo run`、`cargo run --example custom_error`

Rust 没有异常。错误被分成两种，并由类型系统强制区分：

| 类别 | 机制 | 典型场景 | C++ 对应 |
| --- | --- | --- | --- |
| 不可恢复 | `panic!` | 数组越界、断言失败、逻辑不可能 | `assert` / `std::terminate` / 异常逃逸 |
| 可恢复 | `Result<T, E>` | 文件不存在、解析失败、网络超时 | `std::expected<T, E>` / 错误码 |

> 编译器不会强迫你处理 `panic`，但 `Result` 标了 `#[must_use]`，
> 忽略它会有 warning（甚至可以升级为 error）。

---

## 1. `panic!`：程序 bug 或不变量被破坏

```rust
fn main() {
    let v = vec![1, 2, 4];
    println!("{}", v[99]);   // panic: index out of bounds
}
```

panic 的常见来源：

- 索引越界：`v[99]`；
- `unwrap()` / `expect()` 遇到 `None` / `Err`；
- 整数溢出（debug profile）；
- `assert!` / `assert_eq!` / `debug_assert!`；
- 显式 `panic!("...")`；
- 除零（整数）/ 非法切片边界。

panic 时会发生什么：

1. 打印错误信息 + 位置（默认到 stderr）；
2. 展开栈（unwind），调用每个栈帧中变量的 `Drop`；
3. 若在主线程且未捕获，进程以非零码退出。

```bash
RUST_BACKTRACE=1 cargo run     # 打印 backtrace
RUST_BACKTRACE=full cargo run
```

### 1.1 `unwrap` / `expect`

```rust
let n: Option<i32> = None;
let x = n.unwrap();              // panic: called `Option::unwrap()` on a `None` value

let s = std::fs::read_to_string("config.toml")
    .expect("config.toml should exist");   // panic 信息带上下文
```

- `unwrap`：失败即 panic，适合“逻辑上不可能失败”的原型/测试；
- `expect("...")`：同上，但 panic 信息更清晰；
- 生产代码中，公开 API 应返回 `Result`；内部不变量可以用 `expect`；
- 不要用 `unwrap` 处理用户输入或外部资源。

### 1.2 `panic = "abort"`

```toml
[profile.release]
panic = "abort"
```

- 不展开栈，二进制更小、启动更快；
- `catch_unwind` 失效；
- 跨 FFI 边界更安全（但仍建议显式 catch）。

C++ 类比：`-fno-exceptions` + `std::terminate`，但 Rust 的 `Result` 仍然可用。

### 1.3 `catch_unwind`

```rust
use std::panic;

let result = panic::catch_unwind(|| {
    let v = vec![1, 2, 3];
    v[99]
});
println!("caught: {}", result.is_err());
```

用途：

- 测试框架；
- 线程边界（子线程 panic 不会杀死主线程，`join()` 返回 `Err`）；
- FFI 边界：C++ 不能处理 Rust panic，必须先 catch 再转错误码。

注意：`catch_unwind` 不是“异常控制流”，不应拿它做业务错误处理；
捕获后数据可能处于不一致状态（需要 `UnwindSafe` / `AssertUnwindSafe` 约束）。

---

## 2. `Result<T, E>`：可恢复错误

```rust
enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

```rust
use std::fs;

fn read_config(path: &str) -> Result<String, std::io::Error> {
    let content = fs::read_to_string(path)?;
    Ok(content)
}
```

`?` 的语义：

```rust
let content = fs::read_to_string(path)?;
// 近似于：
let content = match fs::read_to_string(path) {
    Ok(c) => c,
    Err(e) => return Err(From::from(e)),
};
```

`?` 会自动调用 `From::from` 把错误转换成函数返回类型：

```rust
fn run() -> Result<(), Box<dyn std::error::Error>> {
    let n: i32 = "42".parse()?;   // ParseIntError -> Box<dyn Error>
    Ok(())
}
```

> `?` 只能用在返回 `Result`（或 `Option`）的函数中。
> 在 `main` 中可用 `fn main() -> Result<(), E>`。

### 2.1 组合子

```rust
let n = "42".parse::<i32>()
    .map(|v| v * 2)
    .unwrap_or(0);

let path = std::env::var("CONFIG")
    .ok()
    .unwrap_or_else(|| String::from("default.toml"));

let value = maybe_value.ok_or("missing value")?;       // Option -> Result
let value = result.map_err(|e| format!("bad input: {e}"))?;
```

| 方法 | 作用 | C++ 类比 |
| --- | --- | --- |
| `map` | 转换 `Ok` 值 | `expected::transform` |
| `map_err` | 转换 `Err` | 手动包装 |
| `and_then` | 链式返回 `Result` | `and_then` / monadic bind |
| `ok_or` | `Option` → `Result` | 无直接对应 |
| `unwrap_or` | 提供默认值 | `value_or` |
| `?` | 错误提前返回 | 异常传播 / `TRY` 宏 |

---

## 3. 错误类型：从字符串到自定义类型

### 3.1 `Box<dyn Error>`

```rust
use std::error::Error;

fn run() -> Result<(), Box<dyn Error>> {
    let content = std::fs::read_to_string("hello.txt")?;
    println!("{content}");
    Ok(())
}
```

- 适合应用层、原型、`main`；
- 类型擦除，失去具体错误类型信息（可以 downcast）；
- 类似返回 `std::exception_ptr`，但 `?` 使用更方便。

### 3.2 自定义错误

```rust
#[derive(Debug)]
enum AppError {
    Io(std::io::Error),
    EmptyFile,
    InvalidNumber(String),
}

impl std::fmt::Display for AppError { /* ... */ }
impl std::error::Error for AppError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> { /* ... */ }
}
impl From<std::io::Error> for AppError {
    fn from(e: std::io::Error) -> Self { AppError::Io(e) }
}
```

实现 `Display + Error + From` 后，`?` 就能自动转换：

```rust
fn read_first_line(path: &str) -> Result<String, AppError> {
    let content = std::fs::read_to_string(path)?;   // io::Error -> AppError
    content.lines().next()
        .map(str::to_string)
        .ok_or(AppError::EmptyFile)
}
```

`source()` 用于错误链：

```rust
let mut source = std::error::Error::source(&err);
while let Some(cause) = source {
    eprintln!("caused by: {cause}");
    source = cause.source();
}
```

C++23 对照：

| Rust | C++23 |
| --- | --- |
| `Result<T, E>` | `std::expected<T, E>` |
| `Box<dyn Error>` | `std::exception_ptr` / 基类指针 |
| `Error::source` | `std::throw_with_nested` / 嵌套异常 |
| `From<E>` + `?` | catch 中的错误转换 |
| `#[derive(Debug)]` + `Display` | `what()` + `operator<<` |

### 3.3 生态库（可选）

实际项目常用：

- `thiserror`：为 enum 错误自动实现 `Display` / `Error` / `From`；
- `anyhow`：应用层错误上下文（`Context`、`anyhow!`）；
- `eyre`：更丰富的报告。

本教程为了零依赖，手写实现；理解原理后再用库会更踏实。

---

## 4. `error` crate 走读

```rust
use std::io::Read;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut f = std::fs::File::open("hello.txt")?;
    let mut buf = String::new();
    f.read_to_string(&mut buf)?;
    println!("read file: {buf}");
    Ok(())
}
```

要点：

- `main` 返回 `Result` 时，`Err` 会打印 Debug 并设置退出码（通常 1）；
- `File::open` / `read_to_string` 的错误自动转成 `Box<dyn Error>`；
- `hello.txt` 是仓库提供的示例输入；若从其他目录运行，需使用绝对路径；
- `return Ok(())` 可以简写为 `Ok(())`（clippy 会提示）。

`examples/custom_error.rs` 演示：

- 自定义 `AppError`；
- `From<io::Error>`；
- `map_err` 增加上下文；
- `source()` 错误链；
- 在 `main` 中打印 `Debug` 错误。

---

## 5. 错误处理的设计原则

1. **库的公开 API 返回 `Result`**，不要直接用 `panic!` 表达可预期失败；
2. **错误类型要能区分失败原因**（enum 或结构化 error）；
3. **`panic!` 只用于 bug / 不变量破坏 / 原型**；
4. **在边界处加 context**：`map_err(|e| ...)` 或 `anyhow::Context`；
5. **不要忽略 `Result`**：`let _ = ...` 只应在明确可忽略时使用；
6. **FFI 边界必须 catch panic**，返回错误码；
7. **测试中可以用 `unwrap`**，让失败信息更直接；
8. **`main` 返回 `Result`** 是最简单的“错误即退出”方案。

```rust
// 不推荐：错误信息丢失
let n: i32 = text.parse().unwrap();

// 推荐：区分错误并加 context
let n: i32 = text.trim().parse().map_err(|e| {
    format!("failed to parse {text:?}: {e}")
})?;
```

---

## 6. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `the ? operator can only be used in a function that returns Result` | 在普通函数中用 `?` | 改返回类型，或 `match` |
| `mismatched types: Result<_, E1> vs Result<_, E2>` | 错误类型不同 | `map_err` / `From` |
| `unused Result that must be used` | 忽略返回值 | `?` 或 `let _ =` |
| `cannot use ? on Option in Result fn` | 混用 Option/Result | `.ok_or(...)?` |
| `called Result::unwrap() on an Err` | 未处理失败 | 改用 `match` / `?` |
| `panic in a function that cannot unwind` | FFI / `panic=abort` | 避免 panic 跨边界 |

---

## 7. 练习

1. 给 `error` crate 增加 `examples/error_chain.rs`，构造三层错误并用 `source()` 打印。
2. 写 `fn parse_port(s: &str) -> Result<u16, String>`，拒绝 0 和大于 65535 的值。
3. 把 `guessing_game` 的 `main` 改成返回 `Result`，去掉所有 `unwrap`。
4. 用 `enum ConfigError` 表示“文件不存在 / 格式错误 / 缺少字段”，并为每种情况写测试。
5. 解释为什么 `catch_unwind` 不能替代 `Result`，举一个捕获后状态不一致的例子。

---

## 延伸阅读

- The Book 第 9 章 — <https://doc.rust-lang.org/book/ch09-00-error-handling.html>
- `std::error::Error` — <https://doc.rust-lang.org/std/error/trait.Error.html>
- `thiserror` — <https://docs.rs/thiserror/>
- `anyhow` — <https://docs.rs/anyhow/>
- C++23 `std::expected` — <https://en.cppreference.com/w/cpp/utility/expected>

上一章：[08 · 集合](08-collections.md) ·
下一章：[10 · 泛型、Trait 与 Lifetime](10-generics-traits-lifetimes.md)
