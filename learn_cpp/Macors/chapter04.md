# `Token`粘贴

* 为了避免`FOO_`受`##`直接调用导致的直接展开，通过间接（多加一层调用）就能克服这种问题。

```cpp
    #define FOO_ 0
    #define FOO_1 1
    #define PRIMITIVE_CAT(x, y) x ## y
    #define CAT(x, y) PRIMITIVE_CAT(x, y)

    CAT(FOO_, 1)
    -> PRIMITIVE_CAT(0, 1)
    -> 01
```

# 括号表达式

* 将参数用括号包含使用，这样使用`func-like`的宏不接括号不会被展开的特性可以完成一些有意思的东西。比如：

```cpp
    #define EXPAND_IF_PAREN(x) EXPAND x
    #define EAT(x)

    EXPAND_IF_PAREN((12))
    -> EAT(12)
    ->

    EXPAND_IF_PAREN(12)
    -> EAT 12
```

* 这个例子中如果参数带括号就会返回一个空。当然这个例子是没有什么意义的，但是在下边的例子中你会发现这一技
  巧将被反复的使用。

# 模式匹配

* 通过`Token`粘贴我们能够动态的创建不同的宏定义。即预先定义一个宏定义的结构，与`C++`中的模板匹配正好不谋而合。
  这样就可以实现类似`C++ 17`中的`if constexpr`，不同的是，这个`if`是在预处理进行条件判断。

```cpp
    #define IIF(c) PRIMITIVE_CAT(IIF_, c)
    #define IIF_1(t, ...) t
    #define IIF_0(t, ...) __VA_ARGS__

    #define IF_TRUE() printf("In true barnch")
    #define IF_FALSE() printf("In false barnch")

    IIF(cond) ( \
        IF_TRUE(), \
        IIF_FALSE() \
    )
```

* 整个过程相当于构造了不同的宏名，如果为`c`值为0就动态生成`IIF_0`，反之亦然。相当于变相决定了宏的展开方向。此时
  如果`cond`等于1则执行第一个参数，如果等于0则执行第二个参数。注意这里用了`PRIMITIVE_CAT`这个宏，因为我们一般希
  望参数能够完全展开。

# 检测

* 检测给了我们这样一种能力，检测某个参数是否是特定的值。众所周知，在宏的基本语法中是不存在`if`这种东西的（宏不
  是预处理的`#if`）。但是根据不同的参数展现出不同的行为又是一个很常见的需求。因此，拥有判断某个参数是否是特定
  值的能力会给我们带来极大的便利。

```cpp
#define GET_SEC(x, n, ...) n
#define CHECK(...) GET_SEC(__VA_ARGS__, 0)
#define PROBE(x) x, 1

// 检测参数是否为空
#define IS_EMPTY(x) CHECK(CAT(PRIMITIVE_CAT(IS_EMPTY_, x), 0))
#define IS_EMPTY_0 PROBE()
IS_EMPTY()
-> CHECK(IS_EMPTY_0)
-> CHECK(PROBE())
-> 1
IS_EMPTY(param)
-> CHECK(IS_EMPTY_param_0)
-> 0

// 检测参数是否是括号表达式
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE()
IS_PAREN(())
-> CHECK(IS_PAREN_PROBE ())
-> CHECK(PROBE())
-> 1
IS_PAREN(xxx)
-> CHECK(IS_PAREN_PROBE xxx)
-> 0

// 实现取反（C 语言中除了0取反是1其他的取反都是0）
#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE()
NOT(das)
-> 0
NOT(0)
-> 1

// 结合这个我们就可以实现预处理的条件判断
#define BOOL(x) COMPL(NOT(x))
#define IF(c) IIF(BOOL(c))
IF(1)
-> 1
IF(0)
-> 0
IF(12)
-> 1
```
