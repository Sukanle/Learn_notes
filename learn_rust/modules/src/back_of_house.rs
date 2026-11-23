//! 后厨模块，演示字段级可见性与 `super`。

pub struct Breakfast {
    pub toast: String,
    seasonal_fruit: String, // 私有字段：外部无法直接赋值
}

impl Breakfast {
    pub fn summer(toast: &str) -> Breakfast {
        Breakfast {
            toast: String::from(toast),
            seasonal_fruit: String::from("peaches"),
        }
    }

    pub fn seasonal_fruit(&self) -> &str {
        &self.seasonal_fruit
    }
}

pub enum Appetizer {
    Soup,
    Salad,
}

pub fn fix_incorrect_order() {
    cook_order();
    // 通过 `super`（crate 根）访问兄弟模块
    super::front_of_house::hosting::add_to_waitlist();
    super::front_of_house::serving::serve_order();
}

fn cook_order() {
    println!("  back_of_house::cook_order (private)");
}
