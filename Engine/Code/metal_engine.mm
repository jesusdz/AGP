#include "metal_engine.h"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <imgui_impl_metal.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

GLFWwindow* GetGlfwWindow();

struct MetalDeviceWrapper
{
    id<MTLDevice> device = nil;
    CAMetalLayer* swapchain = NULL;
    id<MTLCommandQueue> queue = nil;
    id<CAMetalDrawable> surface = nil;
    MTLRenderPassDescriptor* pass = NULL;
    id<MTLCommandBuffer> buffer = nil;
    id<MTLRenderCommandEncoder> encoder = nil;
};

struct MetalBufferWrapper
{
    id<MTLBuffer> handle;
};

static MetalDeviceWrapper s_metalDeviceWrapper = {};

static MetalDeviceWrapper& GetMetalDeviceWrapper(Device& device)
{
    MetalDeviceWrapper* devWrapper = (MetalDeviceWrapper*)device.internal;
    ASSERT(devWrapper, "Metal device was not initialized.");
    return *devWrapper;
}

bool Metal_InitDevice(Device& device)
{
    ASSERT(device.internal == NULL, "Metal device was already initialized");
    device.internal = &s_metalDeviceWrapper;

    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);
    metalDeviceWrapper.device = MTLCreateSystemDefaultDevice();

    metalDeviceWrapper.swapchain = [CAMetalLayer layer];
    metalDeviceWrapper.swapchain.device = metalDeviceWrapper.device;
    metalDeviceWrapper.swapchain.opaque = YES;

    GLFWwindow *window = GetGlfwWindow();
    NSWindow *nswindow = glfwGetCocoaWindow(window);
    nswindow.contentView.layer = metalDeviceWrapper.swapchain;
    nswindow.contentView.wantsLayer = YES;

    metalDeviceWrapper.queue = [metalDeviceWrapper.device newCommandQueue];

    return true;
}

void Metal_BeginFrame(Device& device)
{
    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);

    //@autoreleasepool
    {
        static MTLClearColor color = MTLClearColorMake(0, 0, 0, 1);
        color.red = (color.red > 1.0) ? 0 : color.red + 0.01;

        metalDeviceWrapper.surface = [metalDeviceWrapper.swapchain nextDrawable];

        metalDeviceWrapper.pass = [MTLRenderPassDescriptor renderPassDescriptor];
        metalDeviceWrapper.pass.colorAttachments[0].clearColor = color;
        metalDeviceWrapper.pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
        metalDeviceWrapper.pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        metalDeviceWrapper.pass.colorAttachments[0].texture = metalDeviceWrapper.surface.texture;

        metalDeviceWrapper.buffer = [metalDeviceWrapper.queue commandBuffer];
        metalDeviceWrapper.encoder = [metalDeviceWrapper.buffer renderCommandEncoderWithDescriptor:metalDeviceWrapper.pass];
    }
}

void Metal_Render(Device& device)
{
}

void Metal_EndFrame(Device& device)
{
    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);

    //@autoreleasepool
    {
        [metalDeviceWrapper.encoder endEncoding];
        [metalDeviceWrapper.buffer presentDrawable:metalDeviceWrapper.surface];
        [metalDeviceWrapper.buffer commit];
    }
}


#define METAL_ID(type, var) *(id<type>*)&var

Buffer Metal_CreateBuffer(Device& device, u32 size, BufferType type, BufferUsage usage)
{
    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);

    Buffer buffer = {};
    buffer.size = size;
    buffer.type = type;
    id<MTLBuffer>& handle = METAL_ID(MTLBuffer, buffer.handle);
    handle = [metalDeviceWrapper.device newBufferWithLength:size options:MTLResourceStorageModeShared];
    return buffer;
}

void Metal_BindBuffer(const Buffer& buffer)
{
    // TODO
}

void Metal_MapBuffer(const Buffer& buffer, Access access)
{
    ASSERT(buffer.data == NULL, "The buffer is already mapped");
    id<MTLBuffer> &handle = METAL_ID(MTLBuffer, buffer.handle);
    buffer.data = [handle contents];
}

void Metal_UnmapBuffer(const Buffer& buffer)
{
    ASSERT(buffer.data != NULL, "The buffer is not mapped");

    buffer.data = NULL;
}


// IMGUI ///////////////////////////////////////////////////////////////

bool ImGui_Gfx_Init(Device& device)
{
    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);

    if (!ImGui_ImplMetal_Init(metalDeviceWrapper.device))
    {
        return false;
    }

    return true;
}

void ImGui_Gfx_NewFrame(Device& device)
{
    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);

    //@autoreleasepool
    {
        ImGui_ImplMetal_NewFrame(metalDeviceWrapper.pass);
    }
}

void ImGui_Gfx_DrawData(Device& device)
{
    MetalDeviceWrapper& metalDeviceWrapper = GetMetalDeviceWrapper(device);

    //@autoreleasepool
    {
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), metalDeviceWrapper.buffer, metalDeviceWrapper.encoder);
    }
}

void ImGui_Gfx_Shutdown(Device& device)
{
    ImGui_ImplMetal_Shutdown();
}
