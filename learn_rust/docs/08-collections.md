# 08 · 集合：Vec、String、HashMap

> 对应 The Book 第 8 章
> 示例 crate：[`collections`](../collections)
> 运行：`cargo run`、`cargo run --example vec_ops`、`cargo run --example word_count`

Rust 标准库的三大集合，与 C++ 的对应关系非常直接，但所有权和错误处理方式不同。

| Rust | C++23 | 底层结构 |
| --- | --- | --- |
| `Vec<T>` | `std::vector<T>` | 连续数组，`ptr + len + cap` |
| `String` | `std::string` | `Vec<u8>` + UTF-8 不变量 |
| `&str` | `std::string_view` | 指向 UTF-8 的胖指针 |
| `HashMap<K, V>` | `std::unordered_map<K, V>` | 开放寻址 / SwissTable 风格（hashbrown） |
| `BTreeMap<K, V>` | `std::map<K, V>` | B 树，有序 |
| `HashSet<T>` | `std::unordered_set<T>` | |
| `BTreeSet<T>` | `std::set<T>` | |
| `VecDeque<T>` | `std::deque<T>` | 环形缓冲 |

所有集合都**拥有**自己的元素，离开作用域自动释放。元素可以是任何类型，
包括 `String`、struct、enum，甚至其他集合。

---

## 1. `Vec<T>`

### 1.1 创建与基本操作

```rust
let mut v: Vec<i32> = Vec::new();
v.push(1);
v.push(2);
v.push(3);

let v2 = vec![1, 2, 3];                    // 宏
let v3 = vec![0; 5];                       // 5 个 0
let v4 = Vec::with_capacity(100);          // 预分配，避免多次 realloc
```

| 操作 | Rust | C++ |
| --- | --- | --- |
| 追加 | `push(x)` | `push_back(x)` |
| 弹出 | `pop() -> Option<T>` | `pop_back()`（无返回值） |
| 插入 | `insert(i, x)` | `insert(begin + i, x)` |
| 删除 | `remove(i) -> T` | `erase(begin + i)` |
| 长度 | `len()` | `size()` |
| 容量 | `capacity()` | `capacity()` |
| 预留 | `reserve(n)` | `reserve(n)` |
| 清空 | `clear()` | `clear()` |
| 是否为空 | `is_empty()` | `empty()` |

注意 `pop` 返回 `Option<T>`，空 Vec 不会 UB；`remove` 越界会 panic。

### 1.2 索引 vs `get`

```rust
let v = vec![10, 20, 30];
println!("{}", v[1]);              // 20；越界 panic
match v.get(99) {
    Some(x) => println!("{x}"),
    None => println!("不存在"),
}
```

| 方式 | 越界行为 | C++ 对应 |
| --- | --- | --- |
| `v[i]` | panic（安全） | `operator[]`（UB） |
| `v.get(i)` | `Option<&T>` | `at(i)`（抛异常） |

Rust 的 `[]` 不会返回悬垂引用，也不会越界读写；越界是确定的 panic。
生产代码中若越界是“正常情况”，用 `get`；若越界是 bug，用 `[]` 或 `expect`。

### 1.3 遍历与借用

```rust
let mut v = vec![1, 2, 3];

for x in &v {          // &i32
    print!("{x} ");
}

for x in &mut v {      // &mut i32
    *x *= 10;
}

for x in v {           // i32，v 被 move
    println!("{x}");
}
// println!("{v:?}");  // error: v 已被 move
```

`for x in &v` 等价于 `v.iter()`；`for x in &mut v` 等价于 `v.iter_mut()`；
`for x in v` 等价于 `v.into_iter()`。

### 1.4 存放“不同类型”

Rust 的 `Vec<T>` 是同构的。需要异构时用 enum：

```rust
enum Cell {
    Int(i32),
    Text(String),
}

let row = vec![
    Cell::Int(3),
    Cell::Text(String::from("blue")),
    Cell::Int(7),
];
```

这比 C++ 的 `std::vector<std::variant<int, std::string>>` 更类型安全，
因为 `match` 会强制你处理所有 variant（第 6 章）。

### 1.5 常用算法

```rust
let mut v = vec![3, 1, 4, 1, 5, 9, 2, 6];

v.sort();                       // 稳定排序，类似 std::stable_sort
v.dedup();                      // 去除相邻重复（先排序）
v.retain(|x| x % 2 == 0);       // 类似 std::erase_if
v.extend([10, 12]);             // 追加多个
v.drain(..2);                   // 删除区间并返回迭代器
let squares: Vec<i32> = (1..=5).map(|x| x * x).collect();

let (left, right) = v.split_at_mut(2);  // 两个不重叠的可变切片
```

