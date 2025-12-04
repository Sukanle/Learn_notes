#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // 创建 OpenGL 上下文（无显式窗口）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // macOS 还需：
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(1, 1, "", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 获取 OpenGL 版本字符串
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* vendor   = (const char*)glGetString(GL_VENDOR);
    int nrAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);

    std::cout << "OpenGL Version: " << version << "\n";
    std::cout << "Renderer: " << renderer << "\n";
    std::cout << "Vendor: " << vendor << "\n";
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << "\n";

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
