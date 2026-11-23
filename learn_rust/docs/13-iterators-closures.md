# 13 · 闭包与迭代器

> 对应 The Book 第 13 章
> 示例 crate：[`iterators`](../iterators)
> 运行：`cargo run`、`cargo run --example word_pipeline`

C++ 开发者会同时想到 lambda 与 C++20 ranges。Rust 的闭包和迭代器在设计上更接近
“零成本抽象 + 显式所有权”，理解后可以让代码既简洁又不牺牲性能。

---

## 1. 闭包（Closures）

### 1.1 基本语法

```rust
let add_one = |x: i32| x + 1;
println!("{}", add_one(1));

let add = |a, b| a + b;        // 参数/返回类型通常可推断
println!("{}", add(1, 2));
```

C++：

```cpp
auto add_one = [](int x) { return x + 1; };
auto add = [](auto a, auto b) { return a + b; };  // 泛型 lambda
```

| C++ lambda | Rust closure |
| --- | --- |
| `[=]` / `[&]` 捕获列表 | 编译器自动推断捕获方式 |
| `mutable` | 需要 `FnMut`，闭包变量声明为 `mut` |
| `std::function` | `Box<dyn Fn>` / 泛型 `impl Fn` |
| 泛型 lambda（`auto`） | 泛型函数 + `Fn` trait |
| `noexcept` | 闭包默认不声明异常规格 |

### 1.2 捕获方式

```rust
let x = 10;
let by_ref = || println!("{x}");       // 不可变借用
by_ref();

let mut count = 0;
let mut inc = || {
    count += 1;                        // 可变借用
    count
};
inc();

let name = String::from("Rust");
let consume = move || name;           // 取得所有权
println!("{}", consume());
// println!("{name}");                // error: 已被 move
```

Rust 不写捕获列表，编译器根据闭包体自动选择最小捕获：

- 只读 → `&T`；
- 修改 → `&mut T`；
- 返回值/消耗 → 取得所有权；
- `move` 强制按值捕获（常用于线程、返回闭包）。

### 1.3 三个 trait：`Fn` / `FnMut` / `FnOnce`

| trait | 调用方式 | 典型场景 |
| --- | --- | --- |
| `Fn` | `&self`，可多次调用 | 纯函数、只读捕获 |
| `FnMut` | `&mut self`，可多次调用 | 累加器、状态机 |
| `FnOnce` | `self`，只能调用一次 | 消耗捕获值、`move` 返回 |

```rust
fn apply<F: Fn(i32) -> i32>(f: F, x: i32) -> i32 { f(x) }

fn apply_mut<F: FnMut() -> i32>(mut f: F) -> i32 {
    f();
    f()
}
```

每个 `Fn` 都实现 `FnMut`，每个 `FnMut` 都实现 `FnOnce`。
API 设计原则：**接受你所需的最弱约束**——能 `Fn` 就不要 `FnMut`，能 `FnMut` 就不要 `FnOnce`。

### 1.4 返回闭包

```rust
fn make_adder(n: i32) -> impl Fn(i32) -> i32 {
    move |x| x + n
}

let add_ten = make_adder(10);
println!("{}", add_ten(5));
```

- 必须用 `move` 把 `n` 移入闭包，否则返回后引用悬垂；
- 返回 `impl Fn` 是静态派发，无堆分配；
- 需要根据条件返回不同闭包时，用 `Box<dyn Fn>`。

```rust
fn make_op(op: char) -> Box<dyn Fn(i32, i32) -> i32> {
    match op {
        '+' => Box::new(|a, b| a + b),
        '-' => Box::new(|a, b| a - b),
        _ => Box::new(|a, _| a),
    }
}
```

### 1.5 闭包的存储与性能

- 闭包是匿名类型，大小取决于捕获的字段；
- 泛型参数 `F: Fn()` 会为每个闭包类型单态化，通常可内联；
- `Box<dyn Fn()>` 有间接调用和堆分配；
- `std::function` 类似 `Box<dyn Fn>`，但 Rust 的泛型版本通常更快。

```rust
// 静态派发：零成本
fn run<F: Fn()>(f: F) { f(); }

// 动态派发：灵活但有开销
fn run_dyn(f: &dyn Fn()) { f(); }
```

---

## 2. 迭代器（Iterators）

### 2.1 `Iterator` trait

```rust
pub trait Iterator {
    type Item;
    fn next(&mut self) -> Option<Self::Item>;
}
```

