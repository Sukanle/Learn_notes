//! 第 3.3 节：Functions。
//!
//! C++ 对比：
//! - 参数和返回类型必须显式标注（没有 `auto` 推导参数）；
//! - 最后一个表达式即返回值，不需要 `return`；
//! - 没有函数重载：用泛型 + trait，或不同函数名；
//! - 函数不能捕获环境；需要捕获时用 closure（第 13 章）。

fn main() {
    println!("add(1, 2) = {}", add(1, 2));
    println!("no_return() 返回 {:?}", no_return());
    println!("never_returns() 的类型是 `!`（这里不调用）");
}

/// 最后一个表达式 `a + b` 就是返回值，等价于 `return a + b;`
fn add(a: i32, b: i32) -> i32 {
    // return a + b;
    a + b
}

/// 没有返回类型时，返回 `()`（unit），类似 C++ 的 void 但可以作为值存在。
fn no_return() {
    println!("  no_return 执行了");
}

/// 发散函数（diverging function）返回 `!`，表示永不返回。
/// 类比 C++ 的 `[[noreturn]]`。
#[allow(dead_code)]
fn never_returns() -> ! {
    panic!("this function never returns")
}
