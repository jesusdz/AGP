#pragma once

#include "engine.h"

#define MAKE_GLVERSION(major, minor) (major*10 + minor)
#define MAKE_GLSLVERSION(major, minor) (major*100 + minor*10)

bool OGL_InitDevice(Device& device);
void OGL_BeginFrame();
void OGL_Render(App* app);
void OGL_EndFrame();

