# 18 · 最终项目：多线程 Web Server

> 对应 The Book 第 20 章
> 示例 crate：[`web_server`](../web_server)
> 运行：
> ```bash
> cd web_server
> cargo run
> # 浏览器打开 http://127.0.0.1:7878/
> # 慢请求 http://127.0.0.1:7878/sleep
> ```

这个项目把所有权、trait object、线程、channel、`Arc<Mutex>`、`Drop`、错误处理和 I/O
组合成一个可运行的系统。它只有几百行，却覆盖了 Rust 并发编程的主要模式。

---

## 1. 目标

- 监听 TCP 端口；
- 解析最简单的 HTTP/1.1 请求行；
- 返回静态 HTML；
- 用固定大小线程池处理并发连接；
- 通过 `/sleep` 观察并发效果；
- 优雅关闭（Drop 中通知 worker 退出）。

C++ 对照：

| 组件 | C++ | Rust |
| --- | --- | --- |
| socket | POSIX `socket/bind/listen/accept` | `TcpListener` |
| 连接 | `accept` 返回 fd | `TcpStream` |
| 读请求 | `read` / `recv` | `BufReader::lines()` |
| 写响应 | `write` / `send` | `write_all` |
| 线程池 | 手写队列 + `mutex` + `condition_variable` | `mpsc` + `Arc<Mutex<Receiver>>` |
| 任务 | `std::function<void()>` | `Box<dyn FnOnce() + Send + 'static>` |
| 资源清理 | 析构 + join | `Drop` + `JoinHandle` |

---

## 2. 监听与接受连接

```rust
use std::net::{TcpListener, TcpStream};

let listener = TcpListener::bind("127.0.0.1:7878")
    .expect("无法绑定 127.0.0.1:7878");
let pool = ThreadPool::new(4);

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
```

- `bind` 返回 `Result<TcpListener>`，端口占用会返回 `Err`；
- `incoming()` 是阻塞迭代器，每个元素是 `io::Result<TcpStream>`；
- `pool.execute` 接收 `FnOnce + Send + 'static`，把连接交给 worker；
- `stream` 被 move 进闭包，因此每个连接独立处理。

C++ 的 `accept` 循环类似，但 Rust 的 `for stream in listener.incoming()` 把错误也放进了迭代器，
不需要手动检查 `errno`。

---

## 3. 处理一个连接

```rust
fn handle_connection(mut stream: TcpStream) {
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

    let contents = fs::read_to_string(filename).unwrap_or_else(|_| {
        format!("<h1>404</h1><p>{filename} not found</p>")
    });

    let response = format!(
        "{status_line}\r\nContent-Length: {}\r\n\r\n{contents}",
        contents.len()
    );

    let _ = stream.write_all(response.as_bytes());
}
```

要点：

1. **只读到空行**：HTTP 头以空行结束；`map_while(Result::ok)` 在 I/O 错误时停止；
2. **`take_while`**：遇到空行停止；不读取 body（示例足够）；
3. **请求行匹配**：真实服务器应解析 method/path/version，而不是硬编码字符串；
4. **`/sleep`**：故意阻塞 5 秒，用来观察 4 个 worker 如何并发；
5. **`Content-Length`**：HTTP/1.1 必须正确，否则客户端可能一直等待；
6. **忽略写错误**：客户端可能提前断开，服务端不应崩溃。

### 3.1 借用与写入

`BufReader::new(&stream)` 借用了 `stream`。读取完成后，借用结束（NLL），
然后再 `stream.write_all(...)` 需要 `&mut stream`。如果同时持有读借用和写借用，编译器会拒绝。

---

## 4. 线程池

### 4.1 结构

```rust
type Job = Box<dyn FnOnce() + Send + 'static>;

struct ThreadPool {
    workers: Vec<Worker>,
    sender: Option<mpsc::Sender<Job>>,
}

struct Worker {
    id: usize,
    thread: Option<thread::JoinHandle<()>>,
}
```

| 组件 | 作用 | C++ 对应 |
| --- | --- | --- |
| `Job` | 可调用任务 | `std::function<void()>` |
| `mpsc::Sender<Job>` | 任务队列入口 | 线程安全队列 |
| `mpsc::Receiver<Job>` | worker 取任务 | `condition_variable` + queue |
| `Arc<Mutex<Receiver>>` | 多 worker 共享接收端 | `mutex` + `cv` |
| `Worker` | 线程 + id | `std::thread` |
| `Drop for ThreadPool` | 关闭 channel + join | 析构中通知退出 |

