//! 第 17 章：用 trait object 实现 State pattern。
//!
//! C++ 里常见写法是 `Context` 持有 `std::unique_ptr<State>`，状态对象调用
//! `context` 的接口完成切换。Rust 中借用检查器不允许“通过 &mut self 的状态
//! 方法再可变借用整个 Post”，因此用 `Option<Box<dyn State>>::take()` 把状态
//! 临时移出，再放回新状态。

trait State {
    // `self: Box<Self>` 取得状态对象的所有权，天然支持状态转移
    fn request_review(self: Box<Self>) -> Box<dyn State>;
    fn approve(self: Box<Self>) -> Box<dyn State>;

    // 默认实现：只有 Published 状态返回内容
    fn content<'a>(&self, _post: &'a Post) -> &'a str {
        ""
    }
}

struct Draft;

impl State for Draft {
    fn request_review(self: Box<Self>) -> Box<dyn State> {
        Box::new(PendingReview { approvals: 0 })
    }

    fn approve(self: Box<Self>) -> Box<dyn State> {
        self // 草稿不能直接发布
    }
}

struct PendingReview {
    approvals: u8,
}

impl State for PendingReview {
    fn request_review(self: Box<Self>) -> Box<dyn State> {
        self // 已在审核中，忽略重复请求
    }

    fn approve(self: Box<Self>) -> Box<dyn State> {
        if self.approvals + 1 >= 2 {
            Box::new(Published)
        } else {
            Box::new(PendingReview {
                approvals: self.approvals + 1,
            })
        }
    }
}

struct Published;

impl State for Published {
    fn request_review(self: Box<Self>) -> Box<dyn State> {
        self
    }

    fn approve(self: Box<Self>) -> Box<dyn State> {
        self
    }

    fn content<'a>(&self, post: &'a Post) -> &'a str {
        &post.content
    }
}

pub struct Post {
    state: Option<Box<dyn State>>,
    content: String,
}

impl Post {
    pub fn new() -> Post {
        Post {
            state: Some(Box::new(Draft)),
            content: String::new(),
        }
    }

    pub fn add_text(&mut self, text: &str) {
        // 只有 Draft 允许编辑；这里通过状态查询可以做得更严格
        if self.state.as_ref().unwrap().content(self).is_empty() {
            self.content.push_str(text);
        }
    }

    pub fn content(&self) -> &str {
        self.state.as_ref().unwrap().content(self)
    }

    pub fn request_review(&mut self) {
        if let Some(state) = self.state.take() {
            self.state = Some(state.request_review());
        }
    }

    pub fn approve(&mut self) {
        if let Some(state) = self.state.take() {
            self.state = Some(state.approve());
        }
    }
}

impl Default for Post {
    fn default() -> Self {
        Self::new()
    }
}

fn main() {
    let mut post = Post::new();
    post.add_text("I ate a salad for lunch today");
    assert_eq!(post.content(), "");

    post.request_review();
    assert_eq!(post.content(), "");
    println!("pending 状态下 content 为空: {:?}", post.content());

    post.approve(); // 第一次批准：仍在审核
    assert_eq!(post.content(), "");
    println!("第一次批准后仍不可见");

    post.approve(); // 第二次批准：发布
    println!("发布后 content = {:?}", post.content());
    assert_eq!(post.content(), "I ate a salad for lunch today");
}
