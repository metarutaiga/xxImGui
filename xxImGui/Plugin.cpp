//==============================================================================
// xxImGui : Plugin Source
//
// Copyright (c) 2019-2021 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include "xxGraphic/xxSystem.h"
#include "DearImGui.h"
#include "Renderer.h"
#include "Plugin.h"

static ImVector<void*>                  g_pluginLibraries;
static ImVector<const char*>            g_pluginTitles;
static ImVector<PFN_PLUGIN_CREATE>      g_pluginCreates;
static ImVector<PFN_PLUGIN_SHUTDOWN>    g_pluginShutdowns;
static ImVector<PFN_PLUGIN_UPDATE>      g_pluginUpdates;
static ImVector<PFN_PLUGIN_RENDER>      g_pluginRenders;
//------------------------------------------------------------------------------
void Plugin::Create(const char* path, uint64_t device)
{
    const char* app = xxGetExecutablePath();
    const char* configuration = "";
    const char* arch = "";
    const char* extension = "";
#if defined(xxWINDOWS)
#if defined(__llvm__)
#elif defined(_DEBUG)
    configuration = "Debug";
#elif defined(NDEBUG)
    configuration = "Release";
#endif
#if defined(__llvm__)
#elif defined(_M_ARM64EC)
    arch = ".arm64ec";
#elif defined(_M_HYBRID_X86_ARM64)
    arch = ".chpe";
#elif defined(_M_AMD64)
    arch = ".x64";
#elif defined(_M_IX86)
    arch = ".Win32";
#elif defined(_M_ARM64)
    arch = ".arm64";
#elif defined(_M_ARM)
    arch = ".arm";
#endif
    extension = ".dll";
#elif defined(xxMACOS)
    extension = ".dylib";
#elif defined(xxIOS)
    extension = ".bundle";
#elif defined(xxANDROID)
    extension = ".so";
#endif

    char temp[4096];
#if defined(xxWINDOWS)
    snprintf(temp, 4096, "%s\\%s", app, path);
#elif defined(xxMACOS)
    snprintf(temp, 4096, "%s/../Frameworks", app);
#elif defined(xxIOS)
    snprintf(temp, 4096, "%s/Frameworks", app);
#elif defined(xxANDROID)
    snprintf(temp, 4096, "%s", app);
#endif

    uint64_t handle = 0;
    while (char* filename = xxOpenDirectory(&handle, temp, configuration, arch, extension, nullptr))
    {
#if defined(xxWINDOWS)
        snprintf(temp, 4096, "%s\\%s\\%s", app, path, filename);
#elif defined(xxMACOS)
        snprintf(temp, 4096, "%s/../Frameworks/%s", app, filename);
#elif defined(xxIOS)
        snprintf(temp, 4096, "%s/Frameworks/%s", app, filename);
#elif defined(xxANDROID)
        snprintf(temp, 4096, "%s/%s", app, filename);
#endif
        void* library = xxLoadLibrary(temp);
        if (library == nullptr)
        {
            free(filename);
            continue;
        }

        PFN_PLUGIN_CREATE create = (PFN_PLUGIN_CREATE)xxGetProcAddress(library, "Create");
        PFN_PLUGIN_SHUTDOWN shutdown = (PFN_PLUGIN_SHUTDOWN)xxGetProcAddress(library, "Shutdown");
        PFN_PLUGIN_UPDATE update = (PFN_PLUGIN_UPDATE)xxGetProcAddress(library, "Update");
        PFN_PLUGIN_RENDER render = (PFN_PLUGIN_RENDER)xxGetProcAddress(library, "Render");
        if (create == nullptr || shutdown == nullptr || update == nullptr || render == nullptr)
        {
            xxFreeLibrary(library);
            free(filename);
            continue;
        }
        xxLog("Plugin", "Loaded : %s", filename);
        free(filename);

        g_pluginLibraries.push_back(library);
        g_pluginCreates.push_back(create);
        g_pluginShutdowns.push_back(shutdown);
        g_pluginUpdates.push_back(update);
        g_pluginRenders.push_back(render);
    }
    xxCloseDirectory(&handle);

    CreateData createData;
    createData.device = device;
    createData.baseFolder = app;
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
        xxFreeLibrary(library);
    }

    g_pluginLibraries.clear();
    g_pluginTitles.clear();
    g_pluginCreates.clear();
    g_pluginShutdowns.clear();
    g_pluginUpdates.clear();
    g_pluginRenders.clear();
}
//------------------------------------------------------------------------------
int Plugin::Count()
{
    return (int)g_pluginUpdates.size();
}
//------------------------------------------------------------------------------
bool Plugin::Update()
{
    bool updated = false;

    static int updateCount = 0;
    if (updateCount)
    {
        updateCount--;
        updated = true;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        updateCount = 1;
    }

    UpdateData updateData;
    updateData.instance = Renderer::g_instance;
    updateData.device = Renderer::g_device;
    updateData.renderPass = Renderer::g_renderPass;
    updateData.width = Renderer::g_width;
    updateData.height = Renderer::g_height;
    updateData.time = xxGetCurrentTime();
    updateData.windowScale = ImGui::GetStyle().MouseCursorScale;
    for (int i = 0; i < g_pluginUpdates.size(); ++i)
    {
        PFN_PLUGIN_UPDATE update = g_pluginUpdates[i];
        updated |= update(updateData);
    }

    return updated;
}
//------------------------------------------------------------------------------
void Plugin::Render()
{
    RenderData renderData;
    renderData.instance = Renderer::g_instance;
    renderData.device = Renderer::g_device;
    renderData.renderPass = Renderer::g_renderPass;
    renderData.commandBuffer = Renderer::g_currentCommandBuffer;
    renderData.commandEncoder = Renderer::g_currentCommandEncoder;
    renderData.commandFramebuffer = Renderer::g_currentCommandFramebuffer;
    renderData.width = Renderer::g_width;
    renderData.height = Renderer::g_height;
    for (int i = 0; i < g_pluginRenders.size(); ++i)
    {
        PFN_PLUGIN_RENDER render = g_pluginRenders[i];
        render(renderData);
    }
}
//------------------------------------------------------------------------------
