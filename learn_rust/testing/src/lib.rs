//! 第 11 章：Writing Automated Tests。
//!
//! C++ 对比：
//! - `#[test]` ≈ GoogleTest 的 `TEST()` / Catch2 的 `TEST_CASE`；
//! - `cargo test` 自动发现 `#[test]`、`tests/` 下的集成测试以及文档测试；
//! - `assert_eq!` 失败时会打印两边的 Debug 值；
//! - 单元测试与被测代码放在同一文件，可访问私有项（类似 friend / 同翻译单元）；
//! - `#[should_panic]` ≈ `EXPECT_THROW` / `REQUIRE_THROWS`，但只能匹配 panic 消息。

/// 单元测试可以测试私有函数（同一模块内可见）。
#[cfg(test)]
fn internal_adder(left: i32, right: i32) -> i32 {
    left + right
}

pub fn add(left: u64, right: u64) -> u64 {
    left + right
}

pub fn greeting(name: &str) -> String {
    format!("Hello, {name}!")
}

pub struct Guess {
    value: i32,
}

impl Guess {
    /// 构造时校验范围；非法输入直接 panic（契约式编程）。
    pub fn new(value: i32) -> Self {
        if !(1..=100).contains(&value) {
            panic!("Guess value must be between 1 and 100, got {value}.");
        }
        Self { value }
    }

    pub fn value(&self) -> i32 {
        self.value
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }

    #[test]
    fn can_call_private_function() {
        assert_eq!(internal_adder(2, 3), 5);
    }

    #[test]
    fn larger_can_hold_smaller() {
        let larger = (8u32, 7u32);
        let smaller = (5u32, 1u32);
        assert!(
            larger.0 > smaller.0 && larger.1 > smaller.1,
            "larger = {larger:?}, smaller = {smaller:?}"
        );
    }

    #[test]
    fn greeting_contains_name() {
        let result = greeting("Carol");
        assert!(
            result.contains("Carol"),
            "Greeting did not contain name, value was `{result}`"
        );
    }

    #[test]
    #[should_panic(expected = "between 1 and 100")]
    fn greater_than_100() {
        Guess::new(200);
    }

    #[test]
    #[ignore = "耗时较长，只在 cargo test -- --ignored 时运行"]
    fn expensive_test() {
        std::thread::sleep(std::time::Duration::from_millis(10));
        assert_eq!(add(1, 1), 2);
    }

    // 返回 Result 的测试：用 ? 传播错误，失败时给出 Err 的 Debug 信息
    #[test]
    fn result_style_test() -> Result<(), String> {
        if add(2, 2) == 4 {
            Ok(())
        } else {
            Err(String::from("two plus two did not equal four"))
        }
    }

    #[test]
    fn assert_ne_demo() {
        assert_ne!(add(1, 2), 4);
    }
}
