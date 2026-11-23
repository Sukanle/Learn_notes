//! 用 iterator 管道处理文本：不写显式循环，也让意图更清晰。
//!
//! C++ 对比：更接近 C++20 `std::views` 管道；但 Rust 的 `collect` 目标类型
//! 决定最终容器，且所有权/借用规则保证管道不会悬挂。

use std::collections::BTreeMap;

const DATA: &str = "\
2024-01-01,42,cpu
2024-01-02,17,cpu
2024-01-03,99,mem
2024-01-04,23,mem
2024-01-05,61,io
2024-01-06,8,io
";

#[derive(Debug)]
struct Sample {
    date: &'static str,
    value: u32,
    kind: &'static str,
}

fn parse(line: &'static str) -> Option<Sample> {
    // split 返回 &str，parse 成功才构造 Sample
    let mut fields = line.split(',');
    let date = fields.next()?;
    let value = fields.next()?.parse().ok()?;
    let kind = fields.next()?;
    Some(Sample { date, value, kind })
}

fn main() {
    let samples: Vec<Sample> = DATA.lines().filter_map(parse).collect();
    println!("样本数 = {}", samples.len());

    // 按类别求和：fold 到 BTreeMap（有序，便于稳定输出）
    let totals = samples.iter().fold(BTreeMap::new(), |mut acc, s| {
        *acc.entry(s.kind).or_insert(0u32) += s.value;
        acc
    });
    println!("按类别汇总 = {totals:?}");

    // 过滤 + 映射 + 求均值
    let cpu_values: Vec<u32> = samples
        .iter()
        .filter(|s| s.kind == "cpu")
        .map(|s| s.value)
        .collect();
    let avg = cpu_values.iter().sum::<u32>() as f64 / cpu_values.len() as f64;
    println!("cpu 均值 = {avg:.2}");

    // 最大值与对应日期
    if let Some(max) = samples.iter().max_by_key(|s| s.value) {
        println!("峰值: {} = {}", max.date, max.value);
    }

    // 分组收集：HashMap<&str, Vec<&Sample>>
    let grouped: BTreeMap<&str, Vec<&Sample>> =
        samples.iter().fold(BTreeMap::new(), |mut acc, s| {
            acc.entry(s.kind).or_default().push(s);
            acc
        });
    for (kind, group) in grouped {
        let sum: u32 = group.iter().map(|s| s.value).sum();
        println!("{kind}: {} 条, 总计 {sum}", group.len());
    }

    // 一段式管道：所有 kind 去重并排序
    let mut kinds: Vec<&str> = samples.iter().map(|s| s.kind).collect();
    kinds.sort_unstable();
    kinds.dedup();
    println!("类别 = {kinds:?}");
}
