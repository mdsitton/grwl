//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_WAYLAND)

    #include <cerrno>
    #include <climits>
    #include <linux/input.h>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #include <sys/mman.h>
    #include <sys/timerfd.h>
    #include <unistd.h>
    #include <ctime>

    #include "wayland-client-protocol.h"
    #include "wayland-xdg-shell-client-protocol.h"
    #include "wayland-xdg-decoration-client-protocol.h"
    #include "wayland-viewporter-client-protocol.h"
    #include "wayland-relative-pointer-unstable-v1-client-protocol.h"
    #include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
    #include "wayland-idle-inhibit-unstable-v1-client-protocol.h"
    #include "wayland-text-input-unstable-v1-client-protocol.h"
    #include "wayland-text-input-unstable-v3-client-protocol.h"
    #include "wayland-xdg-activation-v1-client-protocol.h"

// NOTE: Versions of wayland-scanner prior to 1.17.91 named every global array of
//       wl_interface pointers 'types', making it impossible to combine several unmodified
//       private-code files into a single compilation unit
// HACK: We override this name with a macro for each file, allowing them to coexist

    #define types _grwl_wayland_types
    #include "wayland-client-protocol-code.h"
    #undef types

    #define types _grwl_xdg_shell_types
    #include "wayland-xdg-shell-client-protocol-code.h"
    #undef types

    #define types _grwl_xdg_decoration_types
    #include "wayland-xdg-decoration-client-protocol-code.h"
    #undef types

    #define types _grwl_viewporter_types
    #include "wayland-viewporter-client-protocol-code.h"
    #undef types

    #define types _grwl_relative_pointer_types
    #include "wayland-relative-pointer-unstable-v1-client-protocol-code.h"
    #undef types

    #define types _grwl_pointer_constraints_types
    #include "wayland-pointer-constraints-unstable-v1-client-protocol-code.h"
    #undef types

    #define types _grwl_idle_inhibit_types
    #include "wayland-idle-inhibit-unstable-v1-client-protocol-code.h"
    #undef types

    #define types _grwl_text_input_v1_types
    #include "wayland-text-input-unstable-v1-client-protocol-code.h"
    #undef types

    #define types _grwl_text_input_v3_types
    #include "wayland-text-input-unstable-v3-client-protocol-code.h"
    #undef types

    #define types _grwl_xdg_activation_types
    #include "wayland-xdg-activation-v1-client-protocol-code.h"
    #undef types

static void wmBaseHandlePing(void* userData, struct xdg_wm_base* wmBase, uint32_t serial)
{
    xdg_wm_base_pong(wmBase, serial);
}

static const struct xdg_wm_base_listener wmBaseListener = { wmBaseHandlePing };

