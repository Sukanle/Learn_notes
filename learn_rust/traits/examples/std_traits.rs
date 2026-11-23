//! 常用 std trait 与 C++ 惯用法对照：
//! - `Display` / `Debug` ≈ `operator<<` / 调试打印；
//! - `From` / `Into` ≈ 转换构造函数 / `operator T()`，且 `From` 自动提供 `Into`；
//! - `Default` ≈ 默认构造函数；
//! - `Clone` / `Copy` ≈ 拷贝构造 / 平凡拷贝；
//! - `PartialEq` / `Ord` ≈ `operator==` / `operator<=>`；
//! - `Add` ≈ `operator+`。

use std::fmt;
use std::ops::Add;
use std::str::FromStr;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Default)]
struct Version {
    major: u32,
    minor: u32,
    patch: u32,
}

impl Version {
    fn new(major: u32, minor: u32, patch: u32) -> Self {
        Self {
            major,
            minor,
            patch,
        }
    }
}

impl fmt::Display for Version {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}.{}.{}", self.major, self.minor, self.patch)
    }
}

impl From<(u32, u32, u32)> for Version {
    fn from((major, minor, patch): (u32, u32, u32)) -> Self {
        Self::new(major, minor, patch)
    }
}

impl FromStr for Version {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut parts = s.split('.');
        let major = parts.next().ok_or("missing major")?.parse().map_err(|_| "bad major")?;
        let minor = parts.next().ok_or("missing minor")?.parse().map_err(|_| "bad minor")?;
        let patch = parts.next().ok_or("missing patch")?.parse().map_err(|_| "bad patch")?;
        if parts.next().is_some() {
            return Err(String::from("too many components"));
        }
        Ok(Self::new(major, minor, patch))
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Default)]
struct Meters(f64);

impl Add for Meters {
    type Output = Meters;

    fn add(self, rhs: Self) -> Self::Output {
        Meters(self.0 + rhs.0)
    }
}

impl fmt::Display for Meters {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} m", self.0)
    }
}

fn main() {
    let a = Version::new(1, 2, 3);
    let b: Version = (1, 4, 0).into(); // From 自动提供 Into
    println!("a = {a}, b = {b}, debug = {a:?}");
    println!("a < b = {}", a < b); // Ord
    println!("default = {}", Version::default());

    let parsed: Version = "2.10.1".parse().expect("valid version");
    println!("parsed = {parsed}");
    println!("parse error = {:?}", "2.10".parse::<Version>());

    let total = Meters(1.5) + Meters(2.25);
    println!("total = {total}");

    // Clone / Copy
    let c = a;
    let d = a.clone();
    println!("copy = {c}, clone = {d}");
}
