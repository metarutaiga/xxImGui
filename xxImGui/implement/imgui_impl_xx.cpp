// dear imgui: Renderer for XX
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'uint64_t' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [X] Renderer: Multi-viewport support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable'.
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bits indices.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "imgui/imgui.h"
#include "imgui_impl_xx.h"

#include "xxGraphic/xxGraphic.h"

struct ImGuiViewportDataXX
{
    uint64_t                CommandEncoder = 0;
    uint64_t                Swapchain = 0;
    uint64_t                RenderPass = 0;
    float                   Color[4] = {};
    void*                   Handle = nullptr;
    int                     Width = 0;
    int                     Height = 0;

    int                     BufferIndex = 0;
    uint64_t                VertexBuffers[4] = {};
    int                     VertexBufferSizes[4] = {};
    uint64_t                IndexBuffers[4] = {};
    int                     IndexBufferSizes[4] = {};
    uint64_t                ConstantBuffers[4] = {};

    ~ImGuiViewportDataXX()  { IM_ASSERT(Swapchain == 0); IM_ASSERT(RenderPass == 0); }
};

static uint64_t g_instance = 0;
static uint64_t g_device = 0;
static uint64_t g_renderPass = 0;
static uint64_t g_fontTexture = 0;
static uint64_t g_fontSampler = 0;
static uint64_t g_vertexAttribute = 0;
static uint64_t g_vertexShader = 0;
static uint64_t g_fragmentShader = 0;
static uint64_t g_blendState = 0;
static uint64_t g_depthStencilState = 0;
static uint64_t g_rasterizerState = 0;
static uint64_t g_pipeline = 0;
static bool     g_halfPixel = false;

// Forward Declarations
static void ImGui_ImplXX_InitPlatformInterface();
static void ImGui_ImplXX_ShutdownPlatformInterface();
static void ImGui_ImplXX_CreateDeviceObjectsForPlatformWindows();
static void ImGui_ImplXX_InvalidateDeviceObjectsForPlatformWindows();

static void ImGui_ImplXX_SetupRenderState(ImDrawData* draw_data, uint64_t commandEncoder, uint64_t constantBuffer)
{
    int width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    xxSetViewport(commandEncoder, 0, 0, width, height, 0.0f, 1.0f);

    // Setup orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    {
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

        // Half-Pixel Offset in Legacy Direct3D
        if (g_halfPixel)
        {
            L += 0.5f;
            R += 0.5f;
            T += 0.5f;
            B += 0.5f;
        }

        static const float identity[] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        float projection[] =
        {
            2.0f/(R-L),  0.0f,        0.0f, 0.0f,
            0.0f,        2.0f/(T-B),  0.0f, 0.0f,
            0.0f,        0.0f,        0.5f, 0.0f,
            (L+R)/(L-R), (T+B)/(B-T), 0.5f, 1.0f
        };

        char* mapConstantBuffer = (char*)xxMapBuffer(g_device, constantBuffer);
        if (mapConstantBuffer)
        {
            memcpy(mapConstantBuffer + sizeof(identity) * 0, identity, sizeof(identity));
            memcpy(mapConstantBuffer + sizeof(identity) * 1, identity, sizeof(identity));
            memcpy(mapConstantBuffer + sizeof(identity) * 2, projection, sizeof(projection));
            xxUnmapBuffer(g_device, constantBuffer);
        }
    }

    xxSetPipeline(commandEncoder, g_pipeline);
    xxSetVertexConstantBuffer(commandEncoder, constantBuffer, 16 * 3 * sizeof(float));
}

// Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
void ImGui_ImplXX_RenderDrawData(ImDrawData* draw_data, uint64_t commandEncoder)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || draw_data->CmdListsCount == 0)
        return;

    if (draw_data->OwnerViewport == nullptr || draw_data->OwnerViewport->RendererUserData == nullptr)
        return;
    ImGuiViewportDataXX* data = (ImGuiViewportDataXX*)draw_data->OwnerViewport->RendererUserData;

    // Render buffers per viewport
    int&        bufferIndex         = data->BufferIndex;
    uint64_t&   vertexBuffer        = data->VertexBuffers[bufferIndex];
    int&        vertexBufferSize    = data->VertexBufferSizes[bufferIndex];
    uint64_t&   indexBuffer         = data->IndexBuffers[bufferIndex];
    int&        indexBufferSize     = data->IndexBufferSizes[bufferIndex];
    uint64_t&   constantBuffer      = data->ConstantBuffers[bufferIndex];

    // Swap buffer
    bufferIndex++;
    if (bufferIndex >= 4)
        bufferIndex = 0;

    // Create and grow buffers if needed
    if (vertexBuffer == 0 || vertexBufferSize < draw_data->TotalVtxCount)
    {
        xxDestroyBuffer(g_device, vertexBuffer);
        vertexBufferSize = draw_data->TotalVtxCount + 5000;
        vertexBuffer = xxCreateVertexBuffer(g_device, vertexBufferSize * sizeof(ImDrawVert), g_vertexAttribute);
        if (vertexBuffer == 0)
            return;
    }
    if (indexBuffer == 0 || indexBufferSize < draw_data->TotalIdxCount)
    {
        xxDestroyBuffer(g_device, indexBuffer);
        indexBufferSize = draw_data->TotalIdxCount + 10000;
        indexBuffer = xxCreateIndexBuffer(g_device, indexBufferSize * sizeof(ImDrawIdx));
        if (indexBuffer == 0)
            return;
    }
    if (constantBuffer == 0)
    {
        constantBuffer = xxCreateConstantBuffer(g_device, 16 * 3 * sizeof(float));
    }

    // Copy and convert all vertices into a swapped buffer.
    ImDrawVert* vtx_dst = (ImDrawVert*)xxMapBuffer(g_device, vertexBuffer);
    ImDrawIdx* idx_dst = (ImDrawIdx*)xxMapBuffer(g_device, indexBuffer);
    if (vtx_dst == nullptr || idx_dst == nullptr)
        return;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        vtx_dst += cmd_list->VtxBuffer.Size;
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    xxUnmapBuffer(g_device, vertexBuffer);
    xxUnmapBuffer(g_device, indexBuffer);

    xxSetVertexBuffers(commandEncoder, 1, &vertexBuffer, g_vertexAttribute);

    ImGui_ImplXX_SetupRenderState(draw_data, commandEncoder, constantBuffer);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    uint64_t boundTextureID = 0;
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                {
                    xxSetVertexBuffers(commandEncoder, 1, &vertexBuffer, g_vertexAttribute);
                    ImGui_ImplXX_SetupRenderState(draw_data, commandEncoder, constantBuffer);
                    boundTextureID = 0;
                }
                else
                {
                    data->CommandEncoder = commandEncoder;
                    pcmd->UserCallback(cmd_list, pcmd);
                    data->CommandEncoder = 0;
                }
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    int clip_x = (int)clip_rect.x;
                    int clip_y = (int)clip_rect.y;
                    int clip_width = (int)(clip_rect.z - clip_rect.x);
                    int clip_height = (int)(clip_rect.w - clip_rect.y);
                    xxSetScissor(commandEncoder, clip_x, clip_y, clip_width, clip_height);

                    // Texture
                    if (boundTextureID != pcmd->TextureId)
                    {
                        boundTextureID = pcmd->TextureId;

                        xxSetFragmentTextures(commandEncoder, 1, &boundTextureID);
                        xxSetFragmentSamplers(commandEncoder, 1, &g_fontSampler);
                    }

                    // Draw
                    xxDrawIndexed(commandEncoder, indexBuffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

bool ImGui_ImplXX_Init(uint64_t instance, uint64_t device, uint64_t renderPass)
{
    g_instance = instance;
    g_device = device;
    g_renderPass = renderPass;

#if defined(xxWINDOWS)
    const char* deviceString = xxGetDeviceName();
    g_halfPixel = false;
    g_halfPixel |= (strncmp(deviceString, "Direct3D 5", 10) == 0);
    g_halfPixel |= (strncmp(deviceString, "Direct3D 6", 10) == 0);
    g_halfPixel |= (strncmp(deviceString, "Direct3D 7", 10) == 0);
    g_halfPixel |= (strncmp(deviceString, "Direct3D 8", 10) == 0);
    g_halfPixel |= (strncmp(deviceString, "Direct3D 9", 10) == 0);
#endif

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_xx";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_ImplXX_InitPlatformInterface();

    return true;
}

void ImGui_ImplXX_Shutdown()
{
    ImGui_ImplXX_ShutdownPlatformInterface();
    ImGui_ImplXX_InvalidateDeviceObjects();
    g_renderPass = 0;
    g_device = 0;
    g_instance = 0;
}

static bool ImGui_ImplXX_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Upload texture to graphics system
    xxDestroyTexture(g_fontTexture);
    g_fontTexture = xxCreateTexture(g_device, 0, width, height, 1, 1, 1, nullptr);
    if (g_fontTexture == 0)
        return false;
    int stride = 0;
    void* map = xxMapTexture(g_device, g_fontTexture, &stride, 0, 0);
    if (map == nullptr)
        return false;
    for (int y = 0; y < height; y++)
        memcpy((char*)map + stride * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
    xxUnmapTexture(g_device, g_fontTexture, 0, 0);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)g_fontTexture;

    return true;
}

