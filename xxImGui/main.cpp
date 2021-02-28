// dear imgui: standalone example application
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "Renderer.h"
#include "Plugin.h"
#include "DearImGui.h"
#include "implement/imgui_impl_win32.h"
#include "xxMiniCRT/xxMSVCRT.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#define USE_MINICRT 1
#if USE_MINICRT
IMPLEMENT_MINICRT();
#endif

// Forward declarations of helper functions
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID WINAPI ShouldUseDarkMode(HWND hWnd);

// Main code
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, LPWSTR cmdLine, int showCmd)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float scale = ImGui_ImplWin32_GetDpiScaleForHwnd(NULL);

    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_OWNDC, WndProc, 0, 0, instance, NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hWnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui XX Example"), WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * scale), (int)(720 * scale), NULL, NULL, wc.hInstance, NULL);

    Renderer::Create(hWnd, (int)(1280 * scale), (int)(720 * scale));
    DearImGui::Create(hWnd, scale, scale);
    Plugin::Create("plugin", Renderer::g_device);

    // Show the window
    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);

    // Main loop
    MSG msg = {};
    bool imguiUpdate = true;
    bool recreateWindow = false;
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            // Recreate window when SwapEffect is changed.
            if (msg.message == WM_QUIT && recreateWindow)
            {
                msg.message = 0;
                recreateWindow = false;
            }

            imguiUpdate = true;
            continue;
        }

        DearImGui::NewFrame(hWnd);
        imguiUpdate |= Plugin::Update();
        imguiUpdate |= DearImGui::Update(Plugin::Count() == 0);

        if (imguiUpdate)
        {
            uint64_t commandEncoder = Renderer::Begin();
            if (commandEncoder)
            {
                DearImGui::Render(commandEncoder);
                Plugin::Render();
                Renderer::End();
                if (Renderer::Present() == false)
                {
                    DearImGui::Suspend();
                    Renderer::Reset(hWnd, Renderer::g_width, Renderer::g_height);
                    DearImGui::Resume();
                }
            }
        }

        HWND result = (HWND)DearImGui::PostUpdate(hWnd, imguiUpdate);

        if (imguiUpdate == false && DearImGui::PowerSaving())
            xxSleep(1000 / 120);

        imguiUpdate = false;

        if (hWnd != result)
        {
            hWnd = result;
            recreateWindow = true;
            continue;
        }
    }

    Plugin::Shutdown();
    DearImGui::Shutdown();
    Renderer::Shutdown();

    ::DestroyWindow(hWnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_CREATE:
        ShouldUseDarkMode(hWnd);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            Renderer::Reset(hWnd, LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

// Win32 dark mode
VOID WINAPI ShouldUseDarkMode(HWND hWnd)
{
    DWORD darkMode = TRUE;

    HMODULE ntdll = ::LoadLibraryA("ntdll.dll");
    HMODULE uxtheme = ::LoadLibraryA("uxtheme.dll");
    if (ntdll && uxtheme)
    {
        VOID(WINAPI * RtlGetNtVersionNumbers)(DWORD*, DWORD*, DWORD*);
        (void*&)RtlGetNtVersionNumbers = ::GetProcAddress(ntdll, "RtlGetNtVersionNumbers");
        if (RtlGetNtVersionNumbers)
        {
            DWORD major = 0;
            DWORD minor = 0;
            DWORD buildNumber = 0;
            RtlGetNtVersionNumbers(&major, &minor, &buildNumber);
            buildNumber &= ~0xF0000000;
            if (major >= 10 && buildNumber >= 17763)
            {
                BOOL(WINAPI * ShouldAppsUseDarkMode)();
                (void*&)ShouldAppsUseDarkMode = ::GetProcAddress(uxtheme, MAKEINTRESOURCEA(132));
                if (ShouldAppsUseDarkMode)
                {
                    darkMode = ShouldAppsUseDarkMode();
                }

                DWORD(WINAPI * SetPreferredAppMode)(DWORD mode);
                (void*&)SetPreferredAppMode = ::GetProcAddress(uxtheme, MAKEINTRESOURCEA(135));
                if (SetPreferredAppMode)
                {
                    SetPreferredAppMode(darkMode);
                }
            }
        }

        ::FreeLibrary(ntdll);
        ::FreeLibrary(uxtheme);
    }

    HMODULE user32 = ::LoadLibraryA("user32.dll");
    if (user32)
    {
        struct WINCOMPATTRDATA
        {
            DWORD dwAttribute;
            LPCVOID pvAttribute;
            DWORD cbAttribute;
        };

        BOOL(WINAPI * SetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
        (void*&)SetWindowCompositionAttribute = ::GetProcAddress(user32, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            DWORD WCA_USEDARKMODECOLORS = 26;
            WINCOMPATTRDATA data = { WCA_USEDARKMODECOLORS, &darkMode, sizeof(darkMode) };
            SetWindowCompositionAttribute(hWnd, &data);
        }

        ::FreeLibrary(user32);
    }
}
