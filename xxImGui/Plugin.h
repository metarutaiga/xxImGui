//==============================================================================
// xxImGui : Plugin Header
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include "plugin/interface.h"

class Plugin
{
public:
    static void Create(const char* path);
    static void Shutdown();

    static bool Update();
    static void Render(uint64_t commandEncoder);
};
