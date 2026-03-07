#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// 尝试创建指定版本的 OpenGL 上下文
GLFWwindow* try_create_context(int major, int minor) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS 需要

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test", NULL, NULL);
    if (window) {
        glfwMakeContextCurrent(window);
        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return window;
        }
        glfwMakeContextCurrent(NULL);
        glfwDestroyWindow(window);
    }
    return NULL;
}

// 探测最高支持的 OpenGL 版本
void detect_max_opengl_version(int* out_major, int* out_minor) {
    // 常见版本组合（从高到低）
    const int versions[][2] = {
        {4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1},
        {4, 0}, {3, 3}, {3, 2}, {3, 1}, {3, 0}
    };

    for (size_t i = 0; i < sizeof(versions)/sizeof(versions[0]); i++) {
        GLFWwindow* test_window = try_create_context(versions[i][0], versions[i][1]);
        if (test_window) {
            *out_major = versions[i][0];
            *out_minor = versions[i][1];
            glfwDestroyWindow(test_window);
            glfwMakeContextCurrent(NULL);
            return;
        }
    }

    // 如果都失败，尝试最低版本
    *out_major = 3;
    *out_minor = 0;
}

// 获取平台信息
void print_platform_info() {
    #ifdef _WIN32
        printf("平台: Windows\n");
    #elif defined(__APPLE__)
        printf("平台: macOS (OpenGL 最高支持 4.1)\n");
    #elif defined(__linux__)
        printf("平台: Linux\n");
    #else
        printf("平台: 未知\n");
    #endif
}

// 获取 GPU 厂商信息
const char* get_gpu_vendor_hint() {
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    if (strstr(renderer, "NVIDIA")) return "NVIDIA";
    if (strstr(renderer, "AMD") || strstr(renderer, "Radeon")) return "AMD";
    if (strstr(renderer, "Intel")) return "Intel";
    if (strstr(renderer, "Apple")) return "Apple Metal (via OpenGL)";
    return "未知";
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW 初始化失败\n");
        return -1;
    }

    print_platform_info();

    int max_major, max_minor;
    printf("\n正在探测最高支持的 OpenGL 版本...\n");
    detect_max_opengl_version(&max_major, &max_minor);
    printf("✓ 最高支持版本: OpenGL %d.%d\n\n", max_major, max_minor);

    // 使用探测到的最高版本创建正式上下文
    GLFWwindow* window = try_create_context(max_major, max_minor);
    if (!window) {
        fprintf(stderr, "无法创建 OpenGL 上下文\n");
        glfwTerminate();
        return -1;
    }

    // 输出详细信息
    printf("===== OpenGL 详细信息 =====\n");
    printf("GPU 厂商: %s\n", get_gpu_vendor_hint());
    printf("渲染器: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL 版本: %s\n", glGetString(GL_VERSION));
    printf("GLSL 版本: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("厂商: %s\n\n", glGetString(GL_VENDOR));

    // 扩展数量
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    printf("支持扩展数量: %d\n\n", num_extensions);

    // 关键特性检查
    const char* features[] = {
        "GL_ARB_compute_shader",
        "GL_ARB_texture_buffer_object",
        "GL_ARB_shader_storage_buffer_object",
        "GL_ARB_vertex_array_object",
        "GL_ARB_framebuffer_object",
        "GL_ARB_instanced_arrays",
        "GL_ARB_uniform_buffer_object",
        "GL_ARB_direct_state_access",
        "GL_ARB_bindless_texture",
        "GL_KHR_debug"
    };

    printf("===== 关键特性支持 =====\n");
    for (size_t i = 0; i < sizeof(features)/sizeof(features[0]); i++) {
        if (glfwExtensionSupported(features[i])) {
            printf("✓ %s\n", features[i]);
        } else {
            printf("✗ %s\n", features[i]);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
