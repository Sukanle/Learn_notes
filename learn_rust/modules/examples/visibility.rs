//! 可见性规则速查（示例中所有“错误”写法都以注释保留）。

mod outer {
    pub mod inner {
        pub fn public_api() {
            println!("inner::public_api");
        }

        pub(crate) fn crate_only() {
            println!("inner::crate_only (pub(crate))");
        }

        fn private() {
            println!("inner::private");
        }

        pub fn calls_private() {
            private(); // 同一模块内可见
        }
    }
}

fn main() {
    outer::inner::public_api();
    outer::inner::crate_only(); // 同 crate 内可见
    outer::inner::calls_private();

    // outer::inner::private(); // error[E0603]: function `private` is private
}
