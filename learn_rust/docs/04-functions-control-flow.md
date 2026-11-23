# 04 · 函数、分支与循环

> 对应 The Book 第 3.3–3.5 节
> 示例 crate：[`func`](../func)、[`branches`](../branches)
> 运行：`cd func && cargo run`、`cd branches && cargo run`

---

## 1. 函数

```rust
fn main() {
    println!("add(1, 2) = {}", add(1, 2));
}

fn add(a: i32, b: i32) -> i32 {
    a + b // 最后一个表达式就是返回值
}
```

要点：

- 参数类型和返回类型必须显式标注（没有 C++ 的 `auto` 参数推导）；
- 函数体最后一个表达式若不以 `;` 结尾，它的值就是返回值；
- 加了 `;` 就变成语句，返回 `()`，通常会导致类型错误；
- 可以用 `return` 提前返回，但尾部表达式更 idiomatic；
- 没有函数重载：用泛型 + trait，或不同函数名（C++ 的 overload resolution 在 Rust 中不存在）；
- 没有默认参数：用 `Option<T>`、builder 模式或不同构造函数。

```rust
fn early_return(x: i32) -> i32 {
    if x < 0 {
        return 0;   // 提前返回
    }
    x * 2           // 尾部表达式
}
```

### 1.1 `()` 与 `!`

```rust
fn log(msg: &str) {          // 返回 ()
    println!("{msg}");
}

fn abort() -> ! {            // never type，表示永不返回
    panic!("abort");
}
```

`()` 是 unit 类型，类似 C++ 的 `void`，但它是真正存在的值（零大小类型），
因此可以写 `Result<(), Error>`、`Vec<()>`（虽然没意义）。

`!` 类型可以强制转换成任何类型，这让 `panic!()`、`unreachable!()`、`loop {}` 可以出现在
任何需要值的位置。C++ 对应 `[[noreturn]]`，但 Rust 是类型系统的一部分。

### 1.2 语句 vs 表达式

```rust
let x = {
    let y = 3;
    y * 2       // 块的值
};              // x == 6
```

C++ 的三目运算符 `cond ? a : b` 在 Rust 中通常写成 `if cond { a } else { b }`，
因为 `if` 是表达式。注意两个分支的类型必须一致：

```rust
let n = if flag { 1 } else { 2 };       // OK
// let n = if flag { 1 } else { "no" }; // error: if and else have incompatible types
```

### 1.3 函数指针

```rust
fn double(x: i32) -> i32 { x * 2 }

let f: fn(i32) -> i32 = double; // 类似 int(*f)(int)
println!("{}", f(3));

let g = double;                 // 函数 item，可以自动转成函数指针
```

闭包（capturing lambda）见第 13 章；函数指针不能捕获环境。

---

## 2. 条件分支

### 2.1 `if` 的条件必须是 `bool`

```rust
let n = 1;
// if n { ... }          // error[E0308]: expected `bool`, found integer
if n != 0 { /* ... */ }
```

C++ 中整数、指针可以隐式转 `bool`，Rust 不允许。这避免了一堆 `if (ptr)`、`if (x = 1)` 类 bug。

```rust
if x > 0 {
    println!("positive");
} else if x < 0 {
    println!("negative");
} else {
    println!("zero");
}
```

### 2.2 `match`

```rust
let n = 3;
match n {
    0 => println!("zero"),
    1 | 2 => println!("one or two"),
    3..=9 => println!("three to nine"),
    _ => println!("other"),
}
```

- `|` 是多重模式，`..=` 是闭区间，`_` 是通配；
- 编译器检查穷尽性，漏掉情况会报 `non-exhaustive patterns`；
- `match` 也是表达式；
- 模式还能解构 struct / enum / tuple（见第 6、18 章）。

`unreachable!()` 用于“逻辑上不可能”的分支，会在运行时 panic 并带上位置信息。

---

## 3. 循环

### 3.1 `loop`

```rust
let mut count = 0;
let result = loop {
    count += 1;
    if count == 10 {
        break count * 2; // break 可以带值，作为 loop 表达式的值
    }
};
println!("result = {result}");
```

C++ 的 `for (;;)` 没有返回值，Rust 的 `loop` 可以是表达式。

### 3.2 `while`

```rust
let mut n = 3;
while n != 0 {
    println!("{n}");
    n -= 1;
}
```

### 3.3 `for`

```rust
for i in 1..=5 {         // 1, 2, 3, 4, 5
    println!("{i}");
}

let v = vec![10, 20, 30];
for x in &v {            // 只读借用
    println!("{x}");
}

for x in &mut v {        // 可变借用
    *x += 1;
}

for x in v {             // 取得所有权（v 之后不可用）
    println!("{x}");
}
```

Rust 的 `for` 只接受实现了 `IntoIterator` 的对象，没有 C 风格的三段式 `for`：

```cpp
for (int i = 0; i < n; ++i) { ... }   // C++
```

对应 Rust：`for i in 0..n { ... }`。需要索引时用 `.enumerate()`：

```rust
for (i, item) in v.iter().enumerate() {
    println!("v[{i}] = {item}");
}
```

### 3.4 带标签的循环

