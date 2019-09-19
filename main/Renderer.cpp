//==============================================================================
// xxImGui : Renderer Source
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#if defined(xxWINDOWS)
#if defined(_M_IX86)
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

uint64_t Renderer::g_instance = 0;
uint64_t Renderer::g_device = 0;
uint64_t Renderer::g_renderPass = 0;
uint64_t Renderer::g_swapchain = 0;
//==============================================================================
//  Renderer
//==============================================================================
bool Renderer::Create(void* view, const char* shortName)
{
    if (g_instance != 0)
        return false;

    if (view == nullptr)
        return false;

    if (shortName == nullptr)
    {
#if defined(xxWINDOWS)
        shortName = "D3D11";
#elif defined(xxMACOS) || defined(xxIOS)
        shortName = "Metal";
#endif
    }

    switch (xxHash(shortName))
    {
#if defined(xxWINDOWS)
#if defined(_M_IX86)
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
    case xxHash("D3D11"):       g_instance = xxCreateInstanceD3D11();           break;
    case xxHash("D3D11On12"):   g_instance = xxCreateInstanceD3D11On12();       break;
    case xxHash("D3D12"):       g_instance = xxCreateInstanceD3D12();           break;
#endif
    case xxHash("GLES2"):       g_instance = xxCreateInstanceGLES2();           break;
#if defined(xxMACOS) || defined(xxIOS)
    case xxHash("Metal"):       g_instance = xxCreateInstanceMetal();           break;
#endif
    case xxHash("NULL"):        g_instance = xxCreateInstanceNULL();            break;
    case xxHash("Vulkan"):      g_instance = xxCreateInstanceVulkan();          break;
    default:                                                                    break;
    }
    if (g_instance == 0)
        return false;

    g_device = xxCreateDevice(g_instance);
    g_renderPass = xxCreateRenderPass(g_device, true, true, true, true, true, true);
    g_swapchain = xxCreateSwapchain(g_device, g_renderPass, view, 0, 0);
    return true;
}
//------------------------------------------------------------------------------
void Renderer::Reset(void* view, int width, int height)
{
    if (g_swapchain)
    {
        xxDestroySwapchain(g_swapchain);
        xxResetDevice(g_device);
        g_swapchain = 0;
        g_swapchain = xxCreateSwapchain(g_device, g_renderPass, view, width, height);
    }
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
static struct { const char* shortName; const char* fullName; } g_graphicList[] =
{
#if defined(xxWINDOWS)
#if defined(_M_IX86)
    { "D3D8",           xxGetDeviceStringD3D8(0)            },
    { "D3D8PS",         xxGetDeviceStringD3D8PS(0)          },
#endif
    { "D3D9",           xxGetDeviceStringD3D9(0)            },
    { "D3D9PS",         xxGetDeviceStringD3D9PS(0)          },
    { "D3D9Ex",         xxGetDeviceStringD3D9Ex(0)          },
    { "D3D9ExPS",       xxGetDeviceStringD3D9ExPS(0)        },
    { "D3D9On12",       xxGetDeviceStringD3D9On12(0)        },
    { "D3D9On12PS",     xxGetDeviceStringD3D9On12PS(0)      },
    { "D3D9On12Ex",     xxGetDeviceStringD3D9On12Ex(0)      },
    { "D3D9On12ExPS",   xxGetDeviceStringD3D9On12ExPS(0)    },
    { "D3D10",          xxGetDeviceStringD3D10(0)           },
    { "D3D10_1",        xxGetDeviceStringD3D10_1(0)         },
    { "D3D11",          xxGetDeviceStringD3D11(0)           },
    { "D3D11On12",      xxGetDeviceStringD3D11On12(0)       },
    { "D3D12",          xxGetDeviceStringD3D12(0)           },
#endif

    { "GLES2",          xxGetDeviceStringGLES2(0)           },
#if defined(xxMACOS) || defined(xxIOS)
    { "Metal",          xxGetDeviceStringMetal(0)           },
#endif
    { "NULL",           xxGetDeviceStringNULL(0)            },
    { "Vulkan",         xxGetDeviceStringVulkan(0)          },
};
//------------------------------------------------------------------------------
const char* Renderer::GetCurrentFullName()
{
    return xxGetDeviceString(0);
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
