// xmake-type: Target
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// C ISO
#include <math.h>
#include <stdio.h>

// OpenGL
#include <GLFW/glfw3.h>

#define SCREENW 800
#define SCREENH 600

int main() {
    if(glfwInit() == GLFW_FALSE){
        fprintf(stderr,"Failed to initialize glfw, panic!\n");
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(SCREENW, SCREENH, "Gravity Simple", nullptr, nullptr);
    float centerX = SCREENW / 2.0;
    float centerY = SCREENH / 2.0;
    float radius = 50.0;
    int res = 100;

    glfwMakeContextCurrent(window);

    glMatrixMode(GL_PROJECTION);
    glViewport(0, 0, SCREENW, SCREENH);
    while(glfwWindowShouldClose(window) == 0){
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLE_FAN); // triangle fan
        glVertex2d(centerX,centerY);
        for(int i = 0; i <= res ; ++i){
            float angle = 2.0 * M_PI * ((float)i / res);
            float x = centerX + cos(angle) * radius;
            float y = centerY + sin(angle) * radius;
            glVertex2d(x, y);
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
