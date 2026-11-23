//! `Rc<RefCell<T>>` 与 `Weak`：构造双向树，避免引用环导致内存泄漏。
//!
//! C++ 类比：`shared_ptr` 循环引用 + `weak_ptr` 打破环；
//! 但 Rust 在编译期就禁止你通过 `Rc` 获得可变引用，必须显式用 `RefCell`。

use std::cell::RefCell;
use std::rc::{Rc, Weak};

#[derive(Debug)]
struct Node {
    name: &'static str,
    parent: RefCell<Weak<Node>>,
    children: RefCell<Vec<Rc<Node>>>,
}

impl Node {
    fn new(name: &'static str) -> Rc<Self> {
        Rc::new(Self {
            name,
            parent: RefCell::new(Weak::new()),
            children: RefCell::new(Vec::new()),
        })
    }

    fn add_child(parent: &Rc<Node>, child: &Rc<Node>) {
        *child.parent.borrow_mut() = Rc::downgrade(parent); // 强 -> 弱
        parent.children.borrow_mut().push(Rc::clone(child)); // 父持有子的强引用
    }

    fn print_tree(node: &Rc<Node>, depth: usize) {
        println!("{}{}", "  ".repeat(depth), node.name);
        for child in node.children.borrow().iter() {
            Self::print_tree(child, depth + 1);
        }
    }
}

fn main() {
    let root = Node::new("root");
    let child = Node::new("child");
    let grandchild = Node::new("grandchild");

    Node::add_child(&root, &child);
    Node::add_child(&child, &grandchild);

    Node::print_tree(&root, 0);

    println!("root strong = {}", Rc::strong_count(&root));
    println!("child strong = {}", Rc::strong_count(&child));
    // child 被 root.children 和局部变量 child 同时持有
    println!(
        "grandchild weak(parent) 升级成功? {}",
        grandchild.parent.borrow().upgrade().is_some()
    );

    // Weak 不计入 strong_count，因此不会延长生命周期
    let weak_root: Weak<Node> = Rc::downgrade(&root);
    drop(root);
    println!(
        "root 释放后 weak 还能升级吗? {}",
        weak_root.upgrade().is_some()
    );
}
