//! Borrow checker 的三条核心规则，以及 dry-run 常见报错。
//!
//! 规则：
//! 1. 任意时刻，要么有多个 `&T`，要么有且仅有一个 `&mut T`；
//! 2. 引用必须始终有效（不能悬垂）；
//! 3. 所有权只能有一个 owner。

fn main() {
    multiple_immutable();
    single_mutable();
    no_dangling();
    reborrow();
}

fn multiple_immutable() {
    let v = vec![1, 2, 3];
    let a = &v;
    let b = &v;
    println!("多个不可变借用: {a:?} {b:?}");
}

fn single_mutable() {
    let mut v = vec![1, 2, 3];
    {
        let a = &mut v;
        a.push(4);
        // let b = &mut v; // error[E0499]: 不能同时有两个可变借用
        println!("可变借用: {a:?}");
    }
    v.push(5); // 上一个可变借用结束后才可以再次借用
    println!("再次借用: {v:?}");
}

fn no_dangling() {
    // 下面这种写法在 Rust 中无法通过编译：
    // fn dangle() -> &String {
    //     let s = String::from("hello");
    //     &s // error[E0106]: missing lifetime specifier / 返回局部变量引用
    // }
    //
    // 正确做法：直接转移所有权
    fn not_dangle() -> String {
        let s = String::from("hello");
        s
    }
    println!("返回所有权而不是悬垂引用: {}", not_dangle());
}

fn reborrow() {
    // &mut T 可以重借用为 &T，类比 C++ 中 `T*` 隐式转 `const T*`
    fn print_len(s: &str) {
        println!("len = {}", s.len());
    }
    fn append(s: &mut String) {
        print_len(&*s); // 重借用为不可变引用
        s.push('!');
    }
    let mut s = String::from("hi");
    append(&mut s);
    println!("reborrow 之后: {s}");
}
