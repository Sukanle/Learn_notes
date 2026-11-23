//! Slices：对数组 / Vec / String 一段连续内存的借用视图。
//!
//! C++ 对比：
//! - `&[T]` ≈ `std::span<const T>`（C++20）
//! - `&mut [T]` ≈ `std::span<T>`
//! - `&str` ≈ `std::string_view`（但 Rust 保证 UTF-8 边界安全）
//! 区别：span/string_view 不会延长生命周期，Rust 编译器会静态检查。

fn sum(values: &[i32]) -> i32 {
    values.iter().sum()
}

fn double_all(values: &mut [i32]) {
    for v in values {
        *v *= 2;
    }
}

fn first_word(s: &str) -> &str {
    s.split_whitespace().next().unwrap_or("")
}

fn main() {
    let arr = [1, 2, 3, 4, 5];
    println!("整个数组: {}", sum(&arr));
    println!("子切片: {}", sum(&arr[1..4]));

    let mut v = vec![1, 2, 3];
    double_all(&mut v);
    println!("修改后的 Vec: {v:?}");

    let text = String::from("ownership makes Rust unique");
    let word = first_word(&text);
    println!("第一个单词: {word}");
    println!("字节切片: {:?}", &text.as_bytes()[..9]);

    // 索引越界：slice 的 get 返回 Option，类似 C++ 的 at() 但不需要异常
    println!("get(0) = {:?}", arr.get(0));
    println!("get(99) = {:?}", arr.get(99));
}
