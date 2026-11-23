# 00 · 给 C++23 开发者的 Rust 心智模型

> 对应 The Book 第 1、4、6、10、13、15、17 章的综合导论
> 示例：本仓库全部 crate

本文不追求语法完整，而是先建立一套可以与现有 C++ 经验对接的心智模型。
后续章节会逐一展开细节。

---

## 1. 编译模型：从 translation unit 到 crate

| C++ | Rust |
| --- | --- |
| 源文件 + 头文件，`#include` 文本展开 | 一个 **crate** 是编译单元，`mod` 是逻辑模块，无文本包含 |
| 声明与定义分离（`.h` / `.cpp`） | 类型、函数、模块只写一次；rustc 自己解析整个 crate |
| 链接器拼接目标文件 | `rustc` 生成 crate metadata + 目标文件，Cargo 驱动链接 |
| ODR、inline、模板实例化 | 单态化由编译器统一管理，无 ODR 问题 |
| 静态库 / 动态库 / 可执行文件 | `lib` crate / `bin` crate（同一 package 可同时有） |

C++ 需要头文件是因为编译器一次只能看到一个 translation unit；
Rust 的 crate 是完整语义单元，因此不需要重复声明，也不会出现“头文件与实现不一致”。

```rust
// src/lib.rs —— 库 crate 的根
pub mod geometry;          // 对应 src/geometry.rs 或 src/geometry/mod.rs
pub use geometry::Point;   // re-export
```

对应 C++：

```cpp
// geometry.hpp
#pragma once
namespace geometry { struct Point { double x, y; }; }
```

但 Rust 的 `mod` 不只是命名空间：它还决定**可见性边界**和**文件组织方式**。

---

## 2. 所有权：RAII 的加强版

### 2.1 `unique_ptr` 的语义，变成语言默认

C++23：

```cpp
auto p = std::make_unique<Buffer>(1024);
auto q = std::move(p);       // p 仍存在，但处于 valid-but-unspecified
// 仍可调用 p->size()，结果由实现定义
```

Rust：

```rust
let p = Box::new(Buffer::new(1024));
let q = p;                   // 所有权 move
// println!("{}", p.size()); // 编译错误：borrow of moved value
```

差异：

- C++ 的 move 是“资源搬走 + 源对象保持有效”的约定；Rust 的 move 是**编译器禁止再访问源对象**；
- Rust 没有自定义 move constructor；move 本质上是按位复制 + 源对象失效；
- 需要“复制一份”时必须显式 `.clone()`，这类似显式深拷贝，代价可见；
- 实现了 `Drop` 的类型离开作用域自动析构（RAII），但 `Drop::drop` 不能手动调用。

```rust
struct Buffer(Vec<u8>);

impl Drop for Buffer {
    fn drop(&mut self) {
        println!("release {} bytes", self.0.len()); // 类似 ~Buffer()
    }
}
```

### 2.2 借用 = 编译器验证过的指针

| C++ | Rust | 保证 |
| --- | --- | --- |
| `const T*` | `&T` | 非空、有效、只读共享 |
| `T*` | `&mut T` | 非空、有效、独占写入 |
| `T&` | `&mut T` | 无空引用问题 |
| `std::span<const T>` | `&[T]` | 长度 + 边界 |
| `std::string_view` | `&str` | UTF-8 边界 |

核心规则（**aliasing XOR mutability**）：

> 任意时刻，对同一对象要么有多个 `&T`，要么有且仅有一个 `&mut T`。

C++ 里下面这段代码能编译，但行为未定义：

```cpp
std::vector<int> v{1, 2, 3};
int& first = v[0];
v.push_back(4);       // 可能重新分配
std::cout << first;   // 悬垂引用，UB
```

Rust 在编译期拒绝：

```rust
let mut v = vec![1, 2, 3];
let first = &v[0];
v.push_back(4);       // error[E0502]: cannot borrow `v` as mutable ...
println!("{first}");
```

这不是 Rust “更严格所以更麻烦”，而是把 C++ 运行时的 UB 变成了编译错误。
写 Rust 时，应把“借用期间不能修改容器”当成设计约束，而不是障碍。

### 2.3 生命周期标注：接口契约，不是运行时机制

```rust
// 返回的引用来自 x 或 y，因此与两者中较短的生命周期绑定
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() >= y.len() { x } else { y }
}
```

`'a` 不会延长任何对象的寿命，也不会生成运行时代码。
它只是告诉编译器：“返回值的有效性不超过 `'a`”。
类比：给函数签名加 `[[clang::lifetimebound]]`，但 Rust 会强制检查所有调用点。

---

## 3. 错误处理：异常 vs `Result`

