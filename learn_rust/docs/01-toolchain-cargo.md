# 01 · 工具链与 Cargo：从 CMake + vcpkg 迁移

> 对应 The Book 第 1 章、第 14 章
> 示例：本仓库任意 crate

C++ 的工具链通常是“编译器 + 构建系统 + 包管理器 + 测试框架 + 格式化/静态检查”拼装而成；
Rust 把这些能力收进了 `rustup` + `cargo` 两个工具。理解它们的边界，比记住命令更重要。

---

## 1. 工具全景

| 工具 | 作用 | C++ 类比 |
| --- | --- | --- |
| `rustup` | 安装/切换 toolchain、target、组件 | 多版本 g++/clang + SDK 管理 |
| `rustc` | 编译器 | `clang++` / `g++` / `cl.exe` |
| `cargo` | 构建、依赖、测试、文档、发布 | CMake + vcpkg + CTest + Doxygen |
| `rustfmt` | 代码格式化 | clang-format |
| `clippy` | 静态检查 | clang-tidy |
| `rust-analyzer` | LSP / IDE 支持 | clangd |
| `miri` | 解释执行，检测 UB | ASan + UBSan + valgrind |
| `rustdoc` | 文档生成 + 文档测试 | Doxygen |

安装（已装可跳过）：

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
rustup component add rustfmt clippy rust-src
rustc --version
cargo --version
```

`rustup` 常用命令：

```bash
rustup show                 # 当前 toolchain / target
rustup update stable
rustup toolchain install nightly
rustup override set nightly # 为当前目录固定 toolchain（类似 CMake preset）
rustup target add aarch64-unknown-linux-gnu  # 交叉编译目标
```

### 1.1 Hello, world

```bash
cargo new hello
cd hello
cargo run
```

```rust
// src/main.rs
fn main() {
    println!("Hello, world!");
}
```

`println!` 是宏而不是函数：它在编译期检查格式串和参数类型。
C++ 的对应写法是：

```cpp
#include <iostream>
int main() {
    std::cout << "Hello, world!\n";
}
```

Rust 不需要 `#include`，也不需要手动刷新缓冲区（`println!` 换行时刷新）。

---

## 2. 一个 package 的结构

```text
my_app/
├── Cargo.toml          # manifest，类似 CMakeLists.txt + vcpkg.json
├── Cargo.lock          # 精确版本锁定（可执行程序应提交）
├── src/
│   ├── main.rs         # 二进制入口（默认 bin target）
│   └── lib.rs          # 库根（如果同时是 lib crate）
├── examples/           # cargo run --example xxx
├── tests/              # 集成测试
├── benches/            # 基准测试
├── build.rs            # 构建脚本（可选）
└── target/             # 构建产物（不要提交）
```

创建：

```bash
cargo new hello        # 创建 bin package + git
cargo new --lib math   # 创建 lib package
cargo init             # 在现有目录初始化
```

`Cargo.toml` 逐项解释：

```toml
[package]
name = "my_app"         # 同时是 crate 名（- 会自动转成 _）
version = "0.1.0"       # SemVer
edition = "2024"        # 语言 edition，不等于编译器版本
rust-version = "1.88"   # 最低支持的 Rust 版本（MSRV）
authors = ["you"]
description = "demo"
license = "MIT OR Apache-2.0"

[dependencies]
# 版本约束：默认是 caret（^1.2.3 允许 1.x）
# serde = { version = "1", features = ["derive"] }
# 本地路径依赖（类似 add_subdirectory）
# util = { path = "../util" }

[dev-dependencies]
# 只在 tests/benches/examples 中使用
# pretty_assertions = "1"

[features]
default = ["json"]
json = []
extra = ["json"]

[profile.dev]
opt-level = 1           # 依赖用 -O1，自己用 -O0，缩短调试构建时间

[profile.release]
opt-level = 3           # 类似 -O3
lto = "thin"            # 类似 -flto=thin
codegen-units = 1       # 更好的优化，更慢的编译
panic = "abort"         # 可选：减小二进制，但 catch_unwind 失效
strip = "symbols"
```

与 CMake 的对应：

| CMake | Cargo |
| --- | --- |
| `project(my_app)` | `[package] name` |
| `add_executable` | `[[bin]]`（默认 `src/main.rs`） |
| `add_library` | `[lib]`（默认 `src/lib.rs`） |
| `target_link_libraries` | `[dependencies]` |
| `find_package` + `FetchContent` | crates.io 自动下载 |
| `target_compile_definitions` | `[features]` + `cfg!` |
| `set(CMAKE_CXX_STANDARD 23)` | `edition = "2024"` |
| `CMAKE_BUILD_TYPE` | `--release` / profile |
| `enable_testing()` + `add_test` | `cargo test` |
| `install()` / CPack | `cargo install` / `cargo dist`（第三方） |

