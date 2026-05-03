#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <libloaderapi.h>
#  ifdef __cplusplus
extern "C" {
#  endif
WINBASEAPI DWORD WINAPI GetModuleFileNameA(HMODULE hModule, LPCH lpFilename, DWORD nSize);
#  ifdef __cplusplus
}
#  endif
#else
#  include <libgen.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif


#ifdef __APPLE__
#  include <mach-o/dyld.h>
#endif

#include "skl/config.h"

#define NAMESPACE_FILESYSTEM_BEGIN \
    NAMESPACE_SKL_BEGIN            \
    namespace fs {
#define NAMESPACE_FILESYSTEM_END \
    NAMESPACE_SKL_END            \
    }

NAMESPACE_FILESYSTEM_BEGIN
constexpr size_t SKL_PATH_MAX = 1024ULL;
/**
 * 获取可执行文件路径
 * @param buffer 输出缓冲区，用于存储路径
 * @param buffer_size 缓冲区大小
 * @param filename 要附加的文件名，NULL表示获取当前程序路径
 * @return 成功时返回buffer，失败时返回NULL
 */
char *exepath(char *buffer, size_t buffer_size, const char *filename) noexcept;

/**
 * 获取指定文件在可执行文件目录中的完整路径
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功时返回buffer，失败时返回NULL
 */
char *exedir(char *buffer, size_t buffer_size) noexcept;
NAMESPACE_FILESYSTEM_END
