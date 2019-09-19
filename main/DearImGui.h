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
    static void* Update(void* view);
    static void Shutdown();

    static void HandleEventOSX(void* event, void* view);
};
