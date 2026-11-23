//! 第 8 章：常见集合 `Vec<T>`、`String`、`HashMap<K, V>`。
//!
//! C++ 对比：
//! - `Vec<T>` ≈ `std::vector<T>`；
//! - `String` ≈ `std::string`，但内部保证 UTF-8；
//! - `&str` ≈ `std::string_view`；
//! - `HashMap<K, V>` ≈ `std::unordered_map<K, V>`（默认使用 SipHash，可防 HashDoS）；
//! - 所有集合都拥有元素，离开作用域自动释放（RAII）。

use std::collections::HashMap;

fn main() {
    // ================= Vec =================
    let mut v: Vec<i32> = Vec::new();
    v.push(1);
    v.push(2);
    v.push(3);
    println!("vec = {v:?}");

    // 索引越界会 panic；get 返回 Option，等价于“安全的 at()”
    println!("v[1] = {}", v[1]);
    match v.get(99) {
        Some(n) => println!("v[99] = {n}"),
        None => println!("v[99] 不存在"),
    }

    // 遍历：&v 只读借用，&mut v 可修改
    for n in &v {
        print!("{n} ");
    }
    println!();
    for n in &mut v {
        *n *= 10;
    }
    println!("修改后 = {v:?}");

    // 用 enum 让 Vec 存放“不同类型”的同类数据（类型安全的 tagged union 数组）
    #[derive(Debug)]
    enum Cell {
        Int(i32),
        Text(String),
    }
    let row = vec![Cell::Int(3), Cell::Text(String::from("blue")), Cell::Int(7)];
    for cell in &row {
        match cell {
            Cell::Int(n) => println!("  Int({n})"),
            Cell::Text(text) => println!("  Text({text})"),
        }
    }

    // ================= String =================
    let mut s = String::from("hello");
    s.push_str(", world");
    s.push('!');
    println!("string = {s}");

    // format! 不会消耗参数，类比 std::format（C++20）
    let s1 = String::from("tic");
    let s2 = String::from("tac");
    let s3 = String::from("toe");
    let joined = format!("{s1}-{s2}-{s3}");
    println!("format = {joined}, s1 仍可用 = {s1}");

    // Rust 字符串不能按字节任意索引：UTF-8 字符可能跨多个字节
    // let c = s[0]; // error[E0277]: the type `String` cannot be indexed by `{integer}`
    for (i, ch) in "你好, Rust".chars().enumerate() {
        println!("  char[{i}] = {ch}");
    }
    println!("bytes = {:?}", "你好".as_bytes());
    // 切片必须落在 char boundary 上，否则运行时 panic
    println!("slice = {}", &s[0..5]);

    // ================= HashMap =================
    let mut scores = HashMap::new();
    scores.insert(String::from("Blue"), 10);
    scores.insert(String::from("Yellow"), 50);

    // get 返回 Option<&V>
    if let Some(score) = scores.get("Blue") {
        println!("Blue = {score}");
    }

    // insert 的 key 会发生 move（&str 除外，如字面量）
    let key = String::from("Red");
    let value = 30;
    scores.insert(key, value);
    // println!("{key}"); // error[E0382]: key 已被 move 进 map

    // entry API：不存在则插入，类比 C++20 `try_emplace`
    scores.entry(String::from("Blue")).or_insert(99);
    scores.entry(String::from("Green")).or_insert(70);
    println!("scores = {scores:?}");

    // 统计词频：entry + or_insert 的经典用法
    let text = "the quick brown fox jumps over the lazy dog the fox";
    let mut word_count = HashMap::new();
    for word in text.split_whitespace() {
        *word_count.entry(word).or_insert(0) += 1;
    }
    println!("word_count = {word_count:?}");

    // 遍历顺序不确定，类似 unordered_map
    let mut pairs: Vec<_> = word_count.into_iter().collect();
    pairs.sort_by(|a, b| b.1.cmp(&a.1).then(a.0.cmp(b.0)));
    println!("排序后的词频 = {pairs:?}");
}
