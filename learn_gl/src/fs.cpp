#include "skl/fs.hpp"

#ifdef __cplusplus
extern "C" {
namespace skl::fs {
#endif
char* exepath(char* buffer, size_t buffer_size, const char* filename) {
    if (!buffer || buffer_size == 0) { return NULL; }

    char exe_path[PATH_MAX];
    size_t exe_len = 0;

#ifdef _WIN32
    // Windows: 获取当前可执行文件路径
    DWORD len = GetModuleFileNameA(NULL, exe_path, PATH_MAX);
    if (len == 0 || len >= PATH_MAX) { return NULL; }
    exe_len = len;

#elif defined(__APPLE__)
    // macOS: 使用_NSGetExecutablePath
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(exe_path, &size) != 0) { return NULL; }
    exe_len = strlen(exe_path);

#elif defined(__linux__) || defined(__unix__)
    // Linux/Unix: 通过/proc文件系统获取
    ssize_t len = readlink("/proc/self/exe", exe_path, PATH_MAX - 1);
    if (len <= 0) {
        // 备用方案：使用argv[0]（需要在main中保存）
        if (getcwd(exe_path, PATH_MAX) != NULL) {
            strcat(exe_path, "/a.out");   // 默认可执行文件名
            exe_len = strlen(exe_path);
        } else {
            return NULL;
        }
    } else {
        exe_path[len] = '\0';
        exe_len = len;
    }
#else
    // 其他平台：使用当前工作目录
    if (getcwd(exe_path, PATH_MAX) == NULL) { return NULL; }
    exe_len = strlen(exe_path);
    // 移除目录部分，只保留路径
    char* last_slash = strrchr(exe_path, '/');
    if (last_slash) { *last_slash = '\0'; }
#endif

    // 提取目录部分（移除可执行文件名）
    char exe_dir[PATH_MAX];
#ifdef _WIN32
    char* last_backslash = strrchr(exe_path, '\\');
    char* last_forward_slash = strrchr(exe_path, '/');
    char* last_slash = (last_backslash > last_forward_slash) ? last_backslash : last_forward_slash;
#else
    char* last_slash = strrchr(exe_path, '/');
#endif

    if (last_slash) {
        size_t dir_len = last_slash - exe_path;
        if (dir_len >= PATH_MAX) dir_len = PATH_MAX - 1;
        strncpy(exe_dir, exe_path, dir_len);
        exe_dir[dir_len] = '\0';
    } else {
        // 没有目录分隔符，使用当前目录
        exe_dir[0] = '.';
        exe_dir[1] = '\0';
    }

    // 构建最终路径
    if (filename == NULL) {
        // 获取当前程序路径
        if (strlen(exe_path) >= buffer_size) {
            return NULL;   // 缓冲区不足
        }
        strncpy(buffer, exe_path, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    } else {
        // 构建目录+文件名路径
        size_t dir_len = strlen(exe_dir);
        size_t file_len = strlen(filename);

        if (dir_len + 1 + file_len + 1 > buffer_size) {
            return NULL;   // 缓冲区不足
        }

        strcpy(buffer, exe_dir);

// 添加路径分隔符
#ifdef _WIN32
        if (dir_len > 0 && buffer[dir_len - 1] != '\\' && buffer[dir_len - 1] != '/') { strcat(buffer, "\\"); }
#else
        if (dir_len > 0 && buffer[dir_len - 1] != '/') { strcat(buffer, "/"); }
#endif

        strcat(buffer, filename);
    }

    return buffer;
}

char* exedir(char* buffer, size_t buffer_size, const char* filename) {
    return exepath(buffer, buffer_size, filename);
}
#ifdef __cplusplus
}   // namespace skl::fs
}
#endif
