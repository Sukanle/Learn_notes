//! 第 21 章：Final Project —— 多线程 Web Server。
//!
//! C++ 对比：
//! - `TcpListener` / `TcpStream` ≈ POSIX `socket/bind/listen/accept` + `read/write`；
//! - `BufReader` ≈ `std::istream` 的缓冲层；
//! - `mpsc::channel` ≈ 任务队列 + 条件变量（线程池的核心）；
//! - `Arc<Mutex<Receiver<Job>>>` ≈ `shared_ptr<mutex>` + `condition_variable`；
//! - `Drop for ThreadPool` ≈ 析构函数中通知 worker 退出并 join，避免资源泄漏。
//!
//! 运行：`cargo run`，浏览器打开 <http://127.0.0.1:7878/>
//! 慢请求：<http://127.0.0.1:7878/sleep>（用于观察线程池并发）

use std::fs;
use std::io::{BufRead, BufReader, Write};
use std::net::{TcpListener, TcpStream};
use std::sync::{Arc, Mutex, mpsc};
use std::thread;
use std::time::Duration;

fn main() {
    let listener = TcpListener::bind("127.0.0.1:7878").expect("无法绑定 127.0.0.1:7878");
    let pool = ThreadPool::new(4);
    println!("监听 http://127.0.0.1:7878/ (Ctrl+C 退出)");

    // incoming() 是阻塞迭代器；每个连接交给线程池处理
    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                pool.execute(|| {
                    handle_connection(stream);
                });
            }
            Err(err) => eprintln!("连接失败: {err}"),
        }
    }
}

fn handle_connection(mut stream: TcpStream) {
    // 只读取请求头，直到空行；不读取 body（示例足够）
    let buf_reader = BufReader::new(&stream);
    let request_lines: Vec<String> = buf_reader
        .lines()
        .map_while(Result::ok)
        .take_while(|line| !line.is_empty())
        .collect();

    let request_line = request_lines.first().map(String::as_str).unwrap_or("");

    let (status_line, filename) = match request_line {
        "GET / HTTP/1.1" => ("HTTP/1.1 200 OK", "hello.html"),
        "GET /sleep HTTP/1.1" => {
            thread::sleep(Duration::from_secs(5));
            ("HTTP/1.1 200 OK", "hello.html")
        }
        _ => ("HTTP/1.1 404 NOT FOUND", "404.html"),
    };

    let contents = fs::read_to_string(filename)
        .unwrap_or_else(|_| format!("<h1>404</h1><p>{filename} not found</p>"));

    // HTTP/1.1 响应必须带 Content-Length，否则客户端可能一直等待
    let response = format!(
        "{status_line}\r\nContent-Length: {}\r\n\r\n{contents}",
        contents.len()
    );

    // 忽略写入错误：客户端可能提前断开
    let _ = stream.write_all(response.as_bytes());
}

// ==================== 线程池 ====================

type Job = Box<dyn FnOnce() + Send + 'static>;

struct ThreadPool {
    workers: Vec<Worker>,
    // Option 便于在 Drop 中显式关闭 channel
    sender: Option<mpsc::Sender<Job>>,
}

impl ThreadPool {
    /// 创建一个固定大小的线程池。
    /// # Panics
    /// `size == 0` 时 panic。
    fn new(size: usize) -> ThreadPool {
        assert!(size > 0, "线程池大小必须大于 0");

        let (sender, receiver) = mpsc::channel::<Job>();

        // Receiver 不能被多个线程同时持有，因此放进 Arc<Mutex<_>>
        let receiver = Arc::new(Mutex::new(receiver));

        let mut workers = Vec::with_capacity(size);
        for id in 0..size {
            workers.push(Worker::new(id, Arc::clone(&receiver)));
        }

        ThreadPool {
            workers,
            sender: Some(sender),
        }
    }

    fn execute<F>(&self, f: F)
    where
        F: FnOnce() + Send + 'static,
    {
        let job = Box::new(f);
        // sender 在 ThreadPool 生命周期内始终存在
        self.sender
            .as_ref()
            .expect("线程池已关闭")
            .send(job)
            .expect("worker 已全部退出");
    }
}

impl Drop for ThreadPool {
    fn drop(&mut self) {
        // 关闭 sender：所有 worker 的 recv() 会返回 Err，从而退出循环
        drop(self.sender.take());

        for worker in &mut self.workers {
            println!("关闭 worker {}", worker.id);
            if let Some(thread) = worker.thread.take() {
                thread.join().expect("worker thread panicked");
            }
        }
    }
}

struct Worker {
    id: usize,
    thread: Option<thread::JoinHandle<()>>,
}

impl Worker {
    fn new(id: usize, receiver: Arc<Mutex<mpsc::Receiver<Job>>>) -> Worker {
        let thread = thread::spawn(move || {
            loop {
                // recv() 在等待时会持有锁；这保证同一时刻只有一个 worker 取任务。
                // 这也是为什么本实现不是最高效的（可用 crossbeam / rayon 改进）。
                let message = receiver.lock().expect("mutex poisoned").recv();

                match message {
                    Ok(job) => {
                        println!("Worker {id} 执行任务");
                        job();
                    }
                    Err(_) => {
                        println!("Worker {id} 断开，退出");
                        break;
                    }
                }
            }
        });

        Worker {
            id,
            thread: Some(thread),
        }
    }
}
