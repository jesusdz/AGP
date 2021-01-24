// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

bool gGlobalRunning = true;

void OnGlfwMouseEvent(GLFWwindow* window, int button, int event, int modifiers)
{

}

void OnGlfwKeyboardEvent(GLFWwindow* window, int, int, int, int)
{

}

void OnGlfwCharEvent(GLFWwindow* window, unsigned int character)
{

}

void OnGlfwScrollEvent(GLFWwindow* window, double, double)
{

}

void OnGlfwResizeWindow(GLFWwindow* window, int width, int height)
{

}

void OnGlfwCloseWindow(GLFWwindow* window)
{
    gGlobalRunning = false;
}

// GLFW callbacks
// - When calling Init with 'install_callbacks=true': GLFW callbacks will be installed for you. They will call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be installed. You will need to call those function yourself from your own GLFW callbacks.
//IMGUI_IMPL_API void     ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
//IMGUI_IMPL_API void     ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
//IMGUI_IMPL_API void     ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
//IMGUI_IMPL_API void     ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
//IMGUI_IMPL_API void     ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event);

// Backend API
//IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_Init(const char* glsl_version = NULL);
//IMGUI_IMPL_API void     ImGui_ImplOpenGL3_Shutdown();
//IMGUI_IMPL_API void     ImGui_ImplOpenGL3_NewFrame();
//IMGUI_IMPL_API void     ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);
//
//// (Optional) Called by Init/NewFrame/Shutdown
//IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_CreateFontsTexture();
//IMGUI_IMPL_API void     ImGui_ImplOpenGL3_DestroyFontsTexture();
//IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_CreateDeviceObjects();
//IMGUI_IMPL_API void     ImGui_ImplOpenGL3_DestroyDeviceObjects();
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
                printf("Failed to initialize ImGui GLFW wrapper\n");
                return -1;
            }

            if (!ImGui_ImplOpenGL3_Init())
            {
                printf("Failed to initialize ImGui OpenGL wrapper\n");
                return -1;
            }
            
            printf("GPU: %s\n", glGetString(GL_RENDERER));
            printf("OpenGL & Driver version: %s\n", glGetString(GL_VERSION));

            glfwSetMouseButtonCallback(window, OnGlfwMouseEvent);
            glfwSetKeyCallback(window, OnGlfwKeyboardEvent);
            glfwSetCharCallback(window, OnGlfwCharEvent);
            glfwSetCharCallback(window, OnGlfwCharEvent);
            glfwSetScrollCallback(window, OnGlfwScrollEvent);
            glfwSetWindowSizeCallback(window, OnGlfwResizeWindow);
            glfwSetWindowCloseCallback(window, OnGlfwCloseWindow);

            while (gGlobalRunning)
            {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                glfwPollEvents();

                glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                ImGui::ShowDemoWindow();

                ImGui::Render();
                
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Update and Render additional Platform Windows
                // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
                //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }

                glfwSwapBuffers(window);
            }
            
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();

            glfwDestroyWindow(window);
        }

        glfwTerminate();
    }

    return 0;
}
