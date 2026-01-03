// xmake-type: Target
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600
#define UNUSED(var) ((void)var);
#define __ERR(conf, fn, ...) \
if (conf) { \
    fn; \
    glfwTerminate(); \
    (void)fprintf(stderr, __VA_ARGS__); \
    return 1; \
}
#define ERR(conf, ...) __ERR(conf,,__VA_ARGS__)
#define SHADER_ERR(cond, shader, info, ...) \
    __ERR(cond, \
        glGetShaderInfoLog(shader, sizeof(info), NULL, info)\
        , __VA_ARGS__);
#define PROGRAM_ERR(cond, program, info, ...) \
    __ERR(cond, \
        glGetProgramInfoLog(program, sizeof(info), NULL, info)\
        , __VA_ARGS__);

void frameBufferSize(GLFWwindow* window, GLsizei width, GLsizei height);
void processInput(GLFWwindow *window);
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "uniform", NULL, NULL);
    ERR(!window, "Error: Faild to create GLFW window.\n");
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress),
        "Error: Faild to initization GLAD.\n");
    
    const char* vertexSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "}\0";
    const char* fragmentSource =
        "#version 330 core\n"
        "out vec4 FlagColor;\n"
        "uniform vec4 ourColor;\n"
        "void main() {\n"
        "   FlagColor = ourColor;\n"
        "}\0";

    int success;
    char Msg[512];

    (void)fprintf(stdout, "Info: Starting vertex build.\n");
    uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    SHADER_ERR(!success, vertex, Msg, "ERROR::SHADER::VERTEX::COMPILATION_FAILD: %s\n", Msg);

    (void)fprintf(stdout, "Info: Starting fragment build.\n");
    uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    SHADER_ERR(!success, fragment, Msg, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILD: %s\n", Msg);

    (void)fprintf(stdout, "Info: Starting program build.\n");
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    PROGRAM_ERR(!success, program, Msg, "ERROR::SHADER::PROGRAM::LINKING_FAILD: %s\n", Msg);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    (void)fprintf(stdout, "Info: Starting VAO, VBO.\n");
    GLfloat vertices[] = {
        -0.5F, -0.5F, 0.0F,
         0.5F, -0.5F, 0.0F,
         0.0F,  0.5F, 0.0F
    };
    uint32_t VAO = 0;
    uint32_t VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(VAO);

    while(!glfwWindowShouldClose(window)){
        processInput(window);

        glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        double time = glfwGetTime();
        float green = ((float)sin(time) / 2.0F) + 0.5F;
        int vertexColorLocation = glGetUniformLocation(program, "ourColor");
        glUniform4f(vertexColorLocation, 0.0F, green, 0.0F, 1.0F);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}

void frameBufferSize(GLFWwindow* window, GLsizei width, GLsizei height) {
    UNUSED(window);
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
