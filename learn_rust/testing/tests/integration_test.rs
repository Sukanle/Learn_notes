//! 集成测试：只能使用库 crate 的 public API，等价于“以外部用户视角测试”。
//! 每个 `tests/*.rs` 都会被当作独立 crate 编译。

use testing::{Guess, add, greeting};

#[test]
fn integration_add() {
    assert_eq!(add(10, 20), 30);
}

#[test]
fn integration_greeting() {
    assert!(greeting("Rust").starts_with("Hello"));
}

#[test]
#[should_panic(expected = "between 1 and 100")]
fn integration_guess_out_of_range() {
    Guess::new(0);
}
