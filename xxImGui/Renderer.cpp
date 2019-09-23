//==============================================================================
// xxImGui : Renderer Source
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#if defined(xxWINDOWS)
#if defined(_M_IX86)
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
#else
        shortName = "GLES2";
#endif
    }

    switch (xxHash(shortName))
    {
#if defined(xxWINDOWS)
#if defined(_M_IX86)
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
    { "Metal",          xxGetDeviceNameMetal()          },
#endif
    { "NULL",           xxGetDeviceNameNULL()           },
    { "Vulkan",         xxGetDeviceNameVulkan()         },
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
