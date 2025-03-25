# *`文件操作系统`*
* 注意，文件操作系统的头文件`filesystem`头文件在`C++`中属于`C++17`标准头，测试宏定义为：`__cpp_lib_filsystem`(201703L)。
* 文件操作系统提供在文件系统与其组件，如路径（相对或绝对）、常规文件与目录上进行的设施。
* 以下所有类、函数均在名为`std::filesystem`的命名空间内。

| 特性 | C++17 std::filesystem | Python | C/C++ I/O |
|:---:|:---------------------:|:------:|:---------:|
| 简洁 | 高                     | 高      | 低         |
| 迁移 | 好                     | 好      | 差         |
| 效率 | 高                     | 低      | 高         |
* `C++ filesystem`库提供了一系列用于文件和目录操作的API，包括但不限于创建、删除、复制文件和目录，以及查询文件属性等。
> "The function of good software is to make the complex appear to be simple." - Grady Booch
* 这个库就像是一个多功能瑞士军刀，无论你需要进行哪种文件操作，它都能为你提供方便。

## *`文件操作基础`*
* 为了方便表示，作者默认使用`namspace fs = std::filesystem`这个语法，下文出现的`fs`均代指`std::filesystem`。
### 创建文件
* 在`C++`中，使用`fs::create_directory`和`fs::create_directories`「标准文件操作库中的方法」可以轻松创建文件。
```cpp
#include <filesystem>
namspace fs = std::filesystem;
int main(){
    fs::create_directories("new_dir/new_file");
    fs::create_directory("another_file");
    return 0;
}
```
