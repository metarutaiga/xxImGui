//==============================================================================
// xxImGui : Plugin Source
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include "xxGraphic/xxSystem.h"
#include "DearImGui.h"

#include "Plugin.h"

static ImVector<void*>                  g_pluginLibraries;
static ImVector<const char*>            g_pluginTitles;
static ImVector<PFN_PLUGIN_CREATE>      g_pluginCreates;
static ImVector<PFN_PLUGIN_SHUTDOWN>    g_pluginShutdowns;
static ImVector<PFN_PLUGIN_UPDATE>      g_pluginUpdates;
static ImVector<PFN_PLUGIN_RENDER>      g_pluginRenders;
//------------------------------------------------------------------------------
void Plugin::Create(const char* path)
{
    char app[256];
#if defined(xxWINDOWS)
    GetModuleFileNameA(nullptr, app, sizeof(app));
    if (char* slash = strrchr(app, '\\'))
        (*slash) = '\0';
#else
    app[0] = '\0';
#endif

    char temp[256];
#if defined(_M_AMD64)
#   if defined(_DEBUG)
    snprintf(temp, 256, "%s/%s/*.debug.x64.dll", app, path);
#   else
    snprintf(temp, 256, "%s/%s/*.release.x64.dll", app, path);
#   endif
#elif defined(_M_IX86)
#   if defined(_DEBUG)
    snprintf(temp, 256, "%s/%s/*.debug.x86.dll", app, path);
#   else
    snprintf(temp, 256, "%s/%s/*.release.x86.dll", app, path);
#   endif
#else
    snprintf(temp, 256, "%s/%s/*", app, path);
#endif

    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFileA(temp, &data);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do {
            if (data.cFileName[0] == '.')
                continue;

            snprintf(temp, 256, "%s/%s/%s", app, path, data.cFileName);
            HMODULE library = LoadLibraryA(temp);
            if (library == nullptr)
                continue;

            PFN_PLUGIN_CREATE create = (PFN_PLUGIN_CREATE)GetProcAddress(library, "Create");
            PFN_PLUGIN_SHUTDOWN shutdown = (PFN_PLUGIN_SHUTDOWN)GetProcAddress(library, "Shutdown");
            PFN_PLUGIN_UPDATE update = (PFN_PLUGIN_UPDATE)GetProcAddress(library, "Update");
            PFN_PLUGIN_RENDER render = (PFN_PLUGIN_RENDER)GetProcAddress(library, "Render");
            if (create == nullptr || shutdown == nullptr || update == nullptr || render == nullptr)
            {
                FreeLibrary(library);
                continue;
            }

            g_pluginLibraries.push_back(library);
            g_pluginCreates.push_back(create);
            g_pluginShutdowns.push_back(shutdown);
            g_pluginUpdates.push_back(update);
            g_pluginRenders.push_back(render);
        } while (FindNextFileA(handle, &data) == TRUE);

        FindClose(handle);
    }

    CreateData createData;
    for (int i = 0; i < g_pluginCreates.size(); ++i)
    {
        PFN_PLUGIN_CREATE create = g_pluginCreates[i];
        const char* title = create(createData);
        g_pluginTitles.push_back(title);
    }
}
//------------------------------------------------------------------------------
void Plugin::Shutdown()
{
    ShutdownData shutdownData;
    for (int i = 0; i < g_pluginShutdowns.size(); ++i)
    {
        PFN_PLUGIN_SHUTDOWN shutdown = g_pluginShutdowns[i];
        shutdown(shutdownData);
    }

    for (int i = 0; i < g_pluginLibraries.size(); ++i)
    {
        void* library = g_pluginLibraries[i];
#if defined(xxWINDOWS)
        FreeLibrary(HMODULE(library));
#else
        dlclose(library);
#endif
    }

    g_pluginLibraries.clear();
    g_pluginTitles.clear();
    g_pluginCreates.clear();
    g_pluginShutdowns.clear();
    g_pluginUpdates.clear();
    g_pluginRenders.clear();
}
//------------------------------------------------------------------------------
void Plugin::Update()
{
    UpdateData updateData;
    updateData.time = xxGetCurrentTime();
    for (int i = 0; i < g_pluginUpdates.size(); ++i)
    {
        PFN_PLUGIN_UPDATE update = g_pluginUpdates[i];
        update(updateData);
    }
}
//------------------------------------------------------------------------------
void Plugin::Render(uint64_t commandEncoder)
{
    RenderData renderData;
    renderData.commandEncoder = commandEncoder;
    for (int i = 0; i < g_pluginRenders.size(); ++i)
    {
        PFN_PLUGIN_RENDER render = g_pluginRenders[i];
        render(renderData);
    }
}
//------------------------------------------------------------------------------
