#pragma once

#include "engine.h"

bool Metal_InitDevice(Device& device);
void Metal_BeginFrame();
void Metal_Render(App* app);
void Metal_EndFrame();

