//! 引座与排队。

/// `pub`：crate 外部可见。
pub fn add_to_waitlist() {
    println!("  hosting::add_to_waitlist");
}

/// `pub(crate)`：只在当前 crate 内可见，类似 C++ 的 internal linkage。
pub(crate) fn seat_at_table() {
    println!("  hosting::seat_at_table");
}
