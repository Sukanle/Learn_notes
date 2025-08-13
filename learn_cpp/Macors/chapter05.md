# 完整的`if`语句

* 上节中，我们实现了一个`if`的宏定义`IF`，但是使用逗号作为`if-else`的分隔符，这节我们在介绍一个书写起来更加贴合
  `C/C++ if-else`语句实现：

```cpp
    #define IF_ELSE(condition) CAT(_IF_,  BOOL(condition))
    #define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
    #define _IF_0(...)             _IF_0_ELSE
    #define _IF_1_ELSE(...)
    #define _IF_0_ELSE(...) __VA_ARGS__
    IF_ELSE (0) (      \
        True branch    \
    )(                 \
        False branch   \
    )
    -> CAT(_IF_, 0) (True branch) (False branch)
    -> _IF_0 (True branch) (False branch)
    -> _IF_0_ELSE (False branch)
    -> False branch
    IF_ELSE (1) (      \
        True branch    \
    )(                 \
        False branch   \
    )
    -> CAT(_IF_, 1) (True branch) (False branch)
    -> _IF_1 (True branch) (False branch)
    -> True branch _IF_1_ELSE_ (False branch)
    -> True branch
```
* 新的`IF_ELSE`宏可以使用括号作为两个分支的分隔符，写起来也更加接近`C/C++`本身的写法。原理上来说，其实就是利用
  了宏会向后查找一个括号的特性。使得不同的分支会**吞掉**另外一个分支的内容。对于*「真分支」*来说`_IF_1_ELSE_ `会**吞
  掉***「假分支」*的展开过程。对于*「假分支」*来说直接跳过参数处理就可以了。

