// Deps: {{type = "share", name = "skl-gl", cache = true, priority = 1, deps = null}}
#include "skl/graphics/gl/shader.hpp"
#include "skl/graphics/gl/texture.hpp"
#include "skl/utils.hpp"

#include <array>
#include <filesystem>

#define ERR(cond, ...)                      \
    if (cond) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        glfwTerminate();                    \
        return 1;                           \
    }

namespace skl::opengl {}   // namespace skl::opengl

namespace gl = skl::opengl;

GLfloat mixValue = 0.2F;

void frameBufferSize(GLFWwindow *window, GLsizei width, GLsizei height) {
    (void)window;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
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

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "texture", nullptr, nullptr);
    ERR(!window, "Error: Failed to Create GLFW window.\n");
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error: Failed to initization GLAD.\n");

    GLfloat vertices[] = {
        0.5F,  0.5F,  0.0F, 1.0F, 0.0F,
        0.0F,  1.0F,  1.0F,   // top right
        0.5F,  -0.5F, 0.0F, 0.0F, 1.0F,
        0.0F,  1.0F,  0.0F,   // bottom right
        -0.5F, -0.5F, 0.0F, 0.0F, 0.0F,
        1.0F,  0.0F,  0.0F,   // bottom left
        -0.5F, 0.5F,  0.0F, 1.0F, 1.0F,
        0.0F,  0.0F,  1.0F   // top left
        // 0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.55f, 0.55f,   // top right
        // 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.55f, 0.45f,   // bottom right
        // -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.45f, 0.45f,   // bottom left
        // -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.45f, 0.55f    // top left
    };

    GLuint indices[] = {0, 1, 3, 1, 2, 3};

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::BufferOffset<GLfloat>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::BufferOffset<GLfloat>(3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::BufferOffset<GLfloat>(6));
    glEnableVertexAttribArray(2);

    std::string cpath(std::filesystem::current_path().string());

    std::array<gl::texture_t, 2> texture_arr;
    texture_arr[0].path = cpath + "/resources/textures/container.jpg";
    texture_arr[1].path = cpath + "/resources/textures/awesomeface.png";
    stbi_set_flip_vertically_on_load(GL_TRUE);

    for (auto &texture : texture_arr) {
        glGenTextures(1, &texture.ID);
        glBindTexture(GL_TEXTURE_2D, texture.ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texture.data = stbi_load(texture.path.c_str(), &texture.width, &texture.height, &texture.nrChannels, 0);
        if (texture.data) {
            GLint nrChannels = GL_RGB + (texture.nrChannels - 3);
            glTexImage2D(GL_TEXTURE_2D, 0, nrChannels, texture.width, texture.height, 0, nrChannels, GL_UNSIGNED_BYTE,
                         texture.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else
            (void)fprintf(stderr, "Failed to lead texture (id: %d, path: %s).\n", texture.ID, texture.path.c_str());

        stbi_image_free(texture.data);
    }

    {
        std::string vert = cpath + "/shader/start/texture.vert";
        std::string frag = cpath + "/shader/start/texture.frag";
        std::error_code ec;
        gl::Shader shader;
        shader.build(ec, vert.c_str(), frag.c_str());
        ERR(ec, "Error: [shader] ID: %d\n Msg: %s\n", ec.value(), ec.message().c_str());
        shader.use();
        glUniform1i(glGetUniformLocation(shader.getID(), "texture1"), 0);
        shader.set1I("texture2", 1);

        while (!glfwWindowShouldClose(window)) {

            processInput(window);

            glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT);

            for (uint32_t i = 0; i < texture_arr.size(); ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, texture_arr[i].ID);
            }

            shader.set1F("mixValue", mixValue);
            shader.use();
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
