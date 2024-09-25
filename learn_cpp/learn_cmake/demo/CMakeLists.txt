# cmake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目信息
project(Demo)

# 设置C++标准为C++11，以便于支持auto关键字
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成编译时间
string(TIMESTAMP BUILD_TIME "%Y-%m-%d")
# 设置版本号
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION_BUILD ${BUILD_TIME})
set(VERSION_TYPE "Release")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}_${VERSION_TYPE}")
# configure_file函数用于将CMake变量传递给源代码
configure_file(
"config.h.in"
"config.h"
)

# 指定生成目标
add_executable(Demo main.cpp hello.cpp)
