// Deps: {{type = "share", name = "skl-gl", priority = 1, cache = true, deps = null}

#include "skl/camera.hpp"
#include "skl/shader.hpp"
#include "skl/utils.hpp"

#include <array>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

namespace gl = skl::opengl;

void setFramebufferSize(GLFWwindow *ctx, GLsizei width, GLsizei height);
void setCursorPos(GLFWwindow *ctx, GLdouble xpos, GLdouble ypos);
void setScroll(GLFWwindow *ctx, GLdouble xoffset, GLdouble yoffset);
void processInput(GLFWwindow *ctx);

constexpr uint32_t WIDTH = 800U;
constexpr uint32_t HEIGHT = 600U;
constexpr glm::vec3 lightPos(1.2F, 1.0F, 2.0F);

static GLfloat lastX = WIDTH / 2.0F;
static GLfloat lastY = HEIGHT / 2.0F;
static GLfloat detlaTime = 0.0F;
static GLfloat lastFrame = 0.0F;
static GLboolean is_first = GL_TRUE;
static gl::Camera camera(glm::vec3(0.0F, 0.0F, 3.0F));

#define ERR(conf, ...)                      \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        goto ERR_CTX_FREE;                  \
    }

int main () {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif // __APPLE__

    GLFWwindow* ctx = glfwCreateWindow(WIDTH, HEIGHT, "basic_lighting_specular", nullptr, nullptr);
    ERR(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, setFramebufferSize);
    glfwSetCursorPosCallback(ctx, setCursorPos);
    glfwSetScrollCallback(ctx, setScroll);
    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error: Failed to initialize GLAD.\n");
    {
        // clang-format off
        std::array vertices {
            // Front (+Z)
            std::array{-0.5F, -0.5F,  0.5F,  0.0F,  0.0F,  1.0F}, // 0
            std::array{ 0.5F, -0.5F,  0.5F,  0.0F,  0.0F,  1.0F}, // 1
            std::array{ 0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F}, // 2
            std::array{-0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F}, // 3
            // Back (-Z)
            std::array{-0.5F, -0.5F, -0.5F,  0.0F,  0.0F, -1.0F}, // 4
            std::array{ 0.5F, -0.5F, -0.5F,  0.0F,  0.0F, -1.0F}, // 5
            std::array{ 0.5F,  0.5F, -0.5F,  0.0F,  0.0F, -1.0F}, // 6
            std::array{-0.5F,  0.5F, -0.5F,  0.0F,  0.0F, -1.0F}, // 7
            // Left (-X)
            std::array{-0.5F, -0.5F, -0.5F, -1.0F,  0.0F,  0.0F}, // 8
            std::array{-0.5F, -0.5F,  0.5F, -1.0F,  0.0F,  0.0F}, // 9
            std::array{-0.5F,  0.5F,  0.5F, -1.0F,  0.0F,  0.0F}, // 10
            std::array{-0.5F,  0.5F, -0.5F, -1.0F,  0.0F,  0.0F}, // 11
            // Right (+X)
            std::array{ 0.5F, -0.5F, -0.5F,  1.0F,  0.0F,  0.0F}, // 12
            std::array{ 0.5F, -0.5F,  0.5F,  1.0F,  0.0F,  0.0F}, // 13
            std::array{ 0.5F,  0.5F,  0.5F,  1.0F,  0.0F,  0.0F}, // 14
            std::array{ 0.5F,  0.5F, -0.5F,  1.0F,  0.0F,  0.0F}, // 15
            // Top (+Y)
            std::array{-0.5F,  0.5F, -0.5F,  0.0F,  1.0F,  0.0F}, // 16
            std::array{-0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F}, // 17
            std::array{ 0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F}, // 18
            std::array{ 0.5F,  0.5F, -0.5F,  0.0F,  1.0F,  0.0F}, // 19
            // Bottom (-Y)
            std::array{-0.5F, -0.5F, -0.5F,  0.0F, -1.0F,  0.0F}, // 20
            std::array{-0.5F, -0.5F,  0.5F,  0.0F, -1.0F,  0.0F}, // 21
            std::array{ 0.5F, -0.5F,  0.5F,  0.0F, -1.0F,  0.0F}, // 22
            std::array{ 0.5F, -0.5F, -0.5F,  0.0F, -1.0F,  0.0F}, // 23
        };
        std::array indices = {
            // Front
            std::array<uint8_t, 3>{0, 1, 3},
            std::array<uint8_t, 3>{1, 2, 3},
            // Back
            std::array<uint8_t, 3>{5, 4, 7},
            std::array<uint8_t, 3>{6, 5, 7},
            // Left
            std::array<uint8_t, 3>{8, 9, 11},
            std::array<uint8_t, 3>{9, 10, 11},
            // Right
            std::array<uint8_t, 3>{13, 12, 15},
            std::array<uint8_t, 3>{14, 13, 15},
            // Top
            std::array<uint8_t, 3>{16, 17, 19},
            std::array<uint8_t, 3>{17, 18, 19},
            // Bottom
            std::array<uint8_t, 3>{21, 20, 23},
            std::array<uint8_t, 3>{22, 21, 23},
        };
        // clang-format on
        uint32_t lightVAO = 0;
        uint32_t cubeVAO = 0;
        uint32_t VBO = 0;
        uint32_t EBO = 0;

        glGenVertexArrays(1, &lightVAO);
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, skl::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, skl::BufferOffset<GLfloat>(3));
        glEnableVertexAttribArray(1);

        glBindVertexArray(lightVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, skl::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);
    }

ERR_CTX_FREE:
    glfwDestroyWindow(ctx);
    glfwTerminate();

    return is_first ? EXIT_FAILURE : EXIT_SUCCESS;
}
