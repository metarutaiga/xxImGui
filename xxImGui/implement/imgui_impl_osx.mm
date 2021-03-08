// dear imgui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan, Metal..)
// [ALPHA] Early backend, not well tested. If you want a portable application, prefer using the GLFW or SDL platform Backends on Mac.

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: OSX clipboard is supported within core Dear ImGui (no specific code in this backend).
//  [X] Platform: Multi-viewport / platform windows.
// Issues:
//  [ ] Platform: Keys are all generally very broken. Best using [event keycode] and not [event characters]..

// You can copy and use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_osx.h"
#import <Cocoa/Cocoa.h>

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2021-01-27: Inputs: Added a fix for mouse position not being reported when mouse buttons other than left one are down.
//  2020-10-28: Inputs: Added a fix for handling keypad-enter key.
//  2020-05-25: Inputs: Added a fix for missing trackpad clicks when done with "soft tap".
//  2019-12-05: Inputs: Added support for ImGuiMouseCursor_NotAllowed mouse cursor.
//  2019-10-11: Inputs:  Fix using Backspace key.
//  2019-09-07: Implement Multi-viewport.
//  2019-07-21: Re-added clipboard handlers as they are not enabled by default in core imgui.cpp (reverted 2019-05-18 change).
//  2019-05-28: Inputs: Added mouse cursor shape and visibility support.
//  2019-05-18: Misc: Removed clipboard handlers as they are now supported by core imgui.cpp.
//  2019-05-11: Inputs: Don't filter character values before calling AddInputCharacter() apart from 0xF700..0xFFFF range.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-07-07: Initial version.

// Data
static NSWindow*      g_Window = nil;
static id             g_Monitor = nil;
static CFAbsoluteTime g_Time = 0.0;
static NSCursor*      g_MouseCursors[ImGuiMouseCursor_COUNT] = {};
static bool           g_MouseCursorHidden = false;
static bool           g_MouseJustPressed[ImGuiMouseButton_COUNT] = {};
static bool           g_MouseDown[ImGuiMouseButton_COUNT] = {};
static bool           g_WantUpdateMonitors = true;

// Forward Declarations
static void ImGui_ImplOSX_InitPlatformInterface();
static void ImGui_ImplOSX_ShutdownPlatformInterface();
static void ImGui_ImplOSX_UpdateMonitors();

// Undocumented methods for creating cursors.
@interface NSCursor()
+ (id)_windowResizeNorthWestSouthEastCursor;
+ (id)_windowResizeNorthEastSouthWestCursor;
+ (id)_windowResizeNorthSouthCursor;
+ (id)_windowResizeEastWestCursor;
@end

