//! 第 10.1 节：Generic Data Types。
//!
//! C++ 对比：
//! - 泛型 ≈ 模板，单态化（monomorphization）后为每个具体类型生成一份代码；
//! - 区别：Rust 在定义处就检查泛型代码，且必须显式写 trait bound；
//!   C++ 模板是“实例化时才检查”（SFINAE/concepts 改善了这一状况）；
//! - `impl<T> Point<T>` 可以为所有 T 实现方法；`impl Point<f64>` 只针对具体类型。

#[derive(Debug, Clone, Copy)]
struct Point<T> {
    x: T,
    y: T,
}

impl<T> Point<T> {
    fn new(x: T, y: T) -> Self {
        Self { x, y }
    }
}

// move_to 需要复制字段，因此需要 T: Copy（或 Clone + clone()）。
// C++ 模板成员函数只有在被调用时才要求 T 可拷贝。
impl<T: Copy> Point<T> {
    fn move_to(&mut self, pos: &Point<T>) {
        self.x = pos.x;
        self.y = pos.y;
    }
}

// 只为 f64 的 Point 提供方法（类似模板特化）
impl Point<f64> {
    fn distance_to(&self, other: &Point<f64>) -> f64 {
        ((self.x - other.x).powi(2) + (self.y - other.y).powi(2)).sqrt()
    }
}

fn main() {
    let mut p = Point::new(1, 2);
    let target = Point::new(10, 20);
    println!("before = {p:?}");
    p.move_to(&target);
    println!("after  = {p:?}");

    let f = Point::new(0.0, 0.0);
    let g = Point::new(3.0, 4.0);
    println!("distance = {}", f.distance_to(&g));

    // 类型推断：同一个泛型定义实例化为不同具体类型
    let chars = Point::new('a', 'b');
    println!("chars = {chars:?}");
}
