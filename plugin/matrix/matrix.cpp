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
        uint64_t dummy_48:14;
        uint64_t mode_32:1;
        uint64_t vector_matrix_add:1;
    };
    uint64_t value;
};
//------------------------------------------------------------------------------
#define AMX_LDX(V)                                                             \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (0 << 5) | 0)" ::"r"((uint64_t)V)     \
      : "x0", "memory")
#define AMX_LDY(V)                                                             \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (1 << 5) | 0)" ::"r"((uint64_t)V)     \
      : "x0", "memory")
#define AMX_STX(V)                                                             \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (2 << 5) | 0)" ::"r"((uint64_t)V)     \
      : "x0", "memory")
#define AMX_STY(V)                                                             \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (3 << 5) | 0)" ::"r"((uint64_t)V)     \
      : "x0", "memory")
#define AMX_LDZ(V)                                                             \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (4 << 5) | 0)" ::"r"((uint64_t)V)     \
      : "x0", "memory")
#define AMX_STZ(V)                                                             \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (5 << 5) | 0)" ::"r"((uint64_t)V)     \
      : "x0", "memory")
#define AMX_FMA32(V)                                                           \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (12 << 5) | 0)" ::"r"((uint64_t)V)    \
      : "x0", "memory")
#define AMX_MAC16(V)                                                           \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (14 << 5) | 0)" ::"r"((uint64_t)V)    \
      : "x0", "memory")
#define AMX_START()                                                            \
  __asm__ volatile(                                                            \
      "nop \r\n nop \r\n nop \r\n .word (0x201000 | (17 << 5) | 0)" ::         \
          : "memory")
#define AMX_STOP()                                                             \
  __asm__ volatile(                                                            \
      "nop \r\n nop \r\n nop \r\n .word (0x201000 | (17 << 5) | 1)" ::         \
          : "memory")
#define AMX_VECINT(V)                                                          \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (18 << 5) | 0)" ::"r"((uint64_t)V)    \
      : "x0", "memory")
#define AMX_VECFP(V)                                                           \
  __asm__ volatile(                                                            \
      "mov x0, %0 \r\n .word (0x201000 | (19 << 5) | 0)" ::"r"((uint64_t)V)    \
      : "x0", "memory")
//------------------------------------------------------------------------------
union vector
{
    int16_t i16[32];
    float   f32[16];
    double  f64[8];
};
static vector matrixX;
static vector matrixY;
static vector matrixZ[32];
static int mode;
static bool amx;
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
        for (int i = 0; i < 8; ++i)
        {
            matrixX.f32[i * 2 + 0] = 1.0f;
            matrixX.f32[i * 2 + 1] = 1.0f;
            matrixY.f32[i * 2 + 0] = 1.0f;
            matrixY.f32[i * 2 + 1] = 1.0f;
            matrixZ[i].f32[i * 2 + 0] = 1.0f;
            matrixZ[i].f32[i * 2 + 1] = 1.0f;
        }
        break;
    case 2:
        for (int i = 0; i < 8; ++i)
        {
            matrixX.i16[i * 4 + 0] = 1.0f;
            matrixX.i16[i * 4 + 1] = 1.0f;
            matrixX.i16[i * 4 + 2] = 1.0f;
            matrixX.i16[i * 4 + 3] = 1.0f;
            matrixY.i16[i * 4 + 0] = 1.0f;
            matrixY.i16[i * 4 + 1] = 1.0f;
            matrixY.i16[i * 4 + 2] = 1.0f;
            matrixY.i16[i * 4 + 3] = 1.0f;
            matrixZ[i].i16[i * 4 + 0] = 1.0f;
            matrixZ[i].i16[i * 4 + 1] = 1.0f;
            matrixZ[i].i16[i * 4 + 2] = 1.0f;
            matrixZ[i].i16[i * 4 + 3] = 1.0f;
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
            matrixX.f64[i] = i;
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
            matrixX.f32[i] = i;
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
            matrixX.i16[i] = i;
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
            matrixY.f64[i] = i;
        break;
    case 1:
        for (int i = 0; i < 16; ++i)
            matrixY.f32[i] = i;
        break;
    case 2:
        for (int i = 0; i < 32; ++i)
            matrixY.i16[i] = i;
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
            matrixX.i16[i] = rand() % (256);
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
            matrixY.i16[i] = rand() % (256);
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
        for (int j = 0; j < 8; ++j)
            for (int i = 0; i < 16; ++i)
                matrixZ[j].f32[i] = rand() / (float)RAND_MAX;
        break;
    case 2:
        for (int j = 0; j < 8; ++j)
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
            operands.mode_32 = true;
            amx_access access = {};
            access.address = (uint64_t)&matrixZ;
            AMX_START();
            AMX_LDX(&matrixX);
            AMX_LDY(&matrixY);
            AMX_FMA32(operands.value);
            for (int i = 0; i < 8; ++i)
            {
                AMX_STZ(access.value);
                access.address += 0x40;
                access.register_index += 8;
            }
            AMX_STOP();
            break;
        }
