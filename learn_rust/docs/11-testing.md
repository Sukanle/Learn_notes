# 11 · 自动化测试

> 对应 The Book 第 11 章
> 示例 crate：[`testing`](../testing)
> 运行：`cd testing && cargo test`

Rust 把测试做进了 Cargo 和编译器：

- 单元测试与代码放在一起，可访问私有项；
- 集成测试放在 `tests/`，只使用 public API；
- 文档注释里的代码块会被当作 doc test 编译运行；
- `cargo test` 自动发现、并行执行、统一报告。

---

## 1. 第一个测试

```rust
pub fn add(left: u64, right: u64) -> u64 {
    left + right
}

#[cfg(test)]
mod tests {
    use super::*;          // 把父模块的所有项引入

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }
}
```

要点：

- `#[cfg(test)]`：只在 `cargo test` 时编译，不进入生产二进制；
- `#[test]`：标记测试函数，必须无参数、无返回值；
- `use super::*`：测试模块是子模块，可以访问父模块的私有项；
- 测试失败会 panic，Cargo 捕获并报告。

```bash
cargo test
cargo test it_works          # 按名称过滤
cargo test -- --nocapture    # 显示 println! 输出
cargo test -- --test-threads=1
```

---

## 2. 断言

```rust
assert!(condition);
assert!(condition, "value was {value}");        // 自定义消息
assert_eq!(left, right);
assert_eq!(left, right, "context: {context}");
assert_ne!(a, b);
```

| 断言 | C++ 对应 |
| --- | --- |
| `assert!` | `REQUIRE(cond)` / `EXPECT_TRUE` |
| `assert_eq!` | `REQUIRE_EQ` / `EXPECT_EQ` |
| `assert_ne!` | `REQUIRE_NE` |
| `debug_assert!` | `assert`（release 关闭） |

`assert_eq!` 失败时会打印左右值的 `Debug` 表示，因此被比较类型需要 `Debug`。

```rust
#[derive(Debug, PartialEq)]
struct Point { x: i32, y: i32 }

assert_eq!(Point { x: 1, y: 2 }, Point { x: 1, y: 2 });
```

浮点比较不要用 `==`：

```rust
let x = 0.1 + 0.2;
assert!((x - 0.3).abs() < 1e-9);
```

---

## 3. 测试 panic

```rust
#[test]
#[should_panic(expected = "between 1 and 100")]
fn greater_than_100() {
    Guess::new(200);
}
```

- `should_panic` 要求测试**必须 panic**，否则失败；
- `expected` 匹配 panic 消息的子串；
- 只能匹配字符串，不能按 panic 类型捕获（Rust 没有异常类型）；
- 若使用 `panic = "abort"`，`should_panic` 不工作。

对于“应该返回 Err”的测试，不要用 `should_panic`，直接匹配 `Result`：

```rust
#[test]
fn invalid_input_returns_err() {
    let result = parse_port("0");
    assert!(result.is_err());
    assert_eq!(result.unwrap_err(), "port must be > 0");
}
```

---

## 4. 返回 `Result` 的测试

```rust
#[test]
fn result_style_test() -> Result<(), String> {
    if add(2, 2) == 4 {
        Ok(())
    } else {
        Err(String::from("two plus two did not equal four"))
    }
}
```

好处是可以用 `?`：

```rust
#[test]
fn parse_file() -> Result<(), Box<dyn std::error::Error>> {
    let content = std::fs::read_to_string("hello.txt")?;
    assert!(content.contains("hello"));
    Ok(())
}
```

失败时 Cargo 会打印 `Err` 的 `Debug`。注意返回 `Result` 的测试**不能**同时用 `#[should_panic]`。

---

## 5. 忽略、并行与过滤

```rust
#[test]
#[ignore = "slow integration test"]
fn expensive_test() {
    // ...
}
```

```bash
cargo test -- --ignored          # 只跑 ignored
cargo test -- --include-ignored  # 全部
cargo test -- --test-threads=4
cargo test some_filter
```

测试默认多线程并行执行；如果测试共享全局状态/文件/端口，需要：

- 使用独立临时目录/端口；
- 加锁（`Mutex`）或串行执行；
- 避免修改全局环境变量（Rust 2024 中 `set_var` 已是 `unsafe`）。

---

## 6. 单元测试 vs 集成测试

| 类型 | 位置 | 可见性 | 编译单元 |
| --- | --- | --- | --- |
| 单元测试 | 同文件 `#[cfg(test)] mod tests` | 可访问私有项 | 与库一起 |
| 集成测试 | `tests/*.rs` | 只能用 `pub` API | 每个文件独立 crate |
| 文档测试 | `///` 注释中的代码块 | 按文档示例 | 独立 |

`testing/tests/integration_test.rs`：

```rust
use testing::{Guess, add, greeting};

#[test]
fn integration_add() {
    assert_eq!(add(10, 20), 30);
}
```

集成测试模拟外部用户，因此能发现“API 不好用”或“可见性不对”的问题。

### 6.1 共享测试工具

多个集成测试共用辅助代码时：

```text
tests/
├── common/
│   └── mod.rs        # 注意：目录下的 mod.rs 不会被当作测试 crate
├── api_test.rs
└── cli_test.rs
```

