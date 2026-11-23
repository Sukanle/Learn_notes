# 12 · minigrep：一个完整的 I/O 项目

> 对应 The Book 第 12 章
> 示例 crate：[`minigrep`](../minigrep)
> 运行：
> ```bash
> cd minigrep
> cargo run -- the poem.txt
> IGNORE_CASE=1 cargo run -- HOW poem.txt
> cargo test
> ```

这一章把前面学的所有权、`Result`、trait、lifetime、测试串成一个真实的小工具：
在文件中搜索字符串，支持大小写敏感/不敏感两种模式。

---

## 1. 需求与 C++ 对照

```text
minigrep <query> <file>
```

| 需求 | C++23 实现 | Rust 实现 |
| --- | --- | --- |
| 读取命令行 | `argc/argv` | `std::env::args()` 迭代器 |
| 读文件 | `std::ifstream` + `getline` | `fs::read_to_string` |
| 逐行搜索 | `std::string::find` | `line.contains(query)` |
| 错误处理 | 错误码 / 异常 | `Result<T, E>` + `?` |
| 大小写不敏感 | `std::tolower` 逐字符 | `to_lowercase()` |
| 环境变量 | `std::getenv` | `env::var("IGNORE_CASE")` |
| 输出 | `std::cout` | `println!` |

C++ 版本通常会写成：

```cpp
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: minigrep <query> <file>\n";
        return 1;
    }
    std::ifstream file(argv[2]);
    if (!file) return 1;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(argv[1]) != std::string::npos) {
            std::cout << line << '\n';
        }
    }
}
```

Rust 版本把每一步失败都编码进类型，并用 `?` 传播。

---

## 2. 结构：lib + bin

```text
minigrep/
├── Cargo.toml
├── poem.txt
├── src/
│   ├── lib.rs      # 可测试的业务逻辑
│   └── main.rs     # 只负责参数与退出码
└── tests/          # 可选：集成测试
```

为什么拆成 lib + bin？

- 业务逻辑可被集成测试、基准、其他 crate 复用；
- `main.rs` 保持很薄，只做 I/O 和错误报告；
- 单元测试可以访问 lib 内部细节。

这是 Rust 项目非常常见的结构（类似 C++ 中把逻辑放进静态库，`main.cpp` 只做入口）。

---

## 3. 配置解析

```rust
pub struct Config {
    pub query: String,
    pub file_path: String,
    pub ignore_case: bool,
}

impl Config {
    pub fn build(mut args: impl Iterator<Item = String>) -> Result<Config, &'static str> {
        args.next(); // 跳过程序名

        let query = args.next().ok_or("Didn't get a query string")?;
        let file_path = args.next().ok_or("Didn't get a file path")?;

        let ignore_case = env::var("IGNORE_CASE").is_ok();

        Ok(Config { query, file_path, ignore_case })
    }
}
```

设计点：

1. **接收 `impl Iterator<Item = String>`**：调用者可以传 `env::args()`，不必先收集成 `Vec`。
   这是“零成本抽象”的典型用法。
2. **`ok_or` 把 `Option` 转成 `Result`**：缺少参数是可预期错误，不是 panic。
3. **返回 `&'static str` 作为错误**：简单场景足够；真实项目建议自定义错误 enum 或 `anyhow`。
4. **`env::var`**：只判断是否存在（`is_ok()`），不关心值。
5. **跳过程序名**：`env::args()` 的第一个元素是可执行文件路径（类似 `argv[0]`）。

调用：

```rust
let config = Config::build(env::args()).unwrap_or_else(|err| {
    eprintln!("Problem parsing arguments: {err}");
    process::exit(1);
});
```

`unwrap_or_else` 在错误时执行闭包，避免 `unwrap` 的默认 panic 信息。
`process::exit(1)` 类似 C++ 的 `return EXIT_FAILURE`。

---

## 4. 运行逻辑

```rust
pub fn run(config: Config) -> Result<(), Box<dyn Error>> {
    let contents = fs::read_to_string(config.file_path)?;

    let results = if config.ignore_case {
        search_case_insensitive(&config.query, &contents)
    } else {
        search(&config.query, &contents)
    };

    for line in results {
        println!("{line}");
    }

    Ok(())
}
```

- `Box<dyn Error>` 容纳 `io::Error`、`ParseIntError` 等任意错误；
- `?` 在 `read_to_string` 失败时提前返回；
- `contents` 是拥有所有权的 `String`；
- 返回的 `Vec<&str>` 借用 `contents`，因此在 `run` 内打印是安全的。

如果 `run` 返回这些 `&str`，就需要把 `contents` 的所有权一起返回，或者让调用者提供缓冲区。
这是 lifetime 在真实代码中的体现。

---

## 5. 搜索函数与 lifetime

```rust
pub fn search<'a>(query: &str, contents: &'a str) -> Vec<&'a str> {
    contents
        .lines()
        .filter(|line| line.contains(query))
        .collect()
}
```

为什么是 `<'a>` 而不是两个相同的 lifetime？

- 返回的每个 `&str` 都来自 `contents`，与 `query` 无关；
- 如果写成 `query: &'a str, contents: &'a str`，调用者会被迫让 query 和 contents 活得一样久，
  约束过强；
- 省略标注时，编译器无法判断返回值来自哪个输入，因此必须显式。

大小写不敏感：

