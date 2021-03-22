//==============================================================================
// xxImGui : Renderer Source
//
// Copyright (c) 2019-2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#if defined(xxWINDOWS)
#if defined(_M_IX86)
#include <xxGraphic/xxGraphicD3D6.h>
#include <xxGraphic/xxGraphicD3D7.h>
#include <xxGraphic/xxGraphicD3D8.h>
#include <xxGraphic/xxGraphicD3D8PS.h>
#endif
#include <xxGraphic/xxGraphicD3D9.h>
#include <xxGraphic/xxGraphicD3D9PS.h>
#include <xxGraphic/xxGraphicD3D9Ex.h>
#include <xxGraphic/xxGraphicD3D9On12.h>
#include <xxGraphic/xxGraphicD3D10.h>
#include <xxGraphic/xxGraphicD3D10_1.h>
#include <xxGraphic/xxGraphicD3D11.h>
#include <xxGraphic/xxGraphicD3D11On12.h>
#include <xxGraphic/xxGraphicD3D12.h>
#endif
#include <xxGraphic/xxGraphicGLES2.h>
#include <xxGraphic/xxGraphicGLES3.h>
#include <xxGraphic/xxGraphicGLES31.h>
#include <xxGraphic/xxGraphicGLES32.h>
#if defined(xxMACOS) || defined(xxIOS)
#include <xxGraphic/xxGraphicMetal.h>
#include <xxGraphic/xxGraphicMetal2.h>
#endif
#include <xxGraphic/xxGraphicNULL.h>
#include <xxGraphic/xxGraphicVulkan.h>

#include "Renderer.h"

