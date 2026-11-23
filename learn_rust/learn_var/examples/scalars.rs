//! 第 3.2 节：Scalar Types（整数、浮点、bool、char）。
//!
//! C++ 对比：
//! - 整数宽度是固定的（i8/i16/i32/i64/i128/isize），不存在 `int` 宽度依赖平台的问题；
//! - 默认整数是 i32，默认浮点是 f64；
//! - debug 构建下整数溢出会 panic；release 下回绕（与 C++ 无符号一致，但有定义）；
//! - char 是 4 字节 Unicode Scalar Value，不是 C++ 的 1 字节 `char`。

fn main() {
    // 类型后缀与下划线分隔
    let decimal = 98_222;
    let hex = 0xff;
    let octal = 0o77;
    let binary = 0b1111_0000;
    let byte = b'A'; // u8

    println!("decimal={decimal}, hex={hex}, octal={octal}, binary={binary}, byte={byte}");

    // 显式类型
    let x: i8 = -128;
    let y: u8 = 255;
    let big: i128 = 170_141_183_460_469_231_731_687_303_715_884_105_727;
    println!("i8 min={x}, u8 max={y}, i128 max={big}");

    // 溢出：debug 下 panic，release 下回绕
    let mut counter: u8 = 255;
    // counter += 1; // debug: attempt to add with overflow
    counter = counter.wrapping_add(1);
    println!("wrapping_add: {counter}");
    println!("checked_add: {:?}", 255u8.checked_add(1));
    println!("saturating_add: {}", 255u8.saturating_add(1));

    // 浮点：f32 / f64；没有隐式整数转换
    let pi: f64 = 3.141_592_653_589_793;
    let ratio = 1.0_f32 / 3.0;
    println!("pi={pi}, ratio={ratio:.6}");
    // let z: i32 = 3.0; // error[E0308]: mismatched types
    let z = 3.0_f64 as i32; // 显式 cast 才允许
    println!("cast 3.0 -> {z}");

    // bool 与 char
    let flag: bool = true;
    let zh = '中';
    let emoji = '🦀';
    println!("flag={flag}, zh={zh}, emoji={emoji}, char size={}", std::mem::size_of::<char>());

    // shadowing：可以改变类型（不同于 C++ 的重复定义）
    let spaces = "   ";
    let spaces = spaces.len();
    println!("spaces = {spaces}");

    // const 在编译期求值；static 有固定地址
    const MAX_POINTS: u32 = 100_000;
    static GREETING: &str = "hello";
    println!("MAX_POINTS={MAX_POINTS}, GREETING={GREETING}");
}
