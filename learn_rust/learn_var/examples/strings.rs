//! `String` vs `&str`：C++ `std::string` vs `std::string_view`。
//!
//! - `String`：拥有堆上的 UTF-8 缓冲区，可增长（类似 std::string）；
//! - `&str`：借用的 UTF-8 视图（指针 + 长度），不可增长（类似 string_view）；
//! - 字面量 `"hi"` 的类型是 `&'static str`；
//! - 不能在 UTF-8 字符中间切片。

fn print_len(s: &str) {
    println!("  bytes={}, chars={}", s.len(), s.chars().count());
}

fn main() {
    let literal: &str = "hello";
    let owned: String = String::from("hello");
    let from_literal: String = literal.to_string();

    println!("literal={literal}, owned={owned}, from_literal={from_literal}");

    // &String 可以自动 deref coercion 成 &str
    print_len(literal);
    print_len(&owned);

    // 拼接 / 追加
    let mut s = String::with_capacity(16);
    s.push_str("Rust");
    s.push(' ');
    s.push_str("2024");
    let combined = format!("{s} + C++23");
    println!("s={s}, combined={combined}");

    // 遍历 char 与 bytes
    let text = "你好, Rust";
    println!("text = {text}");
    print_len(text);
    for (i, ch) in text.chars().enumerate() {
        print!("[{i}]={ch} ");
    }
    println!();
    println!("bytes = {:?}", &text.as_bytes()[..6]);

    // 查找 / 替换 / 分割
    println!("contains Rust = {}", text.contains("Rust"));
    println!("replaced = {}", text.replace("Rust", "C++"));
    println!("split = {:?}", text.split(", ").collect::<Vec<_>>());
    println!("trimmed = {:?}", "  hi  ".trim());

    // 安全切片：get 返回 Option<&str>，越界/非字符边界都不会 panic
    println!("get(0..6) = {:?}", text.get(0..6));
    println!("get(0..1) = {:?}", text.get(0..1)); // 落在多字节字符中间 -> None
}
