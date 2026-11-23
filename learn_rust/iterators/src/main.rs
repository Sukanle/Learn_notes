//! 第 13 章：Closures 与 Iterators。
//!
//! C++ 对比：
//! - closure ≈ lambda；`Fn` ≈ 可多次调用的 const lambda，`FnMut` ≈ mutable lambda，
//!   `FnOnce` ≈ 只能调用一次（会消耗捕获值）；
//! - iterator ≈ C++20 ranges 的 view + 算法链；但 Rust 的 iterator 是显式 `next()` 协议；
//! - `map/filter/take/collect` 是惰性的，直到消费端才执行（类似 ranges 的 view）；
//! - 迭代器通常会被 LLVM 优化成与手写循环相同的机器码（zero-cost abstraction）。

fn main() {
    closures();
    lazy_evaluation();
    adaptors_and_consumers();
    custom_iterator();
}

fn closures() {
    println!("== 1. Closures ==");

    // 类型通常可由上下文推断；这里显式标注便于阅读
    let add_one = |x: i32| x + 1;
    println!("add_one(1) = {}", add_one(1));

    // FnMut：修改捕获的变量，因此闭包本身需要 mut
    let mut count = 0;
    let mut increment = || {
        count += 1;
        count
    };
    increment();
    increment();
    println!("count = {count}");

    // move：把捕获变量的所有权移入闭包
    let name = String::from("Rust");
    let consume = move || name; // 返回值 move 出闭包，因此是 FnOnce
    println!("move closure 第一次调用: {}", consume());
    // println!("{}", consume()); // error[E0382]: use of moved value

    // 闭包可以像函数一样作为参数（泛型 + Fn trait）
    println!("apply(add_one) = {}", apply(10, add_one));

    // 返回闭包时用 impl Fn，类似 C++ 返回 std::function 但无堆分配
    let make_adder = |n: i32| move |x: i32| x + n;
    let add_ten = make_adder(10);
    println!("add_ten(5) = {}", add_ten(5));
}

fn apply<T, F: Fn(T) -> T>(value: T, f: F) -> T {
    f(value)
}

fn lazy_evaluation() {
    println!("== 2. Lazy iterators ==");

    // Cell 提供内部可变性，便于观察闭包何时被调用
    let calls = std::cell::Cell::new(0);
    let v = vec![1, 2, 3];

    let doubled = v.iter().map(|x| {
        calls.set(calls.get() + 1);
        x * 2
    });

    println!("collect 之前 calls = {}", calls.get()); // 0：适配器尚未执行
    let doubled: Vec<i32> = doubled.collect();
    println!(
        "collect 之后 calls = {}, doubled = {doubled:?}",
        calls.get()
    );
}

fn adaptors_and_consumers() {
    println!("== 3. Adaptors & consumers ==");

    let numbers = 1..=20;

    // 适配器链：filter -> map -> take（都返回新的惰性迭代器）
    let result: Vec<i32> = numbers
        .clone()
        .filter(|x| x % 2 == 0)
        .map(|x| x * x)
        .take(3)
        .collect();
    println!("filter+map+take: {result:?}");

    // 消费端：sum / count / fold / any / all / find
    println!("sum(1..=20) = {}", (1..=20).sum::<i32>());
    println!("fold 阶乘(5) = {}", (1..=5).fold(1, |acc, x| acc * x));
    println!("any(>10) = {}", (1..=20).any(|x| x > 10));
    println!("all(>0) = {}", (1..=20).all(|x| x > 0));
    println!("find(>10) = {:?}", (1..=20).find(|&x| x > 10));

    // zip / chain / enumerate / flat_map
    let names = ["a", "b", "c"];
    let zipped: Vec<_> = names.iter().zip(1..=3).map(|(n, i)| (n, i)).collect();
    println!("zip: {zipped:?}");

    let chained: Vec<i32> = (1..=3).chain(10..=12).collect();
    println!("chain: {chained:?}");

    for (i, ch) in "Rust".chars().enumerate() {
        print!("{i}:{ch} ");
    }
    println!();

    let words = ["hello world", "rust and c++"];
    let letters: Vec<char> = words.iter().flat_map(|s| s.chars()).collect();
    println!("flat_map 字符数 = {}", letters.len());

    // iter / iter_mut / into_iter 的区别
    let mut v = vec![1, 2, 3];
    for x in &v {
        print!("&v: {x} ");
    }
    println!();
    for x in &mut v {
        *x += 1;
    }
    let owned: Vec<i32> = v.into_iter().map(|x| x * 10).collect(); // 消费所有权
    println!("into_iter: {owned:?}");
}

fn custom_iterator() {
    println!("== 4. Custom Iterator ==");

    struct Counter {
        count: u32,
        max: u32,
    }

    impl Counter {
        fn new(max: u32) -> Self {
            Self { count: 0, max }
        }
    }

    // 实现 Iterator 后，所有适配器/消费端自动可用（类比 C++ 满足 range 概念）
    impl Iterator for Counter {
        type Item = u32;

        fn next(&mut self) -> Option<Self::Item> {
            if self.count < self.max {
                self.count += 1;
                Some(self.count)
            } else {
                None
            }
        }
    }

    let squares: Vec<u32> = Counter::new(5).map(|x| x * x).collect();
    println!("custom iterator: {squares:?}");
    println!("对自定义迭代器求和: {}", Counter::new(5).sum::<u32>());
}
