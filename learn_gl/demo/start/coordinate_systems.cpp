// Deps: {{type = "share", name = "skl-gl", cache = true, priority = 1, deps = null}}

#include "skl/shader.hpp"
#include "skl/texture.hpp"
#include "skl/utils.hpp"

#include "stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <array>
#include <filesystem>

namespace gl = skl::opengl;
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

void processInput(GLFWwindow* ctx) {
    if (glfwGetKey(ctx, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(ctx, GL_TRUE);
}

void FrameBufferSize(GLFWwindow* ctx, int width, int height) {
    (void)ctx;
    glViewport(0, 0, width, height);
}
#define ERR(conf, ...)                      \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        glfwTerminate();                    \
        return EXIT_FAILURE;                \
    }
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif   // __APPLE__

    GLFWwindow* ctx = glfwCreateWindow(WIDTH, HEIGHT, "coordinate_systems", nullptr, nullptr);
    ERR(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, FrameBufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error: Faild to initization GLAD.\n");

    glEnable(GL_DEPTH_TEST);

    // clang-format off
    GLfloat vectices[] = {
        // positions          // texture coords
        -0.5F, -0.5F, -0.5F,  0.0F, 0.0F, 
        0.5F, -0.5F, -0.5F,  1.0F, 0.0F,
        0.5F,  0.5F, -0.5F,  1.0F, 1.0F,
        0.5F,  0.5F, -0.5F,  1.0F, 1.0F,
        -0.5F,  0.5F, -0.5F,  0.0F, 1.0F,
        -0.5F, -0.5F, -0.5F,  0.0F, 0.0F,

        -0.5F, -0.5F,  0.5F,  0.0F, 0.0F,
        0.5F, -0.5F,  0.5F,  1.0F, 0.0F,
        0.5F,  0.5F,  0.5F,  1.0F, 1.0F,
        0.5F,  0.5F,  0.5F,  1.0F, 1.0F,
        -0.5F,  0.5F,  0.5F,  0.0F, 1.0F,
        -0.5F, -0.5F,  0.5F,  0.0F, 0.0F,

        -0.5F,  0.5F,  0.5F,  1.0F, 0.0F,
        -0.5F,  0.5F, -0.5F,  1.0F, 1.0F,
        -0.5F, -0.5F, -0.5F,  0.0F, 1.0F,
        -0.5F, -0.5F, -0.5F,  0.0F, 1.0F,
        -0.5F, -0.5F,  0.5F,  0.0F, 0.0F,
        -0.5F,  0.5F,  0.5F,  1.0F, 0.0F,

        0.5F,  0.5F,  0.5F,  1.0F, 0.0F,
        0.5F,  0.5F, -0.5F,  1.0F, 1.0F,
        0.5F, -0.5F, -0.5F,  0.0F, 1.0F,
        0.5F, -0.5F, -0.5F,  0.0F, 1.0F,
        0.5F, -0.5F,  0.5F,  0.0F, 0.0F,
        0.5F,  0.5F,  0.5F,  1.0F, 0.0F,

        -0.5F, -0.5F, -0.5F,  0.0F, 1.0F,
        0.5F, -0.5F, -0.5F,  1.0F, 1.0F,
        0.5F, -0.5F,  0.5F,  1.0F, 0.0F,
        0.5F, -0.5F,  0.5F,  1.0F, 0.0F,
        -0.5F, -0.5F,  0.5F,  0.0F, 0.0F,
        -0.5F, -0.5F, -0.5F,  0.0F, 1.0F,

        -0.5F,  0.5F, -0.5F,  0.0F, 1.0F,
        0.5F,  0.5F, -0.5F,  1.0F, 1.0F,
        0.5F,  0.5F,  0.5F,  1.0F, 0.0F,
        0.5F,  0.5F,  0.5F,  1.0F, 0.0F,
        -0.5F,  0.5F,  0.5F,  0.0F, 0.0F,
        -0.5F,  0.5F, -0.5F,  0.0F, 1.0F
    };
    // clang-format on
    GLuint indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vectices), vectices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(3));
    glEnableVertexAttribArray(1);

    std::string cpath(std::filesystem::current_path().string());
    std::array<gl::texture_t, 2> text_arr;
    text_arr[0].path = cpath + "/resources/textures/container.jpg";
    text_arr[1].path = cpath + "/resources/textures/awesomeface.png";
    stbi_set_flip_vertically_on_load(GL_TRUE);

    for (auto& texture : text_arr) {
        glGenTextures(1, &texture.ID);
        glBindTexture(GL_TEXTURE_2D, texture.ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texture.data = stbi_load(texture.path.c_str(), &texture.width, &texture.height, &texture.nrChannels, 0);
        if (texture.data) {
            GLint format = GL_RGB + (texture.nrChannels - 3);
            glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE,
                         texture.data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(texture.data);
        } else
            (void)fprintf(stderr, "Failed to lead texture (id: %d, path: %s).\n", texture.ID, texture.path.c_str());
    }
    {
        std::string vert = cpath + "/shader/start/3D.vert";
        std::string frag = cpath + "/shader/start/3D.frag";
        gl::Shader shader(vert.c_str(), frag.c_str());
        shader.use();
        shader.set1I("texture0", 0);
        shader.set1I("texture1", 1);

        // clang-format off
        std::array cubePositions = {
            glm::vec3( 0.0F,  0.0F,  0.0F),
            glm::vec3( 2.0F,  5.0F, -15.0F),
            glm::vec3(-1.5F, -2.2F, -2.5F),
            glm::vec3(-3.8F, -2.0F, -12.3F),
            glm::vec3( 2.4F, -0.4F, -3.5F),
            glm::vec3(-1.7F,  3.0F, -7.5F),
            glm::vec3( 1.3F, -2.0F, -2.5F),
            glm::vec3( 1.5F,  2.0F, -2.5F),
            glm::vec3( 1.5F,  0.2F, -1.5F),
            glm::vec3(-1.3F,  1.0F, -1.5F)
        };
        // clang-format on

        while (!glfwWindowShouldClose(ctx)) {
            processInput(ctx);

            glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (uint32_t i = 0; i < text_arr.size(); ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, text_arr[i].ID);
            }

            shader.use();

            // glm::mat4 model(1.0F);
            glm::mat4 view(1.0F);
            glm::mat4 project(1.0F);
            // model = glm::rotate(model, glm::radians(-55.0F), glm::vec3(1.0F, 0.0F, 0.0F));
            // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0F), glm::vec3(0.5F, 1.0F, 0.0F));
            view = glm::translate(view, glm::vec3(0.0F, 0.0F, -3.0F));
            project = glm::perspective(glm::radians(45.0F), (float)WIDTH / (float)HEIGHT, 0.1F, 100.0F);

            // shader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));
            shader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
            shader.setMat4F("project", 1, GL_FALSE, glm::value_ptr(project));

            glBindVertexArray(VAO);
            for (uint32_t i = 0; i < cubePositions.size(); ++i) {
                glm::mat4 model(1.0F);
                model = glm::translate(model, cubePositions[i]);
                float angle = 20.0F * i;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0F, 0.3F, 0.5F));
                shader.setMat4F("model", 1, GL_FALSE,glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

            glfwSwapBuffers(ctx);
            glfwPollEvents();
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();

    return EXIT_SUCCESS;
}
