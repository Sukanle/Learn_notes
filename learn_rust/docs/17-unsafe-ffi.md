# 17 · Unsafe Rust 与 FFI：和 C/C++ 互操作

> 对应 The Book 第 19 章（Advanced Features）+ FFI 实践
> 示例 crate：[`unsafe_ffi`](../unsafe_ffi)
> 运行：`cargo run`、`cargo run --example ffi_safe_wrapper`

`unsafe` 不是“关闭 Rust 检查”，而是把编译器无法验证的责任显式交给程序员。
正确用法是：**在最小边界内使用 unsafe，对外暴露 safe API，并写下 Safety 契约**。

---

## 1. `unsafe` 能做什么

Rust 的 unsafe 只解锁五类操作：

| 操作 | 示例 | 风险 |
| --- | --- | --- |
| 解引用裸指针 | `*ptr` | 空指针、悬垂、别名 |
| 调用 `unsafe fn` | `slice::from_raw_parts` | 违反被调函数契约 |
| 访问/修改 `static mut` | `COUNTER += 1` | 数据竞争 |
| 实现 `unsafe trait` | `unsafe impl Send for X` | 线程安全谎言 |
| 访问 `union` 字段 | `u.i` | 类型双关 |

`unsafe` **不会**：

- 关闭借用检查；
- 允许 use-after-free 自动变“安全”；
- 让数据竞争变成合法行为（仍是 UB）。

```rust
let mut num = 5;
let r = &raw mut num;          // edition 2024 推荐写法

unsafe {
    *r += 1;                    // 只有解引用需要 unsafe
}
println!("{num}");             // 6
```

`&raw const T` / `&raw mut T` 直接创建裸指针，不会先创建引用（避免别名假设问题）。
C++ 对照：`const T*` / `T*`，但 Rust 的裸指针可以为 null、可以悬垂、可以未对齐。

---

## 2. 安全抽象：`split_at_mut`

标准库的 `split_at_mut` 需要返回两个不重叠的可变切片。
借用检查器无法证明“不重叠”，所以内部使用 unsafe：

```rust
use std::slice;

fn split_at_mut(values: &mut [i32], mid: usize) -> (&mut [i32], &mut [i32]) {
    let len = values.len();
    let ptr = values.as_mut_ptr();
    assert!(mid <= len);       // 先检查边界，建立安全前提

    unsafe {
        (
            slice::from_raw_parts_mut(ptr, mid),
            slice::from_raw_parts_mut(ptr.add(mid), len - mid),
        )
    }
}
```

安全契约（必须写在注释/文档里）：

- `mid <= len` 已由 `assert!` 保证；
- 两个切片不重叠；
- 返回的切片生命周期与输入绑定（由签名保证）。

调用者看到的是 safe 函数，不需要 `unsafe`：

```rust
let mut v = vec![1, 2, 3, 4];
let (a, b) = split_at_mut(&mut v, 2);
a[0] = 10;
b[0] = 20;
```

> 这是 Rust 生态的核心模式：**unsafe 藏在库里，safe 暴露给用户**。
> `Vec`、`String`、`HashMap`、`Rc` 内部都有 unsafe，但你写业务代码时不需要碰。

### 2.1 Safety 注释

```rust
// SAFETY: `ptr` 来自 `values.as_mut_ptr()`，`mid <= len`，
// 因此 `ptr..ptr+mid` 和 `ptr+mid..ptr+len` 都在同一分配内且不重叠。
unsafe { ... }
```

Rust 社区习惯用 `// SAFETY:` 前缀，clippy 也有相关 lint。
这不是形式主义：审计 unsafe 代码时，契约是唯一依据。

---

## 3. 调用 C 函数

```rust
unsafe extern "C" {
    fn abs(input: i32) -> i32;
}

fn main() {
    let x = -42;
    let y = unsafe { abs(x) };
    println!("{y}");       // 42
}
```

要点：

- edition 2024 要求 `unsafe extern "C" { ... }`；
- 链接器会自动链接 libc（Linux/macOS）；Windows 上是 CRT；
- 调用外部函数是 unsafe：编译器无法验证参数/返回值契约；
- 需要链接额外库时，在 `build.rs` 中 `println!("cargo:rustc-link-lib=foo")`。

`libc` crate 提供了大量 C 类型和函数声明，省去手写 `extern`：

