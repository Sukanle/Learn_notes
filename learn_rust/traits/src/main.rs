use std::fmt::Display;

struct Pair<T> {
    pub first: T,
    pub second: T,
}

impl<T> Pair<T> {
    fn new(first: T, second: T) -> Self {
        return Self { first, second };
    }
}

impl<T: Display + PartialOrd> Pair<T> {
    fn cmp_display(&self) {
        if self.first > self.second {
            println!("The largest member is first = {}", self.first);
        } else if self.first < self.second {
            println!("The largest member is second = {}", self.second);
        } else {
            println!("The member is same");
        }
    }
}

fn largest<T: PartialOrd>(list: &[T]) -> &T {
    let mut largest = &list[0];

    for item in list {
        if item > largest {
            largest = item;
        }
    }

    return largest;
}

fn main() {
    let number_list = vec![34, 50, 25, 100, 65];

    let result = largest(&number_list);
    println!("The largest number is {}", result);

    let char_list = vec!['y', 'm', 'a', 'q'];

    let result = largest(&char_list);
    println!("The largest char is {}", result);

    let mut p = Pair::new(32, 34);
    p.cmp_display();
    p.first = p.second;
    p.cmp_display();
}
