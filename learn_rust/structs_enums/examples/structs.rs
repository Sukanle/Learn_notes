//! Struct 的几种形态与所有权语义。

#[derive(Debug, Clone, Copy)]
struct Color(u8, u8, u8); // tuple struct ≈ 轻量 wrapper

#[derive(Debug)]
struct AlwaysEqual; // unit struct，不占空间

struct Inventory {
    items: Vec<String>,
}

impl Inventory {
    fn new() -> Self {
        Self { items: Vec::new() }
    }

    fn add(&mut self, item: impl Into<String>) {
        self.items.push(item.into());
    }

    fn take_all(&mut self) -> Vec<String> {
        // std::mem::take 类似 C++ 的移动后重置，但源对象一定是有效空值
        std::mem::take(&mut self.items)
    }
}

fn main() {
    let black = Color(0, 0, 0);
    let Color(r, g, b) = black; // Copy 类型，black 仍可用
    println!("black = {black:?}, r = {r}, g = {g}, b = {b}");

    println!("unit struct: {:?}", AlwaysEqual);

    let mut inventory = Inventory::new();
    inventory.add("keyboard");
    inventory.add(String::from("mouse"));
    println!("inventory = {:?}", inventory.items);

    let taken = inventory.take_all();
    println!("taken = {taken:?}");
    println!("take_all 之后 = {:?}", inventory.items);
}