```toml
[dependencies]
libc = "0.2"
```

```rust
let pid = unsafe { libc::getpid() };
```

---

## 4. 导出给 C/C++ 调用

```rust
#[unsafe(no_mangle)]
pub extern "C" fn rust_add(a: i32, b: i32) -> i32 {
    a + b
}
```

- `#[unsafe(no_mangle)]`：禁止名称修饰，符号名就是 `rust_add`；
- `extern "C"`：使用 C ABI（调用约定、名称、栈清理）；
- `pub`：让符号在库中可见；
- C++ 侧声明：

```cpp
extern "C" int32_t rust_add(int32_t a, int32_t b);
```

### 4.1 生成静态库/动态库

```toml
[lib]
crate-type = ["staticlib"]      # 或 ["cdylib"]（C 动态库）
```

```bash
cargo build --release
# target/release/libyourcrate.a
```

CMake 集成示例：

```cmake
add_executable(app main.cpp)
target_link_libraries(app PRIVATE
    ${CMAKE_SOURCE_DIR}/rust/target/release/libyourcrate.a
    dl
    pthread
    m
)
```

更工程化的方式：

- **cbindgen**：从 Rust 代码生成 C/C++ 头文件；
- **bindgen**：从 C/C++ 头文件生成 Rust `extern` 声明；
- **cxx**：在 Rust 与 C++ 之间生成类型安全的桥接层（推荐用于复杂 C++ 类型）；
- **`cc` crate**：在 `build.rs` 里直接编译 C/C++ 源文件。

---

## 5. 跨语言边界的关键规则

### 5.1 内存分配与释放必须成对

```rust
// Rust 分配，交给 C 使用，再由 Rust 释放
let boxed = Box::new(42);
let raw = Box::into_raw(boxed);

unsafe {
    // C 侧只读
    println!("{}", *raw);

    // 必须用 Box::from_raw 释放，不能用 C 的 free
    drop(Box::from_raw(raw));
}
```

规则：

- 谁分配，谁释放；
- Rust 的 `Box` 必须用 `Box::from_raw` 释放；
- C 的 `malloc` 必须用 `free`；
- 不要把 `Vec` 的指针交给 C 后直接 `delete[]`。

### 5.2 字符串

```rust
use std::ffi::{CString, CStr};

let c_string = CString::new("hello").unwrap();   // 带 NUL 结尾
let ptr = c_string.as_ptr();

// 从 C 返回的字符串构造 Rust 字符串（需要确认 NUL 结尾）
let rust_str = unsafe { CStr::from_ptr(ptr) }.to_str().unwrap();
```

- Rust `&str` 不保证 NUL 结尾，不能直接传给 C；
- `CString` 保证 NUL 结尾，且拒绝内部 NUL；
- `CStr` 是借用的 C 字符串视图；
- 跨边界字符串注意编码（通常是 UTF-8 或 ASCII）。

### 5.3 Panic 不能跨 FFI

Rust panic 跨 `extern "C"` 边界是 UB（Rust 1.81+ 会直接 abort）。
在导出的函数内部捕获：

```rust
use std::panic::{catch_unwind, AssertUnwindSafe};

#[unsafe(no_mangle)]
pub extern "C" fn safe_entry(input: i32) -> i32 {
    match catch_unwind(AssertUnwindSafe(|| {
        if input < 0 { panic!("negative"); }
        input * 2
    })) {
        Ok(v) => v,
        Err(_) => -1,          // 用错误码返回
    }
}
```

C++ 侧不能 `catch` Rust panic；Rust 侧也不能安全地让 C++ 异常穿过。

### 5.4 `repr(C)` 与布局

```rust
#[repr(C)]
pub struct Header {
    magic: u32,
    version: u16,
    flags: u16,
}
```

- 默认 Rust 布局不保证字段顺序；
- `repr(C)` 保证与 C 结构体兼容；
- 注意对齐/padding；可以用 `static_assert`（C++）或 `const` 断言（Rust）验证大小；
- `repr(transparent)` 用于 newtype，保证与内部类型相同布局。

### 5.5 所有权跨边界

C/C++ 没有 Rust 的 move 语义，因此要明确：

- 传指针：借用，谁负责释放？
- 传 `Box::into_raw`：所有权转移，必须约定释放函数；
- 返回指针：可能是借用，也可能是转移，必须在文档中写明；
- 回调：Rust 闭包传给 C 需要 `extern "C"` + user data 指针，注意生命周期。

