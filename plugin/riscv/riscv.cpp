//==============================================================================
// xxImGui : Plugin RISC-V Source
//
// Copyright (c) 2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <riscv/riscv_cpu.h>
#include <interface.h>

#define PLUGIN_NAME     "RISC-V"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static riscv_cpu* cpu = nullptr;
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
    static bool showCPU = false;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("CPU", nullptr, &showCPU);
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

    if (showCPU)
    {
        if (ImGui::Begin("CPU", &showCPU, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::BeginTable("Registers", 3);

            ImGui::TableNextColumn();
            if (cpu == nullptr && ImGui::Button("Start"))
            {
                cpu = new riscv_cpu;
            }
            if (cpu != nullptr && ImGui::Button("Stop"))
            {
                delete cpu;
                cpu = nullptr;
            }
            if (cpu)
            {
                float width = 128.0f;
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("pc", ImGuiDataType_U64, &cpu->pc, nullptr, nullptr, "%016X");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x0", ImGuiDataType_U64, &cpu->x[0], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x1", ImGuiDataType_U64, &cpu->x[1], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x2", ImGuiDataType_U64, &cpu->x[2], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x3", ImGuiDataType_U64, &cpu->x[3], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x4", ImGuiDataType_U64, &cpu->x[4], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x5", ImGuiDataType_U64, &cpu->x[5], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x6", ImGuiDataType_U64, &cpu->x[6], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x7", ImGuiDataType_U64, &cpu->x[7], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x8", ImGuiDataType_U64, &cpu->x[8], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x9", ImGuiDataType_U64, &cpu->x[9], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x10", ImGuiDataType_U64, &cpu->x[10], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x11", ImGuiDataType_U64, &cpu->x[11], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x12", ImGuiDataType_U64, &cpu->x[12], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x13", ImGuiDataType_U64, &cpu->x[13], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x14", ImGuiDataType_U64, &cpu->x[14], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x15", ImGuiDataType_U64, &cpu->x[15], nullptr, nullptr, "%016X");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x16", ImGuiDataType_U64, &cpu->x[16], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x17", ImGuiDataType_U64, &cpu->x[17], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x18", ImGuiDataType_U64, &cpu->x[18], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x19", ImGuiDataType_U64, &cpu->x[19], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x20", ImGuiDataType_U64, &cpu->x[20], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x21", ImGuiDataType_U64, &cpu->x[21], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x22", ImGuiDataType_U64, &cpu->x[22], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x23", ImGuiDataType_U64, &cpu->x[23], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x24", ImGuiDataType_U64, &cpu->x[24], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x25", ImGuiDataType_U64, &cpu->x[25], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x26", ImGuiDataType_U64, &cpu->x[26], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x27", ImGuiDataType_U64, &cpu->x[27], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x28", ImGuiDataType_U64, &cpu->x[28], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x29", ImGuiDataType_U64, &cpu->x[29], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x30", ImGuiDataType_U64, &cpu->x[30], nullptr, nullptr, "%016X");
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("x31", ImGuiDataType_U64, &cpu->x[31], nullptr, nullptr, "%016X");
            }

            ImGui::EndTable();
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
