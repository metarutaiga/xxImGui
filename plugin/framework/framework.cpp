//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2019-2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include "../../ConcurrencyNetworkFramework/Listen.h"
#include "../../ConcurrencyNetworkFramework/Log.h"
#include "../../ConcurrencyNetworkFramework/Framework.h"
#include <mutex>
#include <thread>
#include <interface.h>

#define PLUGIN_NAME     "Framework"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static Framework* framework = nullptr;
static Listen* server = nullptr;
static std::thread* frameworkThread = nullptr;

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
pluginAPI void Update(const UpdateData& updateData)
{
    static bool showAbout = false;
    static bool showFramework = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Framework", nullptr, &showFramework);
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
            ImGui::End();
        }
    }

    if (showFramework)
    {
        if (ImGui::Begin("Framework", &showFramework))
        {
            static std::string log;
            static std::mutex logMutex;

            if (ImGui::Button("Start Framework"))
            {
                if (server == nullptr || framework == nullptr)
                {
                    log.clear();
                    Log::SetOutput([](const char* output)
                    {
                        logMutex.lock();
                        log += output;
                        logMutex.unlock();
                    });

                    server = new Listen("127.0.0.1", "7777");
                    framework = new Framework;
                    framework->Server(server);
                    frameworkThread = new std::thread([]{ framework->Dispatch(); });
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Stop Framework"))
            {
                if (frameworkThread)
                {
                    framework->Terminate();
                    frameworkThread->join();

                    delete frameworkThread;
                    delete framework;
                    delete server;

                    frameworkThread = nullptr;
                    framework = nullptr;
                    server = nullptr;
                }
            }

            ImVec2 windowSize = ImVec2(320 * updateData.windowScale, 240 * updateData.windowScale);

            logMutex.lock();
            ImGui::InputTextMultiline("LOG", &log.front(), log.size(), windowSize, ImGuiInputTextFlags_ReadOnly);
            logMutex.unlock();

            ImGui::End();
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
