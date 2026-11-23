//! 泛型参数可以不止一个；也可以用 `where` 子句整理约束。

#[derive(Debug)]
struct Pair<T, U> {
    first: T,
    second: U,
}

impl<T, U> Pair<T, U> {
    fn new(first: T, second: U) -> Self {
        Self { first, second }
    }

    fn swap(self) -> Pair<U, T> {
        Pair {
            first: self.second,
            second: self.first,
        }
    }
}

// where 子句：约束较多时比 `<T: A + B + C>` 更易读
fn largest<T>(list: &[T]) -> Option<&T>
where
    T: PartialOrd,
{
    let mut max = list.first()?;
    for item in &list[1..] {
        if item > max {
            max = item;
        }
    }
    Some(max)
}

fn main() {
    let pair = Pair::new(1, "one");
    println!("pair = {pair:?}");
    println!("swap = {:?}", pair.swap());

    println!("largest = {:?}", largest(&[3, 9, 2, 7]));
    println!("largest = {:?}", largest(&["apple", "pear", "orange"]));
    println!("largest(empty) = {:?}", largest::<i32>(&[]));
}
