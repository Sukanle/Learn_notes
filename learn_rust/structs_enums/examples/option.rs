//! `Option<T>` 与 C++23 `std::optional<T>` 的对照。
//!
//! 关键差异：
//! - Rust 没有“空引用”；可能缺失的值必须用 Option 表达；
//! - Option 是一个普通 enum，可以用 match / if let / ? 处理；
//! - 组合子 map / and_then / ok_or 等让链式处理更自然。

fn half(x: i32) -> Option<i32> {
    (x % 2 == 0).then(|| x / 2) // bool::then 返回 Option
}

fn parse_positive(s: &str) -> Option<i32> {
    // `?` 在 Option 上会在 None 时提前返回
    let n: i32 = s.trim().parse().ok()?;
    (n > 0).then_some(n)
}

fn main() {
    let values = [Some(10), None, Some(7)];

    for value in values {
        // map: Option<T> -> Option<U>
        println!("map: {:?}", value.map(|v| v * 2));
        // unwrap_or_else: 类比 `value.value_or(0)`，但传的是惰性闭包
        println!("unwrap_or_else: {}", value.unwrap_or_else(|| -1));
    }

    // and_then ≈ 对 optional 做 monadic flatMap
    println!("and_then: {:?}", Some(8).and_then(half));
    println!("and_then: {:?}", Some(9).and_then(half));

    // ok_or 把 Option 转成 Result，便于用 ? 返回错误
    println!("parse_positive(42) = {:?}", parse_positive(" 42 "));
    println!("parse_positive(-1) = {:?}", parse_positive("-1"));
    println!("parse_positive(abc) = {:?}", parse_positive("abc"));

    // Option<&T> 的常用写法
    let v = vec![1, 2, 3];
    if let Some(first) = v.first() {
        println!("first = {first}");
    }
    println!("last = {:?}", v.last());

    // 用 match 同时处理多个 Option
    match (v.first(), v.last()) {
        (Some(first), Some(last)) => println!("range: {first}..={last}"),
        _ => println!("empty"),
    }
}
