//! 第 15 章：Smart Pointers（`Box`、`Rc`、`RefCell`、`Deref`、`Drop`）。
//!
//! C++ 对比：
//! - `Box<T>` ≈ `std::unique_ptr<T>`，独占所有权、零额外开销；
//! - `Rc<T>` ≈ `std::shared_ptr<T>`，但计数非原子（单线程），`Arc<T>` 才是多线程版；
//! - `RefCell<T>` ≈ 在运行时执行 borrow checker 规则，常与 `Rc` 组合模拟共享可变状态；
//! - `Deref` ≈ `operator*`/`operator->`；
//! - `Drop` ≈ 析构函数，离开作用域自动调用（RAII）。

use std::cell::RefCell;
use std::rc::Rc;

// 递归类型必须通过间接层确定大小；Box 是零成本选择。
// 类比 C++：`struct List { int value; std::unique_ptr<List> next; };`
#[derive(Debug)]
enum List {
    Cons(i32, Box<List>),
    Nil,
}

use List::{Cons, Nil};

fn main() {
    box_demo();
    rc_demo();
    refcell_demo();
}

fn sum(list: &List) -> i32 {
    match list {
        Cons(value, next) => value + sum(next),
        Nil => 0,
    }
}

fn box_demo() {
    println!("== 1. Box ==");

    let list = Cons(1, Box::new(Cons(2, Box::new(Cons(3, Box::new(Nil))))));
    println!("list = {list:?}");
    println!("sum = {}", sum(&list));

    let boxed = Box::new(42);
    println!("*boxed = {}", *boxed); // Deref
    println!("boxed = {boxed}"); // Display 通过 Deref coercion 生效

    // Box<dyn Trait>：trait object，类比 unique_ptr<Base>
    let drawables: Vec<Box<dyn std::fmt::Display>> =
        vec![Box::new(1), Box::new("two"), Box::new(3.0)];
    for item in &drawables {
        print!("{item} ");
    }
    println!();
}

fn rc_demo() {
    println!("== 2. Rc ==");

    // 多个所有者共享只读数据。Rc::clone 只增加计数，不复制底层 String。
    let a = Rc::new(String::from("shared ownership"));
    println!("创建 a 后 count = {}", Rc::strong_count(&a));

    let b = Rc::clone(&a);
    let c = Rc::clone(&a);
    println!("克隆 b, c 后 count = {}", Rc::strong_count(&a));
    println!("a = {a}, b = {b}, c = {c}");

    {
        let d = Rc::clone(&a);
        println!("内部作用域 count = {}", Rc::strong_count(&a));
        drop(d);
    }
    println!("离开作用域后 count = {}", Rc::strong_count(&a));

    drop(c);
    drop(b);
    println!("只剩 a 时 count = {}", Rc::strong_count(&a));

    // Rc 只能共享不可变访问；需要可变时配合 RefCell。
}

fn refcell_demo() {
    println!("== 3. Rc<RefCell<T>> ==");

    // 共享 + 可变：单线程下的“内部可变性”
    let shared = Rc::new(RefCell::new(vec![1, 2, 3]));

    let owner1 = Rc::clone(&shared);
    let owner2 = Rc::clone(&shared);

    owner1.borrow_mut().push(4);
    owner2.borrow_mut().push(5);
    println!("shared = {:?}", shared.borrow());

    // borrow()/borrow_mut() 在运行时检查借用规则；违反会 panic（不是 UB）。
    // 下面两行同时存在会 panic: already borrowed: BorrowMutError
    // let _guard = shared.borrow();
    // shared.borrow_mut().push(6);

    let value = shared.borrow();
    println!("只读借用期间 len = {}", value.len());
    drop(value); // 显式结束借用（通常由作用域自动完成）

    // 通过 replace 取出值，避免同时借用
    let taken = shared.replace(vec![7, 8]);
    println!(
        "replace 取出的旧值 = {taken:?}, 新值 = {:?}",
        shared.borrow()
    );
}
