//! 第 4 章：Ownership（所有权）、Borrowing（借用）与 Slices（切片）
//!
//! 对 C++ 开发者来说，Rust 的 ownership 可以理解成：
//! - `String` / `Vec<T>` 类似 `std::unique_ptr` 管理的堆资源：同一时刻只能有一个 owner；
//! - move 类似把 `unique_ptr` 按值传递——源对象失效，而不是 C++ 的“尽量置空”；
//! - `&T` 类似 `const T*`，`&mut T` 类似 `T*`，但有效性由编译器在编译期证明；
//! - `Copy` 类型（整数、bool、char、纯 POD 结构）类似可平凡拷贝的值类型。

fn main() {
    // ---- 1. move：赋值即转移所有权 ----
    let s1 = String::from("hello");
    let s2 = s1; // s1 被 move 到 s2；这里不是浅拷贝，而是一次所有权转移
    // println!("{s1}"); // error[E0382]: borrow of moved value: `s1`
    println!("1. move -> s2 = {s2}");

    // 显式深拷贝，类比 C++ 的 `std::string s3 = s2;`
    let s3 = s2.clone();
    println!("2. clone -> s2 = {s2}, s3 = {s3}");

    // ---- 2. Copy：栈上的平凡值 ----
    let x = 5;
    let y = x; // i32 实现了 Copy，复制后两个变量都可用
    println!("3. Copy -> x = {x}, y = {y}");

    // ---- 3. 函数调用也会 move / return 也会 move ----
    let s4 = takes_and_gives_back(s3);
    println!("4. 函数返回所有权 -> s4 = {s4}");

    // ---- 4. 不可变借用：可以同时存在多个 ----
    let len = calculate_length(&s4);
    println!("5. 借用 -> len({s4}) = {len}");

    // ---- 5. 可变借用：同一作用域只能有一个 ----
    let mut s5 = String::from("hello");
    change(&mut s5);
    println!("6. 可变借用 -> s5 = {s5}");

    // ---- 6. NLL（Non-Lexical Lifetimes）：借用在使用结束后立即释放 ----
    let mut s6 = String::from("abc");
    let r1 = &s6;
    println!("7. r1 = {r1}");
    // r1 最后一次使用发生在这里，之后借用结束，因此可以再创建可变借用
    let r2 = &mut s6;
    r2.push_str("def");
    println!("   r2 = {r2}");

    // ---- 7. Slices：不持有所有权的“视图” ----
    let sentence = String::from("hello world");
    let hello = &sentence[0..5];
    let world = &sentence[6..11];
    println!("8. slice -> {hello} | {world}");

    let word = first_word(&sentence);
    println!("   first_word = {word}");
    // sentence.clear(); // error[E0502]: 不可变借用 word 仍在使用
    println!("   word 仍可用 = {word}");

    // &str 是切片语法的一部分：字符串字面量本身就是 &'static str
    println!("9. 字面量是 &'static str -> {}", first_word("rust and c++"));
}

/// 接收 `String` 的所有权，再原样返回，避免调用者丢失所有权。
/// 这类似 C++ 中按值接收 `std::unique_ptr<T>` 再 `return` 出去。
fn takes_and_gives_back(s: String) -> String {
    s
}

/// 只读借用：不会取得所有权，调用后原变量仍然可用。
fn calculate_length(s: &str) -> usize {
    s.len()
}

/// 可变借用：允许修改，但调用期间原始所有者不能同时被借用。
fn change(s: &mut String) {
    s.push_str(", world");
}

/// 返回 `&str` 切片时，返回值的生命周期与输入 `s` 绑定（第 10 章讲 lifetime elision）。
fn first_word(s: &str) -> &str {
    for (i, byte) in s.bytes().enumerate() {
        if byte == b' ' {
            return &s[..i];
        }
    }
    s
}
