#include "metal_engine.h"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

GLFWwindow* GetGlfwWindow();

id<MTLDevice> gpu = nil;
CAMetalLayer* swapchain = NULL;
id<MTLCommandQueue> queue = nil;
id<CAMetalDrawable> surface = nil;
MTLRenderPassDescriptor* pass = NULL;
id<MTLCommandBuffer> buffer = nil;
id<MTLRenderCommandEncoder> encoder = nil;

bool Metal_Init(Device& device)
{
    gpu = MTLCreateSystemDefaultDevice();
    swapchain = [CAMetalLayer layer];
    swapchain.device = gpu;
    swapchain.opaque = YES;

    GLFWwindow *window = GetGlfwWindow();
    NSWindow *nswindow = glfwGetCocoaWindow(window);
    nswindow.contentView.layer = swapchain;
    nswindow.contentView.wantsLayer = YES;

    queue = [gpu newCommandQueue];

    return true;
}

void Metal_BeginFrame()
{
    //@autoreleasepool
    {
        static MTLClearColor color = MTLClearColorMake(0, 0, 0, 1);
        color.red = (color.red > 1.0) ? 0 : color.red + 0.01;

        surface = [swapchain nextDrawable];

        pass = [MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].clearColor = color;
        pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass.colorAttachments[0].texture = surface.texture;

        buffer = [queue commandBuffer];
        encoder = [buffer renderCommandEncoderWithDescriptor:pass];
    }
}

void Metal_Render(App* app)
{
}

void Metal_EndFrame()
{
    //@autoreleasepool
    {
        [encoder endEncoding];
        [buffer presentDrawable:surface];
        [buffer commit];
    }
}

