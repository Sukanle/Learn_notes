// Deps: {{type = "share", name = "skl-gl", priority = 1, cache = true, deps = null}}

#import "skl/graphics/gl/camera.hpp"
#import "skl/graphics/gl/shader.hpp"
#import "skl/graphics/gl/texture.hpp"
#include "skl/utils/base.hpp"

#include <array>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

#define ERR_FREE(conf, free, ...)           \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        goto ERR_##free##_FREE;             \
    }
#define ERR_CTX(conf, ...) ERR_FREE(conf, CTX, __VA_ARGS__)
#define ERR_EC(ec, free, type) \
    ERR_FREE(ec, free, "Error: [" type "]\nID: %d\nMsg: %s\n", ec.value(), ec.message().c_str())
#define ERR_TEX(ec) ERR_EC(ec, MESH, "texture")
#define ERR_SHD(ec) ERR_EC(ec, SHADER, "shader")

namespace gl = skl::graphics::opengl;

void setFramebufferSize(GLFWwindow *ctx, GLsizei width, GLsizei height) noexcept;
void setCursorPos(GLFWwindow *ctx, GLdouble xpos, GLdouble ypos) noexcept;
void setScroll(GLFWwindow *ctx, GLdouble xoffset, GLdouble yoffset) noexcept;
void processInput(GLFWwindow *ctx) noexcept;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

