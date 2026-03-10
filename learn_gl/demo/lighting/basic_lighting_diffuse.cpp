// Deps: {{type = "share", name = "skl-gl", priority = 1, cache = true, deps = null}

#include "skl/camera.hpp"
#include "skl/shader.hpp"
#include "skl/utils.hpp"

#include <array>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

#define ERR(conf, ...)                      \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        goto ERR_CTX_FREE;                  \
    }
namespace gl = skl::opengl;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr glm::vec3 lightPos(1.2F, 1.0F, 2.0F);

GLsizei gFbWidth = WIDTH;
GLsizei gFbHeight = HEIGHT;

GLfloat deltaTime = 0.0F;
GLfloat lastFrame = 0.0F;
GLfloat lastX = WIDTH / 2.0F;
GLfloat lastY = HEIGHT / 2.0F;
GLboolean is_first = GL_TRUE;

gl::Camera camera(glm::vec3(0.0F, 0.0F, 3.0F));

void setFramebufferSize(GLFWwindow *ctx, GLsizei width, GLsizei height);
void setCursorPos(GLFWwindow *ctx, GLdouble xpos, GLdouble ypos);
void setScroll(GLFWwindow *ctx, GLdouble xoffset, GLdouble yoffset);
void processInput(GLFWwindow *ctx);

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif   // __APPLE__

    GLFWwindow *ctx = glfwCreateWindow(WIDTH, HEIGHT, "basic_lighting_diffuse", nullptr, nullptr);
    ERR(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, setFramebufferSize);
    glfwSetCursorPosCallback(ctx, setCursorPos);
    glfwSetScrollCallback(ctx, setScroll);
    glfwSetInputMode(ctx, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwGetFramebufferSize(ctx, &gFbWidth, &gFbHeight);

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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, skl::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, skl::BufferOffset<GLfloat>(3));
        glEnableVertexAttribArray(1);

        glBindVertexArray(lightCubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, skl::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);

        {
            glEnable(GL_DEPTH_TEST);
            std::string cpath(std::filesystem::current_path().string());

            std::string vert = cpath + "/shader/lighting/basic_lighting.vert";
            std::string frag = cpath + "/shader/lighting/basic_lighting.frag";
            gl::Shader lightingShader(vert.c_str(), frag.c_str());

            vert = cpath + "/shader/lighting/light_cube.vert";
            frag = cpath + "/shader/lighting/light_cube.frag";
            gl::Shader lightCubeShader(vert.c_str(), frag.c_str());
            if (!lightingShader.getSuccess() || !lightCubeShader.getSuccess()) goto ERR_MESH_FREE;

            while (!glfwWindowShouldClose(ctx)) {
                GLfloat cFrame = (GLfloat)glfwGetTime();
                deltaTime = cFrame - lastFrame;
                lastFrame = cFrame;

                processInput(ctx);
                glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glm::mat4 proj(
                    glm::perspective(glm::radians(camera.getZoom()), (GLfloat)gFbWidth / (GLfloat)gFbHeight, 0.1F, 100.0F));
                glm::mat4 view(camera.getView());
                glm::mat4 model(1.0F);

#define RESLOVE_3V(vec) (vec).x, (vec).y, (vec).z

                lightingShader.use();
                lightingShader.set3F("objectColor", 1.0F, 0.5F, 0.31F);
                lightingShader.set3F("lightColor", 1.0F, 1.0F, 1.0F);
                lightingShader.set3F("lightPos", RESLOVE_3V(lightPos));
                lightingShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
                lightingShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
                lightingShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

                glBindVertexArray(cubeVAO);
                glDrawElements(GL_TRIANGLES, indices.size() * indices[0].size(), GL_UNSIGNED_BYTE, nullptr);

                model = glm::translate(glm::mat4(1.0F), lightPos);
                model = glm::scale(model, glm::vec3(0.2F));

                lightCubeShader.use();
                lightCubeShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
                lightCubeShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
                lightCubeShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

                glBindVertexArray(lightCubeVAO);
                glDrawElements(GL_TRIANGLES, indices.size() * indices[0].size(), GL_UNSIGNED_BYTE, nullptr);

                glfwSwapBuffers(ctx);
                glfwPollEvents();
            }
        }
    ERR_MESH_FREE:
        glDisable(GL_DEPTH_TEST);
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteVertexArrays(1, &lightCubeVAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }


ERR_CTX_FREE:
    glfwDestroyWindow(ctx);
    glfwTerminate();
    return 0;
}
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.keyboard(gl::Camera::FRONT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.keyboard(gl::Camera::BACK, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.keyboard(gl::Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.keyboard(gl::Camera::RIGHT, deltaTime);
}

void setFramebufferSize([[maybe_unused]] GLFWwindow *window, GLsizei width, GLsizei height) {
    gFbWidth = width;
    gFbHeight = height > 0 ? height : 1;
    glViewport(0, 0, width, height);
}


void setCursorPos([[maybe_unused]] GLFWwindow *window, GLdouble xposIn, GLdouble yposIn) {
    auto xpos = (GLfloat)xposIn;
    auto ypos = (GLfloat)yposIn;

    if (is_first) {
        lastX = xpos;
        lastY = ypos;
        is_first = GL_FALSE;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.mouseMove(xoffset, yoffset);
}

void setScroll([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] GLdouble xoffset, GLdouble yoffset) {
    camera.mouseScroll((GLfloat)yoffset);
}
