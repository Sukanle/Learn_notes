// Deps: {{type = "share", name = "skl-gl", priority = 1, cache = true, deps = null}

#import "skl/graphics/gl/camera.hpp"
#import "skl/graphics/gl/shader.hpp"
#import "skl/graphics/gl/texture.hpp"
#import "skl/utils.hpp"


#include <array>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

#define ERR(conf, ...)                      \
    if (conf) {                             \
        (void)fprintf(stderr, __VA_ARGS__); \
        goto ERR_CTX_FREE;                  \
    }

namespace gl = skl::opengl;

void setFramebufferSize(GLFWwindow *ctx, GLsizei width, GLsizei height) noexcept;
void setCursorPos(GLFWwindow *ctx, GLdouble xpos, GLdouble ypos);
void setScroll(GLFWwindow *ctx, GLdouble xoffset, GLdouble yoffset);
void processInput(GLFWwindow *ctx);

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

static GLboolean is_first = GL_TRUE;
static GLfloat lastFrame = 0.0F;
static GLfloat deltaTime = 0.0F;
static GLfloat gFbWidth = WIDTH;
static GLfloat gFbHeight = HEIGHT;
static GLfloat lastX = WIDTH / 2.0F;
static GLfloat lastY = HEIGHT / 2.0F;
static glm::vec3 lightPos(1.2F, 1.0F, 2.0F);
static gl::Camera camera(glm::vec3(0.0F, 0.0F, 3.0F));

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif   // __APPLE__

    GLFWwindow *ctx = glfwCreateWindow(WIDTH, HEIGHT, "lighting_maps", nullptr, nullptr);
    ERR(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, setFramebufferSize);
    glfwSetCursorPosCallback(ctx, setCursorPos);
    glfwSetScrollCallback(ctx, setScroll);
    glfwSetInputMode(ctx, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Erro: Failed to initialize GLAD.\n");
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
        // clang-format on
        // first, configure the cube's VAO (and VBO)
        uint32_t cubeVAO = 0;
        uint32_t lightVAO = 0;
        uint32_t VBO = 0;
        uint32_t EBO = 0;
        glGenVertexArrays(1, &cubeVAO);
        glGenVertexArrays(1, &lightVAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        constexpr GLsizei kIndexCount = (GLsizei)(indices.size() * indices[0].size());

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), skl::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), skl::BufferOffset<GLfloat>(3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), skl::BufferOffset<GLfloat>(6));
        glEnableVertexAttribArray(2);

        glBindVertexArray(lightVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), skl::BufferOffset<GLfloat>(0));
        glEnableVertexAttribArray(0);

        auto cpath = std::filesystem::current_path();
        gl::Texture2D::setflipY(true);
        gl::Texture2D diffuseMap;
        auto texPath = cpath / "resources" / "textures" / "container2.png";
        std::error_code ec;
        diffuseMap.load(ec, texPath);
        if (ec) {
            fprintf(stderr, "Error: [texture] ID: %d\nMsg: %s\n", ec.value(), ec.message().c_str());
            goto ERR_MESH_FREE;
        }
        diffuseMap.bind()
            .set_config(GL_TEXTURE_WRAP_S, GL_REPEAT)
            .set_config(GL_TEXTURE_WRAP_T, GL_REPEAT)
            .set_config(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
            .set_config(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            .update()
            .acquire(ec);
            if (ec) {
                (void)fprintf(stderr, "Error: [shader] ID: %d\nmessage: %s\n", ec.value(), ec.message().c_str());
                goto ERR_MESH_FREE;
            }
        {
            glEnable(GL_DEPTH_TEST);
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(1.0F)));

            auto cpath = std::filesystem::current_path();

            auto vert = cpath / "shader" / "lighting" / "lighting_maps.vert";
            auto frag = cpath / "shader" / "lighting" / "lighting_maps.frag";
            std::error_code ec;
            gl::Shader cubeShader;
            gl::Shader lightShader;

            cubeShader.build(ec, vert, frag);
            if (ec) {
                (void)fprintf(stderr, "Error: [shader] ID: %d\nmessage: %s\n", ec.value(), ec.message().c_str());
                goto ERR_SHADER_FREE;
            }

            vert = cpath / "shader" / "lighting" / "light_cube.vert";
            frag = cpath / "shader" / "lighting" / "light_cube.frag";
            lightShader.build(ec, vert, frag);
            if (ec) {
                (void)fprintf(stderr, "Error: [shader] ID: %d\nmessage: %s\n", ec.value(), ec.message().c_str());
                goto ERR_SHADER_FREE;
            }
            cubeShader.use();
            cubeShader.set1I("material.diffuse", diffuseMap.getUnitPos());


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
                cubeShader.set3F("light.position", RESLOVE_3V(lightPos));
                cubeShader.set3F("light.ambient", 0.2F, 0.2F, 0.2F);
                cubeShader.set3F("light.diffuse", 0.5F, 0.5F, 0.5F);
                cubeShader.set3F("light.specular", 1.0F, 1.0F, 1.0F);
                cubeShader.set3F("material.specular", 0.5F, 0.5F, 0.5F);
                cubeShader.set1F("material.shininess", 64.0F);
                cubeShader.set3F("viewPos", RESLOVE_3V(camera.getPosition()));
                cubeShader.setMat3F("normalMatrix", 1, GL_FALSE, glm::value_ptr(normalMatrix));
                cubeShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
                cubeShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
                cubeShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

                diffuseMap.activate(ec);
                if (ec) {
                    (void)fprintf(stderr, "Error: [texture]\nID: %d\nmessage: %s\n", ec.value(), ec.message().c_str());
                    break;
                }

                glBindVertexArray(cubeVAO);
                glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_BYTE, nullptr);
                model = glm::translate(glm::mat4(1.0F), lightPos);
                model = glm::scale(model, glm::vec3(0.2F));

                lightShader.use();
                lightShader.setMat4F("proj", 1, GL_FALSE, glm::value_ptr(proj));
                lightShader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));
                lightShader.setMat4F("model", 1, GL_FALSE, glm::value_ptr(model));

                glBindVertexArray(lightVAO);
                glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_BYTE, nullptr);
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
    return is_first ? EXIT_FAILURE : EXIT_SUCCESS;
}


void setFramebufferSize([[maybe_unused]] GLFWwindow *ctx, GLsizei width, GLsizei height) noexcept {
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
