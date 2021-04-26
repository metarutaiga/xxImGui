//==============================================================================
// xxImGui : Plugin Matrix Source
//
// Copyright (c) 2019-2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <interface.h>

#if defined(_WIN32) || defined(__ANDROID__)
#define _Float16 short
#endif

#define PLUGIN_NAME     "Matrix"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

//------------------------------------------------------------------------------
namespace ImGui
{
bool InputChar(const char* label, signed char* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
{
    int temp = (*v);
    bool result = ImGui::InputInt(label, &temp, step, step_fast, flags);
    (*v) = temp;
    return result;
};
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
union vector
{
    double      f64[8];
    float       f32[16];
    _Float16    f16[32];
    int16_t     i16[32];
    uint16_t    u16[32];
    int8_t      i8[64];
    uint8_t     u8[64];
};
static vector matrixX;
static vector matrixY;
static vector matrixZ[64];
static int mode = 0;
#if defined(__APPLE__) && defined(__aarch64__)
#include <xxYUV/apple_amx.h>
static bool amx = false;
static uint64_t amxX[64];
static uint64_t amxY[64];
static uint64_t amxZ[64 * 64];
//------------------------------------------------------------------------------
static void amxDump()
{
    amx_operands_access access = {};
    access.memory_offset = (uint64_t)amxX;
    access.register_index = 0;
    for (int i = 0; i < 8; ++i)
    {
        amx_stx(access);
        access.memory_offset += 0x40;
        access.register_index++;
    }
    access.memory_offset = (uint64_t)amxY;
    access.register_index = 0;
    for (int i = 0; i < 8; ++i)
    {
        amx_sty(access);
        access.memory_offset += 0x40;
        access.register_index++;
    }
    access.memory_offset = (uint64_t)amxZ;
    access.register_index = 0;
    for (int i = 0; i < 8 * 8; ++i)
    {
        amx_stz(access);
        access.memory_offset += 0x40;
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
static void SetNumber(int value)
{
    SetZero();
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
        {
            matrixX.f64[i] = value;
            matrixY.f64[i] = value;
            matrixZ[i].f64[i] = value;
        }
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
        {
            matrixX.f32[i] = value;
            matrixY.f32[i] = value;
            matrixZ[i].f32[i] = value;
        }
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
        {
            matrixX.f16[i] = value;
            matrixY.f16[i] = value;
            matrixZ[i].f16[i] = value;
        }
        break;
    case 3:
        for (int i = 0; i < 32; ++i)
        {
            matrixX.i16[i] = value;
            matrixY.i16[i] = value;
            matrixZ[i].i16[i] = value;
        }
        break;
    case 4:
        for (int i = 0; i < 64; ++i)
        {
            matrixX.i8[i] = value;
            matrixY.i8[i] = value;
            matrixZ[i].i8[i] = value;
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
    case 4:
        for (int i = 0; i < 64; ++i)
            matrixX.i8[i] = i + 1;
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
    case 4:
        for (int i = 0; i < 64; ++i)
            matrixY.i8[i] = i + 1;
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
    case 4:
        for (int i = 0; i < 64; ++i)
            matrixX.i8[i] = (int8_t)rand();
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
    case 4:
        for (int i = 0; i < 64; ++i)
            matrixY.i8[i] = (int8_t)rand();
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
                matrixZ[j].i16[i] = (int8_t)rand();
        break;
    case 4:
        for (int j = 0; j < 64; ++j)
            for (int i = 0; i < 64; ++i)
                matrixZ[j].i8[i] = (int8_t)rand();
        break;
    }
}
//------------------------------------------------------------------------------
static void VectorMultiplyVectorToMatrix()
{
    switch (mode)
    {
    case 0:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands_access access = {};
            access.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_fma64( .skip_z = true );
            for (int i = 0; i < 8; ++i)
            {
                amx_stz(access);
                access.memory_offset += 0x40;
                access.register_index += 8;
            }
            amxDump();
            amx_clr();
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
            amx_operands_access access = {};
            access.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_fma64( .skip_z = true );
            for (int i = 0; i < 16; ++i)
            {
                amx_stz(access);
                access.memory_offset += 0x40;
                access.register_index += 4;
            }
            amxDump();
            amx_clr();
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
            amx_operands_access access = {};
            access.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_fma16( .skip_z = true );
            for (int i = 0; i < 32; ++i)
            {
                amx_stz(access);
                access.memory_offset += 0x40;
                access.register_index += 2;
            }
            amxDump();
            amx_clr();
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
            amx_operands_access access = {};
            access.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_mac16( .skip_z = true );
            for (int i = 0; i < 32; ++i)
            {
                amx_stz(access);
                access.memory_offset += 0x40;
                access.register_index += 2;
            }
            amxDump();
            amx_clr();
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
    case 4:
        for (int y = 0; y < 64; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                matrixZ[y].i8[x] = matrixX.i8[x] * matrixY.i8[y];
            }
        }
        break;
    }
}
//------------------------------------------------------------------------------
static void VectorMultiplyVectorToVector()
{
    switch (mode)
    {
    case 0:
        for (int i = 0; i < 8; ++i)
        {
            matrixZ[0].f64[i] = matrixX.f64[i] * matrixY.f64[i];
        }
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
        {
            matrixZ[0].f32[i] = matrixX.f32[i] * matrixY.f32[i];
        }
        break;
    case 2:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands_access access = {};
            access.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_vecfp();
            for (int i = 0; i < 32; ++i)
            {
                amx_stz(access);
                access.memory_offset += 0x40;
                access.register_index += 2;
            }
            amxDump();
            amx_clr();
            break;
        }
#endif
        for (int i = 0; i < 32; ++i)
        {
            matrixZ[0].f16[i] = matrixX.f16[i] * matrixY.f16[i];
        }
        break;
    case 3:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            amx_operands_access access = {};
            access.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_vecint();
            for (int i = 0; i < 32; ++i)
            {
                amx_stz(access);
                access.memory_offset += 0x40;
                access.register_index += 2;
            }
            amxDump();
            amx_clr();
            break;
        }
#endif
        for (int i = 0; i < 32; ++i)
        {
            matrixZ[0].i16[i] = matrixX.i16[i] * matrixY.i16[i];
        }
        break;
    case 4:
#if defined(__APPLE__) && defined(__aarch64__)
        if (amx)
        {
            static const uint16_t vector256[32] =
            {
                256, 256, 256, 256, 256, 256, 256, 256,
                256, 256, 256, 256, 256, 256, 256, 256,
                256, 256, 256, 256, 256, 256, 256, 256,
                256, 256, 256, 256, 256, 256, 256, 256,
            };
            amx_operands_access accessZ = {};
            accessZ.memory_offset = (uint64_t)&matrixZ;
            amx_set();
            amx_ldx( .memory_offset = (uint64_t)&matrixX );
            amx_ldy( .memory_offset = (uint64_t)&matrixY );
            amx_ldy( .memory_offset = (uint64_t)&vector256, .register_index = 1 );
            amx_extrx( .offset_y = 0x40, .offset_z = 1, );
            amx_vecint( .offset_x = 0x40, .offset_y = 0, .offset_z = 1 );
            amx_vecint( .offset_x = 0x40, .offset_y = 1, .offset_z = 2 );
            amx_vecint( .offset_x = 0, .offset_y = 0x40, .offset_z = 3 );
            amx_vecint( .offset_x = 1, .offset_y = 0x40, .offset_z = 4 );
            amx_extrx( .offset_y = 0x80, .offset_z = 1 );
            amx_extrx( .offset_y = 0xC0, .offset_z = 2 );
            amx_extrx( .offset_y = 0x100, .offset_z = 3 );
            amx_extrx( .offset_y = 0x140, .offset_z = 4 );
            amx_extry( .offset_x = 0x80, .offset_z = 4, );
            amx_extry( .offset_x = 0xC0, .offset_z = 5, );
            amx_vecint( .offset_x = 0x81, .offset_y = 0x81, .offset_z = 0 );
            amx_vecint( .offset_x = 0xC1, .offset_y = 0xC1, .offset_z = 5 );
            amx_extrx( .offset_y = 0x180, .offset_z = 5 );
            amx_vecint( .offset_x = 0x40, .offset_y = 0x180, .offset_z = 0 );
            for (int i = 0; i < 8; ++i)
            {
                amx_stz(accessZ);
                accessZ.memory_offset += 0x40;
                accessZ.register_index += 8;
            }
            amxDump();
            amx_clr();
            break;
        }
#endif
        for (int i = 0; i < 64; ++i)
        {
            matrixZ[0].i8[i] = matrixX.i8[i] * matrixY.i8[i];
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
    ImGui::SameLine();
    ImGui::RadioButton("8bit Integer", &mode, 4);
#if defined(__APPLE__) && defined(__aarch64__)
    ImGui::SameLine();
    ImGui::Checkbox("Apple AMX", &amx);
    if (amx)
    {
        ImGui::SetNextWindowSize(ImVec2(1280, 512), ImGuiCond_Appearing);
        if (ImGui::Begin("Apple AMX Registers"))
        {
            static int opcode = 0;
            static bool operands[64];
            static int sequenceX = 0;
            static int sequenceY = 0;
            static bool direction = false;
            bool click = false;

            click |= ImGui::RadioButton("MAC16", &opcode, 0);
            ImGui::SameLine();
            click |= ImGui::RadioButton("VECINT", &opcode, 1);
            ImGui::SameLine();
            click |= ImGui::RadioButton("MATINT", &opcode, 2);

            int number = 0;
            for (int i = 64 - 1; i >= 0; --i)
            {
                number = (number << 1) | operands[i];
                click |= ImGui::Checkbox(GetLabel(), &operands[i]);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%d", i);
                }
                if (i != 0 && i != 10 && i != 20 && i != 27 && i != 32 && i != 47 && i != 0 && (opcode == 0 || (i != 42 && i != 58)))
                {
                    ImGui::SameLine();
                }
                else
                {
                    ImGui::SameLine();
                    ImGui::Text("%d", number);
                    number = 0;
                }
            }

            for (int i = 0; i < 8; ++i)
            {
                ImGui::Text("%016llX", direction ? __builtin_bswap64(amxX[i]) : amxX[7 - i]);
                ImGui::SameLine();
            }
            ImGui::SetNextItemWidth(128.0f);
            click |= ImGui::InputInt("X", &sequenceX);

            for (int i = 0; i < 8; ++i)
            {
                ImGui::Text("%016llX", direction ? __builtin_bswap64(amxY[i]) : amxY[7 - i]);
                ImGui::SameLine();
            }
            ImGui::SetNextItemWidth(128.0f);
            click |= ImGui::InputInt("Y", &sequenceY);

            for (int i = 0; i < 8; ++i)
            {
                ImGui::Text("%016llX", direction ? __builtin_bswap64(amxZ[i]) : amxZ[7 - i]);
                ImGui::SameLine();
            }
            ImGui::Checkbox("Direction", &direction);

            for (int i = 0; i < 8; ++i)
            {
                ImGui::Text("%016llX", direction ? __builtin_bswap64(amxY[i + 64]) : amxY[7 - i + 64]);
                ImGui::SameLine();
            }
            ImGui::NewLine();

            if (click)
            {
                auto generate = [](int type, uint64_t* array)
                {
                    switch (type)
                    {
                    case 0:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0;
                        break;
                    case 1:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0xFFFFFFFFFFFFFFFFull;
                        break;
                    case 2:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 1;
                        break;
                    case 3:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = i;
                        break;
                    case 4:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0x0000000100000001ull;
                        break;
                    case 5:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0x0000000200000002ull * i + 0x0000000200000001ull;
                        break;
                    case 6:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0x0001000100010001ull;
                        break;
                    case 7:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0x0004000400040004ull * i + 0x0004000300020001ull;
                        break;
                    case 8:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0x0101010101010101ull;
                        break;
                    case 9:
                        for (uint64_t i = 0; i < 8; ++i)
                            array[i] = 0x0808080808080808ull * i + 0x0807060504030201ull;
                        break;
                    default:
                        break;
                    }
                };
                generate(sequenceX, amxX);
                generate(sequenceY, amxY);
                uint64_t operand = 0;
                for (int i = 0; i < 64; ++i)
                    operand |= (uint64_t)operands[i] << i;
                amx_set();
                amx_ldx( .memory_offset = (uint64_t)amxX, .register_index = 0 );
                amx_ldy( .memory_offset = (uint64_t)amxY, .register_index = 0 );
                switch (opcode)
                {
                case 0:
                    amx_mac16( .value = operand );
                    break;
                case 1:
                    amx_vecint( .value = operand );
                    break;
                case 2:
                    amx_matint( .value = operand );
                    break;
                default:
                    break;
                }
                amxDump();
                amx_clr();
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float width = 16.0f;

            for (int x = 0; x < 64; ++x)
            {
                int blockX = (direction ? x + 8 + 1 : 64 - x - 1);
                ImVec2 rect[2] =
                {
                    ImVec2(pos.x + (blockX + 0) * width, pos.y + 0 * width),
                    ImVec2(pos.x + (blockX + 1) * width, pos.y + 1 * width),
                };
                drawList->AddQuadFilled(ImVec2(rect[0].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[1].y),
                                        ImVec2(rect[0].x, rect[1].y),
                                        amxX[x] == 0 ? x / 8 & 1 ? 0xFF3F3F3F : 0x7F7F7F7F : 0xFFFFFFFF);
                if (ImGui::IsMouseHoveringRect(rect[0], rect[1]))
                    ImGui::SetTooltip("X:%d.%d (%016llX)", x / 8, x % 8, direction ? __builtin_bswap64(amxX[x]) : amxX[x]);
            }
            for (int y = 0; y < 64; ++y)
            {
                int blockX = (direction ? y % 8 + 0 : 64 - y % 8 + 8);
                ImVec2 rect[2] =
                {
                    ImVec2(pos.x + (blockX + 0) * width, pos.y + (y / 8 + 2) * width),
                    ImVec2(pos.x + (blockX + 1) * width, pos.y + (y / 8 + 3) * width),
                };
                drawList->AddQuadFilled(ImVec2(rect[0].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[0].y),
                                        ImVec2(rect[1].x, rect[1].y),
                                        ImVec2(rect[0].x, rect[1].y),
                                        amxY[y] == 0 ? y / 8 & 1 ? 0xFF3F3F3F : 0x7F7F7F7F : 0xFFFFFFFF);
                if (ImGui::IsMouseHoveringRect(rect[0], rect[1]))
                    ImGui::SetTooltip("Y:%d.%d (%016llX)", y / 8, y % 8, direction ? __builtin_bswap64(amxY[y]) : amxY[y]);
                if (y >= 8)
                    continue;
                for (int x = 0; x < 64; ++x)
                {
                    int blockX = (direction ? x + 8 + 1 : 64 - x - 1);
                    ImVec2 rect[2] =
                    {
                        ImVec2(pos.x + (blockX + 0) * width, pos.y + (y + 2) * width),
                        ImVec2(pos.x + (blockX + 1) * width, pos.y + (y + 3) * width),
                    };
                    drawList->AddQuadFilled(ImVec2(rect[0].x, rect[0].y),
                                            ImVec2(rect[1].x, rect[0].y),
                                            ImVec2(rect[1].x, rect[1].y),
                                            ImVec2(rect[0].x, rect[1].y),
                                            amxZ[y * 64 + x] == 0 ? ((x / 8) ^ (y / 8)) & 1 ? 0xFF3F3F3F : 0x7F7F7F7F : 0xFFFFFFFF);
                    if (ImGui::IsMouseHoveringRect(rect[0], rect[1]))
                        ImGui::SetTooltip("Z:%d.%d,%d.%d (%016llX)", x / 8, x % 8, y / 8, y % 8, direction ? __builtin_bswap64(amxZ[y * 64 + x]) : amxZ[y * 64 + x]);
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
                    ImGui::SetTooltip("X:%d (%f)\n"
                                      "Y:%d (%f)\n"
                                      "Z:%d,%d (%f)",
                                      x, matrixX.f64[x],
                                      y, matrixY.f64[y],
                                      x, y, matrixZ[y].f64[x]);
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
                    ImGui::SetTooltip("X:%d (%f)\n"
                                      "Y:%d (%f)\n"
                                      "Z:%d,%d (%f)",
                                      x, matrixX.f32[x],
                                      y, matrixY.f32[y],
                                      x, y, matrixZ[y].f32[x]);
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
                    ImGui::SetTooltip("X:%d (%f)\n"
                                      "Y:%d (%f)\n"
                                      "Z:%d,%d (%f)",
                                      x, (float)matrixX.f16[x],
                                      y, (float)matrixY.f16[y],
                                      x, y, (float)matrixZ[y].f16[x]);
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
                    ImGui::SetTooltip("X:%d (%d) (%d)\n"
                                      "Y:%d (%d) (%d)\n"
                                      "Z:%d,%d (%d) (%d)",
                                      x, matrixX.i16[x], matrixX.u16[x],
                                      y, matrixY.i16[y], matrixY.u16[y],
                                      x, y, matrixZ[y].i16[x], matrixZ[y].u16[x]);
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;
    case 4:
        width = 16.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 64; ++x)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x ? selectedColor : unselectedColor);
            ImGui::InputChar(GetLabel(), &matrixX.i8[x], 0);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverX = x;
                ImGui::SetTooltip("X:%d (%d)", x, matrixX.i8[x]);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 32; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverY == y ? selectedColor : unselectedColor);
            ImGui::InputChar(GetLabel(), &matrixY.i8[y], 0);
            ImGui::PopStyleColor(1);
            if (ImGui::IsItemHovered())
            {
                currentHoverY = y;
                ImGui::SetTooltip("Y:%d (%d)", y, matrixY.i8[y]);
            }
            ImGui::SameLine();
            for (int x = 0; x < 64; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, hoverX == x && hoverY == y ? selectedColor : unselectedColor);
                ImGui::InputChar(GetLabel(), &matrixZ[y].i8[x], 0);
                ImGui::PopStyleColor(1);
                if (ImGui::IsItemHovered())
                {
                    currentHoverX = x;
                    currentHoverY = y;
                    ImGui::SetTooltip("X:%d (%d) (%d)\n"
                                      "Y:%d (%d) (%d)\n"
                                      "Z:%d,%d (%d) (%d)",
                                      x, matrixX.i8[x], matrixX.u8[x],
                                      y, matrixY.i8[y], matrixY.u8[y],
                                      x, y, matrixZ[y].i8[x], matrixZ[y].u8[x]);
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
            if (ImGui::Button("One"))
                SetNumber(1);
            ImGui::SameLine();
            if (ImGui::Button("-1"))
                SetNumber(-1);
            ImGui::SameLine();
            if (ImGui::Button("256"))
                SetNumber(256);
            ImGui::SameLine();
            if (ImGui::Button("-256"))
                SetNumber(-256);
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

            if (ImGui::Button("V * V = M"))
                VectorMultiplyVectorToMatrix();
            ImGui::SameLine();
            if (ImGui::Button("V * V = V"))
                VectorMultiplyVectorToVector();

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