### 4.2 创建

```rust
impl ThreadPool {
    fn new(size: usize) -> ThreadPool {
        assert!(size > 0, "线程池大小必须大于 0");

        let (sender, receiver) = mpsc::channel::<Job>();
        let receiver = Arc::new(Mutex::new(receiver));

        let mut workers = Vec::with_capacity(size);
        for id in 0..size {
            workers.push(Worker::new(id, Arc::clone(&receiver)));
        }

        ThreadPool { workers, sender: Some(sender) }
    }
}
```

- `mpsc::Receiver` 不能被多个线程同时持有，因此放进 `Arc<Mutex<_>>`；
- 每个 worker 克隆 `Arc`，共享同一个接收端；
- `sender` 留在池中，用于提交任务；
- `Option<Sender>` 便于在 Drop 中显式关闭。

### 4.3 提交任务

```rust
fn execute<F>(&self, f: F)
where
    F: FnOnce() + Send + 'static,
{
    let job = Box::new(f);
    self.sender
        .as_ref()
        .expect("线程池已关闭")
        .send(job)
        .expect("worker 已全部退出");
}
```

- `FnOnce`：任务只执行一次；
- `Send`：任务可以跨线程移动；
- `'static`：线程可能比调用者活得久，不能借用栈数据；
- `Box<dyn FnOnce>` 擦除具体闭包类型，统一放进 channel。

### 4.4 Worker 循环

```rust
impl Worker {
    fn new(id: usize, receiver: Arc<Mutex<mpsc::Receiver<Job>>>) -> Worker {
        let thread = thread::spawn(move || {
            loop {
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

        Worker { id, thread: Some(thread) }
    }
}
```

- `lock().recv()`：持锁等待任务；`recv` 返回后临时守卫立即释放（语句结束），
  因此 `job()` 执行时**不持锁**；
- channel 关闭后 `recv` 返回 `Err`，worker 退出；
- `thread` 放进 `Option`，便于 Drop 中 `take()` 后 join。

> 这个实现有一个性能点：`recv` 等待时会持有锁，因此“等待任务”的 worker 会阻塞其他 worker
> 取任务。标准库没有内置线程池，生产环境用 `rayon` / `tokio` 更合适。

### 4.5 Drop：优雅关闭

```rust
impl Drop for ThreadPool {
    fn drop(&mut self) {
        drop(self.sender.take());   // 关闭 channel

        for worker in &mut self.workers {
            if let Some(thread) = worker.thread.take() {
                thread.join().expect("worker thread panicked");
            }
        }
    }
}
```

- `drop(sender)` 让所有 worker 的 `recv` 返回 `Err`；
- 逐个 `join`，保证线程退出；
- 如果某个 worker panic，`join` 返回 `Err`，这里选择 panic 传播；
- 这是 RAII 在线程池上的体现，类似 C++ 析构函数中 `join`。

> 当前 `main` 是无限循环，`ThreadPool` 永远不会 drop；要观察退出逻辑，
> 需要增加 Ctrl+C / 信号处理或限制连接数。见练习。

---

## 5. 运行与验证

```bash
cd web_server
cargo run
```

另开终端：

```bash
curl -i http://127.0.0.1:7878/
# HTTP/1.1 200 OK
# Content-Length: 630
# <!DOCTYPE html>...

curl -o /dev/null -w "%{http_code}\n" http://127.0.0.1:7878/nope
# 404

# 并发观察：同时发起 4 个 /sleep，再发起一个 /
time curl -s http://127.0.0.1:7878/sleep &
time curl -s http://127.0.0.1:7878/sleep &
# 第 5 个请求会被排队，直到有 worker 空闲
```

如果在容器/远程机器运行，需要把绑定地址改成 `0.0.0.0:7878` 并开放端口。

---

## 6. Rust 在这里教会我们什么

1. **所有权决定任务边界**：连接必须 move 进闭包，不能借用循环变量；
2. **`Send + 'static` 是线程池的安全契约**：编译器强制检查；
3. **channel 是天然的线程安全队列**：不需要手写条件变量；
4. **`Arc<Mutex<Receiver>>` 的锁粒度要小心**：避免在持锁时执行任务；
5. **`Drop` 是资源清理的统一入口**：线程、文件、锁都靠它；
6. **panic 不会静默杀死整个进程**：子线程 panic 由 `join` 返回 `Err`；
7. **I/O 错误必须显式处理**：`Result` 不会让你忘记。

---

