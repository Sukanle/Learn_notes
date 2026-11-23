# 07 · Package、Crate 与 Module

> 对应 The Book 第 7 章
> 示例 crate：[`modules`](../modules)
> 运行：`cd modules && cargo run`，以及 `cargo run --example visibility`

C++ 用 **namespace + 头文件 + 构建 target** 组织代码；Rust 用 **package → crate → module** 三层。
理解这三层，才能读懂别人的 crate 布局，也才能设计清晰的 public API。

---

## 1. 三个层级

| 概念 | 定义 | C++ 类比 |
| --- | --- | --- |
| **Package** | 一个 `Cargo.toml` 管理的工程，可包含一个 lib 和多个 bin | 一个 CMake 顶层工程 / solution |
| **Crate** | 一次编译的单元；lib crate 或 bin crate | 一个静态库 / 可执行 target |
| **Module** | crate 内的命名空间与可见性边界 | `namespace` + `internal` 链接性 |

一个 package 的默认布局：

```text
modules/
├── Cargo.toml
└── src/
    ├── lib.rs          # 库 crate 根（package 名 = crate 名）
    ├── main.rs         # 二进制 crate 根（默认 bin）
    ├── front_of_house.rs
    ├── front_of_house/
    │   ├── hosting.rs
    │   └── serving.rs
    └── back_of_house.rs
```

`cargo build` 会产出：

- 库：`libmodules.rlib`（名字来自 package）；
- 二进制：`modules`（`src/main.rs`），它可以把同名库 crate 当作依赖使用。

> 同一个 package 里，`src/main.rs` 和 `src/lib.rs` 是**两个不同的 crate**。
> 二进制通过 `use modules::...` 引用库，而不是 `mod` 进来（否则会重复编译）。

多个二进制：

```text
src/bin/
├── client.rs      # cargo run --bin client
└── server.rs      # cargo run --bin server
```

---

## 2. Module 树

`src/lib.rs`：

```rust
pub mod front_of_house;
pub mod back_of_house;

pub use crate::front_of_house::hosting;   // re-export
```

`src/front_of_house.rs`：

```rust
pub mod hosting;
pub mod serving;
```

`src/front_of_house/hosting.rs`：

```rust
pub fn add_to_waitlist() { ... }
pub(crate) fn seat_at_table() { ... }
```

模块树：

```text
crate
├── front_of_house
│   ├── hosting
│   │   ├── add_to_waitlist   (pub)
│   │   └── seat_at_table     (pub(crate))
│   └── serving
│       ├── take_order
│       ├── serve_order
│       └── take_payment
└── back_of_house
    ├── Breakfast
    ├── Appetizer
    └── fix_incorrect_order
```

### 2.1 文件与模块的对应规则

Rust 2018+ 的规则：

- `mod foo;` 在 crate 根 → 找 `src/foo.rs` 或 `src/foo/mod.rs`；
- `mod foo;` 在 `src/bar.rs` 内 → 找 `src/bar/foo.rs` 或 `src/bar/foo/mod.rs`；
- **不要**同时创建 `foo.rs` 和 `foo/mod.rs`（会报错）。

本仓库使用“文件名 + 同名目录”风格（`front_of_house.rs` + `front_of_house/`），
这是 2018 之后推荐的做法；`mod.rs` 风格仍然可用，但目录里会多一个 `mod.rs`。

---

## 3. 路径与 `use`

```rust
// 绝对路径：从当前 crate 根开始
crate::front_of_house::hosting::add_to_waitlist();

// 相对路径：从当前模块开始（2018 起需要 self:: 显式表达）
self::hosting::add_to_waitlist();
super::hosting::add_to_waitlist();      // 父模块
```

| 路径前缀 | 含义 | C++ 类比 |
| --- | --- | --- |
| `crate::` | 当前 crate 根 | `::project::` |
| `self::` | 当前模块 | 当前 namespace |
| `super::` | 父模块 | `::` 外层 namespace |
| `::std::...` | 外部 crate 的根 | 全局命名空间 |

`use` 把名字引入当前作用域：

```rust
use std::collections::HashMap;
use std::fmt::{self, Display};          // 同时导入 fmt 模块和 Display
use crate::front_of_house::hosting as front_desk;  // 别名

let mut map: HashMap<String, i32> = HashMap::new();
front_desk::add_to_waitlist();
```

`use` 不是 `#include`：

- 不复制代码，只建立名字绑定；
- 作用域严格（默认只在当前模块生效）；
- 不会造成编译时间随包含顺序爆炸；
- 同一路径可以重复 `use`，编译器会去重。

### 3.1 Re-export

```rust
pub use crate::front_of_house::hosting;
```

这把内部路径提升为对外 API，调用者可以写：

```rust
use modules::hosting::add_to_waitlist;
```

而不是暴露完整的内部层级 `modules::front_of_house::hosting`。
好处是重构内部目录时不影响外部用户。

类似 C++ 在公共头文件里 `using internal::Foo;`，但 Rust 的可见性检查更严格。

---

## 4. 可见性

Rust 的默认规则：**私有**（当前模块及其子模块可见）。需要显式放宽：

| 写法 | 可见范围 | C++ 类比 |
| --- | --- | --- |
| 无修饰 | 当前模块 + 子模块 | 内部链接 / private |
| `pub` | 任何地方 | public |
| `pub(crate)` | 当前 crate 内 | internal / hidden visibility |
| `pub(super)` | 父模块内 | protected 的近似 |
| `pub(in path)` | 指定祖先模块内 | 受控可见性 |

