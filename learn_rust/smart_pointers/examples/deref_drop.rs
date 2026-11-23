//! 自定义 `Deref` 与 `Drop`，理解自动解引用与析构顺序。

use std::ops::Deref;

struct MyBox<T>(T);

impl<T> MyBox<T> {
    fn new(value: T) -> Self {
        Self(value)
    }
}

impl<T> Deref for MyBox<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.0
    }
}

struct Logger(&'static str);

impl Drop for Logger {
    fn drop(&mut self) {
        println!("drop Logger({})", self.0);
    }
}

// Deref coercion：`&MyBox<String>` 可以自动转成 `&str`
fn greet(name: &str) {
    println!("Hello, {name}!");
}

fn main() {
    let boxed = MyBox::new(String::from("Rust"));
    println!("*boxed = {}", *boxed);

    // 先 &MyBox<String> -> &String -> &str（多次 deref coercion）
    greet(&boxed);

    // Drop 顺序与 C++ 局部对象相反（后构造先析构）
    let _outer = Logger("outer");
    {
        let _inner = Logger("inner");
        println!("内部作用域结束前");
    }
    println!("内部作用域结束后");

    // std::mem::drop 提前释放，类似显式 delete / reset()
    let early = Logger("early");
    drop(early);
    println!("early 已释放");
}