static void registryHandleGlobal(void* userData, struct wl_registry* registry, uint32_t name, const char* interface,
                                 uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        _grwl.wl.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, _grwl_min(3, version));
    }
    else if (strcmp(interface, "wl_subcompositor") == 0)
    {
        _grwl.wl.subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        _grwl.wl.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        _grwlAddOutputWayland(name, version);
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        if (!_grwl.wl.seat)
        {
            _grwl.wl.seat = wl_registry_bind(registry, name, &wl_seat_interface, _grwl_min(4, version));
            _grwlAddSeatListenerWayland(_grwl.wl.seat);
        }
    }
    else if (strcmp(interface, "wl_data_device_manager") == 0)
    {
        if (!_grwl.wl.dataDeviceManager)
        {
            _grwl.wl.dataDeviceManager = wl_registry_bind(registry, name, &wl_data_device_manager_interface, 1);
        }
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        _grwl.wl.wmBase = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(_grwl.wl.wmBase, &wmBaseListener, NULL);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
    {
        _grwl.wl.decorationManager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "wp_viewporter") == 0)
    {
        _grwl.wl.viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
    }
    else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
    {
        _grwl.wl.relativePointerManager =
            wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0)
    {
        _grwl.wl.pointerConstraints = wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
    {
        _grwl.wl.idleInhibitManager = wl_registry_bind(registry, name, &zwp_idle_inhibit_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_text_input_manager_v1") == 0)
    {
        _grwl.wl.textInputManagerV1 = wl_registry_bind(registry, name, &zwp_text_input_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "zwp_text_input_manager_v3") == 0)
    {
        _grwl.wl.textInputManagerV3 = wl_registry_bind(registry, name, &zwp_text_input_manager_v3_interface, 1);
    }
    else if (strcmp(interface, "xdg_activation_v1") == 0)
    {
        _grwl.wl.activationManager = wl_registry_bind(registry, name, &xdg_activation_v1_interface, 1);
    }
}

static void registryHandleGlobalRemove(void* userData, struct wl_registry* registry, uint32_t name)
{
    for (int i = 0; i < _grwl.monitorCount; ++i)
    {
        _GRWLmonitor* monitor = _grwl.monitors[i];
        if (monitor->wl.name == name)
        {
            _grwlInputMonitor(monitor, GRWL_DISCONNECTED, 0);
            return;
        }
    }
}

static const struct wl_registry_listener registryListener = { registryHandleGlobal, registryHandleGlobalRemove };

void libdecorHandleError(struct libdecor* context, enum libdecor_error error, const char* message)
{
    _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: libdecor error %u: %s", error, message);
}

static const struct libdecor_interface libdecorInterface = { libdecorHandleError };

// Create key code translation tables
//
static void createKeyTables()
{
    memset(_grwl.wl.keycodes, -1, sizeof(_grwl.wl.keycodes));
    memset(_grwl.wl.scancodes, -1, sizeof(_grwl.wl.scancodes));

    _grwl.wl.keycodes[KEY_GRAVE] = GRWL_KEY_GRAVE_ACCENT;
    _grwl.wl.keycodes[KEY_1] = GRWL_KEY_1;
    _grwl.wl.keycodes[KEY_2] = GRWL_KEY_2;
    _grwl.wl.keycodes[KEY_3] = GRWL_KEY_3;
    _grwl.wl.keycodes[KEY_4] = GRWL_KEY_4;
    _grwl.wl.keycodes[KEY_5] = GRWL_KEY_5;
    _grwl.wl.keycodes[KEY_6] = GRWL_KEY_6;
    _grwl.wl.keycodes[KEY_7] = GRWL_KEY_7;
    _grwl.wl.keycodes[KEY_8] = GRWL_KEY_8;
    _grwl.wl.keycodes[KEY_9] = GRWL_KEY_9;
    _grwl.wl.keycodes[KEY_0] = GRWL_KEY_0;
    _grwl.wl.keycodes[KEY_SPACE] = GRWL_KEY_SPACE;
    _grwl.wl.keycodes[KEY_MINUS] = GRWL_KEY_MINUS;
    _grwl.wl.keycodes[KEY_EQUAL] = GRWL_KEY_EQUAL;
    _grwl.wl.keycodes[KEY_Q] = GRWL_KEY_Q;
    _grwl.wl.keycodes[KEY_W] = GRWL_KEY_W;
    _grwl.wl.keycodes[KEY_E] = GRWL_KEY_E;
    _grwl.wl.keycodes[KEY_R] = GRWL_KEY_R;
    _grwl.wl.keycodes[KEY_T] = GRWL_KEY_T;
    _grwl.wl.keycodes[KEY_Y] = GRWL_KEY_Y;
    _grwl.wl.keycodes[KEY_U] = GRWL_KEY_U;
    _grwl.wl.keycodes[KEY_I] = GRWL_KEY_I;
    _grwl.wl.keycodes[KEY_O] = GRWL_KEY_O;
    _grwl.wl.keycodes[KEY_P] = GRWL_KEY_P;
    _grwl.wl.keycodes[KEY_LEFTBRACE] = GRWL_KEY_LEFT_BRACKET;
    _grwl.wl.keycodes[KEY_RIGHTBRACE] = GRWL_KEY_RIGHT_BRACKET;
    _grwl.wl.keycodes[KEY_A] = GRWL_KEY_A;
    _grwl.wl.keycodes[KEY_S] = GRWL_KEY_S;
    _grwl.wl.keycodes[KEY_D] = GRWL_KEY_D;
    _grwl.wl.keycodes[KEY_F] = GRWL_KEY_F;
    _grwl.wl.keycodes[KEY_G] = GRWL_KEY_G;
    _grwl.wl.keycodes[KEY_H] = GRWL_KEY_H;
    _grwl.wl.keycodes[KEY_J] = GRWL_KEY_J;
    _grwl.wl.keycodes[KEY_K] = GRWL_KEY_K;
    _grwl.wl.keycodes[KEY_L] = GRWL_KEY_L;
    _grwl.wl.keycodes[KEY_SEMICOLON] = GRWL_KEY_SEMICOLON;
    _grwl.wl.keycodes[KEY_APOSTROPHE] = GRWL_KEY_APOSTROPHE;
    _grwl.wl.keycodes[KEY_Z] = GRWL_KEY_Z;
    _grwl.wl.keycodes[KEY_X] = GRWL_KEY_X;
    _grwl.wl.keycodes[KEY_C] = GRWL_KEY_C;
    _grwl.wl.keycodes[KEY_V] = GRWL_KEY_V;
    _grwl.wl.keycodes[KEY_B] = GRWL_KEY_B;
    _grwl.wl.keycodes[KEY_N] = GRWL_KEY_N;
    _grwl.wl.keycodes[KEY_M] = GRWL_KEY_M;
    _grwl.wl.keycodes[KEY_COMMA] = GRWL_KEY_COMMA;
    _grwl.wl.keycodes[KEY_DOT] = GRWL_KEY_PERIOD;
    _grwl.wl.keycodes[KEY_SLASH] = GRWL_KEY_SLASH;
    _grwl.wl.keycodes[KEY_BACKSLASH] = GRWL_KEY_BACKSLASH;
    _grwl.wl.keycodes[KEY_ESC] = GRWL_KEY_ESCAPE;
    _grwl.wl.keycodes[KEY_TAB] = GRWL_KEY_TAB;
    _grwl.wl.keycodes[KEY_LEFTSHIFT] = GRWL_KEY_LEFT_SHIFT;
    _grwl.wl.keycodes[KEY_RIGHTSHIFT] = GRWL_KEY_RIGHT_SHIFT;
    _grwl.wl.keycodes[KEY_LEFTCTRL] = GRWL_KEY_LEFT_CONTROL;
    _grwl.wl.keycodes[KEY_RIGHTCTRL] = GRWL_KEY_RIGHT_CONTROL;
    _grwl.wl.keycodes[KEY_LEFTALT] = GRWL_KEY_LEFT_ALT;
    _grwl.wl.keycodes[KEY_RIGHTALT] = GRWL_KEY_RIGHT_ALT;
    _grwl.wl.keycodes[KEY_LEFTMETA] = GRWL_KEY_LEFT_SUPER;
    _grwl.wl.keycodes[KEY_RIGHTMETA] = GRWL_KEY_RIGHT_SUPER;
    _grwl.wl.keycodes[KEY_COMPOSE] = GRWL_KEY_MENU;
    _grwl.wl.keycodes[KEY_NUMLOCK] = GRWL_KEY_NUM_LOCK;
    _grwl.wl.keycodes[KEY_CAPSLOCK] = GRWL_KEY_CAPS_LOCK;
    _grwl.wl.keycodes[KEY_PRINT] = GRWL_KEY_PRINT_SCREEN;
    _grwl.wl.keycodes[KEY_SCROLLLOCK] = GRWL_KEY_SCROLL_LOCK;
    _grwl.wl.keycodes[KEY_PAUSE] = GRWL_KEY_PAUSE;
    _grwl.wl.keycodes[KEY_DELETE] = GRWL_KEY_DELETE;
    _grwl.wl.keycodes[KEY_BACKSPACE] = GRWL_KEY_BACKSPACE;
    _grwl.wl.keycodes[KEY_ENTER] = GRWL_KEY_ENTER;
    _grwl.wl.keycodes[KEY_HOME] = GRWL_KEY_HOME;
    _grwl.wl.keycodes[KEY_END] = GRWL_KEY_END;
    _grwl.wl.keycodes[KEY_PAGEUP] = GRWL_KEY_PAGE_UP;
    _grwl.wl.keycodes[KEY_PAGEDOWN] = GRWL_KEY_PAGE_DOWN;
    _grwl.wl.keycodes[KEY_INSERT] = GRWL_KEY_INSERT;
    _grwl.wl.keycodes[KEY_LEFT] = GRWL_KEY_LEFT;
    _grwl.wl.keycodes[KEY_RIGHT] = GRWL_KEY_RIGHT;
    _grwl.wl.keycodes[KEY_DOWN] = GRWL_KEY_DOWN;
    _grwl.wl.keycodes[KEY_UP] = GRWL_KEY_UP;
    _grwl.wl.keycodes[KEY_F1] = GRWL_KEY_F1;
    _grwl.wl.keycodes[KEY_F2] = GRWL_KEY_F2;
    _grwl.wl.keycodes[KEY_F3] = GRWL_KEY_F3;
    _grwl.wl.keycodes[KEY_F4] = GRWL_KEY_F4;
    _grwl.wl.keycodes[KEY_F5] = GRWL_KEY_F5;
    _grwl.wl.keycodes[KEY_F6] = GRWL_KEY_F6;
    _grwl.wl.keycodes[KEY_F7] = GRWL_KEY_F7;
    _grwl.wl.keycodes[KEY_F8] = GRWL_KEY_F8;
    _grwl.wl.keycodes[KEY_F9] = GRWL_KEY_F9;
    _grwl.wl.keycodes[KEY_F10] = GRWL_KEY_F10;
    _grwl.wl.keycodes[KEY_F11] = GRWL_KEY_F11;
    _grwl.wl.keycodes[KEY_F12] = GRWL_KEY_F12;
    _grwl.wl.keycodes[KEY_F13] = GRWL_KEY_F13;
    _grwl.wl.keycodes[KEY_F14] = GRWL_KEY_F14;
    _grwl.wl.keycodes[KEY_F15] = GRWL_KEY_F15;
    _grwl.wl.keycodes[KEY_F16] = GRWL_KEY_F16;
    _grwl.wl.keycodes[KEY_F17] = GRWL_KEY_F17;
    _grwl.wl.keycodes[KEY_F18] = GRWL_KEY_F18;
    _grwl.wl.keycodes[KEY_F19] = GRWL_KEY_F19;
    _grwl.wl.keycodes[KEY_F20] = GRWL_KEY_F20;
    _grwl.wl.keycodes[KEY_F21] = GRWL_KEY_F21;
    _grwl.wl.keycodes[KEY_F22] = GRWL_KEY_F22;
    _grwl.wl.keycodes[KEY_F23] = GRWL_KEY_F23;
    _grwl.wl.keycodes[KEY_F24] = GRWL_KEY_F24;
    _grwl.wl.keycodes[KEY_KPSLASH] = GRWL_KEY_KP_DIVIDE;
    _grwl.wl.keycodes[KEY_KPASTERISK] = GRWL_KEY_KP_MULTIPLY;
    _grwl.wl.keycodes[KEY_KPMINUS] = GRWL_KEY_KP_SUBTRACT;
    _grwl.wl.keycodes[KEY_KPPLUS] = GRWL_KEY_KP_ADD;
    _grwl.wl.keycodes[KEY_KP0] = GRWL_KEY_KP_0;
    _grwl.wl.keycodes[KEY_KP1] = GRWL_KEY_KP_1;
    _grwl.wl.keycodes[KEY_KP2] = GRWL_KEY_KP_2;
    _grwl.wl.keycodes[KEY_KP3] = GRWL_KEY_KP_3;
    _grwl.wl.keycodes[KEY_KP4] = GRWL_KEY_KP_4;
    _grwl.wl.keycodes[KEY_KP5] = GRWL_KEY_KP_5;
    _grwl.wl.keycodes[KEY_KP6] = GRWL_KEY_KP_6;
    _grwl.wl.keycodes[KEY_KP7] = GRWL_KEY_KP_7;
    _grwl.wl.keycodes[KEY_KP8] = GRWL_KEY_KP_8;
    _grwl.wl.keycodes[KEY_KP9] = GRWL_KEY_KP_9;
    _grwl.wl.keycodes[KEY_KPDOT] = GRWL_KEY_KP_DECIMAL;
    _grwl.wl.keycodes[KEY_KPEQUAL] = GRWL_KEY_KP_EQUAL;
    _grwl.wl.keycodes[KEY_KPENTER] = GRWL_KEY_KP_ENTER;
    _grwl.wl.keycodes[KEY_102ND] = GRWL_KEY_WORLD_2;

    for (int scancode = 0; scancode < 256; scancode++)
    {
        if (_grwl.wl.keycodes[scancode] > 0)
        {
            _grwl.wl.scancodes[_grwl.wl.keycodes[scancode]] = scancode;
        }
    }
}

static bool loadCursorTheme()
{
    int cursorSize = 16;

    const char* sizeString = getenv("XCURSOR_SIZE");
    if (sizeString)
    {
        errno = 0;
        const long cursorSizeLong = strtol(sizeString, NULL, 10);
        if (errno == 0 && cursorSizeLong > 0 && cursorSizeLong < INT_MAX)
        {
            cursorSize = (int)cursorSizeLong;
        }
    }

    const char* themeName = getenv("XCURSOR_THEME");

    _grwl.wl.cursorTheme = wl_cursor_theme_load(themeName, cursorSize, _grwl.wl.shm);
    if (!_grwl.wl.cursorTheme)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load default cursor theme");
        return false;
    }

    // If this happens to be NULL, we just fallback to the scale=1 version.
    _grwl.wl.cursorThemeHiDPI = wl_cursor_theme_load(themeName, cursorSize * 2, _grwl.wl.shm);

    _grwl.wl.cursorSurface = wl_compositor_create_surface(_grwl.wl.compositor);
    _grwl.wl.cursorTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlConnectWayland(int platformID, _GRWLplatform* platform)
{
    const _GRWLplatform wayland = {
        GRWL_PLATFORM_WAYLAND,
        _grwlInitWayland,
        _grwlTerminateWayland,
        _grwlGetCursorPosWayland,
        _grwlSetCursorPosWayland,
        _grwlSetCursorModeWayland,
        _grwlSetRawMouseMotionWayland,
        _grwlRawMouseMotionSupportedWayland,
        _grwlCreateCursorWayland,
        _grwlCreateStandardCursorWayland,
        _grwlDestroyCursorWayland,
        _grwlSetCursorWayland,
        _grwlGetScancodeNameWayland,
        _grwlGetKeyScancodeWayland,
        _grwlGetKeyboardLayoutNameWayland,
        _grwlSetClipboardStringWayland,
        _grwlGetClipboardStringWayland,
        _grwlUpdatePreeditCursorRectangleWayland,
        _grwlResetPreeditTextWayland,
        _grwlSetIMEStatusWayland,
        _grwlGetIMEStatusWayland,
    #if defined(GRWL_BUILD_LINUX_JOYSTICK)
        _grwlInitJoysticksLinux,
        _grwlTerminateJoysticksLinux,
        _grwlPollJoystickLinux,
        _grwlGetMappingNameLinux,
        _grwlUpdateGamepadGUIDLinux,
    #else
        _grwlInitJoysticksNull,
        _grwlTerminateJoysticksNull,
        _grwlPollJoystickNull,
        _grwlGetMappingNameNull,
        _grwlUpdateGamepadGUIDNull,
    #endif
        _grwlFreeMonitorWayland,
        _grwlGetMonitorPosWayland,
        _grwlGetMonitorContentScaleWayland,
        _grwlGetMonitorWorkareaWayland,
        _grwlGetVideoModesWayland,
        _grwlGetVideoModeWayland,
        _grwlGetGammaRampWayland,
        _grwlSetGammaRampWayland,
        _grwlCreateWindowWayland,
        _grwlDestroyWindowWayland,
        _grwlSetWindowTitleWayland,
        _grwlSetWindowIconWayland,
        _grwlSetWindowProgressIndicatorWayland,
        _grwlSetWindowBadgeWayland,
        _grwlSetWindowBadgeStringWayland,
        _grwlGetWindowPosWayland,
        _grwlSetWindowPosWayland,
        _grwlGetWindowSizeWayland,
        _grwlSetWindowSizeWayland,
        _grwlSetWindowSizeLimitsWayland,
        _grwlSetWindowAspectRatioWayland,
        _grwlGetFramebufferSizeWayland,
        _grwlGetWindowFrameSizeWayland,
        _grwlGetWindowContentScaleWayland,
        _grwlIconifyWindowWayland,
        _grwlRestoreWindowWayland,
        _grwlMaximizeWindowWayland,
        _grwlShowWindowWayland,
        _grwlHideWindowWayland,
        _grwlRequestWindowAttentionWayland,
        _grwlFocusWindowWayland,
        _grwlSetWindowMonitorWayland,
        _grwlWindowFocusedWayland,
        _grwlWindowIconifiedWayland,
        _grwlWindowVisibleWayland,
        _grwlWindowMaximizedWayland,
        _grwlWindowHoveredWayland,
        _grwlFramebufferTransparentWayland,
        _grwlGetWindowOpacityWayland,
        _grwlSetWindowResizableWayland,
        _grwlSetWindowDecoratedWayland,
        _grwlSetWindowFloatingWayland,
        _grwlSetWindowOpacityWayland,
        _grwlSetWindowMousePassthroughWayland,
        _grwlPollEventsWayland,
        _grwlWaitEventsWayland,
        _grwlWaitEventsTimeoutWayland,
        _grwlPostEmptyEventWayland,
        _grwlCreateUserContextWayland,
        _grwlGetEGLPlatformWayland,
        _grwlGetEGLNativeDisplayWayland,
        _grwlGetEGLNativeWindowWayland,
        _grwlGetRequiredInstanceExtensionsWayland,
        _grwlGetPhysicalDevicePresentationSupportWayland,
        _grwlCreateWindowSurfaceWayland,
    };

    void* module = _grwlPlatformLoadModule("libwayland-client.so.0");
    if (!module)
    {
        if (platformID == GRWL_PLATFORM_WAYLAND)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load libwayland-client");
        }

        return false;
    }

    PFN_wl_display_connect wl_display_connect =
        (PFN_wl_display_connect)_grwlPlatformGetModuleSymbol(module, "wl_display_connect");
    if (!wl_display_connect)
    {
        if (platformID == GRWL_PLATFORM_WAYLAND)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load libwayland-client entry point");
        }

        _grwlPlatformFreeModule(module);
        return false;
    }

    struct wl_display* display = wl_display_connect(NULL);
    if (!display)
    {
        if (platformID == GRWL_PLATFORM_WAYLAND)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to connect to display");
        }

        _grwlPlatformFreeModule(module);
        return false;
    }

    _grwl.wl.display = display;
    _grwl.wl.client.handle = module;

    *platform = wayland;
    return true;
}

