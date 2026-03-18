// Deps: {{type = "share", name = "skl-gl", priortiy = 1, cache = true, deps = null}}

#include "skl/graphics/gl/shader.hpp"
#include "skl/graphics/gl/texture.hpp"
#include "skl/utils.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <filesystem>

constexpr size_t WIDTH = 800ULL;
constexpr size_t HEIGHT = 600ULL;

float deltaTime = 0.0F;
float lastFrame = 0.0F;

glm::vec3 cameraPos(0.0F, 0.0F, 3.0F);
glm::vec3 cameraFront(0.0F, 0.0F, -1.0F);
glm::vec3 cameraUp(0.0F, 1.0F, 0.0F);

#define ERR(conf, ...)                      \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        glfwTerminate();                    \
        return EXIT_FAILURE;                \
    }

void processInput(GLFWwindow *ctx) noexcept;
void setFrameBufferSize(GLFWwindow *ctx, GLsizei width, GLsizei height) noexcept;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif   // __APPLE__

    GLFWwindow *ctx = glfwCreateWindow(WIDTH, HEIGHT, "camera_keyboard_dt", nullptr, nullptr);
    ERR(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, setFrameBufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error: Failed to initialize GLAD.\n");

    // clang-format off
    std::array vertices {
        // Front
        std::array{-0.5F, -0.5F,  0.5F, 0.0F, 0.0F}, // P-F-SW
        std::array{ 0.5F, -0.5F,  0.5F, 1.0F, 0.0F}, // P-F-SE
        std::array{ 0.5F,  0.5F,  0.5F, 1.0F, 1.0F}, // P-F-NE
        std::array{-0.5F,  0.5F,  0.5F, 0.0F, 1.0F}, // P-F-NW
        // Back
        std::array{-0.5F, -0.5F, -0.5F, 0.0F, 0.0F}, // P-F-SW
        std::array{ 0.5F, -0.5F, -0.5F, 1.0F, 0.0F}, // P-F-SE
        std::array{ 0.5F,  0.5F, -0.5F, 1.0F, 1.0F}, // P-F-NE
        std::array{-0.5F,  0.5F, -0.5F, 0.0F, 1.0F}, // P-F-NW
    };
    std::array indices = {
        // Front and back
        std::array{0U, 1U, 3U}, // R-F-SW
        std::array{1U, 2U, 3U}, // R-F-NE
        std::array{5U, 4U, 7U}, // R-B-SE
        std::array{6U, 5U, 7U}, // R-B-NE
        // Up and Down
        std::array{3U, 2U, 7U}, // R-U-SW
        std::array{2U, 6U, 7U}, // R-U-NE
        std::array{0U, 5U, 1U}, // R-D-SW
        std::array{0U, 4U, 5U}, // R-D-NE
        // Left and right
        std::array{4U, 0U, 7U}, // R-L-SW
        std::array{0U, 3U, 7U}, // R-L-NE
        std::array{1U, 5U, 2U}, // R-R-SW
        std::array{5U, 6U, 2U}, // R-R-NE
    };
    // clang-format on

    uint32_t VAO = 0;
    uint32_t VBO = 0;
    uint32_t EBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(3));
    glEnableVertexAttribArray(1);

    namespace gl = skl::opengl;
    std::array<gl::texture_t, 2> textures;
    std::string cpath(std::filesystem::current_path().string());
    textures[0].path = cpath + "/resources/textures/container.jpg";
    textures[1].path = cpath + "/resources/textures/awesomeface.png";

    for (auto &texture : textures) {
        glGenTextures(1, &texture.ID);
        glBindTexture(GL_TEXTURE_2D, texture.ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        texture.data = stbi_load(texture.path.c_str(), &texture.width, &texture.height, &texture.nrChannels, 0);
        if (texture.data) {
            GLint format = (texture.nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE,
                         texture.data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(texture.data);
        } else
            (void)fprintf(stderr, "Failed to load texture[id = %d](path = %s).\n", texture.ID, texture.path.c_str());
    }

    {
        std::string vert = cpath + "/shader/start/3D.vert";
        std::string frag = cpath + "/shader/start/3D.frag";
        std::error_code ec;
        gl::Shader shader;
        shader.build(ec, vert.c_str(), frag.c_str());
        ERR(ec, "Error: [shader] ID: %d\n; Msg: %s\n", ec.value(), ec.message().c_str());
        shader.use();
        shader.set1I("texture0", 0);
        shader.set1I("texture1", 1);

        // clang-format off
        std::array cubePositions = {
            glm::vec3( 0.0F,  0.0F,  0.0F),
            glm::vec3( 2.0F,  5.0F, -15.0F),
            glm::vec3(-1.5F, -2.2F, -2.5F),
            glm::vec3(-3.8F, -2.0F, -12.3F),
            glm::vec3 (2.4F, -0.4F, -3.5F),
            glm::vec3(-1.7F,  3.0F, -7.5F),
            glm::vec3( 1.3F, -2.0F, -2.5F),
            glm::vec3( 1.5F,  2.0F, -2.5F),
            glm::vec3( 1.5F,  0.2F, -1.5F),
            glm::vec3(-1.3F,  1.0F, -1.5F),
        };
        // clang-format on
        glm::mat4 proj = glm::perspective(glm::radians(45.0F), (float)WIDTH / (float)HEIGHT, 0.1F, 100.0F);
        shader.setMat4F("project", 1, GL_FALSE, glm::value_ptr(proj));

        glEnable(GL_DEPTH_TEST);
        while (!glfwWindowShouldClose(ctx)) {
            float currentFrame = (float)glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            processInput(ctx);

            glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            for (size_t i = 0; i < textures.size(); ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textures[i].ID);
            }

            shader.use();

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            shader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));

            glBindVertexArray(VAO);
            for (size_t i = 0; i < cubePositions.size(); ++i) {
                glm::mat4 model = glm::translate(glm::mat4(1.0F), cubePositions[i]);
                model = glm::rotate(model, glm::radians(20.0F * i), glm::vec3(1.0F, 0.3F, 0.5F));
                shader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            }

            glfwSwapBuffers(ctx);
            glfwPollEvents();
        }
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    for (auto &tex : textures)
        if (tex.ID) glDeleteTextures(1, &tex.ID);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *ctx) noexcept {
    if (glfwGetKey(ctx, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(ctx, GL_TRUE);

    float cameraSpeed = 2.5F * deltaTime;
    if (glfwGetKey(ctx, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(ctx, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(ctx, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(ctx, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void setFrameBufferSize([[maybe_unused]] GLFWwindow *ctx, GLsizei width, GLsizei height) noexcept {
    glViewport(0, 0, width, height);
}
