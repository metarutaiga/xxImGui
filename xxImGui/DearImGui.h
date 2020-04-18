//==============================================================================
// xxImGui : Dear ImGui Header
//
// Copyright (c) 2019-2020 TAiGA
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

    static void NewFrame(void* view);
    static void Update(bool demo);
    static void* PostUpdate(void* view);

    static void Render(uint64_t commandEncoder);

    static void HandleEventOSX(void* event, void* view);
    static void HandleEventAndroid(int type, float x, float y);

protected:
    static const char*  g_graphicShortName;
    static bool         g_recreateWindow;
};
