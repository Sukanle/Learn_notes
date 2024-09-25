# 认识CMake

## `CMake`是什么
* `CMake`是一个跨平台的构建工具，可以用简单的语法来描述所有平台的构建过程,如果您熟悉`Makefile`或刚学习完`Makefile`，那么您仍会感觉到`Makefile`的语法很复杂，而且需要为不同的平台编写不同的`Makefile`，而`CMake`的语法更加简单，而且可以生成不同平台的构建文件，比如`Makefile`、`Visual Studio`等。
* `CMake`是一个元构建系统，它不直接构建项目，而是生成构建项目所需的文件，比如`Makefile`、`Visual Studio`工程等，然后再使用这些文件来构建项目。

## `CMake`的安装
* `CMake`的官网是[https://cmake.org/](https://cmake.org/)，在这里可以下载到最新的`CMake`安装包，`CMake`的安装非常简单，只需要下载安装包，然后按照安装向导一步一步安装即可。
* 如果您使用的是`Linux`系统，那么可以使用`apt-get`或`yum`等包管理工具来安装`CMake`，比如在以`CentOS`为代表的`RedHat`系列系统上，可以使用如下命令来安装`CMake`：
  ```sh
  sudo yum install cmake #CentOS7及之前版本
  # 或者
  sudo dnf install cmake #CentOS8及CentOS Stream，Fedora等
  ```

## 入门'CMake'
* `CMake`跟`Makefile`一样，也是通过一个描述文件来描述项目的构建过程，这个描述文件就是`CMakeLists.txt`，`CMakeLists.txt`文件中包含了一系列的命令，这些命令用来描述项目的构建过程，比如添加源文件、添加头文件、添加库文件等。
### 注释
* `CMake`的注释跟`Makefile`一样，使用`#`号来注释，比如：
  ```cmake
  # 这是一个注释
  ```
* `CMake`的注释也可以使用`#[[]]`来注释一块区域，比如：
  ```cmake
  #[[
  这是一个注释块
  ]]
  ```

## 目录
1. [CMake基础](./chapter01.md)