```rust
// tests/api_test.rs
mod common;

#[test]
fn uses_helper() {
    common::setup();
}
```

`tests/common/mod.rs` 不会单独编译成测试，只有被 `mod common;` 引入时才参与。

### 6.2 文档测试

```rust
/// 两数相加。
///
/// # Examples
/// ```
/// use testing::add;
/// assert_eq!(add(2, 3), 5);
/// ```
pub fn add(left: u64, right: u64) -> u64 {
    left + right
}
```

`cargo test` 会运行这段代码；`cargo test --doc` 只跑文档测试。
优点：文档不会腐烂。缺点：编译时间增加，可以给不需要的代码块加 `ignore` / `no_run` / `compile_fail`。

```rust
/// ```compile_fail
/// let x: i32 = "not a number";
/// ```
```

---

## 7. C++ 测试框架对照

| GoogleTest / Catch2 | Rust |
| --- | --- |
| `TEST(Suite, Name)` | `#[test] fn name()` |
| `TEST_F(fixture, ...)` | 普通 struct + 辅助函数 |
| `EXPECT_EQ` | `assert_eq!` |
| `EXPECT_THROW` | `#[should_panic]` / 匹配 `Result` |
| `ASSERT_*`（中止） | assert 失败即 panic |
| `SetUp` / `TearDown` | `Drop` / 作用域 |
| `--gtest_filter` | `cargo test filter` |
| `ctest` / CTest | `cargo test` |
| 参数化测试 | 循环 / 宏 / `rstest` crate |
| Mock（gmock） | trait 注入 / `mockall` crate |
| 模糊测试 | `cargo-fuzz` |
| 属性测试 | `proptest` / `quickcheck` |
| 基准测试 | `criterion` / `cargo bench` |
| 覆盖率 | `cargo llvm-cov`（基于 LLVM） |

Rust 的测试没有 fixture 继承机制，通常用：

- 普通函数构造测试数据；
- `Drop` 清理资源；
- trait + 泛型注入依赖（依赖倒置）。

---

## 8. `testing` crate 走读

`src/lib.rs`：

- `add`、`greeting`、`Guess`；
- 私有 `internal_adder`（仅测试可见）；
- 测试模块覆盖：
  - 基本断言；
  - 私有函数；
  - 自定义失败消息；
  - `should_panic(expected)`；
  - `#[ignore]`；
  - 返回 `Result` 的测试；
  - `assert_ne!`。

`tests/integration_test.rs`：

- 只使用 public API；
- 验证 `should_panic` 在集成测试中同样有效。

```bash
cd testing
cargo test                       # 全部
cargo test --lib                 # 只跑单元测试
cargo test --test integration_test
cargo test -- --ignored
```

---

## 9. 测试设计建议

1. **测试行为，不测试实现**：重构内部实现不应破坏测试；
2. **一个测试只验证一件事**：失败信息更清晰；
3. **用 `Result` + `?` 处理测试中的 I/O**，避免满屏 `unwrap`；
4. **边界值**：空输入、0、最大值、UTF-8 多字节、越界；
5. **错误路径也要测**：不仅测 happy path；
6. **不要依赖测试执行顺序**；
7. **CI 中跑 `cargo test --all-targets` + `cargo clippy -D warnings`**；
8. **性能敏感代码加 criterion 基准**，不要用测试计时。

---

## 10. 常见问题

| 问题 | 原因 | 解决 |
| --- | --- | --- |
| 测试不显示 `println!` | 默认捕获 stdout | `-- --nocapture` |
| 测试并行互相干扰 | 共享文件/端口/环境变量 | 隔离资源或 `--test-threads=1` |
| `should_panic` 没生效 | `panic=abort` | 用默认 unwind profile |
| 集成测试找不到私有项 | 只能访问 public API | 移到单元测试，或公开 API |
| doc test 编译失败 | 示例缺少 `use` | 补全导入或用 `no_run` |
| 测试太慢 | 单态化 + 大量依赖 | 拆分、用 `cargo nextest`、并行 |

---

## 11. 练习

1. 给 `testing::Guess` 增加 `try_new(value: i32) -> Result<Guess, String>`，并写成功/失败测试。
2. 为 `minigrep` 增加“空查询串”“文件不存在”的错误测试。
3. 用 `proptest`（可选依赖）验证 `add` 的交换律 `add(a, b) == add(b, a)`。
4. 把一个集成测试拆成 `tests/common/mod.rs` + 两个测试文件，避免重复代码。
5. 给 `web_server` 的 `handle_connection` 抽取纯函数 `parse_request_line`，并为它写单元测试。

---

## 延伸阅读

- The Book 第 11 章 — <https://doc.rust-lang.org/book/ch11-00-testing.html>
- `cargo test` 文档 — <https://doc.rust-lang.org/cargo/commands/cargo-test.html>
- `proptest` — <https://docs.rs/proptest/>
- `criterion` — <https://docs.rs/criterion/>
- `cargo-nextest` — <https://nexte.st/>

上一章：[10 · 泛型、Trait 与 Lifetime](10-generics-traits-lifetimes.md) ·
下一章：[12 · minigrep 项目](12-minigrep.md)
