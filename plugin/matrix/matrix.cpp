//==============================================================================
// xxImGui : Plugin Matrix Source
//
// Copyright (c) 2019-2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <interface.h>

#if defined(xxANDROID) || defined(xxMACOS) || defined(xxIOS)
#   include <sys/time.h>
#endif

#define PLUGIN_NAME     "Matrix"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

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
            union vector
            {
                int16_t i16[32];
                float   f32[16];
                double  f64[8];
            };
            static vector matrixX;
            static vector matrixY;
            static vector matrixZ[8];
            static int mode;

            ImGui::RadioButton("64bit Floating", &mode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("32bit Floating", &mode, 1);
            ImGui::SameLine();
            ImGui::RadioButton("16bit Integer", &mode, 2);

            char label[16];
            float width = 0.0f;
            switch (mode)
            {
            case 0:
                width = 64.0f;
                ImGui::InvisibleButton("", ImVec2(width, 0.0f));
                ImGui::SameLine();
                for (int x = 0; x < 8; ++x)
                {
                    snprintf(label, 16, "##%d", x + 10000);
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputDouble(label, &matrixX.f64[x]);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("X:%d", x);
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                for (int y = 0; y < 8; ++y)
                {
                    snprintf(label, 16, "##%d", y + 20000);
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputDouble(label, &matrixY.f64[y]);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Y:%d", y);
                    ImGui::SameLine();
                    for (int x = 0; x < 8; ++x)
                    {
                        snprintf(label, 16, "##%02d%02d", x, y);
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputDouble(label, &matrixZ[y].f64[x]);
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
                    snprintf(label, 16, "##%d", x + 10000);
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputFloat(label, &matrixX.f32[x * 2 + 0]);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("X:%d,%d", x, 0);
                    ImGui::SameLine();
                    snprintf(label, 16, "##%d", x + 20000);
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputFloat(label, &matrixX.f32[x * 2 + 1]);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("X:%d,%d", x, 1);
                    ImGui::SameLine();
                }
                ImGui::NewLine();
                for (int y = 0; y < 8; ++y)
                {
                    snprintf(label, 16, "##%d", y + 30000);
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputFloat(label, &matrixY.f32[y * 2 + 0]);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Y:%d,%d", y, 0);
                    ImGui::SameLine();
                    snprintf(label, 16, "##%d", y + 40000);
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputFloat(label, &matrixY.f32[y * 2 + 1]);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Y:%d,%d", y, 1);
                    ImGui::SameLine();
                    for (int x = 0; x < 8; ++x)
                    {
                        snprintf(label, 16, "##%02d%02d", x * 2 + 0, y);
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputFloat(label, &matrixZ[y].f32[x * 2 + 0]);
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Z:%d,%d,%d", x, y, 0);
                        ImGui::SameLine();
                        snprintf(label, 16, "##%02d%02d", x * 2 + 1, y);
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputFloat(label, &matrixZ[y].f32[x * 2 + 0]);
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Z:%d,%d,%d", x, y, 1);
                        ImGui::SameLine();
                    }
                    ImGui::NewLine();
                }
                break;

            default:
                break;
            }

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
