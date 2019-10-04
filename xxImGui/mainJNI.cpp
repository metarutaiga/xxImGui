#include <android/native_window_jni.h>
#include <jni.h>
#include <string>

#include "Renderer.h"
#include "DearImGui.h"

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_create(JNIEnv* env, jobject obj, jobject surface)
{
    void* window = nullptr;
    if (surface != nullptr)
    {
        window = ANativeWindow_fromSurface(env, surface);
    }

    Renderer::Create(window);
    DearImGui::Create(window, 2.0f);
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_resize(JNIEnv* env, jobject obj, jint width, jint height, jobject surface)
{
    void* window = nullptr;
    if (surface != nullptr)
    {
        window = ANativeWindow_fromSurface(env, surface);
    }

    Renderer::Reset(window, width, height);
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_step(JNIEnv* env, jobject obj)
{
    DearImGui::NewFrame(Renderer::g_view);
    DearImGui::Update();

    uint64_t commandEncoder = Renderer::Begin();
    if (commandEncoder)
    {
        DearImGui::Render(commandEncoder);
        Renderer::End();
        if (Renderer::Present() == false)
        {
            Renderer::Reset(Renderer::g_view);
        }
    }

    DearImGui::PostUpdate(Renderer::g_view);
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_shutdown(JNIEnv* env, jobject obj)
{
    DearImGui::Shutdown();
    Renderer::Shutdown();
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_pause(JNIEnv* env, jobject obj)
{
}

extern "C" JNIEXPORT void JNICALL Java_com_xx_Activity_resume(JNIEnv* env, jobject obj)
{
}
