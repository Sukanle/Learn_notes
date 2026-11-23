//! 综合练习：统计词频并输出 Top N。
//! 这里只使用 std，避免引入任何第三方依赖。

use std::collections::HashMap;

const TEXT: &str = r#"
Rust is a systems programming language.
Rust focuses on safety, speed, and concurrency.
C++ is a systems programming language too.
Safety and speed matter, but safety is not optional in Rust.
"#;

fn main() {
    let mut counts: HashMap<String, usize> = HashMap::new();

    for word in TEXT.split_whitespace() {
        // 去掉标点并转小写；`trim_matches` 接收字符集合
        let cleaned: String = word
            .trim_matches(|c: char| !c.is_alphanumeric())
            .to_lowercase();
        if cleaned.is_empty() {
            continue;
        }
        *counts.entry(cleaned).or_insert(0) += 1;
    }

    let mut ranking: Vec<(String, usize)> = counts.into_iter().collect();
    ranking.sort_by(|a, b| b.1.cmp(&a.1).then_with(|| a.0.cmp(&b.0)));

    println!("共 {} 个不同单词", ranking.len());
    for (rank, (word, count)) in ranking.iter().take(5).enumerate() {
        println!("{:2}. {:12} {}", rank + 1, word, count);
    }
}
