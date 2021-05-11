//==============================================================================
// xxImGui : Plugin RISC-V Source
//
// Copyright (c) 2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <riscv/riscv_cpu.h>
#include <riscv/libelf/elf.h>
#include <interface.h>

#include "riscv-tests/rv64ui-p-add.h"
#include "riscv-tests/rv64ui-p-addiw.h"
#include "riscv-tests/rv64ui-p-addw.h"
#include "riscv-tests/rv64ui-p-and.h"
#include "riscv-tests/rv64ui-p-andi.h"
#include "riscv-tests/rv64ui-p-auipc.h"
#include "riscv-tests/rv64ui-p-beq.h"
#include "riscv-tests/rv64ui-p-bge.h"
#include "riscv-tests/rv64ui-p-bgeu.h"
#include "riscv-tests/rv64ui-p-blt.h"
#include "riscv-tests/rv64ui-p-bltu.h"
#include "riscv-tests/rv64ui-p-bne.h"
#include "riscv-tests/rv64ui-p-fence_i.h"
#include "riscv-tests/rv64ui-p-jal.h"
#include "riscv-tests/rv64ui-p-jalr.h"
#include "riscv-tests/rv64ui-p-xor.h"

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
            ImGui::BeginTable("Registers", 5);

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
                if (ImGui::Button("Run"))
                {
                    cpu->run();
                }
                if (ImGui::Button("Run Once"))
                {
                    cpu->runOnce();
                }
                static int nameType = 0;
                ImGui::RadioButton("ABI Name", &nameType, 0);
                ImGui::RadioButton("Register Name", &nameType, 1);

                ImGui::TableNextColumn();
                ImU32 unselectedColor = ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]);
                ImU32 selectedColor = ImColor(ImColor(unselectedColor).Value.z, ImColor(unselectedColor).Value.y, ImColor(unselectedColor).Value.x);
                auto test = [&](const char* name, auto&& code)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, cpu->begin == (uintptr_t)code ? selectedColor : unselectedColor);
                    if (ImGui::Button(name))
                    {
                        elf_t elf = { .elfFile = code, .elfSize = sizeof(code), .elfClass = ELFCLASS64 };
                        if (elf_checkFile(&elf) == 0)
                        {
                            for (size_t i = 0, count = elf_getNumSections(&elf); i < count; ++i)
                            {
                                uintptr_t address = elf_getSectionAddr(&elf, i);
                                size_t offset = elf_getSectionOffset(&elf, i);
                                size_t size = elf_getSectionSize(&elf, i);
                                const char* name = elf_getSectionName(&elf, i);
                                xxLog("RISC-V", "%zd : %010p %8d %8d %s", i, address,
                                                                             offset,
                                                                             size,
                                                                             name);
                                if (strncmp(name, ".text", 5) == 0)
                                {
                                    cpu->program((char*)code + offset, size);
                                }
                            }
                        }
                        else
                        {
                            cpu->program(code, sizeof(code));
                        }
                    }
                    ImGui::PopStyleColor();
                };
                test("rv64ui-p-add",    rv64ui_p_add);
                test("rv64ui-p-addiw",  rv64ui_p_addiw);
                test("rv64ui-p-addw",   rv64ui_p_addw);
                test("rv64ui-p-and",    rv64ui_p_and);
                test("rv64ui-p-andi",   rv64ui_p_andi);
                test("rv64ui-p-auipc",  rv64ui_p_auipc);
                test("rv64ui-p-beq",    rv64ui_p_beq);
                test("rv64ui-p-bge",    rv64ui_p_bge);
                test("rv64ui-p-bgeu",   rv64ui_p_bgeu);
                test("rv64ui-p-blt",    rv64ui_p_blt);
                test("rv64ui-p-bltu",   rv64ui_p_bltu);
                test("rv64ui-p-bne",    rv64ui_p_bne);
                test("rv64ui-p-fence_i",rv64ui_p_fence_i);
                test("rv64ui-p-jal",    rv64ui_p_jal);
                test("rv64ui-p-jalr",   rv64ui_p_jalr);
                test("rv64ui-p-xor",    rv64ui_p_xor);

                ImGui::TableNextColumn();
                static const char* const registerName[32] =
                {
                    "x0",   "x1",   "x2",   "x3",   "x4",   "x5",   "x6",   "x7",
                    "x8",   "x9",   "x10",  "x11",  "x12",  "x13",  "x14",  "x15",
                    "x16",  "x17",  "x18",  "x19",  "x20",  "x21",  "x22",  "x23",
                    "x24",  "x25",  "x26",  "x27",  "x28",  "x29",  "x30",  "x31",
                };
                static const char* const abiName[32] =
                {
                    "zero", "ra",   "sp",   "gp",   "tp",   "t0",   "t1",   "t2",
                    "s0/fp","s1",   "a0",   "a1",   "a2",   "a3",   "a4",   "a5",
                    "a6",   "a7",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",
                    "s8",   "s9",   "s10",  "s11",  "t3",   "t4",   "t5",   "t6",
                };
                float width = 128.0f;
                for (int i = 0; i < 16; ++i)
                {
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputScalar(nameType ? registerName[i] : abiName[i], ImGuiDataType_U64, &cpu->x[i], nullptr, nullptr, "%016X");
                }
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("pc", ImGuiDataType_U64, &cpu->pc, nullptr, nullptr, "%016X");
                ImGui::TableNextColumn();
                for (int i = 16; i < 32; ++i)
                {
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputScalar(nameType ? registerName[i] : abiName[i], ImGuiDataType_U64, &cpu->x[i], nullptr, nullptr, "%016X");
                }

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(width); ImGui::InputScalar("format", ImGuiDataType_U32, &cpu->format, nullptr, nullptr, "%08X");
                ImGui::Text("opcode : %X", cpu->opcode);
                ImGui::Text("rd : %X", cpu->rd);
                ImGui::Text("funct3 : %X", cpu->funct3);
                ImGui::Text("rs1 : %X", cpu->rs1);
                ImGui::Text("rs2 : %X", cpu->rs2);
                ImGui::Text("funct7 : %X", cpu->funct7);
                ImGui::Text("immediateI : %X", cpu->immI());
                ImGui::Text("immediateS : %X", cpu->immS());
                ImGui::Text("immediateB : %X", cpu->immB());
                ImGui::Text("immediateU : %X", cpu->immU());
                ImGui::Text("immediateJ : %X", cpu->immJ());
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
