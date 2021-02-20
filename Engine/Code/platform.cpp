//
// platform.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#define WINDOW_TITLE  "Advanced Graphics Programming"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

void OnGlfwError(int errorCode, const char *errorMessage)
{
	fprintf(stderr, "glfw failed with error %d: %s\n", errorCode, errorMessage);
}

void OnGlfwMouseEvent(GLFWwindow* window, int button, int event, int modifiers)
{

}

void OnGlfwScrollEvent(GLFWwindow* window, double xoffset, double yoffset)
{

}

void OnGlfwKeyboardEvent(GLFWwindow* window, int, int, int, int)
{

}

void OnGlfwCharEvent(GLFWwindow* window, unsigned int character)
{

}

void OnGlfwResizeFramebuffer(GLFWwindow* window, int width, int height)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->displaySize = vec2(width, height);
}

void OnGlfwCloseWindow(GLFWwindow* window)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->isRunning = false;
}

int main()
{
    App app         = {};
    app.deltaTime   = 1.0f/60.0f;
    app.displaySize = ivec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.isRunning   = true;

		glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit())
    {
        ELOG("glfwInit() failed\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window)
    {
        ELOG("glfwCreaateWindow() failed\n");
        return -1;
    }

    glfwSetWindowUserPointer(window, &app);

    glfwSetMouseButtonCallback(window, OnGlfwMouseEvent);
    glfwSetScrollCallback(window, OnGlfwScrollEvent);
    glfwSetKeyCallback(window, OnGlfwKeyboardEvent);
    glfwSetCharCallback(window, OnGlfwCharEvent);
    glfwSetFramebufferSizeCallback(window, OnGlfwResizeFramebuffer);
    glfwSetWindowCloseCallback(window, OnGlfwCloseWindow);

    glfwMakeContextCurrent(window);

    // Load all OpenGL functions using the glfw loader function
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        ELOG("Failed to initialize OpenGL context\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        ELOG("ImGui_ImplGlfw_InitForOpenGL() failed\n");
        return -1;
    }

    if (!ImGui_ImplOpenGL3_Init())
    {
        ELOG("Failed to initialize ImGui OpenGL wrapper\n");
        return -1;
    }

    f64 lastFrameTime = glfwGetTime();

    Init(&app);

    while (app.isRunning)
    {
        // Tell GLFW to call platform callbacks
        glfwPollEvents();

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        Gui(&app);
        ImGui::Render();

        // Update
        Update(&app);

        // Render
        Render(&app);

        // ImGui Render
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // Present image on screen
        glfwSwapBuffers(window);

        // Frame time
        f64 currentFrameTime = glfwGetTime();
        app.deltaTime = (f32)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

String ReadTextFile(const char* filename)
{
    String fileText = {};

    FILE* file = fopen(filename, "r");

    if (file)
    {
        fseek(file, 0, SEEK_END);
        fileText.length = ftell(file);
        fseek(file, 0, SEEK_SET);
        fileText.str = (char*)calloc(fileText.length + 1, sizeof(char));
        fread(fileText.str, sizeof(char), fileText.length, file);
        fclose(file);
    }
    else
    {
        ELOG("fopen() failed reading file %s", filename);
    }

    return fileText;
}

void FreeString(String str)
{
    if (str.str) free(str.str);
}

void LogString(const char* str)
{
#ifdef _WIN32
    OutputDebugStringA(str);
    OutputDebugStringA("\n");
#else
		fprintf(stderr, "%s\n", str);
#endif
}
