//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <mutex>
#include <thread>
#include <interface.h>
#include "../../ConcurrencyNetworkFramework/Connect.h"
#include "../../ConcurrencyNetworkFramework/Listen.h"
#include "../../ConcurrencyNetworkFramework/Log.h"
#include "../../ConcurrencyNetworkFramework/Framework.h"
#include "client.h"

#define PLUGIN_NAME     "Framework"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static Listen* server = nullptr;
static Framework* framework = nullptr;
static std::thread* frameworkThread = nullptr;
static std::vector<Client*> clients;

static std::vector<Connect*> connects;
static bool benchmark = false;

static std::string logInfo;
static std::string logError;
static std::mutex logMutex;

//------------------------------------------------------------------------------
static void startFramework()
{
    if (server == nullptr || framework == nullptr)
    {
        logMutex.lock();
        logInfo.clear();
        logError.clear();
        logMutex.unlock();
        Log::SetOutput([](int level, const char* output)
        {
            logMutex.lock();
            (level < 0 ? logError : logInfo) += output;
            logMutex.unlock();
        });

        server = new Listen("127.0.0.1", "7777");
        framework = new Framework;
        framework->Server(server);
        frameworkThread = new std::thread([]{ framework->Dispatch(); });
    }
}
//------------------------------------------------------------------------------
static void shutdownFramework()
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
//------------------------------------------------------------------------------
pluginAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
pluginAPI void Shutdown(const ShutdownData& shutdownData)
{
    shutdownFramework();
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
            ImGui::MenuItem("Framework", nullptr, &showFramework);
            ImGui::Separator();
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
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
        ImVec2 pos = {};

        if (ImGui::Begin("Framework", &showFramework))
        {
            pos = ImGui::GetWindowPos();
            pos.x += ImGui::GetWindowWidth();

            if (ImGui::Button("Start Framework"))
            {
                startFramework();
                benchmark = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("Stop Framework"))
            {
                shutdownFramework();
                benchmark = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("Add Client"))
            {
                clients.emplace_back(new Client("127.0.0.1", "7777"));
            }

            ImGui::SameLine();
            if (ImGui::Button("Benchmark"))
            {
                benchmark = !benchmark;
            }
            if (benchmark)
            {
                size_t bucket = 64;
                size_t create = rand() % 64;
                size_t remove = rand() % 64;

                while (connects.size() < create)
                {
                    Connect* connect = new Connect("127.0.0.1", "7777");
                    if (connect && connect->Start() == false)
                    {
                        connect->Disconnect();
                        continue;
                    }
                    connects.emplace_back(connect);
                }

                while (connects.size() > remove)
                {
                    Connect* connect = connects.back();
                    connect->Disconnect();
                    connects.pop_back();
                }
            }
            else
            {
                for (Connect* connect : connects)
                    connect->Disconnect();
                connects.clear();
            }

            ImVec2 windowSize = ImVec2(800 * updateData.windowScale, 200 * updateData.windowScale);

            logMutex.lock();
            ImGui::InputTextMultiline("INFO", &logInfo.front(), logInfo.size(), windowSize, ImGuiInputTextFlags_ReadOnly);
            ImGui::BeginChild("INFO");
            ImGui::SetScrollHere(1);
            ImGui::EndChild();
            ImGui::InputTextMultiline("ERROR", &logError.front(), logError.size(), windowSize, ImGuiInputTextFlags_ReadOnly);
            ImGui::BeginChild("ERROR");
            ImGui::SetScrollHere(1);
            ImGui::EndChild();
            logMutex.unlock();

            ImGui::End();
        }

        for (auto it = clients.begin(); it != clients.end(); ++it)
        {
            ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
            if ((*it)->Update(updateData) == false)
            {
                delete (*it);
                clients.erase(it);
                break;
            }

            pos.y += 96;
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