---

## 3. 日常命令

```bash
cargo check                 # 只做类型检查，最快；开发时最常用
cargo build                 # 编译 dev profile
cargo build --release       # 优化构建，输出 target/release/
cargo run                   # 构建并运行 bin
cargo run --example <name>   # 运行 examples/<name>.rs
cargo run -- arg1 arg2      # -- 之后的参数传给程序

cargo test                  # 单元 + 集成 + 文档测试
cargo test keyword          # 按名称过滤
cargo test -- --nocapture   # 显示 println! 输出
cargo test -- --test-threads=1
cargo test --lib            # 只跑库的单元测试

cargo fmt                   # 格式化
cargo fmt --check           # CI 检查
cargo clippy --all-targets -- -D warnings
cargo doc --open            # 生成并打开文档

cargo add rand              # 添加依赖（修改 Cargo.toml）
cargo remove rand
cargo update                # 在 SemVer 范围内升级，更新 Cargo.lock
cargo tree                  # 依赖树
cargo metadata --format-version 1
cargo clean
```

调试构建与发布构建的差异（profile）：

| 配置 | dev（默认） | release（`--release`） |
| --- | --- | --- |
| 优化级别 | 0 | 3 |
| 调试符号 | 有 | 默认无（可 `debug = true`） |
| 整数溢出 | **panic**（`overflow-checks = true`） | 回绕 |
| 断言 `debug_assert!` | 启用 | 关闭 |
| 增量编译 | 有 | 部分 |

> 面试常问：Rust 的整数溢出的行为依赖 profile，不是“永远 panic”。需要确定行为时用
> `checked_add` / `wrapping_add` / `saturating_add`。

---

## 4. 依赖、版本与 lockfile

### 4.1 SemVer 与 caret

```toml
rand = "0.10.3"          # 等价于 ^0.10.3：允许 0.10.x，不允许 0.11
serde = "=1.0.200"       # 精确锁定
tokio = { version = "1", features = ["rt-multi-thread", "macros"] }
```

`Cargo.lock` 记录**实际解析出的精确版本 + checksum**，类似 `vcpkg.json` + baseline 或
`conan.lock`。经验规则：

- 可执行程序 / 应用：提交 `Cargo.lock`，保证可复现；
- 库：通常不提交 lockfile，让下游自行解析。

> 本仓库的示例都通过 `**/Cargo.lock` 忽略了锁文件：每个 crate 只是独立的教学示例，
> 不追求可复现构建，也避免 21 份 lockfile 随依赖升级产生噪音。真正发布应用时应提交它。

### 4.2 features

feature 是**编译期开关**，类似 CMake option + `target_compile_definitions`：

```toml
[features]
default = ["std"]
std = []
serde = ["dep:serde"]     # 可选依赖
```

```rust
#[cfg(feature = "serde")]
#[derive(serde::Serialize)]
struct Config { /* ... */ }

#[cfg(not(feature = "std"))]
compile_error!("this crate requires std");
```

与 C++ 的 `#ifdef` 相比：

- feature 组合由 Cargo 统一求解，**同一依赖在图里只编译一次**；
- 没有宏污染；
- 但 feature 应该是**可加的（additive）**，不要用 feature 做互斥的 ABI 切换。

### 4.3 workspace

多个相关 crate 可以放进 workspace 共享 `Cargo.lock` 和 `target/`，类似 CMake 顶层工程：

```toml
# 顶层 Cargo.toml
[workspace]
members = ["app", "core", "cli"]
resolver = "3"

[workspace.dependencies]
serde = { version = "1", features = ["derive"] }

[workspace.package]
edition = "2024"
version = "0.1.0"
```

```toml
# app/Cargo.toml
[package]
name = "app"
edition.workspace = true

[dependencies]
core = { path = "../core" }
serde.workspace = true
```

本教程为了“每章独立可运行”，没有建 workspace；实际项目建议使用 workspace。

### 4.4 发布到 crates.io

发布前补充元数据（否则 `cargo publish` 会拒绝）：

```toml
[package]
description = "A small demo crate"
license = "MIT OR Apache-2.0"
repository = "https://github.com/you/my_app"
keywords = ["demo", "tutorial"]
categories = ["development-tools"]
readme = "README.md"
```

```bash
cargo login                 # 保存 API token（只需一次）
cargo publish --dry-run     # 打包检查，不真正上传
cargo publish               # 上传；发布后版本不可修改，只能 yank
```

