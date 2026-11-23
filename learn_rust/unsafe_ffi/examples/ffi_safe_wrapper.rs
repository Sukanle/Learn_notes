//! 用“薄封装”把 unsafe 隔离在边界内：外部 API 保持 safe。
//!
//! C++ 对比：类似给 C API 写 RAII wrapper（例如 unique_ptr + 自定义 deleter），
//! 让业务代码不需要接触裸指针和手动资源管理。

use std::ffi::{CString, c_char};
use std::os::raw::c_int;

unsafe extern "C" {
    fn strlen(s: *const c_char) -> usize;
    fn atoi(s: *const c_char) -> c_int;
}

/// 安全封装：保证传入的 Rust 字符串已正确 NUL 结尾，并处理内部 NUL 错误。
fn c_strlen(text: &str) -> Result<usize, std::ffi::NulError> {
    let c_text = CString::new(text)?;
    // SAFETY: CString 保证指针以 NUL 结尾且在本函数内有效
    Ok(unsafe { strlen(c_text.as_ptr()) })
}

fn c_atoi(text: &str) -> Result<i32, std::ffi::NulError> {
    let c_text = CString::new(text)?;
    // SAFETY: 指针有效且以 NUL 结尾；atoi 不要求 UTF-8
    Ok(unsafe { atoi(c_text.as_ptr()) })
}

fn main() {
    println!("strlen(\"hello\") = {:?}", c_strlen("hello"));
    println!("strlen(\"你好\") = {:?}（按字节计）", c_strlen("你好"));
    println!("atoi(\"42\") = {:?}", c_atoi("42"));
    println!("atoi(\"not a number\") = {:?}", c_atoi("not a number"));

    // 含内部 NUL 的字符串会被 CString 拒绝，而不是悄悄截断
    println!("含 NUL 的错误 = {:?}", c_strlen("a\0b"));
}
