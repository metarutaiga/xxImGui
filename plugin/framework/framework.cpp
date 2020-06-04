//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2019-2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include "../../ConcurrencyNetworkFramework/Listen.h"
#include "../../ConcurrencyNetworkFramework/Framework.h"
#include <interface.h>

#define PLUGIN_NAME     "Framework"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static Framework* framework = nullptr;
static Listen* server = nullptr;
static pthread_t frameworkThread = nullptr;

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
            ImGui::MenuItem("Start Framework", nullptr, &showFramework);
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
        if (server == nullptr)
        {
            server = new Listen("127.0.0.1", "7777");
        }
        if (framework == nullptr)
        {
            framework = new Framework;
            framework->Server(server);
            pthread_create(&frameworkThread, nullptr, [](void*) -> void* { framework->Dispatch(); return nullptr; }, nullptr);
        }
        if (ImGui::Begin("Framework", &showFramework))
        {
            ImGui::End();
        }
    }
    else if (frameworkThread)
    {
        framework->Terminate();
        pthread_join(frameworkThread, nullptr);

        delete server;
        delete framework;

        server = nullptr;
        framework = nullptr;
        frameworkThread = nullptr;
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