// Functions
bool ImGui_ImplOSX_Init(NSView* view)
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup backend capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;      // We can create multi-viewports on the Platform side (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
    io.BackendPlatformName = "imgui_impl_osx";

    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    g_Window = [view window];
    if (g_Window == nil)
    {
        g_Window = [NSApp orderedWindows].firstObject;
    }
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw = (__bridge_retained void*)g_Window;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_ImplOSX_InitPlatformInterface();

    // Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeyDown[] array.
    const int offset_for_function_keys = 256 - 0xF700;
    io.KeyMap[ImGuiKey_Tab]             = '\t';
    io.KeyMap[ImGuiKey_LeftArrow]       = NSLeftArrowFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_RightArrow]      = NSRightArrowFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_UpArrow]         = NSUpArrowFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_DownArrow]       = NSDownArrowFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_PageUp]          = NSPageUpFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_PageDown]        = NSPageDownFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_Home]            = NSHomeFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_End]             = NSEndFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_Insert]          = NSInsertFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_Delete]          = NSDeleteFunctionKey + offset_for_function_keys;
    io.KeyMap[ImGuiKey_Backspace]       = 127;
    io.KeyMap[ImGuiKey_Space]           = 32;
    io.KeyMap[ImGuiKey_Enter]           = 13;
    io.KeyMap[ImGuiKey_Escape]          = 27;
    io.KeyMap[ImGuiKey_KeyPadEnter]     = 3;
    io.KeyMap[ImGuiKey_A]               = 'A';
    io.KeyMap[ImGuiKey_C]               = 'C';
    io.KeyMap[ImGuiKey_V]               = 'V';
    io.KeyMap[ImGuiKey_X]               = 'X';
    io.KeyMap[ImGuiKey_Y]               = 'Y';
    io.KeyMap[ImGuiKey_Z]               = 'Z';

    // Load cursors. Some of them are undocumented.
    g_MouseCursorHidden = false;
    g_MouseCursors[ImGuiMouseCursor_Arrow] = [NSCursor arrowCursor];
    g_MouseCursors[ImGuiMouseCursor_TextInput] = [NSCursor IBeamCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = [NSCursor closedHandCursor];
    g_MouseCursors[ImGuiMouseCursor_Hand] = [NSCursor pointingHandCursor];
    g_MouseCursors[ImGuiMouseCursor_NotAllowed] = [NSCursor operationNotAllowedCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)] ? [NSCursor _windowResizeNorthSouthCursor] : [NSCursor resizeUpDownCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)] ? [NSCursor _windowResizeEastWestCursor] : [NSCursor resizeLeftRightCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)] ? [NSCursor _windowResizeNorthEastSouthWestCursor] : [NSCursor closedHandCursor];
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)] ? [NSCursor _windowResizeNorthWestSouthEastCursor] : [NSCursor closedHandCursor];

    // Note that imgui.cpp also include default OSX clipboard handlers which can be enabled
    // by adding '#define IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS' in imconfig.h and adding '-framework ApplicationServices' to your linker command-line.
    // Since we are already in ObjC land here, it is easy for us to add a clipboard handler using the NSPasteboard api.
    io.SetClipboardTextFn = [](void*, const char* str) -> void
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pasteboard setString:[NSString stringWithUTF8String:str] forType:NSPasteboardTypeString];
    };

    io.GetClipboardTextFn = [](void*) -> const char*
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        NSString* available = [pasteboard availableTypeFromArray: [NSArray arrayWithObject:NSPasteboardTypeString]];
        if (![available isEqualToString:NSPasteboardTypeString])
            return NULL;

        NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
        if (string == nil)
            return NULL;

        const char* string_c = (const char*)[string UTF8String];
        size_t string_len = strlen(string_c);
        static ImVector<char> s_clipboard;
        s_clipboard.resize((int)string_len + 1);
        strcpy(s_clipboard.Data, string_c);
        return s_clipboard.Data;
    };

    return true;
}

void ImGui_ImplOSX_Shutdown()
{
    ImGui_ImplOSX_ShutdownPlatformInterface();
    g_Window = nil;
    if (g_Monitor != nil)
    {
        [NSEvent removeMonitor:g_Monitor];
        g_Monitor = nil;
    }
}

static void ImGui_ImplOSX_UpdateMouseCursorAndButtons()
{
    // Update buttons
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[i] = g_MouseJustPressed[i] || g_MouseDown[i];
        g_MouseJustPressed[i] = false;
    }

    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        if (!g_MouseCursorHidden)
        {
            g_MouseCursorHidden = true;
            [NSCursor hide];
        }
    }
    else
    {
        // Show OS mouse cursor
        [g_MouseCursors[g_MouseCursors[imgui_cursor] ? imgui_cursor : ImGuiMouseCursor_Arrow] set];
        if (g_MouseCursorHidden)
        {
            g_MouseCursorHidden = false;
            [NSCursor unhide];
        }
    }
}

