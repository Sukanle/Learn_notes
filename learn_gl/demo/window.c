// xmake-type: Target
// C ISO
#include <stdio.h>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void callback(GLFWwindow* wobj, int width, int height){
    glViewport(0, 0, width, height);
}
void Input(GLFWwindow* wobj){
    if(glfwGetKey(wobj, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(wobj, GL_TRUE);
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* wobj = glfwCreateWindow(800, 600, "learn OpenGL", NULL, NULL);

    if(wobj == NULL){
        fprintf(stderr, "Error: Faild to create GLFW window.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(wobj);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        fprintf(stderr, "Error: Faild to initialize GLAD.\n");
        return 1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(wobj, callback);

    // Rendering Loop
    while(!glfwWindowShouldClose(wobj)){
        Input(wobj);

        // Rendering instructions
        glClearColor(0.73, 0.22, 0.46, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(wobj);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
