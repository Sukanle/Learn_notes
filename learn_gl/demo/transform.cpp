// Deps: {{type = "share", name = "skl-gl", cache = true, priority = 1, deps = null}}
#include "skl/shader.hpp"
#include "skl/texture.hpp"
#include "skl/utils.hpp"

#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

constexpr const GLsizei WIDTH = 800;
constexpr const GLsizei HEIGHT = 600;
namespace gl = skl::opengl;

GLfloat mixValue = 0.2F;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        mixValue += 0.01F;
        mixValue = mixValue >= 1.0F ? 1.0F : mixValue;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        mixValue -= 0.01F;
        mixValue = mixValue <= 0.0F ? 0.0F : mixValue;
    }
}

void frameBufferSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

#define ERR(cond, ...)                      \
    if (cond) {                             \
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
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "transform", nullptr, nullptr);
    ERR(!window, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error: Failed to initization GLAD.\n");

    GLfloat vertices[] = {
        0.5F,  0.5F,  0.0F, 1.0F, 1.0F,   // top right
        0.5F,  -0.5F, 0.0F, 1.0F, 0.0F,   // bottom right
        -0.5F, -0.5F, 0.0F, 0.0F, 0.0F,   // bottom left
        -0.5F, 0.5F,  0.0F, 0.0F, 1.0F    // top left
    };

    GLint indices[] = {
        0, 1, 3,   // top right
        1, 2, 3    // bottom left
    };

    uint32_t VAO = 0;
    uint32_t VBO = 0;
    uint32_t EBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, skl::BufferOffset<GLfloat>(3));
    glEnableVertexAttribArray(1);

    std::string cpath(std::filesystem::current_path().string());

    std::array<gl::texture_t, 2> text_arr{};
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
            GLsizei format = GL_RGB + (texture.nrChannels - 3);
            glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE,
                         texture.data);
            stbi_image_free(texture.data);
        } else
            (void)fprintf(stderr, "Failed to lead texture (id: %d, path: %s).\n", texture.ID, texture.path.c_str());
    }
    {
        std::string vert(cpath + "/shader/transform.vert");
        std::string frag(cpath + "/shader/transform.frag");
        gl::Shader shader(vert.c_str(), frag.c_str());

        shader.use();
        shader.set1I("texture0", 0);
        shader.set1I("texture1", 1);

        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT);

            for (uint32_t i = 0; i < text_arr.size(); ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, text_arr[i].ID);
            }

            glm::mat4 tran(1.0F);
            tran = glm::translate(tran, glm::vec3(0.5F, -0.5F, 0.0F));
            tran = glm::rotate(tran, static_cast<float>(glfwGetTime()), glm::vec3(0.0F, 0.0F, 1.0F));

            shader.use();
            glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "transform"), 1, GL_FALSE, glm::value_ptr(tran));
            shader.set1F("mixValue", mixValue);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

            tran = glm::mat4(1.0F);
            tran = glm::translate(tran, glm::vec3(-0.5F, 0.5F, 0.0F));
            tran = glm::scale(tran, glm::vec3(glm::abs(glm::sin(static_cast<float>(glfwGetTime())))));

            glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "transform"), 1, GL_FALSE, glm::value_ptr(tran));
            shader.set1F("mixValue", mixValue);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        glfwTerminate();
        return 0;
    }
}