## 7. 生产环境还缺什么

这个示例是教学用的最小实现。真实 Web 服务需要：

| 主题 | 说明 | 常用方案 |
| --- | --- | --- |
| HTTP 解析 | 请求方法/路径/版本/头/body | `httparse`、`hyper` |
| 异步 I/O | 高并发下线程模型成本高 | `tokio` + `axum` / `actix-web` |
| 连接管理 | keep-alive、超时、背压 | `hyper` |
| 路由 | 路径参数、中间件 | `axum` / `warp` |
| 安全 | TLS、请求大小限制、路径穿越 | `rustls`、`tower-http` |
| 日志 | 结构化日志、请求 id | `tracing` |
| 优雅关闭 | 信号、等待在途请求 | `tokio::signal` |
| 线程池 | 更高效的任务调度 | `rayon`、`tokio` |
| 性能 | 零拷贝、缓冲区复用 | `bytes`、`mio` |
| 可观测性 | metrics、tracing | `prometheus`、`opentelemetry` |

Rust 生态的 Web 开发与 C++ 的 ASIO + 手写 HTTP 不同：通常直接使用成熟框架，
而不是从 socket 开始写。

---

## 8. C++ 对照：一个 ASIO 风格的心智模型

```cpp
// 伪代码
asio::io_context io;
asio::ip::tcp::acceptor acceptor(io, {tcp::v4(), 7878});

void accept_loop() {
    acceptor.async_accept([&](auto ec, tcp::socket socket) {
        asio::post(thread_pool, [socket = std::move(socket)] {
            handle(std::move(socket));
        });
        accept_loop();
    });
}
```

Rust 版本：

- `TcpListener` ↔ `acceptor`；
- `thread::spawn` ↔ `asio::post(thread_pool, ...)`；
- `mpsc` ↔ 任务队列；
- `Arc<Mutex<Receiver>>` ↔ 线程安全的任务队列；
- `Drop` ↔ 析构中关闭 acceptor + join。

如果使用 `tokio`，代码会更接近 ASIO 的异步风格：

```rust
#[tokio::main]
async fn main() {
    let listener = tokio::net::TcpListener::bind("127.0.0.1:7878").await.unwrap();
    loop {
        let (stream, _) = listener.accept().await.unwrap();
        tokio::spawn(async move {
            handle(stream).await;
        });
    }
}
```

---

## 9. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | 监听、连接处理、ThreadPool、Worker、Drop |
| `hello.html` | 正常页面 |
| `404.html` | 错误页面 |

```bash
cd web_server
cargo run
cargo check --all-targets
```

---

## 10. 练习

1. **限制连接数**：`listener.incoming().take(10)`，让程序在 10 个连接后退出，
   观察 `ThreadPool::drop` 的输出。
2. **优雅关闭**：用 `Arc<AtomicBool>` + 非阻塞 accept 或信号处理，实现 Ctrl+C 退出。
3. **解析请求**：把请求行拆成 method/path/version，支持任意路径映射到 `public/` 目录。
4. **目录安全**：拒绝 `..`，防止路径穿越。
5. **keep-alive**：在一个连接上处理多个请求，正确解析 `Content-Length` 和 `Connection`。
6. **超时**：为每个连接设置读/写超时（`TcpStream::set_read_timeout`）。
7. **替换线程池**：用 `rayon` 或 `tokio` 重写，比较代码量和吞吐。
8. **压测**：用 `wrk` / `oha` 测试 `/` 和 `/sleep`，观察线程池大小的影响。

---

## 延伸阅读

- The Book 第 20 章 — <https://doc.rust-lang.org/book/ch20-00-final-project-a-web-server.html>
- `std::net` — <https://doc.rust-lang.org/std/net/>
- `tokio` — <https://tokio.rs/>
- `axum` — <https://docs.rs/axum/>
- `hyper` — <https://hyper.rs/>
- C++ ASIO — <https://think-async.com/Asio/>

---

恭喜你走完全部章节。下一步建议：

1. 用 `cargo new` 写一个自己的 CLI 工具；
2. 读一个成熟 crate 的源码（`serde`、`regex`、`tokio`）；
3. 尝试把一个小型 C++ 模块用 Rust 重写，并通过 FFI 与现有代码集成；
4. 学习 async Rust（Tokio 官方教程）；
5. 关注 Rust 2024 edition 的新特性与 clippy lint。

上一章：[17 · Unsafe 与 FFI](17-unsafe-ffi.md) ·
返回：[README](../README.md)
