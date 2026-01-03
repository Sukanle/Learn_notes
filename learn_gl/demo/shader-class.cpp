// xmake-type: Target
// xmake-deps: type = dynamic, path = {"default"}, lib = {{"shader"}} xmake-deps-end
#include "skl/shader.hpp"
#include <filesystem>
#include "skl/fs.hpp"
#include <string>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

void frameBufferSize([[maybe_unused]] GLFWwindow* window, GLsizei width,
                     GLsizei height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

#define ERR(cond, ...)                      \
    if (cond) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        glfwTerminate();                    \
        return 1;                           \
    }

namespace gl = skl::opengl;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window =
        glfwCreateWindow(WIDTH, HEIGHT, "shader-class", nullptr, nullptr);
    ERR(!window, "Error: Faild to Create GLFW window.\n");
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress),
        "Error: Faild to initization GLAD.\n");

    std::string pwd(std::filesystem::current_path());
    // std::string exe{};
    // skl::fs::exedir(exe.data(), )
    std::string vert = pwd + "/shader/test.vert";
    std::string frag = pwd + "/shader/test.frag";

    {
        gl::Shader shader(vert.data(), frag.data());
        if (!shader.getSuccess()) return 1;

        GLfloat vertices[] = {
            // positions         // colors
            0.5F,  -0.5F, 0.0F, 1.0F, 0.0F, 0.0F,   // bottom right
            -0.5F, -0.5F, 0.0F, 0.0F, 1.0F, 0.0F,   // bottom left
            0.0F,  0.5F,  0.0F, 0.0F, 0.0F, 1.0F    // top
        };
        uint32_t VAO = 0;
        uint32_t VBO = 0;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                              ((void*)0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6,
                              ((void*)(sizeof(GLfloat) * 3)));
        glEnableVertexAttribArray(1);

        shader.use();

        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    glfwTerminate();
    return 0;
}