实现 `next` 后，标准库自动提供上百个方法。
`for` 循环本质上是 `IntoIterator::into_iter` + 反复调用 `next`。

```rust
let v = vec![1, 2, 3];

let mut iter = v.iter();
assert_eq!(iter.next(), Some(&1));
assert_eq!(iter.next(), Some(&2));
assert_eq!(iter.next(), Some(&3));
assert_eq!(iter.next(), None);
```

### 2.2 三种遍历

```rust
let mut v = vec![1, 2, 3];

for x in &v { }        // v.iter()       -> &T
for x in &mut v { }    // v.iter_mut()   -> &mut T
for x in v { }         // v.into_iter()  -> T（v 被 move）
```

| 方法 | 产出 | 是否消耗集合 |
| --- | --- | --- |
| `.iter()` | `&T` | 否 |
| `.iter_mut()` | `&mut T` | 否 |
| `.into_iter()` | `T` | 是 |

`for x in v` 会自动调用 `into_iter()`；之后不能再使用 `v`。

### 2.3 惰性适配器

这些方法返回新的迭代器，**不会立即执行**：

| 适配器 | 作用 | C++20 ranges |
| --- | --- | --- |
| `map(f)` | 转换每个元素 | `views::transform` |
| `filter(p)` | 保留满足条件的元素 | `views::filter` |
| `filter_map(f)` | 过滤 + 转换（`Option`） | `views::transform` + `filter` |
| `flat_map(f)` | 展平嵌套 | `views::join` |
| `take(n)` | 取前 n 个 | `views::take` |
| `take_while(p)` | 满足条件时取 | `views::take_while` |
| `skip(n)` | 跳过前 n 个 | `views::drop` |
| `skip_while(p)` | 跳过满足条件的前缀 | `views::drop_while` |
| `chain(other)` | 连接两个迭代器 | `views::concat` |
| `zip(other)` | 配对 | `views::zip` |
| `enumerate()` | 加上索引 | `views::enumerate` |
| `rev()` | 反转（要求双端） | `views::reverse` |
| `step_by(n)` | 步长 | `views::stride` |
| `inspect(f)` | 观察但不改变 | 无直接对应 |
| `peekable()` | 可预读一个元素 | 无直接对应 |

```rust
let result: Vec<i32> = (1..=20)
    .filter(|x| x % 2 == 0)
    .map(|x| x * x)
    .take(3)
    .collect();
// [4, 16, 36]
```

### 2.4 消费端

| 方法 | 结果 | 说明 |
| --- | --- | --- |
| `collect()` | 容器 | 需要目标类型 |
| `sum()` / `product()` | 数值 | 需要 `Sum` / `Product` |
| `fold(init, f)` | 累积值 | 类似 `std::accumulate` |
| `count()` | 元素个数 | |
| `any(p)` / `all(p)` | bool | 短路 |
| `find(p)` | `Option<T>` | 短路 |
| `position(p)` | `Option<usize>` | 索引 |
| `max()` / `min()` | `Option<T>` | 需要 `Ord` |
| `max_by_key(f)` | `Option<T>` | |
| `nth(n)` | `Option<T>` | |
| `last()` | `Option<T>` | |
| `for_each(f)` | `()` | 类似 `for` 循环 |

```rust
let total: i32 = (1..=5).sum();
let factorial = (1..=5).fold(1, |acc, x| acc * x);
let first_even = (1..).find(|x| x % 2 == 0);
```

`collect()` 的目标类型通常需要标注：

```rust
let v: Vec<i32> = (1..=3).collect();
let s: String = "abc".chars().rev().collect();
let set: std::collections::HashSet<_> = vec![1, 1, 2].into_iter().collect();
```

### 2.5 惰性求值

```rust
let calls = std::cell::Cell::new(0);
let v = vec![1, 2, 3];

let iter = v.iter().map(|x| {
    calls.set(calls.get() + 1);
    x * 2
});

println!("{}", calls.get());   // 0：还没消费
let doubled: Vec<_> = iter.collect();
println!("{}", calls.get());   // 3
```

这与 C++ 的 `views` 惰性求值一致，但有两点不同：

- `map` 的闭包必须满足借用规则；
- 迭代器被消费后不能重用（`iter` 被 move 进 `collect`）。

### 2.6 自定义迭代器

