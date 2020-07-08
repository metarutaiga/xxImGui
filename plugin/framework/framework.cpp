//==============================================================================
// xxImGui : Plugin Framework Source
//
// Copyright (c) 2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <interface.h>
#include "ConcurrencyNetworkFramework/Connection.h"
#include "ConcurrencyNetworkFramework/Framework.h"
#include "ConcurrencyNetworkFramework/Listener.h"
#include "ConcurrencyNetworkFramework/Log.h"
#include "client.h"

#define PLUGIN_NAME     "Framework"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

#define SERVER_IP       "127.0.0.1"
#define SERVER_PORT     "5227"

static Listener* server = nullptr;
static Framework* framework = nullptr;
static std::thread* frameworkThread = nullptr;
static std::vector<Client*> clients;

static std::vector<Connection*> connects;
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

        server = new Listener(SERVER_IP, SERVER_PORT);
        framework = new Framework;
        framework->Server(server);
        frameworkThread = new std::thread([]{ framework->Dispatch(4); });
    }
}
//------------------------------------------------------------------------------
static void shutdownFramework()
{
    for (Connection* connect : connects)
        connect->Disconnect();
    connects.clear();
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
static void benchmarkFramework()
{
    if (benchmark && frameworkThread)
    {
        size_t bucket = 32;
        size_t create = rand() % bucket;
        size_t remove = rand() % bucket;

        while (connects.size() < create)
        {
            Connection* connect = new Connection(SERVER_IP, SERVER_PORT);
            if (connect && connect->ConnectTCP() == false)
            {
                connect->Disconnect();
                continue;
            }
            connect->Send(Buffer::Get(4));
            connect->ConnectUDP();
            connect->Send(Buffer::Get(4));
            connects.emplace_back(connect);
        }

        while (connects.size() > remove)
        {
            Connection* connect = connects.back();
            connect->Disconnect();
            connects.pop_back();
        }
    }
    else
    {
        for (Connection* connect : connects)
            connect->Disconnect();
        connects.clear();
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
    for (Client* client : clients)
    {
        delete client;
    }
    clients.clear();
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
            if (ImGui::MenuItem("Add Client", nullptr, nullptr))
            {
                clients.emplace_back(new Client(SERVER_IP, SERVER_PORT));
            }
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

    ImVec2 pos = {};

    if (showFramework)
    {
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
                clients.emplace_back(new Client(SERVER_IP, SERVER_PORT));
            }

            ImGui::SameLine();
            if (ImGui::Button("Benchmark"))
            {
                benchmark = !benchmark;
            }
            benchmarkFramework();

            ImGui::Text("Active Thread : %d/%d/%d/%d Pool Size: %d/%d", Connection::GetActiveThreadCount(0),
                                                                        Connection::GetActiveThreadCount(1),
                                                                        Connection::GetActiveThreadCount(2),
                                                                        Connection::GetActiveThreadCount(3),
                                                                        Buffer::Used(),
                                                                        Buffer::Unused());

            ImVec2 windowSize = ImVec2(800 * updateData.windowScale, 200 * updateData.windowScale);

            logMutex.lock();
            ImGui::InputTextMultiline("INFO", logInfo.size() ? &logInfo.front() : (char*)"", logInfo.size(), windowSize, ImGuiInputTextFlags_ReadOnly);
            ImGui::BeginChild("INFO");
            ImGui::SetScrollHereY(1);
            ImGui::EndChild();
            ImGui::InputTextMultiline("ERROR", logError.size() ? &logError.front() : (char*)"", logError.size(), windowSize, ImGuiInputTextFlags_ReadOnly);
            ImGui::BeginChild("ERROR");
            ImGui::SetScrollHereY(1);
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

            pos.y += 90 * updateData.windowScale;
        }
    }
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
