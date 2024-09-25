# CMake基础

## 构建一个基本项目

* 项目源文件：
  * [main.cpp](demo/main.cpp) - 包含main函数的源文件
  * [hello.h](demo/hello.h) - 包含`hello`函数声明的头文件
  * [hello.cpp](demo/hello.cpp) - 包含`hello`函数定义的源文件

* 编写一个最基本的`CMakeLists.txt`文件：
  1. 在项目根目录下创建一个`CMakeLists.txt`文件
  2. 任何一个`CMake`项目都必须在最顶层使用`cmake_minimum_required`命令指定`CMake`的最低版本要求，这会确保编写的`CMake`函数使用兼容版本的`CMake`运行。
  3. 使用`project`命令指定项目的名称、版本，（这个变量名的意义不大，不需要特别关注，但一定要有这个命令存在）
  4. 使用`add_executable`命令定义一个可执行文件，该命令的第一个参数是可执行文件的名称，后面的参数是可执行文件的源文件列表。<br>
  以下是一个最基本的`CMakeLists.txt`文件示例：
  ```cmake
  # cmake 最低版本要求
  cmake_minimum_required(VERSION 3.10)
  # 项目信息
  project(Demo)
  # 指定生成目标
  add_executable(Demo main.cpp hello.cpp)
  ```
  5. 在项目根目录下创建一个`build`目录，用于存放编译生成的中间文件和最终生成的可执行文件。
  6. 在`build`目录下执行`cmake ..`命令，`CMake`会在`build`目录下生成相应的构建系统文件。
  7. 在`build`目录下执行`make`命令，`make`会根据`CMake`生成的构建系统文件编译源文件，生成可执行文件。
  8. 在`build`目录下执行`./Demo`命令，运行生成的可执行文件。

## 设置C++标准
* `CMake`跟`Makefile`一样，可以设置编译器的编译选项，比如编译标准，当然这些选项跟`Makefile`的自动变量一样，是一种特殊的变量，如果不设置，`CMake`会使用默认的编译选项，许多变量以`CMAKE_`开头，表示`CMake`的内置变量，比如`CMAKE_CXX_STANDARD`表示`C++`的编译标准，`CMAKE_CXX_FLAGS`表示`C++`的编译选项。
* 以先前的`CMakeLists.txt`为例，如果要设置`C++`的编译标准为`C++11`，可以在`CMakeLists.txt`中添加如下代码：
  ```cmake
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  ```
  这样就设置了`C++`的编译标准为`C++11`，并且强制使用这个标准，当然记得在指定生成目标之前设置这个变量，还有记得重新为项目添加一些`C++11`的特性，这里我使用了`auto`关键字。

## 添加版本号和配置的头文件
* `CMake`可以通过`configure_file`命令将一个文件中的内容替换为`CMake`变量的值，这样可以在源文件中使用这些变量，比如在源文件中输出版本号。
* 在`CMakeLists.txt`中添加如下代码：
  ```cmake
  # 生成编译时间
  string(TIMESTAMP BUILD_TIME "%Y-%m-%d")
  # 设置版本号
  set(VERSION_MAJOR 0)
  set(VERSION_MINOR 1)
  set(VERSION_PATCH 0)
  set(VERSION_BUILD ${BUILD_TIME})
  set(VERSION_TYPE "Release")
  # 配置一个头文件，将CMake变量传递到源文件
  configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
  )
  ```
  ```cpp
  // config.h.in
  # 版本号
  #define VERSION "@VERSION@"
  # 主版本号，当底层代码有重大变动时，主版本号+1
  #define VERSION_MAJOR "@VERSION_MAJOR@"
  # 次版本号，当业务逻辑有较大变动并且不兼容之前版本时，次版本号+1
  #define VERSION_MINOR "@VERSION_MINOR@"
  # 修订版本号，当有较小的改动或者bug修复时，修订版本号+1
  #define VERSION_PATCH "@VERSION_PATCH@"
  # 编译版本号，以时间戳为准
  #define VERSION_BUILD "@VERSION_BUILD@"
  # 发布版本类型：base（框架）、Alpha（内测）、Beta（公测）、RC（候选）、Release（正式）
  #define VERSION_TYPE "@VERSION_TYPE@"
  ```
  这样就将`config.h.in`中的内容替换为`CMake`变量的值，然后将替换后的内容写入到`config.h`中，这样就可以在源文件中包含`config.h`文件，使用`CMake`变量的值，当然我们更常用的是在可执行文件的文件名中添加版本号，这样可以区分不同版本的可执行文件。

## 总结
* 通过这章的学习，我们学会了如何构建一个基本的`CMake`项目，如何设置`C++`的编译标准，如何添加版本号和配置的头文件，这些都是`CMake`项目的基础，下一章我们将学习如何添加库文件，以及如何添加子目录。
>[返回目录](./README.md)<br>
>[下一章](./chapter02.md)