```rust
struct Counter { count: u32, max: u32 }

impl Iterator for Counter {
    type Item = u32;

    fn next(&mut self) -> Option<Self::Item> {
        if self.count < self.max {
            self.count += 1;
            Some(self.count)
        } else {
            None
        }
    }
}

let squares: Vec<u32> = Counter { count: 0, max: 5 }
    .map(|x| x * x)
    .collect();
```

只要实现 `next`，`map` / `filter` / `sum` / `collect` 等全部自动可用。
C++ 对应：满足 `input_iterator` concept 的类型；Rust 用 trait 显式约束。

### 2.7 性能：零成本抽象

```rust
// 命令式
let mut total = 0u64;
for i in 1..=1000 {
    if i % 2 == 0 {
        total += i * i;
    }
}

// 迭代器
let total2: u64 = (1..=1000).filter(|i| i % 2 == 0).map(|i| i * i).sum();
```

在 release 模式下，两者通常生成相同的机器码。
Rust 的迭代器经过内联和循环优化，不会比手写循环慢。
需要注意的反而可能是：

- 闭包捕获大量数据；
- `Box<dyn Iterator>` 带来动态派发；
- 不必要的 `collect()` 产生中间容器；
- 在热循环中 `clone()`。

---

## 3. C++20 ranges 对照

| C++20 | Rust |
| --- | --- |
| `views::transform` | `map` |
| `views::filter` | `filter` |
| `views::take` / `drop` | `take` / `skip` |
| `views::join` | `flat_map` |
| `views::zip` | `zip` |
| `views::enumerate` | `enumerate` |
| `views::reverse` | `rev` |
| `ranges::sort` | `Vec::sort` |
| `ranges::accumulate` | `fold` / `sum` |
| `ranges::find` | `find` / `position` |
| `std::ranges::range` concept | `IntoIterator` / `Iterator` |

差异：

- Rust 的适配器是**方法链**，C++ 是管道 `|`；
- Rust 的 `collect` 明确决定目标容器；
- Rust 没有 view 的悬垂问题（所有权/借用检查）；
- C++ ranges 的编译错误以难读著称，Rust 的 trait 错误相对直接。

---

## 4. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | 闭包捕获/FnOnce、惰性求值、适配器、消费端、自定义 `Counter` |
| `examples/word_pipeline.rs` | `split`/`filter_map`/`fold`/`BTreeMap` 的文本处理管道 |

```bash
cd iterators
cargo run
cargo run --example word_pipeline
```

---

## 5. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `borrow of moved value: iter` | 迭代器被消费后重用 | 重新创建，或先 `by_ref()` |
| `cannot infer type for collect` | 目标容器不明确 | `let v: Vec<_> = ...` |
| `closure may outlive ...` | 返回闭包缺 `move` | `move \|x\| ...` |
| `cannot borrow as mutable` | 闭包修改变量但没 `mut` | `let mut closure` |
| `FnOnce` not satisfied | 闭包只能调用一次 | 改 API 为 `FnOnce`，或不要消耗捕获值 |
| `temporary value dropped` | `&collect()` 临时值 | 先绑定变量 |
| `borrow of moved value: v` | `for x in v` 后继续用 | 用 `&v` |

---

## 6. 练习

1. 用迭代器实现 `fn word_lengths(s: &str) -> Vec<usize>`，只保留长度 > 3 的单词。
2. 写一个 `Fibonacci` 自定义迭代器，支持 `take(10).sum::<u64>()`。
3. 用 `fold` 实现 `count` / `max` / `position`，体会消费端的通用性。
4. 把 `for` 循环版本改写为迭代器链，并用 `cargo asm` 或基准比较（可选）。
5. 实现 `fn group_by_key<K, V>(items: Vec<(K, V)>) -> HashMap<K, Vec<V>>`，只用迭代器。
6. 解释为什么从函数返回 `|| &local` 会失败（`local` 是局部变量），而 `move || local` 可以返回一个拥有所有权的值。

---

## 延伸阅读

- The Book 第 13 章 — <https://doc.rust-lang.org/book/ch13-00-functional-features.html>
- `std::iter::Iterator` — <https://doc.rust-lang.org/std/iter/trait.Iterator.html>
- `std::ops::Fn` — <https://doc.rust-lang.org/std/ops/trait.Fn.html>
- C++20 ranges — <https://en.cppreference.com/w/cpp/ranges>

上一章：[12 · minigrep](12-minigrep.md) ·
下一章：[14 · 智能指针](14-smart-pointers.md)
