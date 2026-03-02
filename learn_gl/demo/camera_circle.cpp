// Deps: {{type = "share", name = "skl-gl", priority = 1, cache = true, deps = null}, {type = "share", name = "skl-fs", priority = 2, cache = true, deps = null}}

/**
 * @brief Vertex & Index Naming Convention for Cube Geometry
 *
 * This project uses a structured naming scheme for VBO (Vertex Buffer Object)
 * and EBO (Element Buffer Object) identifiers to ensure spatial clarity.
 *
 * FORMAT: [Primitive]-[Face]-[Orientation]
 *
 * 1. PRIMITIVE TYPE:
 *    - P : Point (Single Vertex coordinate)
 *    - R : Rectangle (Quad composed of 2 triangles / 4 vertices)
 *
 * 2. CUBE FACE (Global Space):
 *    - U : Up    (Top)
 *    - D : Down  (Bottom)
 *    - F : Front (Facing Camera)
 *    - B : Back  (Opposite to Front)
 *    - L : Left
 *    - R : Right
 *
 * 3. LOCAL ORIENTATION (Face Space):
 *    - NW: North-West (Top-Left)
 *    - NE: North-East (Top-Right)
 *    - SW: South-West (Bottom-Left)
 *    - SE: South-East (Bottom-Right)
 *    - N : North (Top-Center)
 *    - S : South (Bottom-Center)
 *    - W : West  (Left-Center)
 *    - E : East  (Right-Center)
 *    - C : Center
 *
 * NOTE: 'North' direction on any face always points towards the global Up (U) face.
 *
 * EXAMPLES:
 *    - P-F-NW  : Vertex at the Top-Left corner of the Front face.
 *    - R-U-SE  : Rectangle region at the Bottom-Right of the Up face.
 *    - P-D-C   : Vertex at the exact center of the Down face.
 */

#include "skl/shader.hpp"
#include "skl/texture.hpp"
#include "skl/utils.hpp"
#include "skl/fs.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <array>
#include <filesystem>


void processInput(GLFWwindow* ctx) {
    if (glfwGetKey(ctx, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(ctx, GL_TRUE);
}

void frameBufferSizeCallback([[maybe_unused]] GLFWwindow* ctx, GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);
}

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

namespace gl = skl::opengl;
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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    GLFWwindow* ctx = glfwCreateWindow(WIDTH, HEIGHT, "camera", nullptr, nullptr);
    ERR(!ctx, "Error: Failed to create GLFW window.\n");
    glfwMakeContextCurrent(ctx);
    glfwSetFramebufferSizeCallback(ctx, frameBufferSizeCallback);
    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Error Failed to ininization GLAD.\n");

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

    char exepath[skl::fs::SKL_PATH_MAX]= {0};
    skl::fs::exedir(exepath, skl::fs::SKL_PATH_MAX);
    std::array<gl::texture_t, 2> arr_texture;
    std::string cpath(exepath);
    arr_texture[0].path = cpath + "/../../../../resources/textures/container.jpg";
    arr_texture[1].path = cpath + "/../../../../resources/textures/awesomeface.png";
    stbi_set_flip_vertically_on_load(GL_TRUE);

    for (auto& texture : arr_texture) {
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
            texture.data = nullptr;
        } else
            (void)fprintf(stderr, "Failed to load texture[id = %d](path = %s).\n", texture.ID, texture.path.c_str());
    }

    {
        std::string vert = cpath + "/../../../../shader/camera_circle.vert";
        std::string frag = cpath + "/../../../../shader/camera_circle.frag";
        gl::Shader shader(vert.c_str(), frag.c_str());
        shader.use();
        shader.set1I("texture0", 0);
        shader.set1I("texture1", 1);
        glm::mat4 project = glm::perspective(glm::radians(45.0F), (float)WIDTH / (float)HEIGHT, 0.1F, 100.0F);
        shader.setMat4F("project", 1, GL_FALSE, glm::value_ptr(project));
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

        glEnable(GL_DEPTH_TEST);

        while (!glfwWindowShouldClose(ctx)) {
            processInput(ctx);

            glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (size_t i = 0; i < arr_texture.size(); ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, arr_texture[i].ID);
            }

            shader.use();

            glm::mat4 view(1.0F);
            float radius = 10.0F;
            float camX = sinf((float)glfwGetTime()) * radius;
            float camZ = cosf(glfwGetTime()) * radius;
            view = glm::lookAt(glm::vec3(camX, 0.0F, camZ), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 1.0F, 0.0F));
            shader.setMat4F("view", 1, GL_FALSE, glm::value_ptr(view));

            glBindVertexArray(VAO);

            for (uint32_t i = 0; i < 10; ++i) {
                glm::mat4 model(1.0F);
                model = glm::translate(model, cubePositions[i]);
                float angle = 20.0F * i;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0F, 0.3F, 0.5F));
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

    for (auto& tex : arr_texture)
        if (tex.ID) glDeleteTextures(1, &tex.ID);

    return EXIT_SUCCESS;
}
