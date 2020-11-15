//==============================================================================
// xxImGui : Plugin Sample Source
//
// Copyright (c) 2019-2020 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <interface.h>

#if defined(xxANDROID) || defined(xxMACOS) || defined(xxIOS)
#   include <sys/time.h>
#endif

#define PLUGIN_NAME     "Sample"
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
    static bool showClock = false;
    static bool showTimer = false;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Clock", nullptr, &showClock);
            ImGui::Separator();
            ImGui::MenuItem("Timer", nullptr, &showTimer);
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

    if (showClock)
    {
        if (ImGui::Begin("Clock", &showClock))
        {
            char textPrevious[64];
            char textCurrent[64];
            time_t t = time(nullptr);
            struct tm* tm = localtime(&t);
            strftime(textCurrent, sizeof(textCurrent), "%c", tm);
            ImGui::TextUnformatted(textCurrent);
            if (strcmp(textPrevious, textCurrent) != 0)
            {
                strcpy(textPrevious, textCurrent);
                updated = true;
            }
        }
    }

    if (showTimer)
    {
        if (ImGui::Begin("Timer", &showTimer))
        {
            ImGui::Text("Current Time : %f", ImGui::GetTime());
            ImGui::Text("Delta Time : %f", ImGuiIO().DeltaTime);
#if defined(xxANDROID) || defined(xxMACOS) || defined(xxIOS)
            static timeval startSystem;
            static float startLibrary;
            timeval system;
            gettimeofday(&system, nullptr);
            float library = ImGui::GetTime() * 1000.0f;
            if (startSystem.tv_sec == 0)
                startSystem = system;
            if (startLibrary == 0.0f)
                startLibrary = library;
            float floatSystem = (system.tv_sec - startSystem.tv_sec) * 1000.0f + (system.tv_usec - startSystem.tv_usec) / 1000.0f;
            float accuracy = (library - startLibrary) - floatSystem;
            ImGui::Text("Accuracy Time : %f=%f-%f", accuracy, (library - startLibrary), floatSystem);
#endif
            updated = true;
        }
    }

    return updated;
}
//------------------------------------------------------------------------------
pluginAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