void ImGui_ImplOSX_NewFrame(NSView* view)
{
    // Set other windows to floating when mouse hit the main window
    NSRect rect = [g_Window frame];
    NSPoint mouse = [NSEvent mouseLocation];
    NSArray<NSWindow*>* orderedWindows = [NSApp orderedWindows];
    if ([g_Window isMiniaturized])
    {
        for (NSUInteger i = orderedWindows.count; i > 0; --i)
        {
            NSWindow* window = orderedWindows[i - 1];
            if (window.parentWindow != g_Window)
                continue;
            [window setLevel:NSNormalWindowLevel];
            [window setIsVisible:NO];
            [window setParentWindow:g_Window];
        }
    }
    else if (mouse.x >= rect.origin.x && mouse.x <= rect.origin.x + rect.size.width &&
        mouse.y >= rect.origin.y && mouse.y <= rect.origin.y + rect.size.height)
    {
        if ([NSApp isActive] && [g_Window isMiniaturized] == NO)
        {
            for (NSUInteger i = orderedWindows.count; i > 0; --i)
            {
                NSWindow* window = orderedWindows[i - 1];
                if (window.parentWindow != g_Window)
                    continue;
                [window setLevel:NSFloatingWindowLevel];
                [window setIsVisible:YES];
                [window setParentWindow:g_Window];
            }
        }
    }
    else
    {
        bool foundMainWindow = false;
        for (NSUInteger i = orderedWindows.count; i > 0; --i)
        {
            NSWindow* window = orderedWindows[i - 1];
            if (window == g_Window)
            {
                foundMainWindow = true;
                continue;
            }
            if (window.parentWindow != g_Window)
                continue;
            // Reorder other windows when the main window is focused
            if (foundMainWindow == false)
            {
                [window orderWindow:NSWindowAbove relativeTo:0];
            }
            [window setLevel:NSNormalWindowLevel];
            [window setIsVisible:YES];
            [window setParentWindow:g_Window];
        }
    }

    // Setup display size
    ImGuiIO& io = ImGui::GetIO();
    if (view)
    {
        const float dpi = (float)[view.window backingScaleFactor];
        io.DisplaySize = ImVec2((float)view.bounds.size.width, (float)view.bounds.size.height);
        io.DisplayFramebufferScale = ImVec2(dpi, dpi);
    }
    if (g_WantUpdateMonitors)
        ImGui_ImplOSX_UpdateMonitors();

    // Setup time step
    if (g_Time == 0.0)
        g_Time = CFAbsoluteTimeGetCurrent();
    CFAbsoluteTime current_time = CFAbsoluteTimeGetCurrent();
    io.DeltaTime = (float)(current_time - g_Time);
    g_Time = current_time;

    ImGui_ImplOSX_UpdateMouseCursorAndButtons();
}

static int mapCharacterToKey(int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    if (c == 25) // SHIFT+TAB -> TAB
        return 9;
    if (c >= 0 && c < 256)
        return c;
    if (c >= 0xF700 && c < 0xF700 + 256)
        return c - 0xF700 + 256;
    return -1;
}

static void resetKeys()
{
    ImGuiIO& io = ImGui::GetIO();
    for (int n = 0; n < IM_ARRAYSIZE(io.KeysDown); n++)
        io.KeysDown[n] = false;
}