| C++23 | Rust |
| --- | --- |
| `throw` / `try` / `catch` | `panic!`（不可恢复）或 `Result<T, E>`（可恢复） |
| `std::expected<T, E>` | `Result<T, E>` |
| `noexcept` | 默认没有异常；panic 可 unwind 或 abort |
| 异常类型擦除 | `Box<dyn Error>` 可类型擦除 |
| 构造函数抛异常 | 构造函数不能返回 `Result`，用 `new` 关联函数 |

Rust 把错误分成两类：

1. **程序 bug / 不变量被破坏** → `panic!`，例如数组越界、`unwrap()` 到 `None`；
2. **可预期的失败** → `Result<T, E>`，例如文件不存在、解析失败。

```rust
use std::fs;
use std::io;

fn read_config(path: &str) -> Result<String, io::Error> {
    let content = fs::read_to_string(path)?; // ? 等价于“失败则提前 return Err”
    Ok(content)
}
```

`?` 是零成本展开，类似：

```cpp
std::expected<std::string, std::error_code> content = read_file(path);
if (!content) return std::unexpected(content.error());
return content;
```

`panic` 更像是“立即终止 + 栈展开”，而不是业务异常。跨 FFI 边界 panic 是未定义行为，
所以 C ABI 函数内部必须 `catch_unwind` 或配置 `panic = "abort"`。

---

## 4. 多态：模板、concept、虚函数

### 4.1 泛型 ≈ 模板，但契约写在定义处

C++：

```cpp
template <typename T>
concept Printable = requires(const T& x) { std::cout << x; };

template <Printable T>
void print_all(const std::vector<T>& items) {
    for (const auto& item : items) std::cout << item << '\n';
}
```

Rust：

```rust
use std::fmt::Display;

fn print_all<T: Display>(items: &[T]) {
    for item in items {
        println!("{item}");
    }
}
```

- 两者都做**单态化**：为每个具体类型生成一份代码；
- Rust 在定义泛型函数时就检查 `T` 是否满足 `Display`；C++ 模板历史上要到实例化才检查；
- Rust 的 trait 可以为已有类型实现（只要类型或 trait 至少一个在本地 crate），类似 concept 的“外部可扩展性”；
- 一个类型可以实现多个 trait，没有菱形继承问题。

### 4.2 `dyn Trait` ≈ 虚函数

```rust
trait Draw {
    fn draw(&self);
}

struct Screen {
    items: Vec<Box<dyn Draw>>, // ≈ vector<unique_ptr<Drawable>>
}
```

| 场景 | Rust 写法 | C++ 写法 |
| --- | --- | --- |
| 编译期已知类型 | `T: Draw` / `impl Draw` | 模板 / concept |
| 运行时异构集合 | `Box<dyn Draw>` | `unique_ptr<Base>` |
| 借用异构对象 | `&dyn Draw` | `const Base&` |

Rust 没有继承字段。代码复用靠**组合 + trait 默认方法**；
需要“基类数据”时，通常把公共数据放进结构体，行为放进 trait。

---

## 5. 枚举是 ADT，不是整数别名

C++23：

```cpp
using Value = std::variant<int, std::string>;

void print(const Value& v) {
    std::visit([](const auto& x) { std::cout << x; }, v);
}
```

Rust：

```rust
enum Value {
    Int(i32),
    Text(String),
}

fn print(v: &Value) {
    match v {
        Value::Int(n) => println!("{n}"),
        Value::Text(s) => println!("{s}"),
    }
}
```

Rust 的 `enum` 是**带标签的联合体**：每个 variant 可以有自己的字段，`match` 强制穷尽。
`Option<T>` 和 `Result<T, E>` 只是标准库里的两个普通 enum。

内存布局上，`Option<Box<T>>`、`Option<&T>` 等还享受 **niche optimization**：用空指针表示 `None`，
大小与裸指针相同。这比 `std::optional<unique_ptr<T>>` 更紧凑。

---

## 6. 资源管理与智能指针对照

| 需求 | Rust | C++ |
| --- | --- | --- |
| 独占堆对象 | `Box<T>` | `unique_ptr<T>` |
| 多个所有者（单线程） | `Rc<T>` | `shared_ptr<T>`（但 C++ 是线程安全的计数） |
| 多个所有者（多线程） | `Arc<T>` | `shared_ptr<T>` |
| 弱引用 | `Weak<T>` | `weak_ptr<T>` |
| 内部可变性（单线程） | `RefCell<T>` / `Cell<T>` | `mutable` 成员 |
| 共享可变（单线程） | `Rc<RefCell<T>>` | `shared_ptr<T>` + 无锁约定（易错） |
| 共享可变（多线程） | `Arc<Mutex<T>>` | `shared_ptr<T>` + `mutex` |
| 可选值 | `Option<T>` | `optional<T>` |
| 延迟初始化 | `OnceLock<T>` / `OnceCell<T>` | `std::call_once` / `once_flag` |

