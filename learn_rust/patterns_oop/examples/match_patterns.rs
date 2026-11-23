//! 第 18 章：Patterns and Matching 速查。
//!
//! 模式可以出现在 match、if let、while let、let、函数参数、闭包参数等位置。
//! C++ 对比：接近 `std::visit` + structured bindings + switch 的组合，
//! 但 Rust 会检查穷尽性（exhaustiveness）。

#[derive(Debug)]
struct Point {
    x: i32,
    y: i32,
}

enum Shape {
    Circle {
        center: Point,
        radius: f64,
    },
    Rectangle {
        top_left: Point,
        bottom_right: Point,
    },
    Triangle(Point, Point, Point),
}

fn main() {
    println!("== 1. 字面量 / 范围 / 多重模式 ==");
    for x in [0, 1, 5, 13, 42] {
        let description = match x {
            0 => String::from("zero"),
            1 | 2 => String::from("one or two"),
            3..=9 => String::from("three to nine"),
            n @ 10..=19 => format!("teen: {n}"), // @ 绑定整个范围
            _ => String::from("other"),
        };
        println!("{x}: {description}");
    }

    println!("== 2. 解构 struct / enum / tuple ==");
    let p = Point { x: 0, y: 7 };
    match p {
        Point { x: 0, y } => println!("在 y 轴上: y = {y}"),
        Point { x, y: 0 } => println!("在 x 轴上: x = {x}"),
        Point { x, y } => println!("任意点: ({x}, {y})"),
    }

    let shapes = [
        Shape::Circle {
            center: Point { x: 0, y: 0 },
            radius: 1.5,
        },
        Shape::Rectangle {
            top_left: Point { x: 0, y: 0 },
            bottom_right: Point { x: 10, y: 10 },
        },
        Shape::Triangle(
            Point { x: 0, y: 0 },
            Point { x: 3, y: 0 },
            Point { x: 0, y: 4 },
        ),
    ];

    for shape in &shapes {
        match shape {
            Shape::Circle { center, radius } => {
                println!("circle center = {center:?}, r = {radius}");
            }
            Shape::Rectangle {
                top_left,
                bottom_right,
            } => println!("rect {top_left:?} -> {bottom_right:?}"),
            Shape::Triangle(a, b, c) => println!("triangle {a:?} {b:?} {c:?}"),
        }
    }

    // let 也是模式：tuple 解构 / 忽略部分字段
    let ((a, b), Point { x, .. }) = ((1, 2), Point { x: 3, y: 4 });
    println!("a = {a}, b = {b}, x = {x}");

    println!("== 3. Guard 与 @ ==");
    for n in [3, -3, 0] {
        match n {
            n if n > 0 => println!("{n} 是正数"),
            n @ -10..=-1 => println!("{n} 是负数（在 -10..=-1 内）"),
            _ => println!("{n} 是零或范围外"),
        }
    }

    println!("== 4. matches! 与 let-else ==");
    let value = Some(7);
    println!("is Some(1..=9): {}", matches!(value, Some(1..=9)));

    let Some(v) = value else {
        unreachable!();
    };
    println!("v = {v}");

    println!("== 5. if let 链（Rust 2024）==");
    let config: Option<(u8, &str)> = Some((3, "release"));
    if let Some((level, mode)) = config
        && level > 1
    {
        println!("level = {level}, mode = {mode}");
    }

    println!("== 6. while let ==");
    let mut stack = vec![10, 20, 30];
    while let Some(top) = stack.pop() {
        print!("{top} ");
    }
    println!();
}
