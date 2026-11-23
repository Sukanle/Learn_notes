//! 第 7 章：Packages、Crates、Modules 与路径。
//!
//! C++ 对比：
//! - crate ≈ 一个编译单元/库目标（类似一个 CMake target）；
//! - package ≈ 一个带 Cargo.toml 的工程（可包含 lib + bin + tests + examples）；
//! - module ≈ namespace，但支持可见性（pub）并且可以对应文件/目录；
//! - `use` ≈ using 声明，但作用域更严格，且可以重命名 / 重导出。

pub mod back_of_house;
pub mod front_of_house;

// `pub use` 把内部路径提升为对外 API（re-export），
// 类似在头文件中用 using 把实现细节的命名空间暴露出去。
pub use crate::front_of_house::hosting;

pub fn eat_at_restaurant() {
    // 绝对路径：crate 是当前 crate 的根
    crate::front_of_house::hosting::add_to_waitlist();

    // 相对路径（Rust 2018+ 推荐以 crate/self/super 开头，避免歧义）
    front_of_house::hosting::add_to_waitlist();

    // 通过 `use` 引入后可以直接用短名
    hosting::add_to_waitlist();

    // struct 字段的可见性独立于 struct 本身
    let mut meal = back_of_house::Breakfast::summer("Rye");
    meal.toast = String::from("Wheat");
    println!(
        "I'd like {} toast please, with {}",
        meal.toast,
        meal.seasonal_fruit()
    );
    // meal.seasonal_fruit = String::from("blueberries"); // error: field is private

    // enum 只要本身是 pub，所有 variant 都对外可见（与 struct 不同）
    let appetizer = back_of_house::Appetizer::Soup;
    match appetizer {
        back_of_house::Appetizer::Soup => println!("Soup"),
        back_of_house::Appetizer::Salad => println!("Salad"),
    }
}