static GLboolean is_first = GL_TRUE;
static GLfloat lastFrame = 0.0F;
static GLfloat deltaTime = 0.0F;
static GLfloat gFbWidth = WIDTH;
static GLfloat gFbHeight = HEIGHT;
static GLfloat lastX = WIDTH / 2.0F;
static GLfloat lastY = HEIGHT / 2.0F;
static gl::Camera camera(glm::vec3(0.0F, 0.0F, 3.0F));

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GL_TRUE);
#endif   // __APPLE__

    GLFWwindow *ctx = glfwCreateWindow(WIDTH, HEIGHT, "casters_spot", nullptr, nullptr);
    ERR_CTX(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, setFramebufferSize);
    glfwSetCursorPosCallback(ctx, setCursorPos);
    glfwSetScrollCallback(ctx, setScroll);
    glfwSetInputMode(ctx, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ERR_CTX(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error: Failed to initialize GLAD.\n");
    {
        // clang-format off
        std::array vertices = {
            // back
            std::array{-0.5F, -0.5F, -0.5F,  0.0F,  0.0F, -1.0F,  0.0F,  0.0F},
            std::array{ 0.5F, -0.5F, -0.5F,  0.0F,  0.0F, -1.0F,  1.0F,  0.0F},
            std::array{ 0.5F,  0.5F, -0.5F,  0.0F,  0.0F, -1.0F,  1.0F,  1.0F},
            std::array{-0.5F,  0.5F, -0.5F,  0.0F,  0.0F, -1.0F,  0.0F,  1.0F},
            // front
            std::array{-0.5F, -0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  0.0F,  0.0F},
            std::array{ 0.5F, -0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  1.0F,  0.0F},
            std::array{ 0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  1.0F,  1.0F},
            std::array{-0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  0.0F,  1.0F},
            // left
            std::array{-0.5F,  0.5F,  0.5F, -1.0F,  0.0F,  0.0F,  1.0F,  0.0F},
            std::array{-0.5F,  0.5F, -0.5F, -1.0F,  0.0F,  0.0F,  1.0F,  1.0F},
            std::array{-0.5F, -0.5F, -0.5F, -1.0F,  0.0F,  0.0F,  0.0F,  1.0F},
            std::array{-0.5F, -0.5F,  0.5F, -1.0F,  0.0F,  0.0F,  0.0F,  0.0F},
            // right
            std::array{0.5F,  0.5F,  0.5F,  1.0F,  0.0F,  0.0F,  1.0F,  0.0F},
            std::array{0.5F,  0.5F, -0.5F,  1.0F,  0.0F,  0.0F,  1.0F,  1.0F},
            std::array{0.5F, -0.5F, -0.5F,  1.0F,  0.0F,  0.0F,  0.0F,  1.0F},
            std::array{0.5F, -0.5F,  0.5F,  1.0F,  0.0F,  0.0F,  0.0F,  0.0F},
            // bottom
            std::array{-0.5F, -0.5F, -0.5F,  0.0F, -1.0F,  0.0F,  0.0F,  1.0F},
            std::array{ 0.5F, -0.5F, -0.5F,  0.0F, -1.0F,  0.0F,  1.0F,  1.0F},
            std::array{ 0.5F, -0.5F,  0.5F,  0.0F, -1.0F,  0.0F,  1.0F,  0.0F},
            std::array{-0.5F, -0.5F,  0.5F,  0.0F, -1.0F,  0.0F,  0.0F,  0.0F},
            // top
            std::array{-0.5F,  0.5F, -0.5F,  0.0F,  1.0F,  0.0F,  0.0F,  1.0F},
            std::array{ 0.5F,  0.5F, -0.5F,  0.0F,  1.0F,  0.0F,  1.0F,  1.0F},
            std::array{ 0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F,  0.0F},
            std::array{-0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  0.0F,  0.0F}
        };

        std::array indices = {
            std::array<uint8_t, 6>{ 0,  1,  2,  2,  3,  0},
            std::array<uint8_t, 6>{ 4,  5,  6,  6,  7,  4},
            std::array<uint8_t, 6>{ 8,  9, 10, 10, 11,  8},
            std::array<uint8_t, 6>{12, 13, 14, 14, 15, 12},
            std::array<uint8_t, 6>{16, 17, 18, 18, 19, 16},
            std::array<uint8_t, 6>{20, 21, 22, 22, 23, 20}
        };

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

        GLuint cubeVAO = 0;
        GLuint lightVAO = 0;
        GLuint VBO = 0;
        GLuint EBO = 0;
        constexpr auto kIndexCount = (GLsizei)(indices.size() * indices[0].size());

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);


        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::utils::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::utils::BufferOffset<GLfloat>(3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::utils::BufferOffset<GLfloat>(6));
        glEnableVertexAttribArray(2);

        glBindVertexArray(lightVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, skl::utils::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);

        std::error_code ec;
        gl::Texture2D::setFlipY(true);
        gl::Texture2D diffuseMap;
        gl::Texture2D specularMap;
        auto cpath = std::filesystem::current_path();
        auto texPath = cpath / "resources" / "textures" / "container2.png";

        diffuseMap.load(ec, texPath);
        ERR_TEX(ec);
        diffuseMap.set_config(GL_TEXTURE_WRAP_S, GL_REPEAT)
            .set_config(GL_TEXTURE_WRAP_T, GL_REPEAT)
            .set_config(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
            .set_config(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            .update()
            .acquire(ec);
        ERR_TEX(ec);

        texPath = cpath / "resources" / "textures" / "container2_specular.png";

        specularMap.load(ec, texPath);
        ERR_TEX(ec);
        specularMap.set_config(diffuseMap.getCfgs()).update().acquire(ec);
        ERR_TEX(ec);

        {
            glEnable(GL_DEPTH_TEST);
            auto vert = cpath / "shader" / "lighting" / "lighting_maps.vert";
            auto frag = cpath / "shader" / "lighting" / "casters_spot.frag";
            gl::Shader cubeShader;

            cubeShader.build(ec, vert, frag);
            ERR_SHD(ec);

            cubeShader.use();
            cubeShader.set1I("material.diffuse", diffuseMap.getUnitPos());
            cubeShader.set1I("material.specular", specularMap.getUnitPos());


            while (!glfwWindowShouldClose(ctx)) {
                GLfloat cFrame = (GLfloat)glfwGetTime();
                deltaTime = cFrame - lastFrame;
                lastFrame = cFrame;

                processInput(ctx);
                glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glm::mat4 proj(glm::perspective(glm::radians(camera.getZoom()), (GLfloat)gFbWidth / (GLfloat)gFbHeight,
                                                0.1F, 100.0F));
                glm::mat4 view(camera.getView());
                glm::mat4 model(1.0F);

#define RESLOVE_3V(vec) (vec).x, (vec).y, (vec).z

                cubeShader.use();
                cubeShader.set1F("light.constant", 1.0F);
                cubeShader.set1F("light.linear", 0.09F);
                cubeShader.set1F("light.quadratic", 0.032F);
                cubeShader.set1F("light.cutOff", cosf(glm::radians(12.5F)));
                cubeShader.set1F("light.outerCutOff", cosf(glm::radians(17.5F)));
                cubeShader.set1F("material.shininess", 32.0F);
                cubeShader.set3F("light.ambient", 0.1F, 0.1F, 0.1F);
                cubeShader.set3F("light.diffuse", 0.8F, 0.8F, 0.8F);
                cubeShader.set3F("light.specular", 1.0F, 1.0F, 1.0F);
                cubeShader.set3F("light.position", RESLOVE_3V(camera.getPosition()));
                cubeShader.set3F("light.direction", RESLOVE_3V(camera.getFront()));
                cubeShader.set3F("viewPos", RESLOVE_3V(camera.getPosition()));
                cubeShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
                cubeShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
                cubeShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

                diffuseMap.activate(ec);
                if (ec) {
                    (void)fprintf(stderr, "Error: [texture]\nID: %d\nmessage: %s\n", ec.value(), ec.message().c_str());
                    break;
                }
                specularMap.activate(ec);
                if (ec) {
                    (void)fprintf(stderr, "Error: [texture]\nID: %d\nmessage: %s\n", ec.value(), ec.message().c_str());
                    break;
                }

                glBindVertexArray(cubeVAO);
                for (unsigned int i = 0; i < 10; i++) {
                    model = glm::translate(glm::mat4(1.0F), cubePositions[i]);
                    model = glm::rotate(model, glm::radians(20.0F * i), glm::vec3(1.0F, 0.3F, 0.5F));
                    cubeShader.setMat3F("normalMatrix", 1, GL_FALSE,
                                        glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(model)))));
                    cubeShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

                    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_BYTE, nullptr);
                }

                glfwSwapBuffers(ctx);
                glfwPollEvents();
            }
        ERR_SHADER_FREE:
            glDisable(GL_DEPTH_TEST);
        }
    ERR_MESH_FREE:
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteVertexArrays(1, &lightVAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

ERR_CTX_FREE:
    glfwDestroyWindow(ctx);
    glfwTerminate();

    return is_first;
}

