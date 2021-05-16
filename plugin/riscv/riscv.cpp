//==============================================================================
// xxImGui : Plugin RISC-V Source
//
// Copyright (c) 2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <riscv/riscv_cpu.h>
#include <riscv/libelf/elf.h>
#include <string>
#include <vector>
#include <interface.h>

#define PLUGIN_NAME     "RISC-V"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static riscv_cpu* cpu = nullptr;
static std::vector<char> memory;
static std::vector<std::pair<std::string, std::vector<char>>> riscv_tests;
//------------------------------------------------------------------------------
pluginAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{
    memory = std::vector<char>();
    riscv_tests = std::vector<std::pair<std::string, std::vector<char>>>();
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
        if (riscv_tests.empty())
        {
            const char* app = xxGetExecutablePath();

            char temp[4096];
            snprintf(temp, 4096, "%s/../../../../../riscv/riscv-tests", app);

            uint64_t handle = 0;
            while (char* filename = xxOpenDirectory(&handle, temp, "rv64", nullptr))
            {
                snprintf(temp, 4096, "%s/../../../../../riscv/riscv-tests/%s", app, filename);
                FILE* file = fopen(temp, "rb");
                if (file)
                {
                    fseek(file, 0, SEEK_END);
                    size_t size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    riscv_tests.resize(riscv_tests.size() + 1);
                    riscv_tests.back().first = filename;
                    riscv_tests.back().second.resize(size);
                    fread(riscv_tests.back().second.data(), size, 1, file);
                }
                free(filename);
            }
            xxCloseDirectory(&handle);

            if (riscv_tests.empty())
                riscv_tests.resize(riscv_tests.size() + 1);
            std::sort(riscv_tests.begin(), riscv_tests.end());
        }

        if (ImGui::Begin("CPU", &showCPU, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::BeginTable("Registers", 7 + (int)riscv_tests.size() / 16);

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
                static int testType = 0;
                ImGui::RadioButton("Integer", &testType, 0);
                ImGui::RadioButton("Multiply", &testType, 1);
                ImGui::RadioButton("Atomic", &testType, 2);
                ImGui::RadioButton("Single Floating", &testType, 3);
                ImGui::RadioButton("Double Floating", &testType, 4);
                static const char* testName = nullptr;
                static std::string message;
                static bool autoTest = false;
                if (ImGui::Button("Auto Test"))
                {
                    message.clear();
                    autoTest = true;
                }

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
                    ImGui::InputScalar(nameType ? registerName[i] : abiName[i], ImGuiDataType_U64, &cpu->x[i], nullptr, nullptr, "%016llX");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Signed : %lld\n"
                                          "Unsigned : %llu", cpu->x[i].u,
                                                             cpu->x[i].s);
                    }
                }
                ImGui::SetNextItemWidth(width);
                ImGui::InputScalar("pc", ImGuiDataType_U64, &cpu->pc, nullptr, nullptr, "%016llX");
                ImGui::TableNextColumn();
                for (int i = 16; i < 32; ++i)
                {
                    ImGui::SetNextItemWidth(width);
                    ImGui::InputScalar(nameType ? registerName[i] : abiName[i], ImGuiDataType_U64, &cpu->x[i], nullptr, nullptr, "%016llX");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Signed : %lld\n"
                                          "Unsigned : %llu", cpu->x[i].u,
                                                             cpu->x[i].s);
                    }
                }
                uintptr_t offset = cpu->pc - cpu->begin;
                ImGui::SetNextItemWidth(width);
                if (ImGui::InputScalar("offset", ImGuiDataType_U64, &offset, nullptr, nullptr, "%016llX", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    cpu->pc = cpu->begin + offset;
                }

                if (testType == 3)
                {
                    static const char* const registerName[32] =
                    {
                        "f0",   "f1",   "f2",   "f3",   "f4",   "f5",   "f6",   "f7",
                        "f8",   "f9",   "f10",  "f11",  "f12",  "f13",  "f14",  "f15",
                        "f16",  "f17",  "f18",  "f19",  "f20",  "f21",  "f22",  "f23",
                        "f24",  "f25",  "f26",  "f27",  "f28",  "f29",  "f30",  "f31",
                    };
                    ImGui::TableNextColumn();
                    for (int i = 0; i < 16; ++i)
                    {
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputScalar(registerName[i], ImGuiDataType_U64, &cpu->f[i], nullptr, nullptr, "%016llX");
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("%f", (float)cpu->f[i].f);
                        }
                    }
                    ImGui::SetNextItemWidth(width); ImGui::InputScalar("fscr", ImGuiDataType_U32, &cpu->fcsr, nullptr, nullptr, "%08X");
                    ImGui::TableNextColumn();
                    for (int i = 16; i < 32; ++i)
                    {
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputScalar(registerName[i], ImGuiDataType_U64, &cpu->f[i], nullptr, nullptr, "%016llX");
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("%f", (float)cpu->f[i].f);
                        }
                    }
                }

                if (testType == 4)
                {
                    static const char* const registerName[32] =
                    {
                        "d0",   "d1",   "d2",   "d3",   "d4",   "d5",   "d6",   "d7",
                        "d8",   "d9",   "d10",  "d11",  "d12",  "d13",  "d14",  "d15",
                        "d16",  "d17",  "d18",  "d19",  "d20",  "d21",  "d22",  "d23",
                        "d24",  "d25",  "d26",  "d27",  "d28",  "d29",  "d30",  "d31",
                    };
                    ImGui::TableNextColumn();
                    for (int i = 0; i < 16; ++i)
                    {
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputScalar(registerName[i], ImGuiDataType_U64, &cpu->f[i], nullptr, nullptr, "%016llX");
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("%f", (double)cpu->f[i].d);
                        }
                    }
                    ImGui::SetNextItemWidth(width); ImGui::InputScalar("fscr", ImGuiDataType_U32, &cpu->fcsr, nullptr, nullptr, "%08X");
                    ImGui::TableNextColumn();
                    for (int i = 16; i < 32; ++i)
                    {
                        ImGui::SetNextItemWidth(width);
                        ImGui::InputScalar(registerName[i], ImGuiDataType_U64, &cpu->f[i], nullptr, nullptr, "%016llX");
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("%f", (double)cpu->f[i].d);
                        }
                    }
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

                ImGui::TableNextColumn();
                ImU32 unselectedColor = ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]);
                ImU32 selectedColor = ImColor(ImColor(unselectedColor).Value.z, ImColor(unselectedColor).Value.y, ImColor(unselectedColor).Value.x);
                auto test = [&](const char* name, const void* code, size_t size)
                {
                    bool selected = (testName == name);
                    ImGui::PushStyleColor(ImGuiCol_Button, selected ? selectedColor : unselectedColor);
                    if (ImGui::Button(name) || autoTest)
                    {
                        testName = name;
                        elf_t elf = { .elfFile = code, .elfSize = size, .elfClass = ELFCLASS64 };
                        if (elf_checkFile(&elf) == 0)
                        {
                            uintptr_t min = 0;
                            uintptr_t max = 0;
                            elf_getMemoryBounds(&elf, PHYSICAL, &min, &max);
                            xxLog("RISC-V", "Memory Minimum : %p", min);
                            xxLog("RISC-V", "Memory Maximum : %p", max);
                            size_t memorySize = max - min;
                            if (memorySize < 1048576)
                            {
                                memory.resize(memorySize);
                            }
                            for (size_t i = 0, count = elf_getNumSections(&elf); i < count; ++i)
                            {
                                uintptr_t address = elf_getSectionAddr(&elf, i);
                                size_t offset = elf_getSectionOffset(&elf, i);
                                size_t size = elf_getSectionSize(&elf, i);
                                size_t flags = elf_getSectionFlags(&elf, i);
                                const char* name = elf_getSectionName(&elf, i);
                                xxLog("RISC-V", "%zd : %010p %8d %8d %-20s%s%s", i, address,
                                                                                    offset,
                                                                                    size,
                                                                                    name,
                                                                                    flags & SHF_WRITE ? " WRITE" : "",
                                                                                    flags & SHF_EXECINSTR ? " EXECUTE" : "");
                                if (memory.empty() == false)
                                {
                                    if (address >= min && address + size <= max)
                                    {
                                        memcpy(memory.data() + address - min, (char*)code + offset, size);
                                    }
                                    if (flags & SHF_EXECINSTR)
                                    {
                                        cpu->program(memory.data() + address - min, size);
                                        message.clear();
                                    }
                                }
                            }
                        }
                        else
                        {
                            cpu->program(code, size);
                            message.clear();
                        }
                        cpu->environmentCall = [](riscv_cpu& cpu)
                        {
                            if (cpu.x[3] > 1)
                            {
                                message = "failed";
                                autoTest = false;
                            }
                            if (message.empty())
                            {
                                message = "succeed";
                            }
                            xxLog("RISC-V", "ECALL : %016p %llu", cpu.pc, cpu.x[3].u);
                        };
                        if (ImGui::GetIO().KeyCtrl || autoTest)
                        {
                            cpu->run();
                        }
                    }
                    ImGui::PopStyleColor();
                };
                int index = 0;
                for (auto& pair : riscv_tests)
                {
                    bool match = false;
                    if (match == false && testType == 0 && pair.first.find("rv64ui") == 0)
                        match = true;
                    if (match == false && testType == 1 && pair.first.find("rv64um") == 0)
                        match = true;
                    if (match == false && testType == 2 && pair.first.find("rv64ua") == 0)
                        match = true;
                    if (match == false && testType == 3 && pair.first.find("rv64uf") == 0)
                        match = true;
                    if (match == false && testType == 4 && pair.first.find("rv64ud") == 0)
                        match = true;
                    if (match == false)
                        continue;
                    test(pair.first.c_str(), pair.second.data(), pair.second.size());
                    if ((index++ % 16) == 15)
                    {
                        ImGui::TableNextColumn();
                    }
                }
                ImGui::TextUnformatted(message.c_str());
                autoTest = false;
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