bool ImGui_ImplOSX_HandleEvent(NSEvent* event, NSView* view)
{
    ImGuiIO& io = ImGui::GetIO();

    if (event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeOtherMouseDown)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < IM_ARRAYSIZE(g_MouseDown))
            g_MouseDown[button] = g_MouseJustPressed[button] = true;
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp || event.type == NSEventTypeOtherMouseUp)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < IM_ARRAYSIZE(g_MouseDown))
            g_MouseDown[button] = false;
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged || event.type == NSEventTypeRightMouseDragged || event.type == NSEventTypeOtherMouseDragged)
    {
        NSSize size;
        NSPoint mousePoint;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            NSScreen* screen = [[view window] screen];
            size = [screen frame].size;
            mousePoint = [NSEvent mouseLocation];
        }
        else
        {
            size = [view bounds].size;
            mousePoint = event.locationInWindow;
        }
        mousePoint = NSMakePoint(mousePoint.x, size.height - mousePoint.y);
        io.MousePos = ImVec2((float)mousePoint.x, (float)mousePoint.y);
    }

    if (event.type == NSEventTypeScrollWheel)
    {
        double wheel_dx = 0.0;
        double wheel_dy = 0.0;

        #if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
        if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        {
            wheel_dx = [event scrollingDeltaX];
            wheel_dy = [event scrollingDeltaY];
            if ([event hasPreciseScrollingDeltas])
            {
                wheel_dx *= 0.1;
                wheel_dy *= 0.1;
            }
        }
        else
        #endif // MAC_OS_X_VERSION_MAX_ALLOWED
        {
            wheel_dx = [event deltaX];
            wheel_dy = [event deltaY];
        }

        if (fabs(wheel_dx) > 0.0)
            io.MouseWheelH += (float)wheel_dx * 0.1f;
        if (fabs(wheel_dy) > 0.0)
            io.MouseWheel += (float)wheel_dy * 0.1f;
        return io.WantCaptureMouse;
    }

    // FIXME: All the key handling is wrong and broken. Refer to GLFW's cocoa_init.mm and cocoa_window.mm.
    if (event.type == NSEventTypeKeyDown)
    {
        NSString* str = [event characters];
        NSUInteger len = [str length];
        for (NSUInteger i = 0; i < len; i++)
        {
            int c = [str characterAtIndex:i];
            if (!io.KeyCtrl && !(c >= 0xF700 && c <= 0xFFFF) && c != 127)
                io.AddInputCharacter((unsigned int)c);

            // We must reset in case we're pressing a sequence of special keys while keeping the command pressed
            int key = mapCharacterToKey(c);
            if (key != -1 && key < 256 && !io.KeyCtrl)
                resetKeys();
            if (key != -1)
                io.KeysDown[key] = true;
        }
        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeKeyUp)
    {
        NSString* str = [event characters];
        NSUInteger len = [str length];
        for (NSUInteger i = 0; i < len; i++)
        {
            int c = [str characterAtIndex:i];
            int key = mapCharacterToKey(c);
            if (key != -1)
                io.KeysDown[key] = false;
        }
        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeFlagsChanged)
    {
        unsigned int flags = [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;

        bool oldKeyCtrl = io.KeyCtrl;
        bool oldKeyShift = io.KeyShift;
        bool oldKeyAlt = io.KeyAlt;
        bool oldKeySuper = io.KeySuper;
        io.KeyCtrl      = flags & NSEventModifierFlagControl;
        io.KeyShift     = flags & NSEventModifierFlagShift;
        io.KeyAlt       = flags & NSEventModifierFlagOption;
        io.KeySuper     = flags & NSEventModifierFlagCommand;

        // We must reset them as we will not receive any keyUp event if they where pressed with a modifier
        if ((oldKeyShift && !io.KeyShift) || (oldKeyCtrl && !io.KeyCtrl) || (oldKeyAlt && !io.KeyAlt) || (oldKeySuper && !io.KeySuper))
            resetKeys();
        return io.WantCaptureKeyboard;
    }

    return false;
}

void ImGui_ImplOSX_AddTrackingArea(NSViewController* _Nonnull controller)
{
    // Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                                                  owner:controller
                                                               userInfo:nil];
    [controller.view addTrackingArea:trackingArea];

    // If we want to receive key events, we either need to be in the responder chain of the key view,
    // or else we can install a local monitor. The consequence of this heavy-handed approach is that
    // we receive events for all controls, not just Dear ImGui widgets. If we had native controls in our
    // window, we'd want to be much more careful than just ingesting the complete event stream.
    // To match the behavior of other backends, we pass every event down to the OS.
    if (g_Monitor)
        return;
    NSEventMask eventMask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged | NSEventTypeScrollWheel;
    g_Monitor = [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^NSEvent * _Nullable(NSEvent *event)
    {
        ImGui_ImplOSX_HandleEvent(event, controller.view);
        return event;
    }];
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ImGuiViewportDataOSX
{
    NSWindow*               Window;
    bool                    WindowOwned;
    
    ImGuiViewportDataOSX()  { WindowOwned = false; }
    ~ImGuiViewportDataOSX() { IM_ASSERT(Window == nil); }
};

@interface ImGui_ImplOSX_View : NSView
@end

@implementation ImGui_ImplOSX_View
@end

@interface ImGui_ImplOSX_ViewController : NSViewController
@end

