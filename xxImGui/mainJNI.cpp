#include <android/native_window_jni.h>
#include <jni.h>
#include <string>

#include "Renderer.h"
#include "Plugin.h"
#include "DearImGui.h"

static bool g_initialized = false;

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_create(JNIEnv* env, jclass obj, jobject surface)
{
    if (g_initialized)
        return;
    g_initialized = true;

    ANativeWindow* window = nullptr;
    if (surface != nullptr)
    {
        window = ANativeWindow_fromSurface(env, surface);
    }

    int width = 1280;
    int height = 720;
    if (window != nullptr)
    {
        width = ANativeWindow_getWidth(window);
        height = ANativeWindow_getHeight(window);
    }

    xxJNIEnv = env;

    Renderer::Create(window, width, height);
    DearImGui::Create(window, 3.0f);
    Plugin::Create("plugin", Renderer::g_device);
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_resize(JNIEnv* env, jclass obj, jint width, jint height, jobject surface)
{
    ANativeWindow* window = nullptr;
    if (surface != nullptr)
    {
        window = ANativeWindow_fromSurface(env, surface);
    }

    Renderer::Reset(window, width, height);
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_step(JNIEnv* env, jclass obj)
{
    DearImGui::NewFrame(Renderer::g_view);
    DearImGui::Update(Plugin::Update() == false);

    uint64_t commandEncoder = Renderer::Begin();
    if (commandEncoder)
    {
        DearImGui::Render(commandEncoder);
        Renderer::End();
        if (Renderer::Present() == false)
        {
            DearImGui::Suspend();
            Renderer::Reset(Renderer::g_view, Renderer::g_width, Renderer::g_height);
            DearImGui::Resume();
        }
    }

    DearImGui::PostUpdate(Renderer::g_view);
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_shutdown(JNIEnv* env, jclass obj)
{
    Plugin::Shutdown();
    DearImGui::Shutdown();
    Renderer::Shutdown();

    g_initialized = false;
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_pause(JNIEnv* env, jclass obj)
{
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_resume(JNIEnv* env, jclass obj)
{
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_touch(JNIEnv* env, jclass obj, jint type, jfloat x, jfloat y)
{
    DearImGui::HandleEventAndroid(type, x, y);
}
