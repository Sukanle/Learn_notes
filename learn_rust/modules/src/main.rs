//! 二进制 crate 通过 package 名引用同名的库 crate。
//! 类比：一个 CMake target 链接同一个 package 里的静态库。

// 可以给导入的项起别名，解决命名冲突
use modules::front_of_house::hosting as front_desk;

fn main() {
    println!("== 从库 crate 调用 ==");
    modules::eat_at_restaurant();

    println!("== 直接使用 re-export ==");
    modules::hosting::add_to_waitlist();

    println!("== 使用别名 ==");
    front_desk::add_to_waitlist();
    // front_desk::seat_at_table(); // 不可见：pub(crate) 只在库 crate 内生效
}
