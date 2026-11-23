//! 点单与上菜。`super` 指向父模块 `front_of_house`。

pub fn take_order() {
    println!("  serving::take_order");
    super::hosting::seat_at_table(); // 兄弟模块，通过父模块访问
}

pub fn serve_order() {
    println!("  serving::serve_order");
}

pub fn take_payment() {
    println!("  serving::take_payment");
}