`sort_by` / `sort_by_key` / `sort_unstable` 对应 C++ 的 `std::sort` 家族。
`sort` 是稳定的（Timsort 变体），`sort_unstable` 更快但不稳定。

---

## 2. `String`

### 2.1 创建与修改

```rust
let mut s = String::new();
s.push_str("hello");
s.push(' ');
s.push_str("world");
s.push('!');

let s2 = String::from("hello");
let s3 = "hello".to_string();
let s4 = format!("{s2}, {s3}");
```

| 操作 | Rust | C++ |
| --- | --- | --- |
| 追加字符串 | `push_str(&str)` | `+=` / `append` |
| 追加字符 | `push(char)` | `push_back` |
| 格式化 | `format!` | `std::format` |
| 长度（字节） | `len()` | `size()` |
| 字符数 | `chars().count()` | 无直接对应 |
| 子串 | `&s[a..b]`（字节边界） | `substr` |
| 查找 | `find` / `contains` | `find` |
| 替换 | `replace` | `std::regex_replace` / 手写 |
| 分割 | `split` / `split_whitespace` | 手写 / ranges |

### 2.2 为什么不能 `s[0]`

```rust
let s = String::from("你好");
// let c = s[0];        // error: String cannot be indexed by integer
let first = s.chars().next().unwrap();   // '你'
let bytes = s.as_bytes();                // &[u8]
```

Rust 选择“不提供按字节的 `operator[]`”，因为 UTF-8 字符是多字节的，
按字节索引很容易得到半个字符。需要：

- 按 Unicode 标量：`.chars()`；
- 按字节：`.bytes()` / `.as_bytes()`；
- 按用户可见字符：`unicode-segmentation` crate；
- 切片：`&s[..n]` 必须落在字符边界，否则 panic。

### 2.3 拼接的所有权

```rust
let a = String::from("hello");
let b = String::from("world");

let c = a + " " + &b;    // a 被 move，b 仍然可用
// println!("{a}");      // error: a 已被 move

let d = format!("{a} {b}"); // 如果 a 还在，推荐 format!
```

`+` 的签名近似 `fn add(self, other: &str) -> String`，因此左侧会被消耗。
`format!` 不会消耗参数，是最常用的安全选择。

---

## 3. `HashMap<K, V>`

```rust
use std::collections::HashMap;

let mut scores = HashMap::new();
scores.insert(String::from("Blue"), 10);
scores.insert(String::from("Yellow"), 50);

let score = scores.get("Blue");        // Option<&i32>
for (key, value) in &scores {          // 顺序不确定
    println!("{key}: {value}");
}
```

### 3.1 所有权

```rust
let key = String::from("Red");
scores.insert(key, 30);
// println!("{key}");   // error: key 已被 move 进 map
```

- key 和 value 的所有权会移入 map；
- `get` 接受 `&Q`，可以用 `&str` 查询 `HashMap<String, _>`（`Borrow` trait）；
- map 销毁时，所有 key/value 自动 drop。

### 3.2 Entry API

```rust
scores.entry(String::from("Blue")).or_insert(99);
scores.entry(String::from("Green")).or_insert(70);
```

对应 C++20 的 `try_emplace`，但 `entry` 更通用：

```rust
// 统计词频
let text = "the quick brown fox jumps over the lazy dog the fox";
let mut counts = HashMap::new();
for word in text.split_whitespace() {
    *counts.entry(word).or_insert(0) += 1;
}
```

`or_insert` 只在不存在时插入，返回 `&mut V`，因此可以直接 `+=`。

其他 entry 方法：`or_insert_with(closure)`、`and_modify(f)`、`or_default()`。

### 3.3 顺序与哈希

- `HashMap` 默认使用 SipHash（防 HashDoS），迭代顺序**不确定**；
- 需要稳定顺序：`BTreeMap`（按 key 排序）；
- 需要自定义哈希：`HashMap::with_hasher` + `BuildHasher`（如 `ahash`、`fxhash`）；
- 键必须实现 `Eq + Hash`；浮点不能做 key（没有 `Eq`/`Hash`）。

```rust
use std::collections::BTreeMap;

let mut ordered = BTreeMap::new();
ordered.insert("banana", 2);
ordered.insert("apple", 1);
for (k, v) in &ordered {
    println!("{k} = {v}");   // apple, banana
}
```

