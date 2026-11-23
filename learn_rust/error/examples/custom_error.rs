//! 自定义错误类型：实现 `Display` + `Error` + `From`，即可无缝使用 `?`。
//!
//! C++ 对比：
//! - `Result<T, E>` ≈ `std::expected<T, E>`（C++23）；
//! - `Box<dyn Error>` ≈ `std::exception_ptr` / 类型擦除的异常；
//! - `From<E>` 配合 `?` ≈ 在 catch 处做异常转换，但转换发生在每个 `?` 点且是显式的。

use std::error::Error;
use std::fmt;
use std::fs;
use std::io;

#[derive(Debug)]
enum AppError {
    Io(io::Error),
    EmptyFile,
    InvalidNumber(String),
}

impl fmt::Display for AppError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            AppError::Io(err) => write!(f, "I/O error: {err}"),
            AppError::EmptyFile => write!(f, "file is empty"),
            AppError::InvalidNumber(text) => write!(f, "invalid number: {text:?}"),
        }
    }
}

impl Error for AppError {
    // source() 用于错误链，类似 std::exception 的嵌套 cause
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            AppError::Io(err) => Some(err),
            _ => None,
        }
    }
}

// 让 `?` 自动把 io::Error 转成 AppError
impl From<io::Error> for AppError {
    fn from(err: io::Error) -> Self {
        AppError::Io(err)
    }
}

fn sum_first_line(path: &str) -> Result<i64, AppError> {
    let contents = fs::read_to_string(path)?;
    let line = contents.lines().next().ok_or(AppError::EmptyFile)?;
    let numbers: Result<Vec<i64>, _> = line
        .split_whitespace()
        .map(|word| word.parse::<i64>())
        .collect();

    // 把 ParseIntError 映射成带上下文的自定义错误
    let numbers = numbers.map_err(|_| AppError::InvalidNumber(line.to_string()))?;
    Ok(numbers.iter().sum())
}

fn main() -> Result<(), Box<dyn Error>> {
    println!("sum(hello.txt) = {:?}", sum_first_line("hello.txt"));
    println!("sum(不存在) = {:?}", sum_first_line("no-such-file.txt"));

    // main 返回 Result 时，Err 会打印 Debug 并设置退出码
    Ok(())
}