bool ImGui_ImplXX_CreateDeviceObjects()
{
    if (g_device == 0)
        return false;
    if (ImGui_ImplXX_CreateFontsTexture() == false)
        return false;
    ImGui_ImplXX_CreateDeviceObjectsForPlatformWindows();

    ImDrawVert vert;
    int attributes[] =
    {
        0, (int)xxOffsetOf(ImDrawVert, pos),  3, xxSizeOf(vert.pos) + xxSizeOf(vert.z),
        0, (int)xxOffsetOf(ImDrawVert, col),  4, xxSizeOf(vert.col),
        0, (int)xxOffsetOf(ImDrawVert, uv),   2, xxSizeOf(vert.uv)
    };

    g_fontSampler = xxCreateSampler(g_device, false, false, false, true, true, true, 1);
    g_vertexAttribute = xxCreateVertexAttribute(g_device, 3, attributes);
    g_vertexShader = xxCreateVertexShader(g_device, "default", g_vertexAttribute);
    g_fragmentShader = xxCreateFragmentShader(g_device, "default");
    g_blendState = xxCreateBlendState(g_device, BLEND_FACTOR_SRC_ALPHA, BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    g_depthStencilState = xxCreateDepthStencilState(g_device, false, false);
    g_rasterizerState = xxCreateRasterizerState(g_device, false, true);
    g_pipeline = xxCreatePipeline(g_device, g_renderPass, g_blendState, g_depthStencilState, g_rasterizerState, g_vertexAttribute, g_vertexShader, g_fragmentShader);

    return true;
}

void ImGui_ImplXX_InvalidateDeviceObjects()
{
    if (g_device == 0)
        return;
    xxDestroyTexture(g_fontTexture);
    xxDestroySampler(g_fontSampler);
    xxDestroyVertexAttribute(g_vertexAttribute);
    xxDestroyShader(g_device, g_vertexShader);
    xxDestroyShader(g_device, g_fragmentShader);
    xxDestroyBlendState(g_blendState);
    xxDestroyDepthStencilState(g_depthStencilState);
    xxDestroyRasterizerState(g_rasterizerState);
    xxDestroyPipeline(g_pipeline);
    g_fontTexture = 0;
    g_fontSampler = 0;
    g_vertexAttribute = 0;
    g_vertexShader = 0;
    g_fragmentShader = 0;
    g_blendState = 0;
    g_depthStencilState = 0;
    g_rasterizerState = 0;
    g_pipeline = 0;
    ImGui_ImplXX_InvalidateDeviceObjectsForPlatformWindows();
}

void ImGui_ImplXX_NewFrame()
{
    if (g_fontTexture == 0)
        ImGui_ImplXX_CreateDeviceObjects();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void ImGui_ImplXX_CreateWindow(ImGuiViewport* viewport)
{
    ImGuiViewportDataXX* data = IM_NEW(ImGuiViewportDataXX)();
    viewport->RendererUserData = data;

    if (ImGui::GetMainViewport() == viewport)
        return;

    // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level handle (e.g. GLFWWindow*, SDL_Window*).
    // Some back-ends will leave PlatformHandleRaw NULL, in which case we assume PlatformHandle will contain the HWND.
    void* handle = viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw : viewport->PlatformHandle;
    IM_ASSERT(handle != 0);

    data->Swapchain = xxCreateSwapchain(g_device, g_renderPass, handle, (int)viewport->Size.x, (int)viewport->Size.y, 0);
    data->RenderPass = xxCreateRenderPass(g_device, true, true, true, true, true, true);
    data->Handle = handle;
    data->Width = (int)viewport->Size.x;
    data->Height = (int)viewport->Size.y;
    IM_ASSERT(data->Swapchain != 0);
    IM_ASSERT(data->RenderPass != 0);
}

static void ImGui_ImplXX_DestroyWindow(ImGuiViewport* viewport)
{
    // The main viewport (owned by the application) will always have RendererUserData == NULL since we didn't create the data for it.
    if (ImGuiViewportDataXX* data = (ImGuiViewportDataXX*)viewport->RendererUserData)
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            xxDestroyBuffer(g_device, data->VertexBuffers[i]);
            xxDestroyBuffer(g_device, data->IndexBuffers[i]);
            xxDestroyBuffer(g_device, data->ConstantBuffers[i]);
            data->VertexBuffers[i] = 0;
            data->IndexBuffers[i] = 0;
            data->ConstantBuffers[i] = 0;
        }
        xxDestroySwapchain(data->Swapchain);
        xxDestroyRenderPass(data->RenderPass);
        data->Swapchain = 0;
        data->RenderPass = 0;
        IM_DELETE(data);
    }
    viewport->RendererUserData = nullptr;
}

