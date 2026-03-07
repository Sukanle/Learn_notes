// Deps: {{type = "share", name = "skl-gl", priority = 1, cache = true, deps = null}

#include "skl/camera.hpp"
#include "skl/shader.hpp"
#include "skl/utils.hpp"

#include "glm/gtc/type_ptr.hpp"

#include <array>
#include <filesystem>

namespace gl = skl::opengl;
gl::Camera camera(glm::vec3(0.0F, 0.0F, 3.0F));
constexpr uint32_t WIDHT = 800;
constexpr uint32_t HEIGHT = 800;
GLfloat lastX = WIDHT / 2.0F;
GLfloat lastY = HEIGHT / 2.0F;
GLboolean is_first = false;

float deltaTime = 0.0F;
float lastFrame = 0.0F;

constexpr glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

void setFrameBufferSize(GLFWwindow *ctx, GLsizei width, GLsizei height);
void setCursorPos(GLFWwindow *ctx, GLdouble xops, GLdouble ypos);
void setScroll(GLFWwindow *ctx, GLdouble xoffset, GLdouble yoffset);
void processInput(GLFWwindow *ctx);

#define ERR(conf, windows, ...)             \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        glfwDestroyWindow(windows);         \
        glfwTerminate();                    \
        return EXIT_FAILURE;                \
    }

int main() {

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif   // __APPLE__

    GLFWwindow *ctx = glfwCreateWindow(WIDHT, HEIGHT, "colors", nullptr, nullptr);
    ERR(!ctx, ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, setFrameBufferSize);
    glfwSetCursorPosCallback(ctx, setCursorPos);
    glfwSetScrollCallback(ctx, setScroll);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), ctx, "Error: Failed to initialize GLAD.\n");

    // clang-format off
    std::array vertices {
        // Front
        std::array{-0.5F, -0.5F,  0.5F}, // P-F-SW
        std::array{ 0.5F, -0.5F,  0.5F}, // P-F-SE
        std::array{ 0.5F,  0.5F,  0.5F}, // P-F-NE
        std::array{-0.5F,  0.5F,  0.5F}, // P-F-NW
        // Back
        std::array{-0.5F, -0.5F, -0.5F}, // P-F-SW
        std::array{ 0.5F, -0.5F, -0.5F}, // P-F-SE
        std::array{ 0.5F,  0.5F, -0.5F}, // P-F-NE
        std::array{-0.5F,  0.5F, -0.5F}, // P-F-NW
    };
    std::array indices = {
        // Front and back
        std::array{0, 1, 3}, // R-F-SW
        std::array{1, 2, 3}, // R-F-NE
        std::array{5, 4, 7}, // R-B-SE
        std::array{6, 5, 7}, // R-B-NE
        // Up and Down
        std::array{3, 2, 7}, // R-U-SW
        std::array{2, 6, 7}, // R-U-NE
        std::array{0, 5, 1}, // R-D-SW
        std::array{0, 4, 5}, // R-D-NE
        // Left and right
        std::array{4, 0, 7}, // R-L-SW
        std::array{0, 3, 7}, // R-L-NE
        std::array{1, 5, 2}, // R-R-SW
        std::array{5, 6, 2}, // R-R-NE
    };
    // clang-format on

    uint32_t cubeVAO = 0;
    uint32_t lightCubeVAO = 0;
    uint32_t VBO = 0;
    uint32_t EBO = 0;

    glGenVertexArrays(1, &cubeVAO);
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(0));
    glEnableVertexAttribArray(0);

    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), skl::BufferOffset<GLfloat>(0));
    glEnableVertexAttribArray(0);

    {
        glEnable(GL_DEPTH_TEST);
        std::string cpath(std::filesystem::current_path().string());

        std::string vert = cpath + "/shader/lighting/colors.vert";
        std::string frag = cpath + "/shader/lighting/colors.frag";
        gl::Shader lightingShader(vert.c_str(), frag.c_str());

        vert = cpath + "/shader/lighting/light_cube.vert";
        frag = cpath + "/shader/lighting/light_cube.frag";
        gl::Shader lightCubeShader(vert.c_str(), frag.c_str());

        while (!glfwWindowShouldClose(ctx)) {
            GLfloat cFrame = (GLfloat)glfwGetTime();
            deltaTime = cFrame - lastFrame;
            lastFrame = cFrame;

            processInput(ctx);
            glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 proj(
                glm::perspective(glm::radians(camera.getZoom()), (GLfloat)WIDHT / (GLfloat)HEIGHT, 0.1F, 100.0F));
            glm::mat4 view(camera.getView());
            glm::mat4 model(1.0F);

            lightingShader.use();
            lightingShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
            lightingShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
            lightingShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, indices.size() * indices[0].size(), GL_UNSIGNED_INT, nullptr);

            model = glm::translate(glm::mat4(1.0F), lightPos);
            model = glm::scale(model, glm::vec3(0.2F));

            lightCubeShader.use();
            lightCubeShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
            lightCubeShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
            lightCubeShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(lightCubeVAO);
            glDrawElements(GL_TRIANGLES, indices.size() * indices[0].size(), GL_UNSIGNED_INT, nullptr);

            glfwSwapBuffers(ctx);
            glfwPollEvents();
        }
    }
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwDestroyWindow(ctx);
    glfwTerminate();
    return EXIT_SUCCESS;
}
void setFrameBufferSize([[maybe_unused]] GLFWwindow *ctx, GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);
}
void setCursorPos([[maybe_unused]] GLFWwindow *ctx, GLdouble Inxpos, GLdouble Inypos) {
    GLfloat xpos = (float)Inxpos;
    GLfloat ypos = (float)Inypos;

    if (is_first) {
        lastX = xpos;
        lastY = ypos;
        is_first = GL_FALSE;
    }

    GLfloat xoffset = lastX - xpos;
    GLfloat yoffset = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    camera.mouseMove(xoffset, yoffset);
}
void setScroll([[maybe_unused]] GLFWwindow *ctx, [[maybe_unused]] GLdouble xoffset, GLdouble yoffset) {
    camera.mouseScroll((float)yoffset);
}
void processInput(GLFWwindow *ctx) {
    if (glfwGetKey(ctx, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(ctx, GL_TRUE);

    if (glfwGetKey(ctx, GLFW_KEY_W) == GLFW_PRESS) camera.keyboard(gl::Camera::FRONT, deltaTime);
    if (glfwGetKey(ctx, GLFW_KEY_S) == GLFW_PRESS) camera.keyboard(gl::Camera::BACK, deltaTime);
    if (glfwGetKey(ctx, GLFW_KEY_A) == GLFW_PRESS) camera.keyboard(gl::Camera::LEFT, deltaTime);
    if (glfwGetKey(ctx, GLFW_KEY_D) == GLFW_PRESS) camera.keyboard(gl::Camera::RIGHT, deltaTime);
}