---

## 6. C++ 对照

| C++ | Rust |
| --- | --- |
| `extern "C"` | `unsafe extern "C"`（调用）/ `extern "C" fn`（导出） |
| `__attribute__((no_mangle))` | `#[unsafe(no_mangle)]` |
| `T*` / `const T*` | `*mut T` / `*const T` |
| `reinterpret_cast` | `as` / `transmute`（极慎用） |
| `std::launder` / 指针 provenance | `&raw` / `addr_of!` |
| `malloc/free` | `Box::into_raw` / `from_raw`、`alloc` |
| 虚析构 | `Drop` + vtable |
| `std::string` 跨 ABI | 不要跨 ABI 传 STL 类型，用 C ABI |
| `catch (...)` | `catch_unwind` |
| `-fsanitize=address` | Miri / ASan（配合 nightly） |

---

## 7. 代码地图

| 文件 | 内容 |
| --- | --- |
| `src/main.rs` | 裸指针读写、`split_at_mut` 安全抽象、调用 C `abs`、导出 `rust_add` |
| `examples/ffi_safe_wrapper.rs` | 用 `CString` 封装 `strlen` / `atoi`，返回 `Result`，隔离 unsafe |

```bash
cd unsafe_ffi
cargo run
cargo run --example ffi_safe_wrapper
```

`ffi_safe_wrapper.rs` 的模式值得记住：

```rust
fn c_strlen(text: &str) -> Result<usize, std::ffi::NulError> {
    let c_text = CString::new(text)?;
    // SAFETY: CString 保证 NUL 结尾且在本函数内有效
    Ok(unsafe { strlen(c_text.as_ptr()) })
}
```

- 外部 API 是 safe 的；
- 失败（内部有 NUL）通过 `Result` 返回；
- unsafe 只出现在一行，并有 Safety 注释。

---

## 8. 常见错误

| 错误 | 原因 | 修复 |
| --- | --- | --- |
| `use of unsafe` 未加 `unsafe` | edition 2024 更严格 | 包在 `unsafe {}` 中 |
| `extern block must be unsafe` | 2024 新规则 | `unsafe extern "C" { ... }` |
| `#[no_mangle]` 报错 | 2024 要求 unsafe attribute | `#[unsafe(no_mangle)]` |
| 链接错误 `undefined reference` | 没链接库 | `build.rs` / `-l` |
| 运行时崩溃/乱码 | ABI/布局不一致 | `repr(C)`、检查类型大小 |
| `free(): invalid pointer` | 分配/释放方式不匹配 | 成对使用同一分配器 |
| `panic in extern "C"` | 未捕获 | `catch_unwind` 或 `panic=abort` |
| 数据竞争 | 手动 `unsafe impl Sync` 错误 | 不要轻易手写，用 `Mutex` |

---

## 9. 练习

1. 写一个 `extern "C" fn rust_strlen(*const c_char) -> usize`，从 C 调用（可用 `cc` 编译测试程序）。
2. 用 `bindgen` 为一个简单 C 头文件生成绑定，并封装成 safe API。
3. 用 `cbindgen` 为 `unsafe_ffi` 生成头文件，对比手写声明。
4. 实现 `fn add_arrays(a: &[f32], b: &[f32]) -> Vec<f32>` 的 unsafe SIMD 版本（可选，需 nightly）。
5. 实现一个 C 回调接口：Rust 传闭包 + user data 给 C，C 调用后返回结果，注意生命周期与 panic。
6. 用 Miri 运行 `unsafe_ffi` 的示例，观察它能否检测出越界/悬垂问题。

---

## 延伸阅读

- The Book 第 19 章 — <https://doc.rust-lang.org/book/ch19-01-unsafe-rust.html>
- Rustonomicon — <https://doc.rust-lang.org/nomicon/>
- `bindgen` — <https://rust-lang.github.io/rust-bindgen/>
- `cbindgen` — <https://github.com/mozilla/cbindgen>
- `cxx` — <https://cxx.rs/>
- Rust FFI Omnibus — <https://jakegoulding.com/rust-ffi-omnibus/>

上一章：[16 · OOP 与模式匹配](16-oop-patterns.md) ·
下一章：[18 · Web Server 项目](18-web-server.md)
