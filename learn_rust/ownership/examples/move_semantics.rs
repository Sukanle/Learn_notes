//! Move 的真实含义：所有权转移 + Drop 时机。
//!
//! C++ 对比：
//! - C++ 的 move constructor 会“搬走”资源并把源对象置为 valid-but-unspecified；
//! - Rust 的 move 是编译器层面的浅拷贝 + 禁止再使用源对象，没有 move constructor；
//! - 需要自定义资源释放逻辑时实现 `Drop`（类比析构函数），但 Drop 不能在 move 时手动调用。

#[derive(Debug)]
struct Buffer {
    name: &'static str,
    data: Vec<u8>,
}

impl Drop for Buffer {
    fn drop(&mut self) {
        // 类似析构函数：离开作用域或容器销毁时自动执行
        println!(
            "drop Buffer(name = {}, len = {})",
            self.name,
            self.data.len()
        );
    }
}

fn consume(buffer: Buffer) -> Buffer {
    println!("consume 取得所有权: {}", buffer.name);
    buffer
}

fn main() {
    let a = Buffer {
        name: "a",
        data: vec![1, 2, 3],
    };

    // a 被 move 到 b；编译器保证之后不再使用 a
    let b = a;
    // println!("{:?}", a); // error[E0382]

    let c = consume(b);
    println!("当前 owner 是 c: {:?}", c);

    {
        let scoped = Buffer {
            name: "scoped",
            data: vec![9],
        };
        println!("进入内部作用域: {}", scoped.name);
    } // scoped 在此 drop，早于 c

    println!("main 即将结束");
} // c 在此 drop