static void ImGui_ImplXX_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiViewportDataXX* data = (ImGuiViewportDataXX*)viewport->RendererUserData;
    data->Swapchain = xxCreateSwapchain(g_device, g_renderPass, data->Handle, (int)size.x, (int)size.y, data->Swapchain);
    data->Width = (int)size.x;
    data->Height = (int)size.y;
    IM_ASSERT(data->Swapchain != 0);
}

static void ImGui_ImplXX_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGuiViewportDataXX* data = (ImGuiViewportDataXX*)viewport->RendererUserData;

    float scale = 1.0f;
    uint64_t commandBuffer = xxGetCommandBuffer(g_device, data->Swapchain);
    uint64_t framebuffer = xxGetFramebuffer(g_device, data->Swapchain, &scale);
    xxBeginCommandBuffer(commandBuffer);

    viewport->DpiScale = scale;
    viewport->DrawData->FramebufferScale = ImVec2(scale, scale);

    int width = (int)(data->Width * scale);
    int height = (int)(data->Height * scale);
    uint64_t commandEncoder = xxBeginRenderPass(commandBuffer, framebuffer, data->RenderPass, width, height, data->Color, 1.0f, 0);
    ImGui_ImplXX_RenderDrawData(viewport->DrawData, commandEncoder);
    xxEndRenderPass(commandEncoder, framebuffer, data->RenderPass);

    xxEndCommandBuffer(commandBuffer);
    xxSubmitCommandBuffer(commandBuffer, data->Swapchain);
}

static void ImGui_ImplXX_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGuiViewportDataXX* data = (ImGuiViewportDataXX*)viewport->RendererUserData;
    xxPresentSwapchain(data->Swapchain);
}

static void ImGui_ImplXX_InitPlatformInterface()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow = ImGui_ImplXX_CreateWindow;
    platform_io.Renderer_DestroyWindow = ImGui_ImplXX_DestroyWindow;
    platform_io.Renderer_SetWindowSize = ImGui_ImplXX_SetWindowSize;
    platform_io.Renderer_RenderWindow = ImGui_ImplXX_RenderWindow;
    platform_io.Renderer_SwapBuffers = ImGui_ImplXX_SwapBuffers;
}

static void ImGui_ImplXX_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}

static void ImGui_ImplXX_CreateDeviceObjectsForPlatformWindows()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int i = 0; i < platform_io.Viewports.Size; i++)
        if (!platform_io.Viewports[i]->RendererUserData)
            ImGui_ImplXX_CreateWindow(platform_io.Viewports[i]);
}

static void ImGui_ImplXX_InvalidateDeviceObjectsForPlatformWindows()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int i = 0; i < platform_io.Viewports.Size; i++)
        if (platform_io.Viewports[i]->RendererUserData)
            ImGui_ImplXX_DestroyWindow(platform_io.Viewports[i]);
}
