# 16 · OOP、Trait Object 与模式匹配

> 对应 The Book 第 17–18 章
> 示例 crate：[`patterns_oop`](../patterns_oop)
> 运行：`cargo run`、`cargo run --example state_pattern`、`cargo run --example match_patterns`

Rust 没有 `class` 继承、没有虚函数关键字、没有 `std::visit` 的 visitor 重载集。
它用 **struct + trait + enum + match** 表达 OOP 的意图，而且通常更安全。

---

## 1. Rust 的“面向对象”

The Book 给出的定义：OOP 语言通常支持 **对象、封装、继承、多态**。

| 特性 | C++ | Rust |
| --- | --- | --- |
| 对象（数据 + 行为） | class | `struct` + `impl` |
| 封装 | `private` / `protected` | 模块可见性 + 私有字段 |
| 继承（代码复用） | class inheritance | **无**，用组合 + trait 默认方法 |
| 继承（子类型多态） | virtual + override | `dyn Trait` + vtable |
| 重载 | overload | 泛型 + trait |
| 接口 | 抽象基类 / concept | trait |
| 构造函数 | constructor | 关联函数 `new` |
| 析构函数 | destructor | `Drop` |

Rust 选择**组合优于继承**，因为继承会带来：

- 脆弱的基类问题（fragile base class）；
- 菱形继承；
- 对象切片；
- 基类与派生类强耦合。

---

## 2. Trait Object 与动态派发

```rust
trait Draw {
    fn draw(&self);
}

struct Screen {
    components: Vec<Box<dyn Draw>>,
}

impl Screen {
    fn run(&self) {
        for component in &self.components {
            component.draw();
        }
    }
}
```

- `Box<dyn Draw>` 类似 `std::unique_ptr<Drawable>`；
- vtable 在运行时查找 `draw`；
- `Screen` 不知道也不关心具体类型；
- 新增组件类型不需要修改 `Screen`（开闭原则）。

C++ 对照：

```cpp
struct Drawable {
    virtual ~Drawable() = default;
    virtual void draw() const = 0;
};

struct Screen {
    std::vector<std::unique_ptr<Drawable>> components;
    void run() const {
        for (const auto& c : components) c->draw();
    }
};
```

| 维度 | C++ 虚函数 | Rust `dyn Trait` |
| --- | --- | --- |
| vtable 位置 | 对象内（vptr） | 胖指针内（数据 + vtable） |
| 对象大小 | 多一个 vptr | 普通指针 + vtable 指针 |
| 析构 | 虚析构函数必须写 | `Drop` 自动正确 |
| 对象切片 | 可能发生 | 默认不会（`dyn` 是 unsized） |
| 向下转型 | `dynamic_cast` | `Any` + `downcast_ref` |

> Rust 的 `Box<dyn Trait>` 不需要手动写虚析构：`drop` 会通过 vtable 调用正确的析构函数。

### 2.1 静态 vs 动态派发

```rust
// 静态：每个 T 生成一份代码
fn notify_static<T: Summary>(item: &T) { ... }

// 动态：一份代码，vtable 调用
fn notify_dynamic(item: &dyn Summary) { ... }

// 异构集合只能用 dyn
let items: Vec<&dyn Summary> = vec![&article, &tweet];
```

选择建议：

- 热路径、类型在编译期已知 → 泛型 / `impl Trait`；
- 异构集合、插件式架构、二进制体积敏感 → `dyn Trait`；
- 不要为了“看起来像 OOP”而无脑用 `dyn`。

### 2.2 对象安全（Object Safety）

不是所有 trait 都能做 `dyn Trait`：

```rust
trait NotObjectSafe {
    fn clone_me(&self) -> Self;       // 返回 Self -> 不 object-safe
    fn generic<T>(&self, x: T);       // 泛型方法 -> 不 object-safe
}
```

规则（简化）：

- 方法不能返回 `Self`（除非 `where Self: Sized`）；
- 不能有泛型类型参数；
- 不能使用 `Self` 作为参数/返回值（除接收者）；
- 关联常量需要 `where Self: Sized`。

`Clone` 不是 object-safe，因此没有 `Box<dyn Clone>`；需要克隆 trait object 时，
通常手动加 `fn clone_box(&self) -> Box<dyn Trait>`。

### 2.3 默认方法

```rust
trait Summary {
    fn author(&self) -> String;

    fn summarize(&self) -> String {
        format!("(Read more from {}...)", self.author())
    }
}
```

- 默认方法类似基类的虚函数默认实现；
- 实现者可以覆盖；
- 默认方法可以调用其他必需方法；
- trait 不能有数据字段（没有继承状态）。

---

## 3. State Pattern：trait object 版本

The Book 用博客文章的状态机演示 OOP 风格：

```text
Draft --request_review--> PendingReview --approve x2--> Published
```

