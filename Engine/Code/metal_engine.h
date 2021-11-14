#pragma once

#include "engine.h"

bool Metal_InitDevice(Device& device);
void Metal_BeginFrame(Device& device);
void Metal_Render(Device& device);
void Metal_EndFrame(Device& device);

Buffer Metal_CreateBuffer(Device& device, u32 size, BufferType type, BufferUsage usage);
void Metal_BindBuffer(const Buffer& buffer);
void Metal_MapBuffer(const Buffer& buffer, Access access);
void Metal_UnmapBuffer(const Buffer& buffer);
