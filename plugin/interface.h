//==============================================================================
// xxImGui : Plugin Interface Header
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include <xxGraphic/xxSystem.h>
#include <imgui/imgui.h>

#if defined(_MSC_VER) && defined(PLUGIN_BUILD_LIBRARY)
#   define pluginAPI xxEXTERN __declspec(dllexport)
#elif defined(_MSC_VER)
#   define pluginAPI xxEXTERN __declspec(dllimport)
#else
#   define pluginAPI xxEXTERN
#endif

struct CreateData
{
    uint64_t    device;
    const char* baseFolder;
};

struct ShutdownData
{

};

struct UpdateData
{
    uint64_t    instance;
    uint64_t    device;
    uint64_t    renderPass;
    int         width;
    int         height;
    float       time;
    float       windowScale;
};

struct RenderData
{
    uint64_t    instance;
    uint64_t    device;
    uint64_t    renderPass;
    uint64_t    commandBuffer;
    uint64_t    commandEncoder;
    uint64_t    commandFramebuffer;
    int         width;
    int         height;
};

typedef const char* (*PFN_PLUGIN_CREATE)(const CreateData&);
typedef void (*PFN_PLUGIN_SHUTDOWN)(const ShutdownData&);
typedef bool (*PFN_PLUGIN_UPDATE)(const UpdateData&);
typedef void (*PFN_PLUGIN_RENDER)(const RenderData&);
