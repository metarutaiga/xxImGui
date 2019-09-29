//==============================================================================
// xxImGui : Plugin Interface Header
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include <imgui/imgui.h>
#include <xxGraphic/xxSystem.h>

struct CreateData
{

};

struct ShutdownData
{

};

struct UpdateData
{
    float       time;
};

struct RenderData
{
    uint64_t    commandEncoder;
};

typedef const char* (*PFN_PLUGIN_CREATE)(const CreateData&);
typedef void (*PFN_PLUGIN_SHUTDOWN)(const ShutdownData&);
typedef void (*PFN_PLUGIN_UPDATE)(const UpdateData&);
typedef void (*PFN_PLUGIN_RENDER)(const RenderData&);
