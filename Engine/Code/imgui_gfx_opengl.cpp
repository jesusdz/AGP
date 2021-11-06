#include <imgui_impl_opengl3.h>

bool ImGui_Gfx_Init()
{
    if (!ImGui_ImplOpenGL3_Init())
    {
        //ELOG("Failed to initialize ImGui OpenGL wrapper\n");
        return false;
    }

    return true;
}

void ImGui_Gfx_NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
}

void ImGui_Gfx_DrawData()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGui_Gfx_Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
}

