//==============================================================================
// xxImGui : Dear ImGui Header
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include <imgui/imgui.h>

class DearImGui
{
public:
    static void Create(void* view, float scale);
    static void Shutdown();

    static void Suspend();
    static void Resume();

    static void NewFrame();
    static void Update();
    static void* PostUpdate(void* view);

    static void Render(uint64_t commandEncoder);

    static void HandleEventOSX(void* event, void* view);

protected:
    static const char*  g_graphicShortName;
    static bool         g_recreateWindow;
};
