//! panic 的几种触发方式与捕获方式。
//!
//! C++ 对比：
//! - `panic!` ≈ `throw`，但默认策略是 unwind；也可以配置 `panic = "abort"`；
//! - panic 不是异常：不能按类型 catch 业务错误，业务错误应该用 Result；
//! - `catch_unwind` 类似 catch(...)，主要用于 FFI 边界 / 测试框架 / 线程隔离；
//! - Panic safety ≠ 内存安全：跨 FFI 边界 panic 是 UB（需要 catch 后转错误码）。

use std::panic;

fn main() {
    // 自定义 panic hook：类似 std::set_terminate / 全局异常处理器
    panic::set_hook(Box::new(|info| {
        eprintln!("[自定义 hook] {info}");
    }));

    let result = panic::catch_unwind(|| {
        let v = vec![1, 2, 4];
        println!("准备越界访问...");
        v[99] // panic: index out of bounds
    });

    println!("catch_unwind 捕获到 panic? {}", result.is_err());

    // 恢复默认 hook
    let _ = panic::take_hook();

    // 显式 panic
    let explicit = panic::catch_unwind(|| {
        panic!("something went wrong: code = {}", 42);
    });
    println!("显式 panic 被捕获? {}", explicit.is_err());

    // unwrap 在 None/Err 时 panic；生产代码中应优先用 match / ? / unwrap_or
    let ok: Option<i32> = Some(1);
    println!("unwrap = {}", ok.unwrap());
    println!("unwrap_or = {}", None.unwrap_or(0));

    // panic::catch_unwind 要求闭包是 UnwindSafe；捕获 &mut 引用时需 AssertUnwindSafe
    let mut counter = 0;
    let mut_ref = &mut counter;
    let caught = panic::catch_unwind(panic::AssertUnwindSafe(|| {
        *mut_ref += 1;
        panic!("state may be inconsistent");
    }));
    println!("状态可能不一致但仍被捕获? {}", caught.is_err());
}
