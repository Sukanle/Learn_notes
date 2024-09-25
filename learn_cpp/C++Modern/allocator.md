# *`std::allocator`动态内存分配器*

## 何为`std::allocator`？
* `std::allocator`是一个动态内存分配器，用于分配和释放内存，它在`C++标准库`中被广泛使用，是`stl`容器的默认内存分配器。实际上，所有的`stl`容器的第二个参数缺省值都是`std::allocator`。
> `std::allocator`本质上是一个模板类, 定义在头文件`<memory>`中。

## 理解`std::allocator`