@implementation ImGui_ImplOSX_ViewController
-(void)loadView                         { self.view = [NSView new]; }
-(void)keyUp:(NSEvent *)event           { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)keyDown:(NSEvent *)event         { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)flagsChanged:(NSEvent *)event    { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseDown:(NSEvent *)event       { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseUp:(NSEvent *)event         { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseMoved:(NSEvent *)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseDragged:(NSEvent *)event    { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)scrollWheel:(NSEvent *)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
@end

static void ImGui_ImplOSX_CreateWindow(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = IM_NEW(ImGuiViewportDataOSX)();
    viewport->PlatformUserData = data;

    NSScreen* screen = [g_Window screen];
    NSSize size = [screen frame].size;
    NSRect rect = NSMakeRect(viewport->Pos.x, size.height - viewport->Pos.y - viewport->Size.y, viewport->Size.x, viewport->Size.y);

    NSWindow* window = [[NSWindow alloc] initWithContentRect:rect styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:YES];
    [window setTitle:@"Untitled"];
    [window setAcceptsMouseMovedEvents:YES];
    [window setOpaque:NO];
    [window setParentWindow:g_Window];

    ImGui_ImplOSX_View* view = [[ImGui_ImplOSX_View alloc] initWithFrame:rect];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        [view setWantsBestResolutionOpenGLSurface:YES];
#endif // MAC_OS_X_VERSION_MAX_ALLOWED >= 1070

    ImGui_ImplOSX_ViewController* viewController = [ImGui_ImplOSX_ViewController new];
    window.contentViewController = viewController;
    window.contentViewController.view = view;
    ImGui_ImplOSX_AddTrackingArea(viewController);

    data->Window = window;
    data->WindowOwned = true;
    viewport->PlatformRequestResize = false;
    viewport->PlatformHandle = viewport->PlatformHandleRaw = (__bridge_retained void*)window;
}

static void ImGui_ImplOSX_DestroyWindow(ImGuiViewport* viewport)
{
    NSWindow* window = (__bridge_transfer NSWindow*)viewport->PlatformHandleRaw;
    window = nil;

    if (ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData)
    {
        NSWindow* window = data->Window;
        if (window != nil && data->WindowOwned)
        {
            [window setContentView:nil];
            [window setContentViewController:nil];
            [window orderOut:nil];
        }
        data->Window = nil;
        IM_DELETE(data);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = viewport->PlatformHandleRaw = NULL;
}

static void ImGui_ImplOSX_ShowWindow(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != nil);
}

static void ImGui_ImplOSX_UpdateWindow(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);
}

static ImVec2 ImGui_ImplOSX_GetWindowPos(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    NSWindow* window = data->Window;
    NSScreen* screen = [window screen];
    NSSize size = [screen frame].size;
    NSRect frame = [window frame];
    NSRect rect = [window contentLayoutRect];
    return ImVec2(frame.origin.x, size.height - frame.origin.y - rect.size.height);
}

static void ImGui_ImplOSX_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    NSWindow* window = data->Window;
    NSScreen* screen = [window screen];
    NSSize size = [screen frame].size;
    NSRect rect = [window contentLayoutRect];
    NSRect origin = NSMakeRect(pos.x, size.height - pos.y - rect.size.height, 0, 0);
    [window setFrameOrigin:origin.origin];
}

static ImVec2 ImGui_ImplOSX_GetWindowSize(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    NSWindow* window = data->Window;
    NSSize size = [window contentLayoutRect].size;
    return ImVec2(size.width, size.width);
}

static void ImGui_ImplOSX_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    NSWindow* window = data->Window;
    NSRect rect = [window frame];
    rect.origin.y -= (size.y - rect.size.height);
    rect.size.width = size.x;
    rect.size.height = size.y;
    [window setFrame:rect display:YES];
}

static void ImGui_ImplOSX_SetWindowFocus(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    [data->Window orderWindow:NSWindowAbove relativeTo:0];
}

static bool ImGui_ImplOSX_GetWindowFocus(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    NSArray<NSWindow*>* array = [NSApp orderedWindows];
    for (NSInteger i = 0; i < array.count; ++i)
    {
        NSWindow* window = array[i];
        if (window != g_Window && window.parentWindow != g_Window)
            continue;
        return (window == data->Window);
    }
    return false;
}

static bool ImGui_ImplOSX_GetWindowMinimized(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    return [data->Window isMiniaturized];
}

static void ImGui_ImplOSX_SetWindowTitle(ImGuiViewport* viewport, const char* title)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    [data->Window setTitle:[NSString stringWithUTF8String:title]];
}

