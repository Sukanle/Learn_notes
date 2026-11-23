//! `thread::scope`：借用栈上数据的并行计算，无需 Arc。
//!
//! C++ 对比：类似 `std::jthread` + `std::span` + 手动 join，
//! 但 Rust 在编译期保证所有子线程在 scope 结束前 join，引用不会悬垂。

use std::thread;

fn main() {
    // 1..=1_000_000 的平方和，分块并行
    let data: Vec<u64> = (1..=1_000_000).collect();

    let started = std::time::Instant::now();
    let partial_sums: Vec<u64> = thread::scope(|scope| {
        let handles: Vec<_> = data
            .chunks(250_000)
            .map(|chunk| {
                // chunk 是 &[u64]，借用 data；scope 保证线程不会活过 data
                scope.spawn(move || chunk.iter().map(|x| x * x).sum::<u64>())
            })
            .collect();

        handles
            .into_iter()
            .map(|handle| handle.join().unwrap())
            .collect()
    });

    let total: u64 = partial_sums.iter().sum();
    println!("分块结果 = {partial_sums:?}");
    println!("总和 = {total}");
    println!("耗时 = {:?}", started.elapsed());

    // 在 scope 中把数组的不同元素交给不同线程：
    // 每个线程拿到互不重叠的 &mut u64，借用检查器可以证明安全。
    let mut buffer = vec![0u64; 4];
    thread::scope(|scope| {
        for (slot, value) in buffer.iter_mut().zip(1..=4u64) {
            scope.spawn(move || *slot = value * value);
        }
    });
    println!("scope 写回 buffer = {buffer:?}");
}