void setFramebufferSize([[maybe_unused]] GLFWwindow *ctx, GLsizei width, GLsizei height) noexcept {
    glViewport(0, 0, width, height);
}

void setCursorPos(GLFWwindow *ctx, GLdouble xpos_in, GLdouble ypos_in) noexcept {
    auto xpos = (GLfloat)xpos_in;
    auto ypos = (GLfloat)ypos_in;

    // GLfloat xoffset = lastX - xpos;
    GLfloat xoffset = is_first ? 0 : xpos - lastX;
    // GLfloat yoffset = ypos - lastY;
    GLfloat yoffset = is_first ? 0 : lastY - ypos;
    is_first = GL_FALSE;

    lastX = xpos;
    lastY = ypos;

    camera.mouseMove(xoffset, yoffset);
}

void setScroll([[maybe_unused]] GLFWwindow *ctx, [[maybe_unused]] GLdouble xoffset, GLdouble yoffset) noexcept {
    camera.mouseScroll((float)yoffset);
}

void processInput(GLFWwindow *ctx) noexcept {
    if (glfwGetKey(ctx, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(ctx, GL_TRUE);

    if (glfwGetKey(ctx, GLFW_KEY_W) == GLFW_PRESS) camera.keyboard(gl::Camera::FRONT, deltaTime);
    if (glfwGetKey(ctx, GLFW_KEY_S) == GLFW_PRESS) camera.keyboard(gl::Camera::BACK, deltaTime);
    if (glfwGetKey(ctx, GLFW_KEY_A) == GLFW_PRESS) camera.keyboard(gl::Camera::LEFT, deltaTime);
    if (glfwGetKey(ctx, GLFW_KEY_D) == GLFW_PRESS) camera.keyboard(gl::Camera::RIGHT, deltaTime);
}
