//! 第 19-20 章：Unsafe Rust 与 FFI（和 C/C++ 互操作）。
//!
//! C++ 对比：
//! - `unsafe` 不会关闭借用检查，只允许五类“编译器无法验证”的操作：
//!   解引用裸指针、调用 unsafe 函数、访问/修改可变 static、实现 unsafe trait、访问 union 字段；
//! - 裸指针 `*const T` / `*mut T` ≈ `const T*` / `T*`，但可以为 null、可以悬垂；
//! - `extern "C"` ≈ `extern "C"` + `__attribute__((cdecl))`，ABI 兼容 C；
//! - C++ 侧调用 Rust 需要 `#[unsafe(no_mangle)] pub extern "C"`（edition 2024 新语法）。

use std::slice;

fn main() {
    raw_pointers();
    split_at_mut_demo();
    call_c_function();
    exported_to_c();
}

fn raw_pointers() {
    println!("== 1. Raw pointers ==");

    let mut num = 5;

    // 创建裸指针是 safe 的；解引用需要 unsafe
    // edition 2024 推荐 `&raw const` / `&raw mut`，避免先创建引用再转型
    let r1 = &raw const num;
    let r2 = &raw mut num;

    unsafe {
        println!("r1 = {}", *r1);
        *r2 += 1; // 通过裸指针修改
    }
    println!("num 现在是 {num}");

    // 任意地址转裸指针——只是数值，不解引用就安全
    let address = 0x0123_4567usize;
    let r = address as *const i32;
    println!("伪造的地址指针 = {r:?}（未解引用）");
}

/// 用裸指针实现 `split_at_mut`：这是标准库内部的真实思路。
/// 编译器无法证明两个切片不重叠，因此需要 unsafe + 文档化的安全契约。
fn split_at_mut(values: &mut [i32], mid: usize) -> (&mut [i32], &mut [i32]) {
    let len = values.len();
    let ptr = values.as_mut_ptr();
    assert!(mid <= len);

    unsafe {
        (
            slice::from_raw_parts_mut(ptr, mid),
            slice::from_raw_parts_mut(ptr.add(mid), len - mid),
        )
    }
}

fn split_at_mut_demo() {
    println!("== 2. Safe abstraction over unsafe ==");

    let mut v = vec![1, 2, 3, 4, 5, 6];
    let (left, right) = split_at_mut(&mut v, 3);
    left[0] = 10;
    right[0] = 40;
    println!("left = {left:?}, right = {right:?}");

    // 对比：直接对同一个 Vec 同时借用可变切片会被借用检查器拒绝
    // let (a, b) = (&mut v[..3], &mut v[3..]); // error[E0499]
}

// 声明外部 C 函数；edition 2024 起 extern block 本身必须写 unsafe
unsafe extern "C" {
    fn abs(input: i32) -> i32;
}

fn call_c_function() {
    println!("== 3. Call C from Rust ==");

    let value = -42;
    // 调用外部函数是 unsafe：编译器无法校验参数/返回值契约
    let result = unsafe { abs(value) };
    println!("C abs({value}) = {result}");
}

/// 导出给 C/C++ 调用的函数。
/// 对应 C 头文件声明：`int32_t rust_add(int32_t a, int32_t b);`
#[unsafe(no_mangle)]
pub extern "C" fn rust_add(a: i32, b: i32) -> i32 {
    a + b
}

fn exported_to_c() {
    println!("== 4. Export Rust to C ==");

    // 在 Rust 内部也可以直接调用（extern "C" 函数是 safe 的）
    println!("rust_add(2, 3) = {}", rust_add(2, 3));
    println!("符号名 rust_add 可供 C/C++ 链接");
}
