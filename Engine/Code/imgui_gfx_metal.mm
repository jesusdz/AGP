#include <imgui_impl_metal.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

extern id<MTLDevice> gpu;
extern id<MTLCommandQueue> queue;
extern id<CAMetalDrawable> surface;
extern MTLRenderPassDescriptor* pass;
extern id<MTLCommandBuffer> buffer;
extern id<MTLRenderCommandEncoder> encoder;

bool ImGui_Gfx_Init()
{
    if (!ImGui_ImplMetal_Init(gpu))
    {
        return false;
    }

    return true;
}

void ImGui_Gfx_NewFrame()
{
    //@autoreleasepool
    {
        ImGui_ImplMetal_NewFrame(pass);
    }
}

void ImGui_Gfx_DrawData()
{
    //@autoreleasepool
    {
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), buffer, encoder);
    }
}

void ImGui_Gfx_Shutdown()
{
    ImGui_ImplMetal_Shutdown();
}

