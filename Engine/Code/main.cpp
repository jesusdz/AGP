// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdio.h>

bool gGlobalRunning = true;

void OnGlfwMouseEvent(GLFWwindow*, int button, int event, int modifiers)
{
}

void OnGlfwResizeWindow(GLFWwindow* window, int width, int height)
{

}

void OnGlfwCloseWindow(GLFWwindow* window)
{
    gGlobalRunning = false;
}

int main()
{
    if (glfwInit())
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(800, 600, "Main window", NULL, NULL);

        if (window)
        {
            glfwMakeContextCurrent(window);

            // Load all OpenGL functions using the glfw loader function
            // If you use SDL you can use: https://wiki.libsdl.org/SDL_GL_GetProcAddress
            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
            {
                printf("Failed to initialize OpenGL context\n");
                return -1;
            }
            
            printf("GPU: %s\n", glGetString(GL_RENDERER));
            printf("OpenGL & Driver version: %s\n", glGetString(GL_VERSION));

            glfwSetMouseButtonCallback(window, OnGlfwMouseEvent);
            glfwSetWindowSizeCallback(window, OnGlfwResizeWindow);
            glfwSetWindowCloseCallback(window, OnGlfwCloseWindow);

            while (gGlobalRunning)
            {
                glfwPollEvents();

                glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glfwSwapBuffers(window);
            }

            glfwDestroyWindow(window);
        }

        glfwTerminate();
    }

    return 0;
}
