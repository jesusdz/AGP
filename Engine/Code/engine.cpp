#pragma once

#include "engine.h"
#include <imgui.h>
#include <glad/glad.h>

void OnInit(App* app)
{
}

void OnGui(App* app)
{
    ImGui::Begin("Engine");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::End();
}

void OnUpdate(App* app)
{

}

void OnRender(App* app)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
