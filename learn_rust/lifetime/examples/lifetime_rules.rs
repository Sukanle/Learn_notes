//! Lifetime elision（省略规则）与常见修正方式。
//!
//! 三条省略规则：
//! 1. 每个输入引用获得自己的 lifetime 参数；
//! 2. 只有一个输入 lifetime 时，输出 lifetime 等于它；
//! 3. 有 `&self`/`&mut self` 时，输出 lifetime 等于 self 的。

/// 规则 2：只有一个输入引用，输出自动绑定到它。
fn first_word(s: &str) -> &str {
    s.split_whitespace().next().unwrap_or("")
}

/// 规则 3：方法返回引用默认绑定到 &self。
struct Parser<'a> {
    input: &'a str,
}

impl<'a> Parser<'a> {
    fn input(&self) -> &str {
        self.input
    }

    /// 如果想返回与结构体字段相同的 'a，需要显式标注。
    fn input_explicit(&self) -> &'a str {
        self.input
    }
}

/// 两个输入引用且返回引用：必须显式标注（规则 2 不适用）。
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() >= y.len() { x } else { y }
}

/// 返回拥有所有权的值，就不需要 lifetime。
fn make_greeting(name: &str) -> String {
    format!("Hello, {name}!")
}

fn main() {
    let text = String::from("hello rust world");
    let word = first_word(&text);
    println!("first_word = {word}");

    let parser = Parser { input: &text };
    println!("input = {}", parser.input());
    println!("input_explicit = {}", parser.input_explicit());

    println!("longest = {}", longest(&text, "short"));

    let greeting = make_greeting("C++ developer");
    println!("{greeting}");
}