```rust
'outer: for i in 0..3 {
    for j in 0..3 {
        if i * j == 2 {
            break 'outer; // 直接跳出外层
        }
        if j == 1 {
            continue 'outer;
        }
    }
}
```

标签类似 C++ 的 `goto` 目标，但只能用于循环，且由编译器检查跳转合法性。

---

## 4. 案例走读：`branches`

`branches/src/main.rs` 是一个菜单驱动的小程序：

```rust
'exit_program: loop {
    // 打印菜单
    let mut input = String::new();
    std::io::stdin().read_line(&mut input).unwrap();

    let choice: u32 = match input.trim().parse() {
        Ok(num @ (0..3)) => num,
        Ok(_) => {
            println!("错误：请输入 0、1 或 2");
            continue;
        }
        Err(_) => {
            println!("错误：请输入数字");
            continue;
        }
    };

    match choice {
        0 => { /* 温度转换 */ }
        1 => println!("斐波那契数结果：{}", inputfibonacci()),
        2 => break 'exit_program,
        _ => unreachable!(), // 已被 0..3 过滤
    }
}
```

值得注意的点：

1. **`Ok(num @ (0..3))`**：`@` 把匹配到的值绑定到 `num`，同时限制范围是 `0..3`（即 0、1、2）。
   这是一个 range pattern + binding 的组合。
2. **两处 `continue`**：输入非法时不退出程序，而是回到菜单。
3. **`unreachable!()`**：外层 match 的 `_` 分支理论上不会到达，因为上一段 match 已过滤。
   这是“把不变量写进代码”的方式。
4. **内层 `loop`**：`tempconver` 和 `inputfibonacci` 都循环到输入合法为止。
   注意每次 `read_line` 前调用了 `input.clear()`——否则新输入会追加到旧字符串后面。
5. **tuple 返回**：`tempconver() -> (f64, String)` 同时返回温度和单位，类似 `std::pair`。

`fibonacci` 用元组交换迭代，避免递归：

```rust
fn fibonacci(n: u32) -> usize {
    let (mut a, mut b) = (0, 1);
    for _ in 0..n {
        (a, b) = (b, a + b);
    }
    b
}
```

这是 Rust 中很常见的“状态打包”写法；`(a, b) = (b, a + b)` 是并行赋值，不需要临时变量。

### 4.1 `read_line` 的三个坑

| 坑 | 说明 | 处理 |
| --- | --- | --- |
| 追加而非覆盖 | `read_line` 把内容 append 到 `String` | 复用变量时先 `clear()` |
| 带换行符 | 输入 `"42\n"` | `.trim()` |
| EOF 返回 `Ok(0)` | 管道输入结束，不是错误 | 检查返回值，避免死循环 |

此外，`.unwrap()` 在真实程序中应替换成 `?`、`expect("...")` 或显式错误处理。

---

## 5. C++ 对照速查

| C++23 | Rust | 备注 |
| --- | --- | --- |
| `void f()` | `fn f()` | 返回 `()` |
| `[[noreturn]]` | `-> !` | never type |
| `if (x)` | `if x`（必须 bool） | 无隐式转换 |
| `cond ? a : b` | `if cond { a } else { b }` | 两分支类型一致 |
| `switch` | `match` | 强制穷尽，可解构 |
| `for (;;)` | `loop` | 可作为表达式 |
| `while (cond)` | `while cond` | |
| `for (auto x : v)` | `for x in &v` | 注意借用/所有权 |
| `break` / `continue` | 同名 | 支持标签 |
| `goto` | 无 | 用循环/函数重构 |
| `std::function` | `impl Fn` / `Box<dyn Fn>` | 见第 13 章 |

---

## 6. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `expected bool, found integer` | `if n` | `if n != 0` |
| `mismatched types`（if 分支） | 两分支返回类型不同 | 统一类型 |
| `non-exhaustive patterns` | `match` 漏分支 | 补全或 `_ => ...` |
| `cannot break outside of a loop` | `break` 出现在闭包/函数中 | 用返回值或标志位 |
| `unused must use` | 忽略 `Result` | `let _ = ...` 或处理错误 |
| `borrow of moved value` | `for x in v` 后再用 `v` | 用 `&v` / `&mut v` |

---

## 7. 练习

1. 把 `branches` 的菜单选择改成 `enum Choice { Temp, Fib, Quit }`，用 `match` 解析字符串。
2. 让 `main` 返回 `Result<(), Box<dyn Error>>`，去掉所有 `unwrap`。
3. 实现 `fn gcd(a: u64, b: u64) -> u64` 的迭代版和递归版，比较可读性。
4. 写一个带标签的循环，在二维数组中查找目标值并返回坐标。
5. 用 `loop` 实现一个“读入一行、累加数字、输入空行结束”的小程序，并处理 EOF。

---

## 延伸阅读

- The Book 3.3 — <https://doc.rust-lang.org/book/ch03-03-how-functions-work.html>
- The Book 3.5 — <https://doc.rust-lang.org/book/ch03-05-control-flow.html>
- Rust by Example：Flow of Control — <https://doc.rust-lang.org/rust-by-example/flow_control.html>

上一章：[03 · 变量、类型与字符串](03-variables-types.md) ·
下一章：[05 · Ownership 与 Borrowing](05-ownership.md)