```rust
pub fn search_case_insensitive<'a>(query: &str, contents: &'a str) -> Vec<&'a str> {
    let query = query.to_lowercase();
    contents
        .lines()
        .filter(|line| line.to_lowercase().contains(&query))
        .collect()
}
```

注意：

- 每次比较都会为 `line` 分配一个小写副本，O(n) 额外分配；
- 对大小写不敏感搜索，这已经够用；
- 性能敏感时可以用 `eq_ignore_ascii_case`（仅 ASCII）或 `unicode-normalization`。

> `to_lowercase()` 可能把一个字符变成多个字符（例如 `İ`），因此不能简单地逐字节比较。

---

## 6. 入口与退出码

```rust
fn main() {
    let config = Config::build(env::args()).unwrap_or_else(|err| {
        eprintln!("Problem parsing arguments: {err}");
        process::exit(1);
    });

    if let Err(err) = minigrep::run(config) {
        eprintln!("Application error: {err}");
        process::exit(1);
    }
}
```

- 错误写 stderr，正常输出写 stdout；
- 非零退出码便于 shell/CI 判断；
- 也可以用 `fn main() -> Result<(), Box<dyn Error>>`，但 `process::exit` 能更明确地控制退出码。

C++ 对照：

```cpp
int main(int argc, char** argv) {
    auto config = parse_args(argc, argv);
    if (!config) { std::cerr << "...\n"; return 1; }
    return run(*config) ? 0 : 1;
}
```

---

## 7. 测试

`src/lib.rs` 中的单元测试：

```rust
#[cfg(test)]
mod tests {
    use super::*;

    const CONTENTS: &str = "\
Rust:
safe, fast, productive.
Pick three.
Trust me.";

    #[test]
    fn case_sensitive() {
        assert_eq!(search("duct", CONTENTS), vec!["safe, fast, productive."]);
    }

    #[test]
    fn case_insensitive() {
        assert_eq!(
            search_case_insensitive("rUsT", CONTENTS),
            vec!["Rust:", "Trust me."]
        );
    }
}
```

覆盖：

- 大小写敏感；
- 大小写不敏感；
- 无匹配；
- 参数不足（`Config::build` 返回 `Err`）。

`tests/cli.rs` 则是真正的 CLI 集成测试：用 `env!("CARGO_BIN_EXE_minigrep")`
拿到编译好的二进制，再通过 `std::process::Command` 断言退出码与 stderr。

```rust
let output = Command::new(env!("CARGO_BIN_EXE_minigrep"))
    .args(["the", "poem.txt"])
    .output()
    .expect("failed to run minigrep");
assert!(output.status.success());
```

这比只测库函数更接近用户实际体验，而且不需要额外的测试框架。

---

## 8. 运行与实验

```bash
cd minigrep

cargo run -- the poem.txt
# Searching for the
# In file poem.txt
# Then there's a pair of us - don't tell!
# To tell your name the livelong day

IGNORE_CASE=1 cargo run -- HOW poem.txt
# How dreary to be somebody!
# How public, like a frog

cargo run -- the missing.txt
# Application error: No such file or directory (os error 2)
echo $?    # 1
```

实验：

- 删除一个参数，观察 `Config::build` 的错误；
- 用不存在的文件，观察 `?` 如何传播；
- 用空查询串，观察所有行都被匹配；
- 在 `run` 中打印 `contents.len()`，理解所有权。

---

## 9. 重构建议与扩展

1. **错误类型化**：用 `enum ConfigError` 替代 `&'static str`；
2. **参数解析库**：`clap`（derive）替代手写；
3. **正则搜索**：`regex` crate；
4. **递归目录**：`walkdir`；
5. **颜色高亮**：`owo-colors` / `termcolor`；
6. **流式处理**：文件很大时用 `BufReader::lines()` 而不是 `read_to_string`；
7. **并发搜索**：`rayon` 按行/文件并行；
8. **基准测试**：`criterion` 比较不同实现。

```rust
// 流式版本示意
use std::io::{BufRead, BufReader};

let file = std::fs::File::open(path)?;
for line in BufReader::new(file).lines() {
    let line = line?;
    if line.contains(query) {
        println!("{line}");
    }
}
```

流式版本的代价是：每行都是新的 `String`，无法返回 `&str` 切片。
这是“内存占用 vs 分配次数”的典型权衡。

---

## 10. 练习

1. 增加 `-i` / `--ignore-case` 参数（替代环境变量），并保持向后兼容。
2. 增加行号输出（`1: ...`），注意不要破坏搜索函数的 lifetime。
3. 支持多个文件路径，统计每个文件的匹配数。
4. 用 `BufReader` 实现流式版本，并写一个 1GB 文件的性能对比（可用 `dd` 生成）。
5. 把 `run` 的错误类型改成自定义 `AppError`，实现 `source()`。
6. 为 `search` 增加“整词匹配”选项，写测试覆盖边界（标点、UTF-8）。

---

## 延伸阅读

- The Book 第 12 章 — <https://doc.rust-lang.org/book/ch12-00-an-io-project.html>
- `std::env::args` — <https://doc.rust-lang.org/std/env/fn.args.html>
- `std::fs::read_to_string` — <https://doc.rust-lang.org/std/fs/fn.read_to_string.html>
- `clap` — <https://docs.rs/clap/>
- `regex` — <https://docs.rs/regex/>

上一章：[11 · 自动化测试](11-testing.md) ·
下一章：[13 · 闭包与迭代器](13-iterators-closures.md)
