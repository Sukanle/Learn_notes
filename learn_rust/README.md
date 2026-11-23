# Learn Rust —— 写给 C++23 开发者的 Rust 教程

基于 [The Rust Programming Language](https://doc.rust-lang.org/book/)（简称 *The Book*）编写的可运行教程。
每个主题都有独立的 Cargo crate、可执行示例和一份中文讲解，重点回答一个问题：

> **这件事在 C++23 里怎么做？Rust 为什么换了一种做法？**

- 语言版本：**Rust 2024 edition**（建议 Rust 1.88+，示例在 1.99 上验证）
- 注释与文档：中文；专业术语保留英文（ownership、borrowing、trait、lifetime…）
- 依赖：除 `guessing_game` 使用 `rand` 外，全部只依赖标准库
- 验证：所有 crate 均通过 `cargo check --all-targets`；测试与非交互示例已实际运行

---

## 1. 从这里开始

```bash
cd learn_rust

# 每个 crate 都是独立工程，进入目录即可运行
cd ownership
cargo run                 # 运行 src/main.rs
cargo run --example borrow_rules
cargo test                # 运行测试（有测试的 crate）
cargo check --all-targets # 只做类型检查，最快
```

> 想一次验证全部章节：

```bash
cd learn_rust
for d in */; do
  [ -f "$d/Cargo.toml" ] && (cd "$d" && cargo check --all-targets) || true
done
```

### 三条学习路线

| 路线 | 顺序 | 适合 |
| --- | --- | --- |
| 快速上手（半天） | [01](docs/01-toolchain-cargo.md) → [02](docs/02-guessing-game.md) → [03](docs/03-variables-types.md) → [04](docs/04-functions-control-flow.md) → [05](docs/05-ownership.md) → [06](docs/06-structs-enums.md) | 只想尽快写出能跑的 Rust |
| 系统学习（2–3 周） | 从 [00](docs/00-cpp-to-rust.md) 按编号读完全部文档，并完成每章练习 | 建立完整心智模型 |
| C++ 迁移重点 | [05](docs/05-ownership.md)、[09](docs/09-error-handling.md)、[10](docs/10-generics-traits-lifetimes.md)、[14](docs/14-smart-pointers.md)、[15](docs/15-concurrency.md)、[17](docs/17-unsafe-ffi.md) | 已有 C++ 经验，专攻差异点 |

---

## 2. 章节导航

| The Book | 主题 | 示例 crate | 文档 | 对应 C++ 知识 |
| --- | --- | --- | --- | --- |
| 1 + 14 | 工具链与 Cargo | 全部 | [01-toolchain-cargo](docs/01-toolchain-cargo.md) | 编译器 + CMake + vcpkg |
| 2 | Guessing game | [`guessing_game`](guessing_game) | [02-guessing-game](docs/02-guessing-game.md) | `cin` / `mt19937` / `expected` |
| 3.1–3.2 | 变量与类型 | [`learn_var`](learn_var) | [03-variables-types](docs/03-variables-types.md) | 基础类型 / `string` / `span` |
| 3.3–3.5 | 函数与控制流 | [`func`](func)、[`branches`](branches) | [04-functions-control-flow](docs/04-functions-control-flow.md) | 函数 / `if` / `switch` / 循环 |
| 4 | Ownership、Borrowing、Slice | [`ownership`](ownership) | [05-ownership](docs/05-ownership.md) | `unique_ptr` / 指针 / `span` |
| 5–6 | Struct、Enum、模式匹配 | [`structs_enums`](structs_enums) | [06-structs-enums](docs/06-structs-enums.md) | `class` / `variant` / `optional` |
| 7 | Package、Crate、Module | [`modules`](modules) | [07-modules](docs/07-modules.md) | namespace / 静态库 target |
| 8 | 集合 | [`collections`](collections) | [08-collections](docs/08-collections.md) | `vector` / `string` / `unordered_map` |
| 9 | 错误处理 | [`panic`](panic)、[`error`](error) | [09-error-handling](docs/09-error-handling.md) | 异常 / `expected` / `noexcept` |
| 10 | 泛型、Trait、Lifetime | [`generics`](generics)、[`traits`](traits)、[`lifetime`](lifetime) | [10-generics-traits-lifetimes](docs/10-generics-traits-lifetimes.md) | 模板 / concepts / 引用标注 |
| 11 | 自动化测试 | [`testing`](testing) | [11-testing](docs/11-testing.md) | GoogleTest / Catch2 |
| 12 | I/O 项目：minigrep | [`minigrep`](minigrep) | [12-minigrep](docs/12-minigrep.md) | `argc/argv` / `ifstream` |
| 13 | 闭包与迭代器 | [`iterators`](iterators) | [13-iterators-closures](docs/13-iterators-closures.md) | lambda / ranges |
| 15 | 智能指针 | [`smart_pointers`](smart_pointers) | [14-smart-pointers](docs/14-smart-pointers.md) | `unique_ptr` / `shared_ptr` / `weak_ptr` |
| 16 | 并发 | [`concurrency`](concurrency) | [15-concurrency](docs/15-concurrency.md) | `jthread` / `mutex` / channel |
| 17–18 | OOP 与模式匹配 | [`patterns_oop`](patterns_oop) | [16-oop-patterns](docs/16-oop-patterns.md) | 虚函数 / `variant` / `visit` |
| 19 | Unsafe 与 FFI | [`unsafe_ffi`](unsafe_ffi) | [17-unsafe-ffi](docs/17-unsafe-ffi.md) | 裸指针 / `extern "C"` |
| 20 | 最终项目：Web Server | [`web_server`](web_server) | [18-web-server](docs/18-web-server.md) | socket / 线程池 / ASIO |

另有一篇总览：[00-cpp-to-rust](docs/00-cpp-to-rust.md) —— 建议先读。

---

## 3. 五分钟心智模型

### 3.1 所有权不是 GC，也不是 `shared_ptr`

| C++23 | Rust | 关键差异 |
| --- | --- | --- |
| `std::unique_ptr<T>` | `Box<T>` | 都独占；Rust move 后**源变量不可用**（编译期） |
| `std::shared_ptr<T>` | `Rc<T>`（单线程）/ `Arc<T>`（多线程） | Rust 中 `Rc<T>` 只给不可变访问 |
| `std::weak_ptr<T>` | `Weak<T>` | 同样用于打破循环引用 |
| `T*` / `const T*` | `&mut T` / `&T` | Rust 编译期证明非空、有效、无别名冲突 |
| `std::string` | `String` | 拥有堆内存，UTF-8 |
| `std::string_view` | `&str` | 借用 UTF-8 视图 |
| `std::span<T>` | `&mut [T]` | 借用连续区间 |
| `std::optional<T>` | `Option<T>` | 必须显式处理 `None` |
| `std::expected<T,E>` | `Result<T,E>` | `?` 是零成本错误传播 |
| `std::variant<A,B>` | `enum { A, B(..) }` | `match` 强制穷尽 |
| 虚函数 / 抽象基类 | `dyn Trait` | 对象安全 + vtable |
| 模板 + concepts | 泛型 + trait bounds | 单态化，但定义处即检查 |
| namespace / header | `mod` / crate | 无头文件，无文本包含 |

一句话：**C++ 把资源生命周期交给程序员约定，Rust 把约定变成类型系统的一部分。**

### 3.2 Mutability XOR Sharing

同一时刻，对同一数据只能满足其一：

- 任意多个只读借用 `&T`；
- 有且仅有一个可变借用 `&mut T`。

这就是 Rust 没有数据竞争的根源，也是很多“C++ 能编译、Rust 不让写”的根源。
遇到借用检查器报错时，先问：**谁拥有数据？谁只需要看？谁需要改？**

---

## 4. 工具链对照

| C++ 生态 | Rust 生态 | 说明 |
| --- | --- | --- |
| `g++` / `clang++` / MSVC | `rustc` | 单一编译器前端，无 ABI 分裂 |
| 多版本编译器 | `rustup toolchain` | 工具链可并存、按项目切换 |
| CMake / Meson | **Cargo** | 构建、依赖、测试、文档、发布一体 |
| `CMakeLists.txt` | `Cargo.toml` | 声明式 manifest |
| `vcpkg.json` / `conanfile` | `Cargo.toml [dependencies]` | 版本约束写在 manifest |
| `vcpkg` / Conan / FetchContent | crates.io | 源码分发，统一构建 |
| `build/` | `target/` | 构建产物（通常不提交） |
| `compile_commands.json` | 通常不需要 | rust-analyzer 直接分析 Cargo 工程 |
| `-O2 -g` / build type | `[profile.release]` / `--release` | 配置化 |
| `-D` / CMake options | Cargo **features** | 条件编译 |
| clang-format | `cargo fmt`（rustfmt） | 官方风格统一 |
| clang-tidy | `cargo clippy` | 官方 lint 集合 |
| ASan / UBSan | safe Rust + **Miri** | 安全代码无需插桩 |
| GoogleTest / Catch2 | `cargo test` 内置 | 单元 + 集成 + 文档测试 |
| Doxygen | `cargo doc`（rustdoc） | Markdown 文档注释 |
| Google Benchmark | `cargo bench` / criterion | 需额外 crate |
| clangd | rust-analyzer | LSP |
| gdb / lldb | `rust-gdb` / `rust-lldb` | 支持 pretty printer |

`Cargo.toml` 与 `CMakeLists.txt` 的对应关系：

```toml
[package]
name = "my_app"          # 目标/工程名
version = "0.1.0"
edition = "2024"

[dependencies]
serde = { version = "1", features = ["derive"] }  # 类似 find_package + target_link_libraries

[profile.release]
opt-level = 3
lto = true                # 类似 -flto
```

---

## 5. 常见编译错误速查

| Rust 错误 | 典型原因 | C++ 中的对应 bug |
| --- | --- | --- |
| `E0382: borrow of moved value` | move 后继续使用 | `std::move` 后使用源对象 |
| `E0505: cannot move out ... borrowed` | 有借用时移动所有权 | 删除仍被引用的对象 |
| `E0499: cannot borrow as mutable more than once` | 两个 `&mut` 重叠 | 两个指针写同一对象 |
| `E0502: cannot borrow as mutable because also borrowed as immutable` | 读写别名 | 迭代器失效 / 数据竞争 |
| `E0515 / E0597: does not live long enough` | 返回局部变量引用 | 悬垂指针 / 返回局部引用 |
| `E0106: missing lifetime specifier` | 返回引用来源不明确 | 接口未说明所有权 |
| `E0277: trait bound not satisfied` | 类型缺少能力 | concept 不满足 |
| `E0308: mismatched types` | 类型不匹配 | 隐式转换失败 |
| `E0507: cannot move out of borrowed content` | 从 `&T` 中搬走字段 | 拷贝/移动语义错误 |
| `E0716: temporary value dropped while borrowed` | 临时值生命周期太短 | 绑定到临时对象的引用 |
| `E0596: cannot borrow as mutable` | 变量未加 `mut` | 缺少非 const 访问 |

> 小技巧：Rust 编译器经常直接给出修复建议。先读错误最后一段 `help:`。

---

## 6. 目录结构

```text
learn_rust/
├── README.md                  # 本文件
├── docs/                      # 分章讲解（建议按编号阅读）
│   ├── 00-cpp-to-rust.md
│   ├── 01-toolchain-cargo.md
│   ├── ...
│   └── 18-web-server.md
├── guessing_game/             # 独立 Cargo package（下同）
├── learn_var/
├── func/
├── branches/
├── ownership/
├── structs_enums/
├── modules/
├── collections/
├── panic/
├── error/
├── generics/
├── traits/
├── lifetime/
├── testing/
├── minigrep/
├── iterators/
├── smart_pointers/
├── concurrency/
├── patterns_oop/
├── unsafe_ffi/
└── web_server/
```

每个 crate 的约定：

- `src/main.rs`：本章主线示例，`cargo run` 直接运行；
- `examples/*.rs`：可选的深入示例，`cargo run --example <name>`；
- `src/lib.rs`：需要对外暴露 API 的章节（`modules`、`testing`、`minigrep`）；
- `tests/`：集成测试（`testing`、`minigrep`）。

---

## 7. 文档写作约定

- 代码中的注释解释“为什么”，而不是逐行翻译语法；
- `C++ 对比` 段落只比较语义，不假设读者使用某个特定库；
- 所有示例都能在 `cargo run` / `cargo test` 下复现；
- 为了突出主题，部分示例省略了生产环境需要的日志、错误上下文和性能优化。

---

## 8. 参考

- The Rust Programming Language: <https://doc.rust-lang.org/book/>
- Rust by Example: <https://doc.rust-lang.org/rust-by-example/>
- Microsoft Rust for C/C++ developers: <https://microsoft.github.io/RustTraining/c-cpp-book/>
- Rust API Guidelines: <https://rust-lang.github.io/api-guidelines/>
- The C++ Core Guidelines: <https://isocpp.github.io/CppCoreGuidelines/>

祝你学习顺利。遇到借用检查器时别急着和它对抗——它通常在提示你：**数据的所有权设计需要再想一步。**
