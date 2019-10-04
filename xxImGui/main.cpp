// dear imgui: standalone example application
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "Renderer.h"
#include "Plugin.h"
#include "DearImGui.h"
#include "implement/imgui_impl_win32.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Forward declarations of helper functions
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float scale = ImGui_ImplWin32_GetDpiScaleForHwnd(NULL);

    // Create application window
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_OWNDC, WndProc, 0, 0, GetModuleHandleW(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hWnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui XX Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280 * scale, 800 * scale, NULL, NULL, wc.hInstance, NULL);

    Renderer::Create(hWnd);
    Plugin::Create("plugin");
    DearImGui::Create(hWnd, scale);

    // Show the window
    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);

    // Main loop
    MSG msg = {};
    bool recreateWindow = false;
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);

            // Recreate window when SwapEffect is changed.
            if (msg.message == WM_QUIT && recreateWindow)
            {
                msg.message = 0;
                recreateWindow = false;
            }
            continue;
        }

        DearImGui::NewFrame(hWnd);
        Plugin::Update();
        DearImGui::Update();

        uint64_t commandEncoder = Renderer::Begin();
        if (commandEncoder)
        {
            Plugin::Render(commandEncoder);
            DearImGui::Render(commandEncoder);
            Renderer::End();
            if (Renderer::Present() == false)
            {
                DearImGui::Suspend();
                Renderer::Reset(hWnd);
                DearImGui::Resume();
            }
        }

        HWND result = (HWND)DearImGui::PostUpdate(hWnd);
        if (hWnd != result)
        {
            hWnd = result;
            recreateWindow = true;
            continue;
        }
    }

    DearImGui::Shutdown();
    Plugin::Shutdown();
    Renderer::Shutdown();

    ::DestroyWindow(hWnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

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
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
