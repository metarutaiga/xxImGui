//==============================================================================
// xxImGui : Plugin Header
//
// Copyright (c) 2019-2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include "plugin/interface.h"

class Plugin
{
public:
    static void Create(const char* path, uint64_t device);
    static void Shutdown();

    static int Count();
    static bool Update();
    static void Render(uint64_t commandEncoder);
};
