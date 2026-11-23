//! 第 10.3 节：Validating References with Lifetimes。
//!
//! C++ 对比：
//! - lifetime 标注不改变引用实际存活时间，只是把“引用之间”的有效性关系告诉编译器，
//!   类似在接口上手写 `[[clang::lifetimebound]]`，但由编译器强制检查；
//! - 大多数情况下有 lifetime elision（省略规则），不必手写；
//! - `'static` 表示整个程序生命周期，字符串字面量就是 `&'static str`；
//! - 返回引用时必须能从输入中找到生命周期来源，否则只能返回拥有所有权的值。

/// 返回值的生命周期与两个输入中较短的那个一致。
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() { x } else { y }
}

#[derive(Debug)]
struct ImportantExcerpt<'a> {
    part: &'a str,
}

impl<'a> ImportantExcerpt<'a> {
    fn level(&self) -> i32 {
        3
    }

    // 显式返回 &'a str：返回值来自结构体持有的 part，而不是 &self
    fn announce_and_return_part(&self, announcement: &str) -> &'a str {
        println!("Attention please: {announcement}");
        self.part
    }
}

fn main() {
    let string1 = String::from("abcd");
    let string2 = String::from("xyz");

    let result = longest(string1.as_str(), &string2);
    println!("longest = {result}");

    // 结构体借用 novel 的一部分，novel 必须比 excerpt 活得久
    let novel = String::from("Call me Ishmael. Some years ago...");
    let first_sentence = novel.split('.').next().expect("could not find a '.'");
    let excerpt = ImportantExcerpt {
        part: first_sentence,
    };
    println!("excerpt = {excerpt:?}, level = {}", excerpt.level());
    println!("part = {}", excerpt.announce_and_return_part("hello"));

    // 'static：字符串字面量
    let s: &'static str = "I have a static lifetime.";
    println!("{s}");

    // 下面这种代码无法通过编译：result 可能借用 string2，而 string2 提前离开作用域
    // let result;
    // {
    //     let string2 = String::from("xyz");
    //     result = longest(string1.as_str(), string2.as_str());
    // }
    // println!("{result}"); // error[E0597]: `string2` does not live long enough
}