```rust
pub struct Breakfast {
    pub toast: String,
    seasonal_fruit: String,   // 私有字段
}

pub enum Appetizer {
    Soup,                     // enum variant 随 enum 一起公开
    Salad,
}
```

- struct 的字段需要逐个标记 `pub`；
- enum 的 variant 只要 enum 是 `pub` 就全部可见（因为 variant 是类型的一部分）；
- 私有字段可以通过 `pub fn` 暴露只读访问或受控修改。

> 设计 API 时，默认私有，按需 `pub`。Rust 没有“friend class”，
> 如果需要模块间共享内部实现，使用 `pub(crate)`。

---

## 5. `modules` crate 走读

### 5.1 库根

```rust
pub mod back_of_house;
pub mod front_of_house;
pub use crate::front_of_house::hosting;

pub fn eat_at_restaurant() {
    crate::front_of_house::hosting::add_to_waitlist();
    front_of_house::hosting::add_to_waitlist();
    hosting::add_to_waitlist();

    let mut meal = back_of_house::Breakfast::summer("Rye");
    meal.toast = String::from("Wheat");
    // meal.seasonal_fruit = ...; // error: private field

    let appetizer = back_of_house::Appetizer::Soup;
    match appetizer { ... }
}
```

三种路径写法都能到达同一个函数，用来演示绝对路径、相对路径和 `use` 之后的短名。

### 5.2 兄弟模块

`serving.rs` 需要访问 `hosting::seat_at_table`：

```rust
pub fn take_order() {
    super::hosting::seat_at_table();
}
```

`super` 指向 `front_of_house`，然后进入兄弟模块 `hosting`。
注意 `seat_at_table` 是 `pub(crate)`，在库内部可见，但二进制 crate 不能直接调用
（二进制是另一个 crate）。

### 5.3 二进制入口

```rust
use modules::front_of_house::hosting as front_desk;

fn main() {
    modules::eat_at_restaurant();
    modules::hosting::add_to_waitlist();  // re-export
    front_desk::add_to_waitlist();
}
```

`modules::` 是 package 名，也是库 crate 名。二进制 crate 通过 Cargo 自动链接同名库。

---

## 6. C++ 对照

| C++ | Rust |
| --- | --- |
| `namespace foo { ... }` | `mod foo { ... }` 或文件 |
| `#include "foo.h"` | `mod foo;` / `use foo::Bar;` |
| 头文件声明 + `.cpp` 定义 | 单处定义，无声明/定义分离 |
| `static` / 匿名 namespace（内部链接） | 默认私有 / `pub(crate)` |
| `using namespace foo;` | `use foo::*;`（不推荐） |
| `using foo::Bar;` | `use foo::Bar;` |
| `namespace alias = foo;` | `use foo as alias;` |
| 前置声明 | 不需要 |
| 循环 include | 模块依赖必须无环（crate 依赖也无环） |
| CMake target / 静态库 | crate / package |
| `target_include_directories` | 模块路径 + visibility |

Rust 的 crate 依赖图**必须是无环的**（DAG）。C++ 头文件可以循环包含（靠 include guard 硬撑），
Rust 不允许循环依赖；需要“互相引用”时，把共享类型抽到第三个 crate。

---

## 7. 文档与测试的模块组织

```rust
/// 这个函数会打印一条问候。
///
/// # Examples
/// ```
/// let x = 1;
/// ```
pub fn hello() {}
```

- `cargo doc` 会为 public 项生成文档；
- 文档注释中的代码块会成为 **doc test**，`cargo test` 会编译并运行；
- 单元测试放在同文件的 `#[cfg(test)] mod tests`，可以访问私有项；
- 集成测试放在 `tests/`，只能使用 public API（第 11 章）。

私有的单元测试模块不会进入最终库，也不影响 API。

---

## 8. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `unresolved import` | 路径写错 / 缺 `pub` | 检查模块树与可见性 |
| `module is private` | 父模块未 `pub` | 逐级 `pub` 或 re-export |
| `field is private` | 字段未 `pub` | 提供方法或 `pub` |
| `function is private` | 函数未 `pub` | 同上 |
| `file not found for module` | 文件位置不符合规则 | `src/foo.rs` 或 `src/foo/mod.rs` |
| `ambiguous name` | 两个 `use` 引入同名 | `as` 别名或限定路径 |
| `circular module` | 模块/ crate 循环 | 抽出共享模块 |

---

## 9. 练习

1. 在 `modules` 中新增 `pub mod kitchen`，提供 `pub fn cook()` 和 `fn secret_recipe()`，
   验证外部只能调用 `cook`。
2. 把 `front_of_house` 重命名为 `dining`，只修改库内部实现，保证 `main.rs` 中通过
   re-export 的调用不需要改动。
3. 新增 `src/bin/report.rs`，使用库 API 打印当天订单数。
4. 给 `Breakfast` 增加 `pub fn toast(&self) -> &str`，让二进制 crate 能读取 toast 字段（不直接暴露字段）。
5. 解释为什么 `src/main.rs` 不能写 `mod front_of_house;` 来复用库里的模块。

---

## 延伸阅读

- The Book 第 7 章 — <https://doc.rust-lang.org/book/ch07-00-managing-growing-projects-with-packages-crates-and-modules.html>
- Rust Reference：Visibility — <https://doc.rust-lang.org/reference/visibility-and-privacy.html>
- Cargo Book：Package Layout — <https://doc.rust-lang.org/cargo/guide/project-layout.html>

上一章：[06 · Struct、Enum 与模式匹配](06-structs-enums.md) ·
下一章：[08 · 集合](08-collections.md)