#endif
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                matrixZ[y].f32[x * 2 + 0] = matrixX.f32[x * 2 + 0] * matrixY.f32[y * 2 + 0];
                matrixZ[y].f32[x * 2 + 1] = matrixX.f32[x * 2 + 1] * matrixY.f32[y * 2 + 1];
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
            AMX_MAC16(operands.value);
            for (int i = 0; i < 8; ++i)
            {
                AMX_STZ(access.value);
                access.address += 0x40;
                access.register_index += 8;
            }
            AMX_STOP();
            break;
        }
#endif
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                matrixZ[y].i16[x * 4 + 0] = matrixX.i16[x * 4 + 0] * matrixY.i16[y * 4 + 0];
                matrixZ[y].i16[x * 4 + 1] = matrixX.i16[x * 4 + 1] * matrixY.i16[y * 4 + 1];
                matrixZ[y].i16[x * 4 + 2] = matrixX.i16[x * 4 + 2] * matrixY.i16[y * 4 + 2];
                matrixZ[y].i16[x * 4 + 3] = matrixX.i16[x * 4 + 3] * matrixY.i16[y * 4 + 3];
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
        for (int j = 0; j < 8; ++j)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (X)
                {
                    matrixZ[j].f64[i] = matrixZ[j].f64[i] * matrixX.f64[i];
                }
                else
                {
                    matrixZ[i].f64[j] = matrixZ[i].f64[j] * matrixY.f64[i];
                }
            }
        }
        break;
    case 1:
        for (int j = 0; j < 8; ++j)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (X)
                {
                    matrixZ[j].f32[i * 2 + 0] = matrixZ[j].f32[i * 2 + 0] * matrixX.f32[i * 2 + 0];
                    matrixZ[j].f32[i * 2 + 1] = matrixZ[j].f32[i * 2 + 1] * matrixX.f32[i * 2 + 1];
                }
                else
                {
                    matrixZ[i].f32[j * 2 + 0] = matrixZ[i].f32[j * 2 + 0] * matrixY.f32[i * 2 + 0];
                    matrixZ[i].f32[j * 2 + 1] = matrixZ[i].f32[j * 2 + 1] * matrixY.f32[i * 2 + 1];
                }
            }
        }
        break;
    case 2:
        for (int j = 0; j < 8; ++j)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (X)
                {
                    matrixZ[j].i16[i * 4 + 0] = matrixZ[j].i16[i * 4 + 0] * matrixX.i16[i * 4 + 0];
                    matrixZ[j].i16[i * 4 + 1] = matrixZ[j].i16[i * 4 + 1] * matrixX.i16[i * 4 + 1];
                    matrixZ[j].i16[i * 4 + 2] = matrixZ[j].i16[i * 4 + 2] * matrixX.i16[i * 4 + 2];
                    matrixZ[j].i16[i * 4 + 3] = matrixZ[j].i16[i * 4 + 3] * matrixX.i16[i * 4 + 3];
                }
                else
                {
                    matrixZ[i].i16[j * 4 + 0] = matrixZ[i].i16[j * 4 + 0] * matrixY.i16[i * 4 + 0];
                    matrixZ[i].i16[j * 4 + 1] = matrixZ[i].i16[j * 4 + 1] * matrixY.i16[i * 4 + 1];
                    matrixZ[i].i16[j * 4 + 2] = matrixZ[i].i16[j * 4 + 2] * matrixY.i16[i * 4 + 2];
                    matrixZ[i].i16[j * 4 + 3] = matrixZ[i].i16[j * 4 + 3] * matrixY.i16[i * 4 + 3];
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

    auto InputShort = [](const char* label, short* v)
    {
        int temp = (*v);
        bool result = ImGui::InputInt(label, &temp, 0, 0, ImGuiInputTextFlags_None);
        (*v) = temp;
        return result;
    };

    ImGui::RadioButton("64bit Floating", &mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("32bit Floating", &mode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("16bit Integer", &mode, 2);
#if defined(__APPLE__) && defined(__aarch64__)
    ImGui::SameLine();
    ImGui::Checkbox("Apple AMX", &amx);
#endif

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
            ImGui::InputDouble(GetLabel(), &matrixX.f64[x]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d", x);
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 8; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::InputDouble(GetLabel(), &matrixY.f64[y]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d", y);
            ImGui::SameLine();
            for (int x = 0; x < 8; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::InputDouble(GetLabel(), &matrixZ[y].f64[x]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d", x, y);
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;
    case 1:
        width = 48.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 8; ++x)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::InputFloat(GetLabel(), &matrixX.f32[x * 2 + 0]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d,%d", x, 0);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            ImGui::InputFloat(GetLabel(), &matrixX.f32[x * 2 + 1]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d,%d", x, 1);
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 8; ++y)
        {
            ImGui::SetNextItemWidth(width);
            ImGui::InputFloat(GetLabel(), &matrixY.f32[y * 2 + 0]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d,%d", y, 0);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            ImGui::InputFloat(GetLabel(), &matrixY.f32[y * 2 + 1]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d,%d", y, 1);
            ImGui::SameLine();
            for (int x = 0; x < 8; ++x)
            {
                ImGui::SetNextItemWidth(width);
                ImGui::InputFloat(GetLabel(), &matrixZ[y].f32[x * 2 + 0]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d,%d", x, y, 0);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(width);
                ImGui::InputFloat(GetLabel(), &matrixZ[y].f32[x * 2 + 1]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d,%d", x, y, 1);
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        break;
    case 2:
        width = 48.0f;
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 8; ++x)
        {
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixX.i16[x * 4 + 0]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d,%d", x, 0);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixX.i16[x * 4 + 1]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d,%d", x, 1);
            ImGui::SameLine();
        }
        ImGui::NewLine();
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        ImGui::InvisibleButton("", ImVec2(width, 0.0f));
        ImGui::SameLine();
        for (int x = 0; x < 8; ++x)
        {
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixX.i16[x * 4 + 2]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d,%d", x, 2);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixX.i16[x * 4 + 3]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("X:%d,%d", x, 3);
            ImGui::SameLine();
        }
        ImGui::NewLine();
        for (int y = 0; y < 8; ++y)
        {
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixY.i16[y * 4 + 0]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d,%d", y, 0);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixY.i16[y * 4 + 1]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d,%d", y, 1);
            ImGui::SameLine();
            for (int x = 0; x < 8; ++x)
            {
                ImGui::SetNextItemWidth(width);
                InputShort(GetLabel(), &matrixZ[y].i16[x * 4 + 0]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d,%d", x, y, 0);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(width);
                InputShort(GetLabel(), &matrixZ[y].i16[x * 4 + 1]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d,%d", x, y, 1);
                ImGui::SameLine();
            }
            ImGui::NewLine();
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixY.i16[y * 4 + 2]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d,%d", y, 2);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            InputShort(GetLabel(), &matrixY.i16[y * 4 + 3]);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Y:%d,%d", y, 3);
            ImGui::SameLine();
            for (int x = 0; x < 8; ++x)
            {
                ImGui::SetNextItemWidth(width);
                InputShort(GetLabel(), &matrixZ[y].i16[x * 4 + 2]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d,%d", x, y, 2);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(width);
                InputShort(GetLabel(), &matrixZ[y].i16[x * 4 + 3]);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Z:%d,%d,%d", x, y, 3);
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
