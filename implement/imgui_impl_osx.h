// dear imgui: Platform Binding for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan, Metal..)
// [ALPHA] Early bindings, not well tested. If you want a portable application, prefer using the GLFW or SDL platform bindings on Mac.

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: OSX clipboard is supported within core Dear ImGui (no specific code in this back-end).
// Issues:
//  [ ] Platform: Keys are all generally very broken. Best using [event keycode] and not [event characters]..
//  [ ] Platform: Multi-viewport / platform windows.

IMGUI_API bool        ImGui_ImplOSX_Init(void* view);
IMGUI_API void        ImGui_ImplOSX_Shutdown();
IMGUI_API void        ImGui_ImplOSX_NewFrame(void* view);
IMGUI_API bool        ImGui_ImplOSX_HandleEvent(void* event, void* view);
