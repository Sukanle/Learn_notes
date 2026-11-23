//! 集成测试：直接运行编译好的二进制，验证 CLI 行为。
//!
//! `CARGO_BIN_EXE_minigrep` 由 Cargo 在集成测试中自动注入，
//! 避免手写 `target/debug/...` 路径。

use std::process::Command;

#[test]
fn cli_finds_matching_lines() {
    let output = Command::new(env!("CARGO_BIN_EXE_minigrep"))
        .args(["the", "poem.txt"])
        .output()
        .expect("failed to run minigrep");

    assert!(output.status.success(), "exit status: {}", output.status);

    let stdout = String::from_utf8(output.stdout).expect("stdout is not UTF-8");
    assert!(stdout.contains("Then there's a pair of us - don't tell!"));
    assert!(stdout.contains("To tell your name the livelong day"));
}

#[test]
fn cli_rejects_missing_arguments() {
    let output = Command::new(env!("CARGO_BIN_EXE_minigrep"))
        .output()
        .expect("failed to run minigrep");

    assert!(!output.status.success());
    let stderr = String::from_utf8(output.stderr).expect("stderr is not UTF-8");
    assert!(stderr.contains("Problem parsing arguments"));
}

#[test]
fn cli_reports_missing_file() {
    let output = Command::new(env!("CARGO_BIN_EXE_minigrep"))
        .args(["the", "no-such-file.txt"])
        .output()
        .expect("failed to run minigrep");

    assert!(!output.status.success());
    let stderr = String::from_utf8(output.stderr).expect("stderr is not UTF-8");
    assert!(stderr.contains("Application error"));
}
