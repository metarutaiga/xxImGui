//==============================================================================
// xxImGui : Plugin Matrix Source
//
// Copyright (c) 2019-2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <interface.h>

#define PLUGIN_NAME     "Matrix"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

//------------------------------------------------------------------------------
namespace ImGui
{
bool InputShort(const char* label, short* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
{
    int temp = (*v);
    bool result = ImGui::InputInt(label, &temp, step, step_fast, flags);
    (*v) = temp;
    return result;
};
bool InputFloat16(const char* label, _Float16* v)
{
    float temp = (*v);
    bool result = ImGui::InputFloat(label, &temp);
    (*v) = temp;
    return result;
};
}
//------------------------------------------------------------------------------
//  https://gist.github.com/dougallj/7cba721da1a94da725ee37c1e9cd1f21
//------------------------------------------------------------------------------
union amx_access
{
    struct
    {
        uint64_t address:56;
        uint64_t register_index:6;
        uint64_t double_width:1;
    };
    uint64_t value;
};
//------------------------------------------------------------------------------
union amx_operands
{
    struct
    {
        uint64_t offset_x:10;
        uint64_t offset_y:10;
        uint64_t offset_z:7;
        uint64_t clear_z:1;
        uint64_t skip_y:1;
        uint64_t skip_x:1;
        uint64_t dummy_30:2;
        uint64_t disable_x:7;
        uint64_t dummy_39:2;
        uint64_t disable_y:7;
        uint64_t accumulate:1;
        uint64_t dummy_49:13;
        uint64_t mode_32:1;
        uint64_t vector_matrix_add:1;
    };
    uint64_t value;
};
//------------------------------------------------------------------------------
#define AMX_LDX(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 0 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_LDY(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 1 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_STX(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 2 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_STY(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 3 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_LDZ(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 4 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_STZ(V)      __asm__ volatile("mov x0, %0        \n .word (0x201000 | ( 5 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_FMA64(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (10 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_FMA32(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (12 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_MAC16(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (14 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_FMA16(V)    __asm__ volatile("mov x0, %0        \n .word (0x201000 | (15 << 5) | 0)" ::"r"(V) : "x0", "memory")
#define AMX_START()     __asm__ volatile("nop \n nop \n nop \n .word (0x201000 | (17 << 5) | 0)" ::: "memory")
#define AMX_STOP()      __asm__ volatile("nop \n nop \n nop \n .word (0x201000 | (17 << 5) | 1)" ::: "memory")
//------------------------------------------------------------------------------
union vector
{
    double      f64[8];
    float       f32[16];
    _Float16    f16[32];
    int16_t     i16[32];
};
static vector matrixX;
static vector matrixY;
static vector matrixZ[32];
static int mode;
#if defined(__APPLE__) && defined(__aarch64__)
static bool amx;
static uint64_t amxX[64];
static uint64_t amxY[64];
static uint64_t amxZ[64 * 64];
//------------------------------------------------------------------------------
static void amxDump()
{
    amx_access access = {};
    access.address = (uint64_t)&amxX;
    access.register_index = 0;
    for (int i = 0; i < 8; ++i)
    {
        AMX_STX(access.value);
        access.address += 0x40;
        access.register_index++;
    }
    access.address = (uint64_t)&amxY;
    access.register_index = 0;
    for (int i = 0; i < 8; ++i)
    {
        AMX_STY(access.value);
        access.address += 0x40;
        access.register_index++;
    }
    access.address = (uint64_t)&amxZ;
    access.register_index = 0;
    for (int i = 0; i < 8 * 8; ++i)
    {
        AMX_STZ(access.value);
        access.address += 0x40;
        access.register_index++;
    }
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
static void SetZeroX()
{
    memset(&matrixX, 0, sizeof(matrixX));
}
//------------------------------------------------------------------------------
static void SetZeroY()
{
    memset(&matrixY, 0, sizeof(matrixY));
}
//------------------------------------------------------------------------------
static void SetZeroZ()
{
    memset(&matrixZ, 0, sizeof(matrixZ));
}
//------------------------------------------------------------------------------
static void SetZero()
{
    SetZeroX();
    SetZeroY();
    SetZeroZ();
}
//------------------------------------------------------------------------------
static void SetIdentity()
{
    SetZero();
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
        {
            matrixX.f64[i] = 1.0;
            matrixY.f64[i] = 1.0;
            matrixZ[i].f64[i] = 1.0;
        }
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
        {
            matrixX.f32[i] = 1.0f;
            matrixY.f32[i] = 1.0f;
            matrixZ[i].f32[i] = 1.0f;
        }
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
        {
            matrixX.f16[i] = 1.0f16;
            matrixY.f16[i] = 1.0f16;
            matrixZ[i].f16[i] = 1.0f16;
        }
        break;
    case 3:
        for (int i = 0; i < 32; ++i)
        {
            matrixX.i16[i] = 1;
            matrixY.i16[i] = 1;
            matrixZ[i].i16[i] = 1;
        }
        break;
    }
}
//------------------------------------------------------------------------------
static void SequenceX()
{
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
            matrixX.f64[i] = i + 1;
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
            matrixX.f32[i] = i + 1;
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
            matrixX.f16[i] = i + 1;
        break;
    case 3:
        for (int i = 0; i < 32; ++i)
            matrixX.i16[i] = i + 1;
        break;
    }
}
//------------------------------------------------------------------------------
static void SequenceY()
{
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
            matrixY.f64[i] = i + 1;
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
            matrixY.f32[i] = i + 1;
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
            matrixY.f16[i] = i + 1;
        break;
    case 3:
        for (int i = 0; i < 32; ++i)
            matrixY.i16[i] = i + 1;
        break;
    }
}
//------------------------------------------------------------------------------
static void RandomX()
{
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
            matrixX.f64[i] = rand() / (double)RAND_MAX;
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
            matrixX.f32[i] = rand() / (float)RAND_MAX;
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
            matrixX.f16[i] = rand() / (float)RAND_MAX;
        break;
    case 3:
        for (int i = 0; i < 32; ++i)
            matrixX.i16[i] = (int8_t)rand();
        break;
    }
}
//------------------------------------------------------------------------------
static void RandomY()
{
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
            matrixY.f64[i] = rand() / (double)RAND_MAX;
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
            matrixY.f32[i] = rand() / (float)RAND_MAX;
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
            matrixY.f16[i] = rand() / (float)RAND_MAX;
        break;
    case 3:
        for (int i = 0; i < 32; ++i)
            matrixY.i16[i] = (int8_t)rand();
        break;
    }
}
//------------------------------------------------------------------------------
static void RandomZ()
{
    switch (mode)
    {
    case 0:
        for (int j = 0; j < 8; ++j)
            for (int i = 0; i < 8; ++i)
                matrixZ[j].f64[i] = rand() / (double)RAND_MAX;
        break;
    case 1:
        for (int j = 0; j < 16; ++j)
            for (int i = 0; i < 16; ++i)
                matrixZ[j].f32[i] = rand() / (float)RAND_MAX;
        break;
    case 2:
        for (int j = 0; j < 32; ++j)
            for (int i = 0; i < 32; ++i)
                matrixZ[j].f16[i] = rand() / (float)RAND_MAX;
        break;
    case 3:
        for (int j = 0; j < 32; ++j)
            for (int i = 0; i < 32; ++i)
                matrixZ[j].i16[i] = rand() % (256);
        break;
    }
}
//------------------------------------------------------------------------------
static void VectorMultiplyVector()
{
    SetZeroZ();
    switch (mode)
    {
    case 0:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands operands = {};
            operands.clear_z = true;
            amx_access access = {};
            access.address = (uint64_t)&matrixZ;
            AMX_START();
            AMX_LDX(&matrixX);
            AMX_LDY(&matrixY);
            AMX_FMA64(operands.value);
            for (int i = 0; i < 8; ++i)
            {
                AMX_STZ(access.value);
                access.address += 0x40;
                access.register_index += 8;
            }
            amxDump();
            AMX_STOP();
            break;
        }
#endif
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                matrixZ[y].f64[x] = matrixX.f64[x] * matrixY.f64[y];
            }
        }
        break;
    case 1:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands operands = {};
            operands.clear_z = true;
            amx_access access = {};
            access.address = (uint64_t)&matrixZ;
            AMX_START();
            AMX_LDX(&matrixX);
            AMX_LDY(&matrixY);
            AMX_FMA32(operands.value);
            for (int i = 0; i < 16; ++i)
            {
                AMX_STZ(access.value);
                access.address += 0x40;
                access.register_index += 4;
            }
            amxDump();
            AMX_STOP();
            break;
        }