int _grwlInitWayland()
{
    _grwlInitDBusPOSIX();

    // These must be set before any failure checks
    _grwl.wl.keyRepeatTimerfd = -1;
    _grwl.wl.cursorTimerfd = -1;

    _grwl.wl.tag = grwlGetVersionString();

    _grwl.wl.client.display_flush =
        (PFN_wl_display_flush)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_flush");
    _grwl.wl.client.display_cancel_read =
        (PFN_wl_display_cancel_read)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_cancel_read");
    _grwl.wl.client.display_dispatch_pending = (PFN_wl_display_dispatch_pending)_grwlPlatformGetModuleSymbol(
        _grwl.wl.client.handle, "wl_display_dispatch_pending");
    _grwl.wl.client.display_read_events =
        (PFN_wl_display_read_events)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_read_events");
    _grwl.wl.client.display_disconnect =
        (PFN_wl_display_disconnect)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_disconnect");
    _grwl.wl.client.display_roundtrip =
        (PFN_wl_display_roundtrip)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_roundtrip");
    _grwl.wl.client.display_get_fd =
        (PFN_wl_display_get_fd)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_get_fd");
    _grwl.wl.client.display_prepare_read =
        (PFN_wl_display_prepare_read)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_display_prepare_read");
    _grwl.wl.client.proxy_marshal =
        (PFN_wl_proxy_marshal)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_marshal");
    _grwl.wl.client.proxy_add_listener =
        (PFN_wl_proxy_add_listener)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_add_listener");
    _grwl.wl.client.proxy_destroy =
        (PFN_wl_proxy_destroy)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_destroy");
    _grwl.wl.client.proxy_marshal_constructor = (PFN_wl_proxy_marshal_constructor)_grwlPlatformGetModuleSymbol(
        _grwl.wl.client.handle, "wl_proxy_marshal_constructor");
    _grwl.wl.client.proxy_marshal_constructor_versioned =
        (PFN_wl_proxy_marshal_constructor_versioned)_grwlPlatformGetModuleSymbol(
            _grwl.wl.client.handle, "wl_proxy_marshal_constructor_versioned");
    _grwl.wl.client.proxy_get_user_data =
        (PFN_wl_proxy_get_user_data)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_get_user_data");
    _grwl.wl.client.proxy_set_user_data =
        (PFN_wl_proxy_set_user_data)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_set_user_data");
    _grwl.wl.client.proxy_get_tag =
        (PFN_wl_proxy_get_tag)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_get_tag");
    _grwl.wl.client.proxy_set_tag =
        (PFN_wl_proxy_set_tag)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_set_tag");
    _grwl.wl.client.proxy_get_version =
        (PFN_wl_proxy_get_version)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_get_version");
    _grwl.wl.client.proxy_marshal_flags =
        (PFN_wl_proxy_marshal_flags)_grwlPlatformGetModuleSymbol(_grwl.wl.client.handle, "wl_proxy_marshal_flags");

    if (!_grwl.wl.client.display_flush || !_grwl.wl.client.display_cancel_read ||
        !_grwl.wl.client.display_dispatch_pending || !_grwl.wl.client.display_read_events ||
        !_grwl.wl.client.display_disconnect || !_grwl.wl.client.display_roundtrip || !_grwl.wl.client.display_get_fd ||
        !_grwl.wl.client.display_prepare_read || !_grwl.wl.client.proxy_marshal ||
        !_grwl.wl.client.proxy_add_listener || !_grwl.wl.client.proxy_destroy ||
        !_grwl.wl.client.proxy_marshal_constructor || !_grwl.wl.client.proxy_marshal_constructor_versioned ||
        !_grwl.wl.client.proxy_get_user_data || !_grwl.wl.client.proxy_set_user_data ||
        !_grwl.wl.client.proxy_get_tag || !_grwl.wl.client.proxy_set_tag)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load libwayland-client entry point");
        return false;
    }

    _grwl.wl.cursor.handle = _grwlPlatformLoadModule("libwayland-cursor.so.0");
    if (!_grwl.wl.cursor.handle)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load libwayland-cursor");
        return false;
    }

    _grwl.wl.cursor.theme_load =
        (PFN_wl_cursor_theme_load)_grwlPlatformGetModuleSymbol(_grwl.wl.cursor.handle, "wl_cursor_theme_load");
    _grwl.wl.cursor.theme_destroy =
        (PFN_wl_cursor_theme_destroy)_grwlPlatformGetModuleSymbol(_grwl.wl.cursor.handle, "wl_cursor_theme_destroy");
    _grwl.wl.cursor.theme_get_cursor = (PFN_wl_cursor_theme_get_cursor)_grwlPlatformGetModuleSymbol(
        _grwl.wl.cursor.handle, "wl_cursor_theme_get_cursor");
    _grwl.wl.cursor.image_get_buffer = (PFN_wl_cursor_image_get_buffer)_grwlPlatformGetModuleSymbol(
        _grwl.wl.cursor.handle, "wl_cursor_image_get_buffer");

    _grwl.wl.egl.handle = _grwlPlatformLoadModule("libwayland-egl.so.1");
    if (!_grwl.wl.egl.handle)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load libwayland-egl");
        return false;
    }

    _grwl.wl.egl.window_create =
        (PFN_wl_egl_window_create)_grwlPlatformGetModuleSymbol(_grwl.wl.egl.handle, "wl_egl_window_create");
    _grwl.wl.egl.window_destroy =
        (PFN_wl_egl_window_destroy)_grwlPlatformGetModuleSymbol(_grwl.wl.egl.handle, "wl_egl_window_destroy");
    _grwl.wl.egl.window_resize =
        (PFN_wl_egl_window_resize)_grwlPlatformGetModuleSymbol(_grwl.wl.egl.handle, "wl_egl_window_resize");

    _grwl.wl.xkb.handle = _grwlPlatformLoadModule("libxkbcommon.so.0");
    if (!_grwl.wl.xkb.handle)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to load libxkbcommon");
        return false;
    }

    _grwl.wl.xkb.context_new =
        (PFN_xkb_context_new)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_context_new");
    _grwl.wl.xkb.context_unref =
        (PFN_xkb_context_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_context_unref");
    _grwl.wl.xkb.keymap_new_from_string =
        (PFN_xkb_keymap_new_from_string)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_keymap_new_from_string");
    _grwl.wl.xkb.keymap_unref =
        (PFN_xkb_keymap_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_keymap_unref");
    _grwl.wl.xkb.keymap_mod_get_index =
        (PFN_xkb_keymap_mod_get_index)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_keymap_mod_get_index");
    _grwl.wl.xkb.keymap_key_repeats =
        (PFN_xkb_keymap_key_repeats)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_keymap_key_repeats");
    _grwl.wl.xkb.keymap_key_get_syms_by_level = (PFN_xkb_keymap_key_get_syms_by_level)_grwlPlatformGetModuleSymbol(
        _grwl.wl.xkb.handle, "xkb_keymap_key_get_syms_by_level");
    _grwl.wl.xkb.state_new = (PFN_xkb_state_new)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_state_new");
    _grwl.wl.xkb.state_unref =
        (PFN_xkb_state_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_state_unref");
    _grwl.wl.xkb.state_key_get_syms =
        (PFN_xkb_state_key_get_syms)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_state_key_get_syms");
    _grwl.wl.xkb.state_update_mask =
        (PFN_xkb_state_update_mask)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_state_update_mask");
    _grwl.wl.xkb.state_key_get_layout =
        (PFN_xkb_state_key_get_layout)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_state_key_get_layout");
    _grwl.wl.xkb.state_mod_index_is_active = (PFN_xkb_state_mod_index_is_active)_grwlPlatformGetModuleSymbol(
        _grwl.wl.xkb.handle, "xkb_state_mod_index_is_active");
    _grwl.wl.xkb.state_serialize_mods =
        (PFN_xkb_state_serialize_mods)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_state_serialize_mods");
    _grwl.wl.xkb.keymap_layout_get_name =
        (PFN_xkb_keymap_layout_get_name)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_keymap_layout_get_name");
    _grwl.wl.xkb.compose_table_new_from_locale = (PFN_xkb_compose_table_new_from_locale)_grwlPlatformGetModuleSymbol(
        _grwl.wl.xkb.handle, "xkb_compose_table_new_from_locale");
    _grwl.wl.xkb.compose_table_unref =
        (PFN_xkb_compose_table_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_compose_table_unref");
    _grwl.wl.xkb.compose_state_new =
        (PFN_xkb_compose_state_new)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_compose_state_new");
    _grwl.wl.xkb.compose_state_unref =
        (PFN_xkb_compose_state_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_compose_state_unref");
    _grwl.wl.xkb.compose_state_feed =
        (PFN_xkb_compose_state_feed)_grwlPlatformGetModuleSymbol(_grwl.wl.xkb.handle, "xkb_compose_state_feed");
    _grwl.wl.xkb.compose_state_get_status = (PFN_xkb_compose_state_get_status)_grwlPlatformGetModuleSymbol(
        _grwl.wl.xkb.handle, "xkb_compose_state_get_status");
    _grwl.wl.xkb.compose_state_get_one_sym = (PFN_xkb_compose_state_get_one_sym)_grwlPlatformGetModuleSymbol(
        _grwl.wl.xkb.handle, "xkb_compose_state_get_one_sym");

    if (_grwl.hints.init.wl.libdecorMode == GRWL_WAYLAND_PREFER_LIBDECOR)
    {
        _grwl.wl.libdecor.handle = _grwlPlatformLoadModule("libdecor-0.so.0");
    }

    if (_grwl.wl.libdecor.handle)
    {
        _grwl.wl.libdecor.libdecor_new_ =
            (PFN_libdecor_new)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_new");
        _grwl.wl.libdecor.libdecor_unref_ =
            (PFN_libdecor_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_unref");
        _grwl.wl.libdecor.libdecor_get_fd_ =
            (PFN_libdecor_get_fd)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_get_fd");
        _grwl.wl.libdecor.libdecor_dispatch_ =
            (PFN_libdecor_dispatch)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_dispatch");
        _grwl.wl.libdecor.libdecor_decorate_ =
            (PFN_libdecor_decorate)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_decorate");
        _grwl.wl.libdecor.libdecor_frame_unref_ =
            (PFN_libdecor_frame_unref)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_frame_unref");
        _grwl.wl.libdecor.libdecor_frame_set_app_id_ = (PFN_libdecor_frame_set_app_id)_grwlPlatformGetModuleSymbol(
            _grwl.wl.libdecor.handle, "libdecor_frame_set_app_id");
        _grwl.wl.libdecor.libdecor_frame_set_title_ = (PFN_libdecor_frame_set_title)_grwlPlatformGetModuleSymbol(
            _grwl.wl.libdecor.handle, "libdecor_frame_set_title");
        _grwl.wl.libdecor.libdecor_frame_set_minimized_ =
            (PFN_libdecor_frame_set_minimized)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                           "libdecor_frame_set_minimized");
        _grwl.wl.libdecor.libdecor_frame_set_fullscreen_ =
            (PFN_libdecor_frame_set_fullscreen)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                            "libdecor_frame_set_fullscreen");
        _grwl.wl.libdecor.libdecor_frame_unset_fullscreen_ =
            (PFN_libdecor_frame_unset_fullscreen)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                              "libdecor_frame_unset_fullscreen");
        _grwl.wl.libdecor.libdecor_frame_map_ =
            (PFN_libdecor_frame_map)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_frame_map");
        _grwl.wl.libdecor.libdecor_frame_commit_ =
            (PFN_libdecor_frame_commit)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_frame_commit");
        _grwl.wl.libdecor.libdecor_frame_set_min_content_size_ =
            (PFN_libdecor_frame_set_min_content_size)_grwlPlatformGetModuleSymbol(
                _grwl.wl.libdecor.handle, "libdecor_frame_set_min_content_size");
        _grwl.wl.libdecor.libdecor_frame_set_max_content_size_ =
            (PFN_libdecor_frame_set_max_content_size)_grwlPlatformGetModuleSymbol(
                _grwl.wl.libdecor.handle, "libdecor_frame_set_max_content_size");
        _grwl.wl.libdecor.libdecor_frame_set_maximized_ =
            (PFN_libdecor_frame_set_maximized)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                           "libdecor_frame_set_maximized");
        _grwl.wl.libdecor.libdecor_frame_unset_maximized_ =
            (PFN_libdecor_frame_unset_maximized)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                             "libdecor_frame_unset_maximized");
        _grwl.wl.libdecor.libdecor_frame_set_capabilities_ =
            (PFN_libdecor_frame_set_capabilities)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                              "libdecor_frame_set_capabilities");
        _grwl.wl.libdecor.libdecor_frame_unset_capabilities_ =
            (PFN_libdecor_frame_unset_capabilities)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                                "libdecor_frame_unset_capabilities");
        _grwl.wl.libdecor.libdecor_frame_set_visibility_ =
            (PFN_libdecor_frame_set_visibility)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                            "libdecor_frame_set_visibility");
        _grwl.wl.libdecor.libdecor_frame_get_xdg_toplevel_ =
            (PFN_libdecor_frame_get_xdg_toplevel)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle,
                                                                              "libdecor_frame_get_xdg_toplevel");
        _grwl.wl.libdecor.libdecor_configuration_get_content_size_ =
            (PFN_libdecor_configuration_get_content_size)_grwlPlatformGetModuleSymbol(
                _grwl.wl.libdecor.handle, "libdecor_configuration_get_content_size");
        _grwl.wl.libdecor.libdecor_configuration_get_window_state_ =
            (PFN_libdecor_configuration_get_window_state)_grwlPlatformGetModuleSymbol(
                _grwl.wl.libdecor.handle, "libdecor_configuration_get_window_state");
        _grwl.wl.libdecor.libdecor_state_new_ =
            (PFN_libdecor_state_new)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_state_new");
        _grwl.wl.libdecor.libdecor_state_free_ =
            (PFN_libdecor_state_free)_grwlPlatformGetModuleSymbol(_grwl.wl.libdecor.handle, "libdecor_state_free");

        if (!_grwl.wl.libdecor.libdecor_new_ || !_grwl.wl.libdecor.libdecor_unref_ ||
            !_grwl.wl.libdecor.libdecor_get_fd_ || !_grwl.wl.libdecor.libdecor_dispatch_ ||
            !_grwl.wl.libdecor.libdecor_decorate_ || !_grwl.wl.libdecor.libdecor_frame_unref_ ||
            !_grwl.wl.libdecor.libdecor_frame_set_app_id_ || !_grwl.wl.libdecor.libdecor_frame_set_title_ ||
            !_grwl.wl.libdecor.libdecor_frame_set_minimized_ || !_grwl.wl.libdecor.libdecor_frame_set_fullscreen_ ||
            !_grwl.wl.libdecor.libdecor_frame_unset_fullscreen_ || !_grwl.wl.libdecor.libdecor_frame_map_ ||
            !_grwl.wl.libdecor.libdecor_frame_commit_ || !_grwl.wl.libdecor.libdecor_frame_set_min_content_size_ ||
            !_grwl.wl.libdecor.libdecor_frame_set_max_content_size_ ||
            !_grwl.wl.libdecor.libdecor_frame_set_maximized_ || !_grwl.wl.libdecor.libdecor_frame_unset_maximized_ ||
            !_grwl.wl.libdecor.libdecor_frame_set_capabilities_ ||
            !_grwl.wl.libdecor.libdecor_frame_unset_capabilities_ ||
            !_grwl.wl.libdecor.libdecor_frame_set_visibility_ || !_grwl.wl.libdecor.libdecor_frame_get_xdg_toplevel_ ||
            !_grwl.wl.libdecor.libdecor_configuration_get_content_size_ ||
            !_grwl.wl.libdecor.libdecor_configuration_get_window_state_ || !_grwl.wl.libdecor.libdecor_state_new_ ||
            !_grwl.wl.libdecor.libdecor_state_free_)
        {
            _grwlPlatformFreeModule(_grwl.wl.libdecor.handle);
            memset(&_grwl.wl.libdecor, 0, sizeof(_grwl.wl.libdecor));
        }
    }

    _grwl.wl.registry = wl_display_get_registry(_grwl.wl.display);
    wl_registry_add_listener(_grwl.wl.registry, &registryListener, NULL);

    createKeyTables();

    _grwl.wl.xkb.context = xkb_context_new(0);
    if (!_grwl.wl.xkb.context)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to initialize xkb context");
        return false;
    }

    // Sync so we got all registry objects
    wl_display_roundtrip(_grwl.wl.display);

    // Sync so we got all initial output events
    wl_display_roundtrip(_grwl.wl.display);

    if (_grwl.wl.libdecor.handle)
    {
        _grwl.wl.libdecor.context = libdecor_new(_grwl.wl.display, &libdecorInterface);

        // Allow libdecor to receive its globals before proceeding
        if (_grwl.wl.libdecor.context)
        {
            libdecor_dispatch(_grwl.wl.libdecor.context, 1);
        }
    }

    #ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    if (wl_seat_get_version(_grwl.wl.seat) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
    {
        _grwl.wl.keyRepeatTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    }
    #endif

    if (!_grwl.wl.wmBase)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to find xdg-shell in your compositor");
        return false;
    }

    if (!_grwl.wl.shm)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to find wl_shm in your compositor");
        return false;
    }

    if (!loadCursorTheme())
    {
        return false;
    }

    if (_grwl.wl.seat && _grwl.wl.dataDeviceManager)
    {
        _grwl.wl.dataDevice = wl_data_device_manager_get_data_device(_grwl.wl.dataDeviceManager, _grwl.wl.seat);
        _grwlAddDataDeviceListenerWayland(_grwl.wl.dataDevice);
    }

    return true;
}

