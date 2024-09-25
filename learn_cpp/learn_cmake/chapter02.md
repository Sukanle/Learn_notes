# CMake的进阶使用

## 1. CMake的变量
* CMake的变量是用来存储数据的，可以是字符串、列表、布尔值等，与`Makefile`一样，`CMake`的变量也是用来方便编写`CMakeLists.txt`文件的。
* 下面是一些CMake的变量的基本操作：
  ```cmake
  set(MAIN "main.cpp")
  set(HELLO "hello.cpp")
  ```
* 上面的代码定义了两个变量`MAIN`和`HELLO`，分别存储了`main.cpp`和`hello.cpp`的文件名，这样在后面的代码中就可以直接使用这两个变量来表示文件的路径了。
