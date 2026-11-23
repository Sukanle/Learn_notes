//! 第 5-6 章：Structs、Methods、Enums、Option 与模式匹配。
//!
//! C++ 对比：
//! - `struct` ≈ C++ `struct`/`class`，但字段默认私有（只有 `pub` 才对外可见）；
//! - `impl` 块 ≈ 把成员函数写在类外，可以分散在多个块中；
//! - `enum` ≈ `std::variant` + tag，但每个 variant 可以有命名字段；
//! - `Option<T>` ≈ `std::optional<T>`，但必须显式处理（没有隐式 bool 转换）；
//! - `match` ≈ `std::visit` + `switch` 的强化版，且编译器强制穷尽。

#[derive(Debug, Clone, Copy)]
struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    // &self ≈ `const Rectangle&`；不会取得所有权
    fn area(&self) -> u32 {
        self.width * self.height
    }

    // 关联函数（associated function）≈ static 成员函数，没有 self
    fn square(size: u32) -> Self {
        Self {
            width: size,
            height: size,
        }
    }

    fn can_hold(&self, other: &Rectangle) -> bool {
        self.width > other.width && self.height > other.height
    }

    // &mut self ≈ `Rectangle&`
    fn scale(&mut self, factor: u32) {
        self.width *= factor;
        self.height *= factor;
    }
}

// enum 的每个 variant 可以携带不同类型的数据，本质上是 tagged union
#[derive(Debug)]
enum Message {
    Quit,                       // unit variant
    Move { x: i32, y: i32 },    // struct variant
    Write(String),              // tuple variant
    ChangeColor(i32, i32, i32), // tuple variant
}

impl Message {
    fn call(&self) {
        match self {
            Message::Quit => println!("Quit"),
            Message::Move { x, y } => println!("Move to ({x}, {y})"),
            Message::Write(text) => println!("Write: {text}"),
            Message::ChangeColor(r, g, b) => println!("ChangeColor: #{r:02X}{g:02X}{b:02X}"),
        }
    }
}

#[derive(Debug)]
enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter(UsState),
}

#[derive(Debug)]
enum UsState {
    Alabama,
    Alaska,
}

fn value_in_cents(coin: &Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter(state) => {
            println!("  Quarter from {state:?}");
            25
        }
    }
}

fn plus_one(x: Option<i32>) -> Option<i32> {
    match x {
        None => None,
        Some(i) => Some(i + 1),
    }
}

fn main() {
    // ---- struct + impl ----
    let scale = 2;
    let rect1 = Rectangle {
        width: dbg!(30 * scale), // dbg! 打印到 stderr 并返回表达式的值
        height: 50,
    };
    let rect2 = Rectangle {
        width: 10,
        height: 40,
    };
    let mut square = Rectangle::square(3);

    println!("rect1 = {rect1:?}, area = {}", rect1.area());
    println!("rect1 能放下 rect2 吗? {}", rect1.can_hold(&rect2));
    square.scale(2);
    println!("缩放后的正方形 = {square:?}, area = {}", square.area());

    // 结构体更新语法 ≈ C++ 逐个字段赋值
    let rect3 = Rectangle { width: 60, ..rect1 };
    println!("rect3 = {rect3:?}");

    // ---- enum + match ----
    let messages = [
        Message::Quit,
        Message::Move { x: 10, y: 20 },
        Message::Write(String::from("hello")),
        Message::ChangeColor(255, 128, 0),
    ];
    for message in &messages {
        message.call();
    }

    let coins = [
        Coin::Penny,
        Coin::Nickel,
        Coin::Dime,
        Coin::Quarter(UsState::Alaska),
        Coin::Quarter(UsState::Alabama),
    ];
    for coin in &coins {
        println!("coin value = {} cents", value_in_cents(coin));
    }

    // Option 必须显式处理；没有 C++ 的隐式 bool / operator* 空指针 UB
    println!("plus_one(None) = {:?}", plus_one(None));
    println!("plus_one(Some(41)) = {:?}", plus_one(Some(41)));

    // if let：只关心一个分支时，避免写完整的 match
    let config_max = Some(3u8);
    if let Some(max) = config_max {
        println!("config max = {max}");
    }

    // let-else（Rust 1.65+）：绑定失败时提前返回
    let Some(max) = config_max else {
        unreachable!("config_max 已知是 Some");
    };
    println!("let-else 解构出的 max = {max}");

    // while let：循环解构
    let mut stack = vec![1, 2, 3];
    while let Some(top) = stack.pop() {
        print!("{top} ");
    }
    println!();
}