```rust
trait State {
    fn request_review(self: Box<Self>) -> Box<dyn State>;
    fn approve(self: Box<Self>) -> Box<dyn State>;
    fn content<'a>(&self, _post: &'a Post) -> &'a str { "" }
}
```

关键设计：

1. **`self: Box<Self>`**：状态转移需要消耗当前状态对象（取得所有权）；
2. **返回 `Box<dyn State>`**：转移到下一个状态；
3. **`Post` 持有 `Option<Box<dyn State>>`**：便于 `take()` 把状态移出，
   调用完再放回，避免“在 &mut self 中再次可变借用 self”；
4. **`content` 默认返回空**，只有 `Published` 覆盖。

```rust
pub fn request_review(&mut self) {
    if let Some(state) = self.state.take() {
        self.state = Some(state.request_review());
    }
}
```

如果直接写 `self.state.as_mut().unwrap().request_review(self)`，
借用检查器会拒绝：状态对象已经借用了 `self.state`，方法又需要 `&mut self`。
`take()` 是 Rust 中处理“状态机 + 所有权”的常用技巧。

### 3.1 enum 版本

很多情况下，enum 比 trait object 更合适：

```rust
enum State {
    Draft,
    PendingReview { approvals: u8 },
    Published,
}

struct Post {
    state: State,
    content: String,
}

impl Post {
    fn request_review(&mut self) {
        self.state = match &self.state {
            State::Draft => State::PendingReview { approvals: 0 },
            State::PendingReview { approvals } => State::PendingReview {
                approvals: *approvals,
            },
            State::Published => State::Published,
        };
    }
}
```

| 方案 | 优点 | 缺点 |
| --- | --- | --- |
| trait object | 易扩展新状态（不改 Post） | 动态派发、堆分配、借用复杂 |
| enum | 无堆分配、状态穷尽、易测试 | 新增状态要改 match |
| 状态 + 数据分离 | 清晰 | 需要管理状态数据 |

The Book 的结论：**状态用 enum 往往更 Rust**；trait object 适合“插件/策略”这类
编译期未知、可外部扩展的场景。

---

## 4. 模式匹配（Patterns）

模式可以出现在：

- `match` 分支；
- `if let` / `while let`；
- `let` 绑定；
- 函数参数、闭包参数（解构）；
- `for` 循环（`for (i, x) in ...`）。

### 4.1 字面量、范围、多重模式

```rust
match x {
    0 => ...,
    1 | 2 => ...,
    3..=9 => ...,
    n @ 10..=19 => println!("teen {n}"),
    _ => ...,
}
```

- `|`：或模式；
- `..=`：闭区间；
- `@`：绑定同时匹配；
- `_`：忽略值（不绑定）。

### 4.2 解构 struct / enum / tuple

```rust
struct Point { x: i32, y: i32 }

let p = Point { x: 0, y: 7 };

match p {
    Point { x: 0, y } => ...,
    Point { x, y: 0 } => ...,
    Point { x, y } => ...,
}

let ((a, b), Point { x, .. }) = ((1, 2), Point { x: 3, y: 4 });
```

- `..` 忽略剩余字段（struct / tuple）；
- 嵌套解构可以很深；
- 绑定默认按引用/可变引用（match ergonomics），2024 edition 下规则更严格。

### 4.3 守卫（guard）

```rust
match n {
    n if n > 0 => ...,
    n if n % 2 == 0 => ...,
    _ => ...,
}
```

守卫是任意 `bool` 表达式，可以访问模式绑定的变量。
注意：守卫不会改变穷尽性检查，编译器仍要求覆盖所有情况。

### 4.4 `matches!` 宏

```rust
let value = Some(7);
assert!(matches!(value, Some(1..=9)));
```

适合只关心“是否匹配”的场景，比写完整 `match` 简洁。

### 4.5 `let-else`

```rust
let Some(user) = users.get(id) else {
    return Err("not found");
};
```

`let-else` 要求模式是**可反驳的**（refutable），否则编译器会警告/报错。
它比 `if let` 更适合“必须有值，否则提前返回”。

### 4.6 `if let` 链（Rust 2024）

```rust
if let Some((level, mode)) = config
    && level > 1
{
    println!("{level} {mode}");
}
```

把多个条件串联，避免嵌套 `if let`。需要 Rust 1.88+ / edition 2024。

### 4.7 可反驳 vs 不可反驳

- 不可反驳（irrefutable）：`let x = 5;`、`let (a, b) = pair;`——总能匹配；
- 可反驳（refutable）：`Some(x)`、`1..=9`——可能失败。

`let` 只接受不可反驳模式（否则用 `let-else` / `if let` / `match`）。
`match` 的分支可以是可反驳的，但整体必须穷尽。

---

## 5. 常用 OOP 模式在 Rust 中的写法

### 5.1 Newtype

```rust
struct Meters(f64);
struct UserId(u64);
```

