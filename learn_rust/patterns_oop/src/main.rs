//! 第 17 章：Object-Oriented Programming 的 Rust 表达方式。
//!
//! C++ 对比：
//! - trait ≈ 抽象基类 / concept，但可以给已有类型“追加”实现（retroactive impl）；
//! - `Box<dyn Trait>` ≈ `std::unique_ptr<Base>`，虚表位于对象内部，动态派发；
//! - `&dyn Trait` ≈ `Base&`，胖指针（数据指针 + vtable 指针）；
//! - 泛型 `T: Trait` ≈ 模板 + concepts，静态派发、单态化，无虚表开销；
//! - Rust 没有继承字段；复用通过组合 + trait 默认方法完成。

trait Draw {
    fn draw(&self);
}

trait Summary {
    fn author(&self) -> String;

    // 默认方法：类似基类中带实现的虚函数，但可被任意类型覆盖
    fn summarize(&self) -> String {
        format!("(Read more from {}...)", self.author())
    }
}

struct Button {
    width: u32,
    height: u32,
    label: String,
}

impl Draw for Button {
    fn draw(&self) {
        println!("[Button {}x{}: {}]", self.width, self.height, self.label);
    }
}

struct SelectBox {
    width: u32,
    height: u32,
    options: Vec<String>,
}

impl Draw for SelectBox {
    fn draw(&self) {
        println!(
            "[SelectBox {}x{}: {:?}]",
            self.width, self.height, self.options
        );
    }
}

struct Text {
    content: String,
}

impl Draw for Text {
    fn draw(&self) {
        println!("[Text: {}]", self.content);
    }
}

/// 屏幕只依赖 trait object，不依赖具体组件类型（面向接口编程）。
struct Screen {
    components: Vec<Box<dyn Draw>>,
}

impl Screen {
    fn run(&self) {
        for component in &self.components {
            component.draw(); // vtable 动态派发，类似 base->draw()
        }
    }
}

struct Article {
    author: String,
}

impl Summary for Article {
    fn author(&self) -> String {
        self.author.clone()
    }
}

struct Tweet {
    username: String,
}

impl Summary for Tweet {
    fn author(&self) -> String {
        format!("@{}", self.username)
    }

    fn summarize(&self) -> String {
        format!("{}: 140 characters or less", self.author())
    }
}

// 静态派发：每个 T 生成一份代码（类似模板实例化），适合热路径
fn notify_static<T: Summary>(item: &T) {
    println!("static : {}", item.summarize());
}

// 动态派发：一份代码处理所有实现（类似虚函数），二进制更小
fn notify_dynamic(item: &dyn Summary) {
    println!("dynamic: {}", item.summarize());
}

fn main() {
    println!("== 1. Trait objects ==");
    let screen = Screen {
        components: vec![
            Box::new(SelectBox {
                width: 75,
                height: 10,
                options: vec![
                    String::from("Yes"),
                    String::from("Maybe"),
                    String::from("No"),
                ],
            }),
            Box::new(Button {
                width: 50,
                height: 10,
                label: String::from("OK"),
            }),
            Box::new(Text {
                content: String::from("hello, dyn"),
            }),
        ],
    };
    screen.run();

    println!("== 2. Default methods & dispatch ==");
    let article = Article {
        author: String::from("Alice"),
    };
    let tweet = Tweet {
        username: String::from("rustlang"),
    };

    notify_static(&article);
    notify_static(&tweet);
    notify_dynamic(&article);
    notify_dynamic(&tweet);

    // trait object 的集合可以混放不同类型
    let summaries: Vec<&dyn Summary> = vec![&article, &tweet];
    for s in summaries {
        println!("boxed  : {}", s.summarize());
    }
}
