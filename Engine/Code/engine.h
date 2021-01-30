#pragma once

#include "platform.h"

struct App
{
    f32  deltaTime;
    bool isRunning;
};

void OnInit(App* app);

void OnGui(App* app);

void OnUpdate(App* app);

void OnRender(App* app);
