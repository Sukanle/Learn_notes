//! 第 16 章：Fearless Concurrency（线程、channel、共享状态）。
//!
//! C++ 对比：
//! - `thread::spawn` ≈ `std::jthread`（Rust 的 JoinHandle 需要显式 join，drop 不会 join）；
//! - `move` 闭包 ≈ 按值捕获，避免悬垂引用；
//! - `mpsc::channel` ≈ 无界队列 + 条件变量（多生产者单消费者消息泵）；
//! - `Arc<Mutex<T>>` ≈ `std::shared_ptr` + mutex，但锁与数据绑定；
//! - `Send`/`Sync` 是编译期 trait，类似给类型加“可跨线程”约束；
//!   例如 `Rc<T>`/`RefCell<T>` 不是 Send/Sync，跨线程使用会编译失败。

use std::sync::{Arc, Mutex, mpsc};
use std::thread;
use std::time::Duration;

fn main() {
    spawn_and_join();
    move_closure();
    channels();
    shared_state();
}

fn spawn_and_join() {
    println!("== 1. spawn & join ==");

    let handle = thread::spawn(|| {
        for i in 1..=3 {
            println!("  子线程: {i}");
            thread::sleep(Duration::from_millis(10));
        }
    });

    // 主线程继续执行
    for i in 1..=3 {
        println!("  主线程: {i}");
        thread::sleep(Duration::from_millis(10));
    }

    // join 等待子线程结束，并取得返回值（Result）
    handle.join().unwrap();
    println!("子线程结束");
}

fn move_closure() {
    println!("== 2. move closure ==");

    let data = vec![1, 2, 3, 4, 5];

    // 不加 move 会借用 data；线程可能比 main 活得久，因此编译器要求 'static
    let handle = thread::spawn(move || data.iter().sum::<i32>());

    // data 已被 move，不能再使用
    // println!("{data:?}"); // error[E0382]
    println!("子线程求和 = {}", handle.join().unwrap());
}

fn channels() {
    println!("== 3. mpsc channel ==");

    let (tx, rx) = mpsc::channel();

    // 克隆 sender，形成多个生产者
    let tx1 = tx.clone();
    thread::spawn(move || {
        for msg in ["hello", "from"] {
            tx1.send(String::from(msg)).unwrap();
            thread::sleep(Duration::from_millis(5));
        }
    });

    thread::spawn(move || {
        tx.send(String::from("worker")).unwrap();
    });

    // 所有 sender 被 drop 后，rx 的迭代自然结束
    for received in rx {
        println!("  收到: {received}");
    }
    println!("channel 已关闭");
}

fn shared_state() {
    println!("== 4. Arc<Mutex<T>> ==");

    let counter = Arc::new(Mutex::new(0));
    let mut handles = Vec::new();

    for id in 0..10 {
        let counter = Arc::clone(&counter);
        handles.push(thread::spawn(move || {
            let mut guard = counter.lock().unwrap(); // RAII：离开作用域自动解锁
            *guard += 1;
            println!("  线程 {id}: counter = {guard}");
        }));
    }

    for handle in handles {
        handle.join().unwrap();
    }

    println!("最终 counter = {}", *counter.lock().unwrap());

    // Mutex 的锁是“守卫”对象：忘记解锁不可能发生（对比手动 lock/unlock）
    // 若某个线程 panic，锁会进入 poisoned 状态，lock() 返回 Err。
}
