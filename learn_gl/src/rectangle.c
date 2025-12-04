#include <stdio.h>
#include <stdint.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define WIDTH 800
#define HEIGHT 600

#define __ERR(cond, fn, ...) \
if (cond) { \
    fn; \
    fprintf(stderr, __VA_ARGS__); \
    return 1; \
}

#define ERR(cond, ...) __ERR(cond, , __VA_ARGS__)
#define SHADER_ERR(cond, shader, info, ...) \
    __ERR(cond, \
        glGetShaderInfoLog(shader, sizeof(info), NULL, info)\
        , __VA_ARGS__);
#define PROGRAM_ERR(cond, program, info, ...) \
    __ERR(cond, \
        glGetProgramInfoLog(program, sizeof(info), NULL, info)\
        , __VA_ARGS__);

#define UNUSED(x) ((void)(x))

void setFramebufferSize(GLFWwindow* window, GLsizei width, GLsizei height){
    UNUSED(window);
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "rectangle", NULL, NULL);
    __ERR(!window, glfwTerminate(),"Error: Faild to create GLFW window.\n");
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, setFramebufferSize);

    ERR(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress),
        "Error: Faild to Initiaztion GLAD.\n");

    const char * vertixShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\n\0";
    const char *fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n"
        "}\n\0";
    int success;
    char info[512];

    uint32_t vertix = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertix, 1, &vertixShaderSource, NULL);
    glCompileShader(vertix);
    glGetShaderiv(vertix, GL_COMPILE_STATUS, &success);
    SHADER_ERR(!success, vertix, info, "ERROR::SHADER::VERTEX::COMPILATION_FAILED: %s\n", info);

    uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    SHADER_ERR(!success, fragment, info, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n", info);

    uint32_t program = glCreateProgram();
    glAttachShader(program, vertix);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    PROGRAM_ERR(!success, program, info,  "ERROR::SHADER::PROGRAM::LINKING_FAILED: %s\n", info);
    glDeleteShader(vertix);
    glDeleteShader(fragment);

    GLfloat vertices[] = {
         0.5,  0.5, 0.0,
         0.5, -0.5, 0.0,
        -0.5, -0.5, 0.0,
        -0.5,  0.5, 0.0
    };

    uint32_t indices [] = {
        0, 1, 3,
        1, 2, 3
    };

    uint32_t VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0); 

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}
