cmake_minimum_required(VERSION 3.00)
project(learn_asio VERSION 0.1 LANGUAGES CXX C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCURL REQUIRED libssl)
pkg_check_modules(LIBCURL REQUIRED libcrypto)
pkg_check_modules(LIBCURL REQUIRED fmt)

set(CMAKE_C_COMPILER "gcc" CACHE STRING "C compiler" FORCE)
set(CMAKE_C_COMPILER "g++" CACHE STRING "C++ compiler" FORCE)

if(DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "${CMAKE_C_COMPILER}" CACHE STRING "C compiler" FORCE)
endif()
if(DEFINED CMAKE_C_COMPILER)
    set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE STRING "C++ compiler" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    link_libraries(stdc++)
endif

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-variable")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIB_OUT ${CMAKE_SOURCE_DIR}/lib64)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(${LIBFMT_INCLUDE_DIRS})

set(LIB_DIR_SHARED ${LIB_OUT}/shared)
set(LIB_DIR_STATIC ${LIB_OUT}/static)

file(GLOB SOURCES ${SRC_DIR}/*\.cpp)

# add_library(learn_asio SHARED ${SOURCES})
# set_target_properties(learn_asio PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR_SHARED})

# file(GLOB test