static void ImGui_ImplOSX_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);
    IM_ASSERT(alpha >= 0.0f && alpha <= 1.0f);

    [data->Window setAlphaValue:alpha];
}

static float ImGui_ImplOSX_GetWindowDpiScale(ImGuiViewport* viewport)
{
    ImGuiViewportDataOSX* data = (ImGuiViewportDataOSX*)viewport->PlatformUserData;
    IM_ASSERT(data->Window != 0);

    return [data->Window backingScaleFactor];
}

// FIXME-DPI: Testing DPI related ideas
static void ImGui_ImplOSX_OnChangedViewport(ImGuiViewport* viewport)
{
    (void)viewport;
#if 0
    ImGuiStyle default_style;
    //default_style.WindowPadding = ImVec2(0, 0);
    //default_style.WindowBorderSize = 0.0f;
    //default_style.ItemSpacing.y = 3.0f;
    //default_style.FramePadding = ImVec2(0, 0);
    default_style.ScaleAllSizes(viewport->DpiScale);
    ImGuiStyle& style = ImGui::GetStyle();
    style = default_style;
#endif
}

static void ImGui_ImplOSX_UpdateMonitors()
{
    ImGui::GetPlatformIO().Monitors.resize(0);

    NSArray* array = [NSScreen screens];
    for (NSUInteger i = 0; i < array.count; ++i)
    {
        NSScreen* screen = array[i];
        NSRect frame = [screen frame];
        NSRect visibleFrame = [screen visibleFrame];

        ImGuiPlatformMonitor imgui_monitor;
        imgui_monitor.MainPos = ImVec2(frame.origin.x, frame.origin.y);
        imgui_monitor.MainSize = ImVec2(frame.size.width, frame.size.height);
        imgui_monitor.WorkPos = ImVec2(visibleFrame.origin.x, visibleFrame.origin.y);
        imgui_monitor.WorkSize = ImVec2(visibleFrame.size.width, visibleFrame.size.height);
        imgui_monitor.DpiScale = [screen backingScaleFactor];

        ImGuiPlatformIO& io = ImGui::GetPlatformIO();
        io.Monitors.push_back(imgui_monitor);
    }

    g_WantUpdateMonitors = false;
}

static void ImGui_ImplOSX_InitPlatformInterface()
{
    ImGui_ImplOSX_UpdateMonitors();

    // Register platform interface (will be coupled with a renderer interface)
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_CreateWindow = ImGui_ImplOSX_CreateWindow;
    platform_io.Platform_DestroyWindow = ImGui_ImplOSX_DestroyWindow;
    platform_io.Platform_ShowWindow = ImGui_ImplOSX_ShowWindow;
    platform_io.Platform_SetWindowPos = ImGui_ImplOSX_SetWindowPos;
    platform_io.Platform_GetWindowPos = ImGui_ImplOSX_GetWindowPos;
    platform_io.Platform_SetWindowSize = ImGui_ImplOSX_SetWindowSize;
    platform_io.Platform_GetWindowSize = ImGui_ImplOSX_GetWindowSize;
    platform_io.Platform_SetWindowFocus = ImGui_ImplOSX_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = ImGui_ImplOSX_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = ImGui_ImplOSX_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = ImGui_ImplOSX_SetWindowTitle;
    platform_io.Platform_SetWindowAlpha = ImGui_ImplOSX_SetWindowAlpha;
    platform_io.Platform_UpdateWindow = ImGui_ImplOSX_UpdateWindow;
    platform_io.Platform_GetWindowDpiScale = ImGui_ImplOSX_GetWindowDpiScale; // FIXME-DPI
    platform_io.Platform_OnChangedViewport = ImGui_ImplOSX_OnChangedViewport; // FIXME-DPI

    // Register main window handle (which is owned by the main application, not by us)
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGuiViewportDataOSX* data = IM_NEW(ImGuiViewportDataOSX)();
    data->Window = g_Window;
    data->WindowOwned = false;
    main_viewport->PlatformUserData = data;
    main_viewport->PlatformHandle = (__bridge void*)g_Window;
}

static void ImGui_ImplOSX_ShutdownPlatformInterface()
{
    
}