#endif
        for (int y = 0; y < 16; ++y)
        {
            for (int x = 0; x < 16; ++x)
            {
                matrixZ[y].f32[x] = matrixX.f32[x] * matrixY.f32[y];
            }
        }
        break;
    case 2:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands operands = {};
            operands.clear_z = true;
            amx_access access = {};
            access.address = (uint64_t)&matrixZ;
            AMX_START();
            AMX_LDX(&matrixX);
            AMX_LDY(&matrixY);
            AMX_FMA16(operands.value);
            for (int i = 0; i < 32; ++i)
            {
                AMX_STZ(access.value);
                access.address += 0x40;
                access.register_index += 2;
            }
            amxDump();
            AMX_STOP();
            break;
        }
#endif
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 32; ++x)
            {
                matrixZ[y].f16[x] = matrixX.f16[x] * matrixY.f16[y];
            }
        }
        break;
    case 3:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands operands = {};
            operands.clear_z = true;
            amx_access access = {};
            access.address = (uint64_t)&matrixZ;
            AMX_START();
            AMX_LDX(&matrixX);
            AMX_LDY(&matrixY);
            AMX_MAC16(operands.value);
            for (int i = 0; i < 32; ++i)
            {
                AMX_STZ(access.value);
                access.address += 0x40;
                access.register_index += 2;
            }
            amxDump();
            AMX_STOP();
            break;
        }