- 强类型，避免单位/ID 混用；
- 零成本（`repr(transparent)` 可保证 ABI）；
- 可以为外部类型实现本地 trait（绕过 orphan rule）。

### 5.2 Builder

```rust
struct ServerConfig {
    host: String,
    port: u16,
    workers: usize,
}

struct ServerConfigBuilder {
    host: Option<String>,
    port: Option<u16>,
    workers: Option<usize>,
}

impl ServerConfigBuilder {
    fn new() -> Self { ... }

    fn host(mut self, host: impl Into<String>) -> Self {
        self.host = Some(host.into());
        self
    }

    fn build(self) -> Result<ServerConfig, String> {
        Ok(ServerConfig {
            host: self.host.ok_or("host required")?,
            port: self.port.unwrap_or(8080),
            workers: self.workers.unwrap_or(4),
        })
    }
}
```

链式调用 + 类型状态（typestate）可以进一步在编译期保证必填字段：
`Builder<NoHost>` → `Builder<WithHost>`。

### 5.3 策略模式

```rust
trait Compression {
    fn compress(&self, data: &[u8]) -> Vec<u8>;
}

struct Gzip;
struct Zstd;

fn save(data: &[u8], strategy: &dyn Compression) { ... }
```

C++ 中用函数指针 / `std::function` / 虚基类；Rust 用 trait object 或泛型。

### 5.4 依赖注入

```rust
trait Clock {
    fn now(&self) -> u64;
}

struct SystemClock;
struct FakeClock(u64);

fn is_expired(clock: &dyn Clock, deadline: u64) -> bool {
    clock.now() > deadline
}
```

测试时传入 `FakeClock`，生产用 `SystemClock`。这是 Rust 中替代 mock 框架的常用方式。

---

## 6. C++ 对照

| C++23 | Rust |
| --- | --- |
| `virtual void draw() = 0;` | `trait Draw { fn draw(&self); }` |
| `override` | `impl Trait for Type` |
| `dynamic_cast` | `Any::downcast_ref` / enum |
| `std::visit` | `match` |
| `std::variant` | `enum` |
| `std::function` | `Box<dyn Fn>` / `impl Fn` |
| CRTP | 泛型 + trait |
| 策略模式（虚基类） | `&dyn Trait` / 泛型 |
| 装饰器（继承） | 组合 + trait |
| `typeid` | `TypeId` |
| 多重继承 | 多个 trait（无状态） |

---

## 7. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | `Draw` trait object、`Screen`、默认方法、静态/动态派发、异构集合 |
| `examples/state_pattern.rs` | 博文状态机、`Box<dyn State>`、`Option::take` |
| `examples/match_patterns.rs` | 字面量/范围/`@`/解构/守卫/`matches!`/`let-else`/`if let` 链 |

```bash
cd patterns_oop
cargo run
cargo run --example state_pattern
cargo run --example match_patterns
```

---

## 8. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `trait cannot be made into an object` | 方法返回 `Self` / 泛型方法 | 加 `where Self: Sized`，或改设计 |
| `cannot borrow self as mutable ...` | 状态机中重复借用 | `Option::take` / enum |
| `non-exhaustive patterns` | 漏分支 | 补全或 `_` |
| `refutable pattern in local binding` | `let Some(x) = ...` | `let-else` / `if let` |
| `cannot move out of borrowed content` | match 中搬字段 | `ref` / `clone` / 按值匹配 |
| `size unknown` | 直接用 `dyn Trait` 作为字段 | `Box<dyn Trait>` / `&dyn Trait` |

---

## 9. 练习

1. 用 trait object 实现插件系统：`trait Plugin { fn name(&self) -> &str; fn run(&self, input: &str) -> String; }`，
   支持运行时注册多个插件。
2. 把 `state_pattern` 改成 enum 版本，比较代码量和可读性。
3. 用 `match` 实现一个简单的 JSON 值解析器（`enum Json { Null, Bool(bool), Number(f64), String(String), Array(Vec<Json>), Object(HashMap<String, Json>) }`）。
4. 用 newtype 实现 `Email(String)`，在构造函数中校验格式，禁止外部直接构造。
5. 用 Builder 模式实现 `HttpRequest`，要求 `url` 必填，其余有默认值。
6. 解释为什么 `Box<dyn Clone>` 不合法，并实现一个 `clone_box` 方案。

---

## 延伸阅读

- The Book 第 17 章 — <https://doc.rust-lang.org/book/ch17-00-oop.html>
- The Book 第 18 章 — <https://doc.rust-lang.org/book/ch18-00-patterns.html>
- Rust Design Patterns — <https://rust-unofficial.github.io/patterns/>
- `std::any` — <https://doc.rust-lang.org/std/any/>

上一章：[15 · 并发](15-concurrency.md) ·
下一章：[17 · Unsafe 与 FFI](17-unsafe-ffi.md)
