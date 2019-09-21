//==============================================================================
// xxImGui : Renderer Header
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include <xxGraphic/xxGraphic.h>

class Renderer
{
public:
    static bool Create(void* view, const char* shortName = nullptr);
    static void Reset(void* view, int width = 0, int height = 0);
    static void Shutdown();

    static const char* GetCurrentFullName();
    static const char* GetGraphicFullName(int index);
    static const char* GetGraphicShortName(int index);

public:
    static uint64_t g_instance;
    static uint64_t g_device;
    static uint64_t g_renderPass;
    static uint64_t g_swapchain;
};