#endif
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 32; ++x)
            {
                matrixZ[y].i16[x] = matrixX.i16[x] * matrixY.i16[y];
            }
        }
        break;
    }
}
//------------------------------------------------------------------------------
static void VectorMultiplyMatrix(bool X)
{
    SetZeroZ();
    switch (mode)
    {
    case 0:
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                if (X)
                {
                    matrixZ[y].f64[x] = matrixZ[y].f64[x] * matrixX.f64[x];
                }
                else
                {
                    matrixZ[x].f64[y] = matrixZ[x].f64[y] * matrixY.f64[x];
                }
            }
        }
        break;
    case 1:
        for (int y = 0; y < 16; ++y)
        {
            for (int x = 0; x < 16; ++x)
            {
                if (X)
                {
                    matrixZ[y].f32[x] = matrixZ[y].f32[x] * matrixX.f32[x];
                }
                else
                {
                    matrixZ[x].f32[y] = matrixZ[x].f32[y] * matrixY.f32[x];
                }
            }
        }
        break;
    case 2:
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 32; ++x)
            {
                if (X)
                {
                    matrixZ[y].f16[x] = matrixZ[y].f16[x] * matrixX.f16[x];
                }
                else
                {
                    matrixZ[x].f16[y] = matrixZ[x].f16[y] * matrixY.f16[x];
                }
            }
        }
        break;
    case 3:
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 32; ++x)
            {
                if (X)
                {
                    matrixZ[y].i16[x] = matrixZ[y].i16[x] * matrixX.i16[x];
                }
                else
                {
                    matrixZ[x].i16[y] = matrixZ[x].i16[y] * matrixY.i16[x];
                }
            }
        }
        break;
    }
}
//------------------------------------------------------------------------------
static void UserInterface()
{
    int label = 0;
    auto GetLabel = [&]()
    {
        static char id[16];
        snprintf(id, 16, "##%d", label++);
        return id;
    };

    ImGui::RadioButton("64bit Floating", &mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("32bit Floating", &mode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("16bit Floating", &mode, 2);
    ImGui::SameLine();
    ImGui::RadioButton("16bit Integer", &mode, 3);
#if defined(__APPLE__) && defined(__aarch64__)
    ImGui::SameLine();
    ImGui::Checkbox("Apple AMX", &amx);
    if (amx)
    {
        ImGui::SetNextWindowSize(ImVec2(512, 512));
        if (ImGui::Begin("Apple AMX Registers", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float width = 7.0f;

            for (int x = 0; x < 64; ++x)
            {
                ImVec2 rect[2] =
                {
                    ImVec2(pos.x + (x * 1 + 2) * width, pos.y + 0 * width),
                    ImVec2(pos.x + (x * 1 + 3) * width, pos.y + 1 * width),
                };
                drawList->AddQuadFilled(ImVec2(rect[0].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[1].y),
                                        ImVec2(rect[0].x, rect[1].y),
                                        amxX[x] == 0 ? x / 8 & 1 ? 0xFF3F3F3F : 0x7F7F7F7F : 0xFFFFFFFF);
                if (ImGui::IsMouseHoveringRect(rect[0], rect[1]))
                    ImGui::SetTooltip("X:%d.%d (%016zX)", x / 8, x % 8, (size_t)amxX[x]);
            }
            for (int y = 0; y < 64; ++y)
            {
                ImVec2 rect[2] =
                {
                    ImVec2(pos.x + 0 * width, pos.y + (y * 1 + 2) * width),
                    ImVec2(pos.x + 1 * width, pos.y + (y * 1 + 3) * width),
                };
                drawList->AddQuadFilled(ImVec2(rect[0].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[1].y),
                                        ImVec2(rect[0].x, rect[1].y),
                                        amxY[y] == 0 ? y / 8 & 1 ? 0xFF3F3F3F : 0x7F7F7F7F : 0xFFFFFFFF);
                if (ImGui::IsMouseHoveringRect(rect[0], rect[1]))
                    ImGui::SetTooltip("Y:%d.%d (%016zX)", y / 8, y % 8, (size_t)amxY[y]);
                for (int x = 0; x < 64; ++x)
                {
                    ImVec2 rect[2] =
                    {
                        ImVec2(pos.x + (x * 1 + 2) * width, pos.y + (y * 1 + 2) * width),
                        ImVec2(pos.x + (x * 1 + 3) * width, pos.y + (y * 1 + 3) * width),
                    };
                    drawList->AddQuadFilled(ImVec2(rect[0].x, rect[0].y),
                                            ImVec2(rect[1].x, rect[0].y),
                                            ImVec2(rect[1].x, rect[1].y),
                                            ImVec2(rect[0].x, rect[1].y),
                                            amxZ[y * 64 + x] == 0 ? ((x / 8) ^ (y / 8)) & 1 ? 0xFF3F3F3F : 0x7F7F7F7F : 0xFFFFFFFF);
                    if (ImGui::IsMouseHoveringRect(rect[0], rect[1]))
                        ImGui::SetTooltip("Z:%d.%d,%d.%d (%016zX)", x / 8, x % 8, y / 8, y % 8, (size_t)amxZ[y * 64 + x]);
                }
            }
            ImGui::End();
        }
    }
#endif

    static int currentHoverX = -1;
    static int currentHoverY = -1;
    int hoverX = currentHoverX;
    int hoverY = currentHoverY;
    currentHoverX = -1;
    currentHoverY = -1;

    ImU32 unselectedColor = ImColor(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
    ImU32 selectedColor = ImColor(ImColor(unselectedColor).Value.z, ImColor(unselectedColor).Value.y, ImColor(unselectedColor).Value.x);

    float width = 0.0f;
    switch (mode)
    {
    case 0:
        width = 64.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 8; ++x)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x ? selectedColor : unselectedColor);
            ImGui::InputDouble(GetLabel(), &matrixX.f64[x]);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverX = x;
                ImGui::SetTooltip("X:%d (%f)", x, matrixX.f64[x]);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 8; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverY == y ? selectedColor : unselectedColor);
            ImGui::InputDouble(GetLabel(), &matrixY.f64[y]);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverY = y;
                ImGui::SetTooltip("Y:%d (%f)", y, matrixY.f64[y]);
            }
            ImGui::SameLine();
            for (int x = 0; x < 8; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x && hoverY == y ? selectedColor : unselectedColor);
                ImGui::InputDouble(GetLabel(), &matrixZ[y].f64[x]);
                ImGui::PopStyleColor(1);
                if (ImGui::IsItemHovered())
                {
                    currentHoverX = x;
                    currentHoverY = y;
                    ImGui::SetTooltip("X:%d (%f)\nY:%d (%f)\nZ:%d,%d (%f)", x, matrixX.f64[x], y, matrixY.f64[y], x, y, matrixZ[y].f64[x]);
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;
    case 1:
        width = 48.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 16; ++x)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x ? selectedColor : unselectedColor);
            ImGui::InputFloat(GetLabel(), &matrixX.f32[x]);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverX = x;
                ImGui::SetTooltip("X:%d (%f)", x, matrixX.f32[x]);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 16; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverY == y ? selectedColor : unselectedColor);
            ImGui::InputFloat(GetLabel(), &matrixY.f32[y]);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverY = y;
                ImGui::SetTooltip("Y:%d (%f)", y, matrixY.f32[y]);
            }
            ImGui::SameLine();
            for (int x = 0; x < 16; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x && hoverY == y ? selectedColor : unselectedColor);
                ImGui::InputFloat(GetLabel(), &matrixZ[y].f32[x]);
                ImGui::PopStyleColor(1);
                if (ImGui::IsItemHovered())
                {
                    currentHoverX = x;
                    currentHoverY = y;
                    ImGui::SetTooltip("X:%d (%f)\nY:%d (%f)\nZ:%d,%d (%f)", x, matrixX.f32[x], y, matrixY.f32[y], x, y, matrixZ[y].f32[x]);
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;
    case 2:
        width = 32.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 32; ++x)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x ? selectedColor : unselectedColor);
            ImGui::InputFloat16(GetLabel(), &matrixX.f16[x]);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverX = x;
                ImGui::SetTooltip("X:%d (%f)", x, (float)matrixX.f16[x]);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 32; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverY == y ? selectedColor : unselectedColor);
            ImGui::InputFloat16(GetLabel(), &matrixY.f16[y]);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverY = y;
                ImGui::SetTooltip("Y:%d (%f)", y, (float)matrixY.f16[y]);
            }
            ImGui::SameLine();
            for (int x = 0; x < 32; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x && hoverY == y ? selectedColor : unselectedColor);
                ImGui::InputFloat16(GetLabel(), &matrixZ[y].f16[x]);
                ImGui::PopStyleColor(1);
                if (ImGui::IsItemHovered())
                {
                    currentHoverX = x;
                    currentHoverY = y;
                    ImGui::SetTooltip("X:%d (%f)\nY:%d (%f)\nZ:%d,%d (%f)", x, (float)matrixX.f16[x], y, (float)matrixY.f16[y], x, y, (float)matrixZ[y].f16[x]);
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;
    case 3:
        width = 32.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 32; ++x)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x ? selectedColor : unselectedColor);
            ImGui::InputShort(GetLabel(), &matrixX.i16[x], 0);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverX = x;
                ImGui::SetTooltip("X:%d (%d)", x, matrixX.i16[x]);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 32; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverY == y ? selectedColor : unselectedColor);
            ImGui::InputShort(GetLabel(), &matrixY.i16[y], 0);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverY = y;
                ImGui::SetTooltip("Y:%d (%d)", y, matrixY.i16[y]);
            }
            ImGui::SameLine();
            for (int x = 0; x < 32; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x && hoverY == y ? selectedColor : unselectedColor);
                ImGui::InputShort(GetLabel(), &matrixZ[y].i16[x], 0);
                ImGui::PopStyleColor(1);
                if (ImGui::IsItemHovered())
                {
                    currentHoverX = x;
                    currentHoverY = y;
                    ImGui::SetTooltip("X:%d (%d)\nY:%d (%d)\nZ:%d,%d (%d)", x, matrixX.i16[x], y, matrixY.i16[y], x, y, matrixZ[y].i16[x]);
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;

    default:
        break;
    }
}
//------------------------------------------------------------------------------
pluginAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{

}
//------------------------------------------------------------------------------
pluginAPI bool Update(const UpdateData& updateData)
{
    static bool showAbout = false;
    static bool showMatrix = false;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Matrix", nullptr, &showMatrix);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showAbout)
    {
        if (ImGui::Begin("About " PLUGIN_NAME, &showAbout))
        {
            ImGui::Text("%s Plugin Version %d.%d", PLUGIN_NAME, PLUGIN_MAJOR, PLUGIN_MINOR);
            ImGui::Separator();
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
#if defined(__clang_version__)
            ImGui::Text("Compiler Version : %s", __clang_version__);
#endif
#if defined(__GNUC__)
            ImGui::Text("Compiler Version : %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#if defined(_MSC_FULL_VER)
            ImGui::Text("Compiler Version : %d.%d.%d", _MSC_FULL_VER / 10000000 % 100, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
#endif
            ImGui::End();
        }
    }

    if (showMatrix)
    {
        if (ImGui::Begin("Matrix", &showMatrix, ImGuiWindowFlags_AlwaysAutoResize))
        {
            UserInterface();

            if (ImGui::Button("Zero"))
                SetZero();
            ImGui::SameLine();
            if (ImGui::Button("Identity"))
                SetIdentity();
            ImGui::SameLine();
            if (ImGui::Button("Sequence X"))
                SequenceX();
            ImGui::SameLine();
            if (ImGui::Button("Sequence Y"))
                SequenceY();
            ImGui::SameLine();
            if (ImGui::Button("Random X"))
                RandomX();
            ImGui::SameLine();
            if (ImGui::Button("Random Y"))
                RandomY();
            ImGui::SameLine();
            if (ImGui::Button("Random Z"))
                RandomZ();

            if (ImGui::Button("V * V"))
                VectorMultiplyVector();
            ImGui::SameLine();
            if (ImGui::Button("Vx * M"))
                VectorMultiplyMatrix(true);
            ImGui::SameLine();
            if (ImGui::Button("Vy * M"))
                VectorMultiplyMatrix(false);
            ImGui::SameLine();

            ImGui::End();
        }
    }

    return updated;
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