关键差异：

- `Rc<T>` 只提供不可变访问；要修改必须 `RefCell`，运行时检查借用规则；
- `RefCell` 违反规则会 **panic**（不是 UB），但会破坏“安全代码不 panic 除外的内存安全”；
- `Arc<Mutex<T>>` 把**锁和数据绑定**，不可能忘记加锁；
- 循环引用仍需 `Weak` 打破，Rust 不自动做 GC。

---

## 7. 宏：不是预处理器

Rust 有两类宏：

- `macro_rules!`：基于语法的卫生宏，类似“安全的模板 + 模式匹配”；
- 过程宏（derive / attribute / function-like）：在编译期生成代码，需要单独的 proc-macro crate。

```rust
println!("x = {}, y = {}", x, y); // 编译期检查格式串
vec![1, 2, 3];                    // 宏，不是函数
#[derive(Debug, Clone)]           // derive 宏，自动生成 impl
struct Point { x: i32, y: i32 }
```

与 C++ 预处理器相比：

- 没有文本替换，没有头文件顺序问题；
- 宏展开遵循作用域和卫生规则；
- `#[derive(...)]` 类似手写 boilerplate 的自动化版本（但只对支持的 trait 生效）。

---

## 8. 从 C++ 迁移时的十个思维转换

1. **默认按值传递 + move**：不要条件反射地传指针；Rust 的 move 很便宜。
2. **函数参数优先 `&str` / `&[T]`**：类比 `string_view` / `span`，调用者保留所有权。
3. **返回引用前先问“它来自谁”**：无法从输入推导生命周期时，返回拥有所有权的值。
4. **不要用共享可变状态建模**：优先“谁拥有、谁修改”，必要时用消息传递。
5. **用 enum 表达状态**：`enum State { Idle, Running { .. }, Done }` 比一堆 bool 更安全。
6. **用 `Result` 而不是错误码 + 全局 errno**：错误必须被处理或显式忽略。
7. **优先组合而不是继承**：trait 默认方法 + 结构体字段。
8. **`clone()` 不是原罪，但要可见**：热路径上避免无意识复制。
9. **`unsafe` 是边界，不是模式**：把 unsafe 封装成 safe API，并写明 Safety 契约。
10. **先让编译器通过，再优化**：借用检查器经常帮你发现真实的数据竞争/悬垂问题。

---

## 9. 什么时候仍然需要 C++ / unsafe

Rust 不是“所有场景都替代 C++”：

- **FFI 边界**：调用现有 C/C++ 库（`bindgen`、`cxx`、`cbindgen`）；
- **极致性能/特定硬件**：SIMD intrinsics（部分仍在 nightly）、GPU、裸机；
- **自定义分配器 / 内核 / 驱动**：`no_std` + `unsafe`；
- **C++ 生态中的成熟库**：先包装，再逐步迁移。

Rust 的 `unsafe` 只解锁五类操作：解引用裸指针、调用 unsafe 函数、访问/修改可变 `static`、
实现 unsafe trait、访问 `union` 字段。它**不会**关闭借用检查，也不会让你随意制造数据竞争。

---

## 10. 常见误解

| 误解 | 事实 |
| --- | --- |
| “Rust 有 GC” | 没有；所有权 + RAII 在编译期决定释放点 |
| “Rust 一定比 C++ 慢” | 零成本抽象；边界检查通常可被优化；需实测 |
| “借用检查器让复杂数据结构写不了” | 需要换建模方式：arena、索引、`Rc<RefCell>`、`unsafe` 封装 |
| “`unsafe` 就是关闭检查” | 只解锁特定操作；安全性责任转移给程序员 |
| “`clone()` 随便用” | 正确但可能昂贵；热点路径要测量 |
| “trait 就是接口” | 还能做泛型约束、运算符重载、扩展方法、静态派发 |
| “Rust 没有异常所以不能处理错误” | `Result` + `?` 是更显式的错误处理 |
| “编译太慢” | 单态化和优化会增加时间；增量编译、`cargo check` 可缓解 |

---

## 延伸阅读

- The Book 第 4 章：Ownership — <https://doc.rust-lang.org/book/ch04-00-understanding-ownership.html>
- The Book 第 10 章：Generics, Traits, Lifetimes — <https://doc.rust-lang.org/book/ch10-00-generics.html>
- Rust for C/C++ developers — <https://microsoft.github.io/RustTraining/c-cpp-book/>
- Rustonomicon（unsafe 与底层语义）— <https://doc.rust-lang.org/nomicon/>

下一章：[01 · 工具链与 Cargo](01-toolchain-cargo.md)