---

## 4. 所有权与借用：集合最容易踩的坑

### 4.1 持有元素引用时修改集合

```rust
let mut v = vec![1, 2, 3];
let first = &v[0];
v.push(4);              // error[E0502]: 不能同时可变借用
println!("{first}");
```

C++ 中这是经典的迭代器/引用失效 UB；Rust 在编译期拒绝。
修复：

- 先复制需要的值（`let first = v[0];`，`i32` 是 Copy）；
- 缩小借用作用域；
- 用索引而不是引用（但索引在插入后也可能失效，需要重新定位）。

### 4.2 循环中删除元素

```rust
let mut v = vec![1, 2, 3, 4];
// for x in &v { if *x % 2 == 0 { v.remove(...); } } // 不能编译
v.retain(|x| x % 2 != 0);   // 推荐
```

### 4.3 同时修改两个元素

```rust
let mut v = vec![1, 2, 3, 4];
let (a, b) = v.split_at_mut(2);
a[0] = 10;
b[0] = 20;
```

或用 `iter_mut` + `enumerate`，或用 `swap` / `windows`。

---

## 5. 复杂度与选型

| 操作 | Vec | HashMap | BTreeMap |
| --- | --- | --- | --- |
| 随机访问 | O(1) | — | — |
| 尾部插入 | 均摊 O(1) | — | — |
| 中间插入/删除 | O(n) | — | — |
| 按 key 查找 | — | 均摊 O(1) | O(log n) |
| 有序遍历 | — | 否 | 是 |
| 范围查询 | — | 否 | 是 |

C++ 对照：

- `Vec` ↔ `vector`；`VecDeque` ↔ `deque`；
- `HashMap` ↔ `unordered_map`；`BTreeMap` ↔ `map`；
- `HashSet` ↔ `unordered_set`；`BTreeSet` ↔ `set`；
- `BinaryHeap` ↔ `priority_queue`。

Rust 的 `HashMap` 不保证迭代顺序，甚至不同运行之间也可能不同（随机种子）。
需要可复现输出时，排序或用 `BTreeMap`。

---

## 6. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | Vec 基础、enum 异构、String UTF-8、HashMap/entry 词频 |
| `examples/vec_ops.rs` | sort/dedup/retain/extend/windows/chunks/split_at_mut/drain |
| `examples/word_count.rs` | 文本清洗 + HashMap 统计 + 排序取 Top N |

```bash
cd collections
cargo run
cargo run --example vec_ops
cargo run --example word_count
```

---

## 7. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `cannot index into String` | 按整数索引 | `.chars()` / `.bytes()` / 切片 |
| `byte index is not a char boundary` | 切片落在多字节字符中间 | `char_indices()` |
| `borrow of moved value` | 集合/元素被 move | 传引用或 clone |
| `cannot borrow as mutable` | 持有元素引用时修改集合 | 缩小作用域 / `retain` / `split_at_mut` |
| `key not found`（逻辑错误） | `[]` 索引 HashMap 不存在 | `get` / `entry` |
| `the trait Hash is not implemented` | 浮点/自定义类型没实现 Hash | 用整数 key 或手写 impl |
| `temporary value dropped` | `&format!(...)` 绑定到临时值 | 先 `let s = format!(...)` |

---

## 8. 练习

1. 给定 `Vec<i32>`，返回中位数和众数（`HashMap` 计数）。
2. 把 `"hello world"` 转成 Pig Latin（每个单词首字母移到末尾加 `"ay"`），保持 UTF-8 安全。
3. 用 `HashMap<&str, Vec<&str>>` 把一组单词按首字母分组，并按字母序输出。
4. 实现一个固定容量的 LRU 缓存（`HashMap` + `VecDeque`），分析复杂度。
5. 比较 `HashMap` 与 `BTreeMap` 在 10 万次插入/查询下的性能（`cargo run --release`）。

---

## 延伸阅读

- The Book 第 8 章 — <https://doc.rust-lang.org/book/ch08-00-common-collections.html>
- `std::collections` — <https://doc.rust-lang.org/std/collections/>
- `hashbrown`（Rust HashMap 的底层实现）— <https://github.com/rust-lang/hashbrown>
- C++ `std::unordered_map` vs Rust `HashMap` — 设计取舍不同，注意迭代顺序与哈希洪水防护

上一章：[07 · Package、Crate 与 Module](07-modules.md) ·
下一章：[09 · 错误处理](09-error-handling.md)
