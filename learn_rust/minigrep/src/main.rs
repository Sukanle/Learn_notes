//! `minigrep` 的二进制入口：只负责解析参数、报告错误与退出码。

use std::env;
use std::process;

use minigrep::Config;

fn main() {
    // build 的错误是 &'static str；用 unwrap_or_else 做“失败即退出”
    let config = Config::build(env::args()).unwrap_or_else(|err| {
        eprintln!("Problem parsing arguments: {err}");
        process::exit(1);
    });

    println!("Searching for {}", config.query);
    println!("In file {}", config.file_path);

    if let Err(err) = minigrep::run(config) {
        eprintln!("Application error: {err}");
        process::exit(1);
    }
}
