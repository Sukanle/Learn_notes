//! 第 12 章：一个 I/O 项目 —— `minigrep`。
//!
//! 设计要点（对照 C++）：
//! - 配置解析返回 `Result<Config, &'static str>`，而不是抛异常 / 返回错误码；
//! - `run` 返回 `Result<(), Box<dyn Error>>`，可以承载任意实现了 Error 的错误类型；
//! - 搜索函数返回 `Vec<&str>`，切片借用输入 contents，零拷贝（类似 string_view）。
//!
//! 运行：
//! ```text
//! cargo run -- the poem.txt
//! IGNORE_CASE=1 cargo run -- the poem.txt
//! ```

use std::env;
use std::error::Error;
use std::fs;

pub struct Config {
    pub query: String,
    pub file_path: String,
    pub ignore_case: bool,
}

impl Config {
    /// 接收参数迭代器可以避免把 `Vec<String>` 整个收集起来。
    /// 第一个参数是可执行文件名，直接跳过。
    pub fn build(mut args: impl Iterator<Item = String>) -> Result<Config, &'static str> {
        args.next();

        let query = args.next().ok_or("Didn't get a query string")?;
        let file_path = args.next().ok_or("Didn't get a file path")?;

        let ignore_case = env::var("IGNORE_CASE").is_ok();

        Ok(Config {
            query,
            file_path,
            ignore_case,
        })
    }
}

/// 返回 `Box<dyn Error>`：调用者只关心“发生了错误”，不关心具体类型。
/// 类似 C++ 中返回 `std::exception_ptr`，但类型信息更强、可 downcast。
pub fn run(config: Config) -> Result<(), Box<dyn Error>> {
    let contents = fs::read_to_string(config.file_path)?;

    let results = if config.ignore_case {
        search_case_insensitive(&config.query, &contents)
    } else {
        search(&config.query, &contents)
    };

    for line in results {
        println!("{line}");
    }

    Ok(())
}

/// 显式标注生命周期：返回的每个 `&str` 都来自 `contents`，而不是 `query`。
pub fn search<'a>(query: &str, contents: &'a str) -> Vec<&'a str> {
    contents
        .lines()
        .filter(|line| line.contains(query))
        .collect()
}

pub fn search_case_insensitive<'a>(query: &str, contents: &'a str) -> Vec<&'a str> {
    let query = query.to_lowercase();
    contents
        .lines()
        .filter(|line| line.to_lowercase().contains(&query))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    const CONTENTS: &str = "\
Rust:
safe, fast, productive.
Pick three.
Trust me.";

    #[test]
    fn case_sensitive() {
        let query = "duct";
        assert_eq!(search(query, CONTENTS), vec!["safe, fast, productive."]);
    }

    #[test]
    fn case_insensitive() {
        let query = "rUsT";
        assert_eq!(
            search_case_insensitive(query, CONTENTS),
            vec!["Rust:", "Trust me."]
        );
    }

    #[test]
    fn no_match_returns_empty_vec() {
        assert!(search("not-there", CONTENTS).is_empty());
    }

    #[test]
    fn config_build_requires_two_args() {
        let args = vec![String::from("minigrep")];
        assert!(Config::build(args.into_iter()).is_err());
    }
}
