# 02 · Guessing Game：第一个完整程序

> 对应 The Book 第 2 章
> 示例 crate：[`guessing_game`](../guessing_game)
> 运行：`cd guessing_game && cargo run`

这一章的价值不在于“猜数字”，而在于把 **输入、解析、错误处理、循环、比较、随机数** 串起来，
让你第一次感受到 Rust 的错误处理与所有权风格。

---

## 1. 完整代码

```rust
use rand::RngExt;
use std::cmp::Ordering;
use std::io;

fn main() {
    println!("Guess the number");

    let secret_number = rand::rng().random_range(1..101);

    loop {
        println!("Please input your guess.");
        let mut guess = String::new();

        io::stdin()
            .read_line(&mut guess)
            .expect("Failed to read line");

        let guess: u32 = match guess.trim().parse() {
            Ok(num) => num,
            Err(_) => continue,
        };

        match guess.cmp(&secret_number) {
            Ordering::Less => println!("Too small"),
            Ordering::Greater => println!("Too big"),
            Ordering::Equal => {
                println!("You win");
                break;
            }
        }
    }
}
```

逐行对照 C++ 心智：

| 代码 | 含义 | C++ 对应 |
| --- | --- | --- |
| `use rand::RngExt;` | 引入 trait，使 `random_range` 可用 | `#include <random>` |
| `rand::rng()` | 取得线程本地 RNG | `std::mt19937 gen(rd());` |
| `random_range(1..101)` | 半开区间 `[1, 101)` | `uniform_int_distribution(1, 100)` |
| `let mut guess = String::new()` | 可增长的 UTF-8 字符串 | `std::string guess;` |
| `&mut guess` | 可变借用，函数会写入 | `std::getline(std::cin, guess)` |
| `.expect(...)` | 失败则 panic | 未检查的 `cin` 状态 / `assert` |
| `match` + `Ok/Err` | 解析成功或失败 | `try { stoi } catch` |
| `guess.cmp(&secret)` | 三路比较 | C++20 `<=>` |
| `Ordering::Less` | `std::strong_ordering::less` | |
| `loop { ... break }` | 无限循环 | `for (;;)` |

---

## 2. 输入：为什么需要 `mut` 和 `&mut`

```rust
let mut guess = String::new();      // 变量本身可变
io::stdin().read_line(&mut guess)   // 把可变借用交给 read_line
```

`read_line` 的签名（简化）：

```rust
fn read_line(&self, buf: &mut String) -> io::Result<usize>
```

- 它需要**修改**传入的 `String`，因此必须 `&mut`；
- 返回 `Result<usize, io::Error>`，`usize` 是读到的字节数（包含换行符）；
- 失败时可以通过 `?` 或 `expect` 处理。

C++ 的 `std::getline` 直接修改引用，失败通过流状态位表达；Rust 把失败放进返回值，
编译器会提醒你处理（`Result` 标了 `#[must_use]`）。

> **注意**：`read_line` 会**追加**到已有内容，不会清空。循环中复用同一个 `String` 时，
> 必须先 `input.clear()`（本仓库的 `branches` 示例就是这么做的）。

读取结果带换行符，所以先 `.trim()`：

```rust
let guess: u32 = guess.trim().parse().expect("Not a number");
```

`.trim()` 返回 `&str`（借用原字符串的切片），`parse::<u32>()` 返回 `Result<u32, ParseIntError>`。

---

## 3. 错误处理：`match` 与 `?`

```rust
let guess: u32 = match guess.trim().parse() {
    Ok(num) => num,
    Err(_) => continue, // 忽略具体错误，重新输入
};
```

等价的 C++：

```cpp
int guess;
while (true) {
    std::string line;
    std::getline(std::cin, line);
    try {
        guess = std::stoi(line);
        break;
    } catch (const std::exception&) {
        continue;
    }
}
```

Rust 版本没有异常、没有隐式转换、没有未初始化变量。`parse` 的目标类型由左侧标注决定：

```rust
let guess: u32 = "42".parse().unwrap();
let guess = "42".parse::<u32>().unwrap(); // 等价写法
```

如果没有类型标注，编译器会报 `type annotations needed`。

---

## 4. 比较：`Ordering` 与 `match`

```rust
match guess.cmp(&secret_number) {
    Ordering::Less => ...,
    Ordering::Greater => ...,
    Ordering::Equal => ...,
}
```

- `cmp` 来自 `Ord` trait，返回 `Ordering`（`Less` / `Equal` / `Greater`）；
- 必须写全三个分支，否则编译失败（穷尽性检查）；
- 对比 C++ 的 `if (a < b) ... else if (a > b) ...`，Rust 用类型系统确保没有漏掉分支。

C++20 的 `<=>` 也能返回 `strong_ordering`，但 Rust 的 `match` 会让编译器检查你是否处理了所有情况。

---

## 5. 随机数：`rand` 0.10

```toml
[dependencies]
rand = "0.10.3"
```

```rust
use rand::RngExt; // 0.10 中 random_range 由 RngExt 提供

let secret_number = rand::rng().random_range(1..101);
```

C++ 对应：

```cpp
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dist(1, 100);
int secret = dist(gen);
```

差异：

- Rust 的 `rand` 默认使用操作系统熵源初始化的 CSPRNG（`ThreadRng`），不要求你手动播种；
- `random_range` 接收 Rust 的 Range，天然表达半开区间；
- 若需要可复现实验，使用 `StdRng::seed_from_u64(...)`。

> **不要**用 `rand::random::<u32>() % 100 + 1` 取范围，取模会引入偏差；
> 用 `random_range` 或 `gen_range`。

---

## 6. 循环与提前退出

```rust
loop {
    // ...
    if done { break; }
}
```

- `loop` 是表达式，可以 `let x = loop { break 42; };`
- `continue` 跳过本次迭代；
- 没有 `goto`；多层嵌套可用 label：

```rust
'outer: loop {
    loop {
        break 'outer; // 跳出外层
    }
}
```

本仓库的 `branches` 示例用了带 label 的 `loop` 来做菜单退出。

---

## 7. 常见编译错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `cannot borrow guess as mutable` | 忘了 `let mut guess` | 加 `mut` |
| `type annotations needed` | `parse()` 不知道目标类型 | `let n: u32 = ...` 或 `parse::<u32>()` |
| `mismatched types: expected &mut String` | 传了 `guess` 而不是 `&mut guess` | 加 `&mut` |
| `use of moved value` | 把 `String` 传给了别的函数 | 传 `&guess` 或 `.clone()` |
| `no method named random_range` | 没导入 `RngExt` | `use rand::RngExt;` |

---

## 8. 练习

1. **限制次数**：最多 7 次，失败时打印答案。
2. **输入 EOF**：当 `read_line` 返回 `Ok(0)` 时退出，而不是无限循环。
3. **去掉 `unwrap/expect`**：让 `main` 返回 `Result<(), Box<dyn Error>>`，用 `?` 传播 I/O 错误。
4. **范围可配置**：从命令行读取上界（`std::env::args`）。
5. **二分策略**：写一个自动猜测的机器人，保证 7 次内猜中 1..=100。

---

## 延伸阅读

- The Book 第 2 章 — <https://doc.rust-lang.org/book/ch02-00-guessing-game-tutorial.html>
- `rand` crate 文档 — <https://docs.rs/rand/>
- `std::cmp::Ordering` — <https://doc.rust-lang.org/std/cmp/enum.Ordering.html>

上一章：[01 · 工具链与 Cargo](01-toolchain-cargo.md) ·
下一章：[03 · 变量与类型](03-variables-types.md)