版本一旦发布就是**不可变**的，遵循 SemVer；需要修复时发布新版本。
文档会自动构建到 <https://docs.rs>。类比：Conan/vcpkg 的包发布流程，但 Cargo 的
构建、元数据和文档是一体的。

### 4.5 离线 / 内网

```bash
cargo build --offline       # 只用本地缓存
cargo vendor vendor/        # 把依赖源码复制到本地
```

```toml
# .cargo/config.toml
[source.crates-io]
replace-with = "vendored-sources"

[source.vendored-sources]
directory = "vendor"
```

类比：把 vcpkg 依赖 vendor 进仓库，或用 Conan 私有仓库。

---

## 5. build.rs：构建脚本

`build.rs` 在编译 crate 前运行，用于代码生成、探测系统库、编译 C/C++ 代码：

```rust
// build.rs
fn main() {
    // 让 rustc 链接系统库（类似 target_link_libraries(m PRIVATE m)）
    println!("cargo:rustc-link-lib=m");

    // 把 include 路径传给 bindgen 等工具
    println!("cargo:rerun-if-changed=wrapper.h");
}
```

常见用途：

- `bindgen`：从 C 头文件生成 Rust FFI 声明；
- `cc` crate：在 `build.rs` 中编译 C/C++ 源文件；
- `prost` / `tonic`：从 `.proto` 生成代码。

> 注意：`build.rs` 在宿主机上执行，不适用于交叉编译目标代码；交叉编译时需要显式
> 设置 `CC_<target>` / `AR_<target>`。

---

## 6. Rust 2024 edition 的变化（与本教程相关）

edition 是**语言层面的兼容边界**，不是编译器版本：

- `extern` block 必须写 `unsafe extern "C" { ... }`（见 `unsafe_ffi`）；
- `#[no_mangle]` 改为 `#[unsafe(no_mangle)]`；
- `if let` 支持链式条件：`if let Some(x) = a && x > 0 { ... }`（见 `patterns_oop`）；
- `gen` 成为保留字；
- `std::env::set_var` / `remove_var` 变为 `unsafe`（多线程环境下不安全）；
- RPIT（return-position impl Trait）默认捕获所有输入 lifetime；
- match ergonomics 更严格，显式 `ref`/`ref mut` 的模式在 2024 下语义调整。

升级 edition：

```bash
cargo fix --edition
# 修改 Cargo.toml edition = "2024" 后
cargo fix --edition-idioms
```

---

## 7. 工具链实战：格式化、lint、调试

```bash
cargo fmt
cargo clippy --all-targets -- -W clippy::pedantic
cargo test
cargo doc --no-deps --open
```

调试：

```bash
rust-gdb target/debug/my_app
rust-lldb target/debug/my_app
```

`rust-gdb` 会加载 Rust pretty printer，能展开 `String`、`Vec`、`HashMap`。

Miri（检测 UB，需要 nightly）：

```bash
rustup +nightly component add miri
cargo +nightly miri test
```

性能：

```bash
cargo build --release
perf record ./target/release/my_app
cargo bench            # 需要 criterion 等框架
```

与 C++ 的 sanitizer 对比：

- safe Rust 已经排除了大部分 use-after-free / double-free / 数据竞争；
- `unsafe` 代码、FFI、`transmute` 仍需 Miri / ASan 检查；
- `debug_assertions` 下的边界检查与溢出检查是额外的安全网。

---

## 8. 在本仓库中练习

```bash
cd learn_rust/func
cargo run
cargo check --all-targets

cd ../error
cargo run                 # 需要 hello.txt（仓库已提供）
cargo run --example custom_error

cd ../web_server
cargo run                 # 然后访问 http://127.0.0.1:7878/
```

练习：

1. 给 `func` 添加一个 `examples/overload.rs`，用泛型 + trait 模拟 C++ 的函数重载。
2. 在任意 crate 中运行 `cargo clippy --all-targets -- -W clippy::pedantic`，修复至少 5 条建议。
3. 给 `guessing_game` 增加一个 `--max 1000` 命令行参数（只使用 `std::env::args`）。
4. 用 `cargo new --lib` 创建一个包，写一个泛型 `largest` 并添加单元测试。

---

## 延伸阅读

- The Cargo Book — <https://doc.rust-lang.org/cargo/>
- The Book 第 1 章 — <https://doc.rust-lang.org/book/ch01-00-getting-started.html>
- The Book 第 14 章 — <https://doc.rust-lang.org/book/ch14-00-more-about-cargo.html>
- Rust 2024 edition guide — <https://doc.rust-lang.org/edition-guide/rust-2024/index.html>

下一章：[02 · Guessing Game](02-guessing-game.md)