void _grwlTerminateWayland()
{
    _grwlTerminateEGL();
    _grwlTerminateOSMesa();

    if (_grwl.wl.libdecor.context)
    {
        libdecor_unref(_grwl.wl.libdecor.context);
    }

    if (_grwl.wl.libdecor.handle)
    {
        _grwlPlatformFreeModule(_grwl.wl.libdecor.handle);
        _grwl.wl.libdecor.handle = NULL;
    }

    if (_grwl.wl.egl.handle)
    {
        _grwlPlatformFreeModule(_grwl.wl.egl.handle);
        _grwl.wl.egl.handle = NULL;
    }

    if (_grwl.wl.xkb.composeState)
    {
        xkb_compose_state_unref(_grwl.wl.xkb.composeState);
    }
    if (_grwl.wl.xkb.keymap)
    {
        xkb_keymap_unref(_grwl.wl.xkb.keymap);
    }
    if (_grwl.wl.xkb.state)
    {
        xkb_state_unref(_grwl.wl.xkb.state);
    }
    if (_grwl.wl.xkb.context)
    {
        xkb_context_unref(_grwl.wl.xkb.context);
    }
    if (_grwl.wl.xkb.handle)
    {
        _grwlPlatformFreeModule(_grwl.wl.xkb.handle);
        _grwl.wl.xkb.handle = NULL;
    }

    if (_grwl.wl.cursorTheme)
    {
        wl_cursor_theme_destroy(_grwl.wl.cursorTheme);
    }
    if (_grwl.wl.cursorThemeHiDPI)
    {
        wl_cursor_theme_destroy(_grwl.wl.cursorThemeHiDPI);
    }
    if (_grwl.wl.cursor.handle)
    {
        _grwlPlatformFreeModule(_grwl.wl.cursor.handle);
        _grwl.wl.cursor.handle = NULL;
    }

    for (unsigned int i = 0; i < _grwl.wl.offerCount; i++)
    {
        wl_data_offer_destroy(_grwl.wl.offers[i].offer);
    }

    _grwl_free(_grwl.wl.offers);

    if (_grwl.wl.cursorSurface)
    {
        wl_surface_destroy(_grwl.wl.cursorSurface);
    }
    if (_grwl.wl.subcompositor)
    {
        wl_subcompositor_destroy(_grwl.wl.subcompositor);
    }
    if (_grwl.wl.compositor)
    {
        wl_compositor_destroy(_grwl.wl.compositor);
    }
    if (_grwl.wl.shm)
    {
        wl_shm_destroy(_grwl.wl.shm);
    }
    if (_grwl.wl.viewporter)
    {
        wp_viewporter_destroy(_grwl.wl.viewporter);
    }
    if (_grwl.wl.decorationManager)
    {
        zxdg_decoration_manager_v1_destroy(_grwl.wl.decorationManager);
    }
    if (_grwl.wl.wmBase)
    {
        xdg_wm_base_destroy(_grwl.wl.wmBase);
    }
    if (_grwl.wl.selectionOffer)
    {
        wl_data_offer_destroy(_grwl.wl.selectionOffer);
    }
    if (_grwl.wl.dragOffer)
    {
        wl_data_offer_destroy(_grwl.wl.dragOffer);
    }
    if (_grwl.wl.selectionSource)
    {
        wl_data_source_destroy(_grwl.wl.selectionSource);
    }
    if (_grwl.wl.dataDevice)
    {
        wl_data_device_destroy(_grwl.wl.dataDevice);
    }
    if (_grwl.wl.dataDeviceManager)
    {
        wl_data_device_manager_destroy(_grwl.wl.dataDeviceManager);
    }
    if (_grwl.wl.pointer)
    {
        wl_pointer_destroy(_grwl.wl.pointer);
    }
    if (_grwl.wl.keyboard)
    {
        wl_keyboard_destroy(_grwl.wl.keyboard);
    }
    if (_grwl.wl.seat)
    {
        wl_seat_destroy(_grwl.wl.seat);
    }
    if (_grwl.wl.relativePointerManager)
    {
        zwp_relative_pointer_manager_v1_destroy(_grwl.wl.relativePointerManager);
    }
    if (_grwl.wl.pointerConstraints)
    {
        zwp_pointer_constraints_v1_destroy(_grwl.wl.pointerConstraints);
    }
    if (_grwl.wl.idleInhibitManager)
    {
        zwp_idle_inhibit_manager_v1_destroy(_grwl.wl.idleInhibitManager);
    }
    if (_grwl.wl.textInputManagerV1)
    {
        zwp_text_input_manager_v1_destroy(_grwl.wl.textInputManagerV1);
    }
    if (_grwl.wl.textInputManagerV3)
    {
        zwp_text_input_manager_v3_destroy(_grwl.wl.textInputManagerV3);
    }
    if (_grwl.wl.activationManager)
    {
        xdg_activation_v1_destroy(_grwl.wl.activationManager);
    }
    if (_grwl.wl.registry)
    {
        wl_registry_destroy(_grwl.wl.registry);
    }
    if (_grwl.wl.display)
    {
        wl_display_flush(_grwl.wl.display);
        wl_display_disconnect(_grwl.wl.display);
    }

    if (_grwl.wl.keyRepeatTimerfd >= 0)
    {
        close(_grwl.wl.keyRepeatTimerfd);
    }
    if (_grwl.wl.cursorTimerfd >= 0)
    {
        close(_grwl.wl.cursorTimerfd);
    }

    _grwl_free(_grwl.wl.clipboardString);
    _grwl_free(_grwl.wl.keyboardLayoutName);

    _grwlTerminateDBusPOSIX();
}

#endif // _GRWL_WAYLAND
