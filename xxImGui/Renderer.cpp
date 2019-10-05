//==============================================================================
// xxImGui : Renderer Source
//
// Copyright (c) 2019 TAiGA
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
#if defined(xxMACOS) || defined(xxIOS)
#include <xxGraphic/xxGraphicMetal.h>
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
float       Renderer::g_clearColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
float       Renderer::g_clearDepth = 1.0f;
char        Renderer::g_clearStencil = 0;
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
#if defined(xxWINDOWS)
        shortName = "D3D11";
#elif defined(xxMACOS) || defined(xxIOS)
        shortName = "MTL";
#else
        shortName = "GLES2";
#endif
    }

    switch (xxHash(shortName))
    {
#if defined(xxWINDOWS)
#if defined(_M_IX86)
    case xxHash("D3D6"):        g_instance = xxCreateInstanceD3D6();            break;
    case xxHash("D3D7"):        g_instance = xxCreateInstanceD3D7();            break;
    case xxHash("D3D8"):        g_instance = xxCreateInstanceD3D8();            break;
    case xxHash("D3D8PS"):      g_instance = xxCreateInstanceD3D8PS();          break;
#endif
    case xxHash("D3D9"):        g_instance = xxCreateInstanceD3D9();            break;
    case xxHash("D3D9PS"):      g_instance = xxCreateInstanceD3D9PS();          break;
    case xxHash("D3D9Ex"):      g_instance = xxCreateInstanceD3D9Ex();          break;
    case xxHash("D3D9ExPS"):    g_instance = xxCreateInstanceD3D9ExPS();        break;
    case xxHash("D3D9On12"):    g_instance = xxCreateInstanceD3D9On12();        break;
    case xxHash("D3D9On12PS"):  g_instance = xxCreateInstanceD3D9On12PS();      break;
    case xxHash("D3D9On12Ex"):  g_instance = xxCreateInstanceD3D9On12Ex();      break;
    case xxHash("D3D9On12ExPS"):g_instance = xxCreateInstanceD3D9On12ExPS();    break;
    case xxHash("D3D10"):       g_instance = xxCreateInstanceD3D10();           break;
    case xxHash("D3D10_1"):     g_instance = xxCreateInstanceD3D10_1();         break;
    default:
    case xxHash("D3D11"):       g_instance = xxCreateInstanceD3D11();           break;
    case xxHash("D3D11On12"):   g_instance = xxCreateInstanceD3D11On12();       break;
    case xxHash("D3D12"):       g_instance = xxCreateInstanceD3D12();           break;
#endif
#if defined(xxANDROID)
    default:
#endif
    case xxHash("GLES2"):       g_instance = xxCreateInstanceGLES2();           break;
#if defined(xxMACOS) || defined(xxIOS)
    default:
    case xxHash("MTL"):         g_instance = xxCreateInstanceMetal();           break;
#endif
    case xxHash("NULL"):        g_instance = xxCreateInstanceNULL();            break;
    case xxHash("VK"):          g_instance = xxCreateInstanceVulkan();          break;
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
    uint64_t framebuffer = xxGetFramebuffer(g_device, g_swapchain);
    xxBeginCommandBuffer(commandBuffer);

    uint64_t commandEncoder = xxBeginRenderPass(commandBuffer, framebuffer, g_renderPass, g_width, g_height, g_clearColor[0], g_clearColor[1], g_clearColor[2], g_clearColor[3], g_clearDepth, g_clearStencil);

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
static struct { const char* shortName; const char* fullName; } g_graphicList[] =
{
#if defined(xxWINDOWS)
#if defined(_M_IX86)
    { "D3D6",           xxGetDeviceNameD3D6()           },
    { "D3D7",           xxGetDeviceNameD3D7()           },
    { "D3D8",           xxGetDeviceNameD3D8()           },
    { "D3D8PS",         xxGetDeviceNameD3D8PS()         },
#endif
    { "D3D9",           xxGetDeviceNameD3D9()           },
    { "D3D9PS",         xxGetDeviceNameD3D9PS()         },
    { "D3D9Ex",         xxGetDeviceNameD3D9Ex()         },
    { "D3D9ExPS",       xxGetDeviceNameD3D9ExPS()       },
    { "D3D9On12",       xxGetDeviceNameD3D9On12()       },
    { "D3D9On12PS",     xxGetDeviceNameD3D9On12PS()     },
    { "D3D9On12Ex",     xxGetDeviceNameD3D9On12Ex()     },
    { "D3D9On12ExPS",   xxGetDeviceNameD3D9On12ExPS()   },
    { "D3D10",          xxGetDeviceNameD3D10()          },
    { "D3D10_1",        xxGetDeviceNameD3D10_1()        },
    { "D3D11",          xxGetDeviceNameD3D11()          },
    { "D3D11On12",      xxGetDeviceNameD3D11On12()      },
    { "D3D12",          xxGetDeviceNameD3D12()          },
#endif
    { "GLES2",          xxGetDeviceNameGLES2()          },
#if defined(xxMACOS) || defined(xxIOS)
    { "MTL",            xxGetDeviceNameMetal()          },
#endif
    { "NULL",           xxGetDeviceNameNULL()           },
    { "VK",             xxGetDeviceNameVulkan()         },
};
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
