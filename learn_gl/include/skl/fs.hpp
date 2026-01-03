// xmake-type: Null
// xmake-deps: path = {"src"}, file = {{"fs.cpp"}} xmake-deps-end
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef __cplusplus
extern "C" {
#endif
WINBASEAPI DWORD WINAPI GetModuleFileNameA(HMODULE hModule, LPCH lpFilename, DWORD nSize);
#ifdef __cplusplus
}
#endif
#else
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define PATH_MAX 1024

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __cplusplus
extern "C" {
namespace skl::fs {
#endif
/**
 * 获取可执行文件路径
 * @param buffer 输出缓冲区，用于存储路径
 * @param buffer_size 缓冲区大小
 * @param filename 要附加的文件名，NULL表示获取当前程序路径
 * @return 成功时返回buffer，失败时返回NULL
 */
char* exepath(char* buffer, size_t buffer_size, const char* filename);

/**
 * 获取指定文件在可执行文件目录中的完整路径
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @param filename 要附加的文件名，NULL表示获取当前程序路径
 * @return 成功时返回buffer，失败时返回NULL
 */
char* exedir(char* buffer, size_t buffer_size, const char* filename);
#ifdef __cplusplus
}   // namespace skl::fs
}
#endif