uint64_t    Renderer::g_instance = 0;
uint64_t    Renderer::g_device = 0;
uint64_t    Renderer::g_renderPass = 0;
uint64_t    Renderer::g_swapchain = 0;
uint64_t    Renderer::g_currentCommandBuffer = 0;
uint64_t    Renderer::g_currentCommandEncoder = 0;
uint64_t    Renderer::g_currentCommandFramebuffer = 0;
void*       Renderer::g_view = nullptr;
int         Renderer::g_width = 0;
int         Renderer::g_height = 0;
float       Renderer::g_scale = 1.0f;
float       Renderer::g_clearColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
float       Renderer::g_clearDepth = 1.0f;
char        Renderer::g_clearStencil = 0;
//==============================================================================
//  List
//==============================================================================
static struct { const char* shortName; const char* fullName; uint64_t (*createInstance)(); } g_graphicList[] =
{
#if defined(xxWINDOWS)
#if defined(_M_IX86)
    { "D3D6",           xxGetDeviceNameD3D6(),          xxCreateInstanceD3D6            },
    { "D3D7",           xxGetDeviceNameD3D7(),          xxCreateInstanceD3D7            },
    { "D3D8",           xxGetDeviceNameD3D8(),          xxCreateInstanceD3D8            },
    { "D3D8PS",         xxGetDeviceNameD3D8PS(),        xxCreateInstanceD3D8PS          },
#endif
    { "D3D9",           xxGetDeviceNameD3D9(),          xxCreateInstanceD3D9            },
    { "D3D9PS",         xxGetDeviceNameD3D9PS(),        xxCreateInstanceD3D9PS          },
    { "D3D9Ex",         xxGetDeviceNameD3D9Ex(),        xxCreateInstanceD3D9Ex          },
    { "D3D9ExPS",       xxGetDeviceNameD3D9ExPS(),      xxCreateInstanceD3D9ExPS        },
    { "D3D9On12",       xxGetDeviceNameD3D9On12(),      xxCreateInstanceD3D9On12        },
    { "D3D9On12PS",     xxGetDeviceNameD3D9On12PS(),    xxCreateInstanceD3D9On12PS      },
    { "D3D9On12Ex",     xxGetDeviceNameD3D9On12Ex(),    xxCreateInstanceD3D9On12Ex      },
    { "D3D9On12ExPS",   xxGetDeviceNameD3D9On12ExPS(),  xxCreateInstanceD3D9On12ExPS    },
    { "D3D10",          xxGetDeviceNameD3D10(),         xxCreateInstanceD3D10           },
    { "D3D10_1",        xxGetDeviceNameD3D10_1(),       xxCreateInstanceD3D10_1         },
    { "D3D11",          xxGetDeviceNameD3D11(),         xxCreateInstanceD3D11           },
    { "D3D11On12",      xxGetDeviceNameD3D11On12(),     xxCreateInstanceD3D11On12       },
    { "D3D12",          xxGetDeviceNameD3D12(),         xxCreateInstanceD3D12           },
#endif
#if defined(xxMACCATALYST)
#else
    { "GLES2",          xxGetDeviceNameGLES2(),         xxCreateInstanceGLES2           },
    { "GLES3",          xxGetDeviceNameGLES3(),         xxCreateInstanceGLES3           },
    { "GLES31",         xxGetDeviceNameGLES31(),        xxCreateInstanceGLES31          },
    { "GLES32",         xxGetDeviceNameGLES32(),        xxCreateInstanceGLES32          },
#endif
#if defined(xxMACOS) || defined(xxIOS)
    { "MTL",            xxGetDeviceNameMetal(),         xxCreateInstanceMetal           },
    { "MTL2",           xxGetDeviceNameMetal2(),        xxCreateInstanceMetal2          },
#endif
    { "NULL",           xxGetDeviceNameNULL(),          xxCreateInstanceNULL            },
    { "VK",             xxGetDeviceNameVulkan(),        xxCreateInstanceVulkan          },
};
//==============================================================================
//  Renderer
//==============================================================================
bool Renderer::Create(void* view, int width, int height, const char* shortName)
{
    if (g_instance != 0)
        return false;

    if (view == nullptr || width == 0 || height == 0)
        return false;

    if (shortName == nullptr)
    {
#if defined(xxWINDOWS) && defined(__llvm__)
        shortName = "D3D9";
#elif defined(xxWINDOWS)
        shortName = "D3D11";
#elif defined(xxMACOS) || defined(xxIOS)
        shortName = "MTL";
#else
        shortName = "GLES2";
#endif
    }

    unsigned int hashShortName = xxHash(shortName);
    for (int i = 0; i < xxCountOf(g_graphicList); ++i)
    {
        if (xxHash(g_graphicList[i].shortName) != hashShortName)
            continue;
        g_instance = g_graphicList[i].createInstance();
        break;
    }
    if (g_instance == 0)
        return false;

    g_device = xxCreateDevice(g_instance);
    g_renderPass = xxCreateRenderPass(g_device, true, true, true, true, true, true);
    g_swapchain = xxCreateSwapchain(g_device, g_renderPass, view, width, height, 0);
    g_view = view;
    g_width = width;
    g_height = height;
    return true;
}
//------------------------------------------------------------------------------
void Renderer::Reset(void* view, int width, int height)
{
    if (g_swapchain == 0)
        return;

    if (view == nullptr || width == 0 || height == 0)
        return;

    uint64_t oldSwapchain = g_swapchain;
    g_swapchain = 0;
    g_swapchain = xxCreateSwapchain(g_device, g_renderPass, view, width, height, oldSwapchain);
    g_view = view;
    g_width = width;
    g_height = height;
}
//------------------------------------------------------------------------------
void Renderer::Shutdown()
{
    xxDestroySwapchain(g_swapchain);
    xxDestroyRenderPass(g_renderPass);
    xxDestroyDevice(g_device);
    xxDestroyInstance(g_instance);
    g_swapchain = 0;
    g_renderPass = 0;
    g_device = 0;
    g_instance = 0;
}
//------------------------------------------------------------------------------
uint64_t Renderer::Begin()
{
    uint64_t commandBuffer = xxGetCommandBuffer(g_device, g_swapchain);
    uint64_t framebuffer = xxGetFramebuffer(g_device, g_swapchain, &g_scale);
    xxBeginCommandBuffer(commandBuffer);

    int width = (int)(g_width * g_scale);
    int height = (int)(g_height * g_scale);
    uint64_t commandEncoder = xxBeginRenderPass(commandBuffer, framebuffer, g_renderPass, width, height, g_clearColor, g_clearDepth, g_clearStencil);

    g_currentCommandBuffer = commandBuffer;
    g_currentCommandEncoder = commandEncoder;
    g_currentCommandFramebuffer = framebuffer;

    return commandEncoder;
}
//------------------------------------------------------------------------------
void Renderer::End()
{
    xxEndRenderPass(g_currentCommandEncoder, g_currentCommandFramebuffer, g_renderPass);

    xxEndCommandBuffer(g_currentCommandBuffer);
    xxSubmitCommandBuffer(g_currentCommandBuffer, g_swapchain);

    g_currentCommandBuffer = 0;
    g_currentCommandEncoder = 0;
    g_currentCommandFramebuffer = 0;
}
//------------------------------------------------------------------------------
bool Renderer::Present()
{
    xxPresentSwapchain(g_swapchain);

    return xxTestDevice(g_device);
}
//------------------------------------------------------------------------------
const char* Renderer::GetCurrentFullName()
{
    return xxGetDeviceName();
}
//------------------------------------------------------------------------------
const char* Renderer::GetGraphicFullName(int index)
{
    if (index >= xxCountOf(g_graphicList))
        return nullptr;

    return g_graphicList[index].fullName;
}
//------------------------------------------------------------------------------
const char* Renderer::GetGraphicShortName(int index)
{
    if (index >= xxCountOf(g_graphicList))
        return nullptr;

    return g_graphicList[index].shortName;
}
//------------------------------------------------------------------------------
