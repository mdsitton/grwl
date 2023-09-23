//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
#pragma once

#if defined(GRWL_BUILD_WIN32_TIMER) || defined(GRWL_BUILD_WIN32_MODULE) || defined(GRWL_BUILD_WIN32_THREAD) || \
    defined(GRWL_BUILD_COCOA_TIMER) || defined(GRWL_BUILD_POSIX_TIMER) || defined(GRWL_BUILD_POSIX_MODULE) ||  \
    defined(GRWL_BUILD_POSIX_THREAD) || defined(GRWL_BUILD_POSIX_POLL) || defined(GRWL_BUILD_LINUX_JOYSTICK)
    #error "You must not define these; define zero or more _GRWL_<platform> macros instead"
#endif

#if defined(_GRWL_WIN32)
    #include "win32/win32_platform.hpp"
#else
    #define GRWL_WIN32_WINDOW_STATE
    #define GRWL_WIN32_MONITOR_STATE
    #define GRWL_WIN32_CURSOR_STATE
    #define GRWL_WIN32_LIBRARY_WINDOW_STATE
    #define GRWL_WGL_CONTEXT_STATE
    #define GRWL_WGL_LIBRARY_CONTEXT_STATE
    #define GRWL_WGL_USER_CONTEXT_STATE
#endif

#if defined(_GRWL_COCOA)
    #include "mac/cocoa_platform.hpp"
#else
    #define GRWL_COCOA_WINDOW_STATE
    #define GRWL_COCOA_MONITOR_STATE
    #define GRWL_COCOA_CURSOR_STATE
    #define GRWL_COCOA_LIBRARY_WINDOW_STATE
    #define GRWL_NSGL_CONTEXT_STATE
    #define GRWL_NSGL_LIBRARY_CONTEXT_STATE
    #define GRWL_NSGL_USER_CONTEXT_STATE
#endif

#if defined(_GRWL_WAYLAND)
    #include "linux/wl_platform.hpp"
#else
    #define GRWL_WAYLAND_WINDOW_STATE
    #define GRWL_WAYLAND_MONITOR_STATE
    #define GRWL_WAYLAND_CURSOR_STATE
    #define GRWL_WAYLAND_LIBRARY_WINDOW_STATE
#endif

#if defined(_GRWL_X11)
    #include "linux/x11_platform.hpp"
#else
    #define GRWL_X11_WINDOW_STATE
    #define GRWL_X11_MONITOR_STATE
    #define GRWL_X11_CURSOR_STATE
    #define GRWL_X11_LIBRARY_WINDOW_STATE
    #define GRWL_GLX_CONTEXT_STATE
    #define GRWL_GLX_LIBRARY_CONTEXT_STATE
    #define GRWL_GLX_USER_CONTEXT_STATE
#endif

#if defined(_GRWL_WIN32)
    #include "win32/win32_joystick.hpp"
#else
    #define GRWL_WIN32_JOYSTICK_STATE
    #define GRWL_WIN32_LIBRARY_JOYSTICK_STATE
#endif

#if defined(_GRWL_COCOA)
    #include "mac/cocoa_joystick.hpp"
#else
    #define GRWL_COCOA_JOYSTICK_STATE
    #define GRWL_COCOA_LIBRARY_JOYSTICK_STATE
#endif

#if (defined(_GRWL_X11) || defined(_GRWL_WAYLAND)) && defined(__linux__)
    #define GRWL_BUILD_LINUX_JOYSTICK
#endif

#if defined(GRWL_BUILD_LINUX_JOYSTICK)
    #include "linux/linux_joystick.hpp"
#else
    #define GRWL_LINUX_JOYSTICK_STATE
    #define GRWL_LINUX_LIBRARY_JOYSTICK_STATE
#endif

#define GRWL_PLATFORM_WINDOW_STATE \
    GRWL_WIN32_WINDOW_STATE        \
    GRWL_COCOA_WINDOW_STATE        \
    GRWL_WAYLAND_WINDOW_STATE      \
    GRWL_X11_WINDOW_STATE

#define GRWL_PLATFORM_MONITOR_STATE \
    GRWL_WIN32_MONITOR_STATE        \
    GRWL_COCOA_MONITOR_STATE        \
    GRWL_WAYLAND_MONITOR_STATE      \
    GRWL_X11_MONITOR_STATE

#define GRWL_PLATFORM_CURSOR_STATE \
    GRWL_WIN32_CURSOR_STATE        \
    GRWL_COCOA_CURSOR_STATE        \
    GRWL_WAYLAND_CURSOR_STATE      \
    GRWL_X11_CURSOR_STATE

#define GRWL_PLATFORM_JOYSTICK_STATE \
    GRWL_WIN32_JOYSTICK_STATE        \
    GRWL_COCOA_JOYSTICK_STATE        \
    GRWL_LINUX_JOYSTICK_STATE

#define GRWL_PLATFORM_LIBRARY_WINDOW_STATE \
    GRWL_WIN32_LIBRARY_WINDOW_STATE        \
    GRWL_COCOA_LIBRARY_WINDOW_STATE        \
    GRWL_WAYLAND_LIBRARY_WINDOW_STATE      \
    GRWL_X11_LIBRARY_WINDOW_STATE

#define GRWL_PLATFORM_LIBRARY_JOYSTICK_STATE \
    GRWL_WIN32_LIBRARY_JOYSTICK_STATE        \
    GRWL_COCOA_LIBRARY_JOYSTICK_STATE        \
    GRWL_LINUX_LIBRARY_JOYSTICK_STATE

#define GRWL_PLATFORM_CONTEXT_STATE \
    GRWL_WGL_CONTEXT_STATE          \
    GRWL_NSGL_CONTEXT_STATE         \
    GRWL_GLX_CONTEXT_STATE

#define GRWL_PLATFORM_LIBRARY_CONTEXT_STATE \
    GRWL_WGL_LIBRARY_CONTEXT_STATE          \
    GRWL_NSGL_LIBRARY_CONTEXT_STATE         \
    GRWL_GLX_LIBRARY_CONTEXT_STATE

#define GRWL_PLATFORM_USER_CONTEXT_STATE \
    GRWL_WGL_USER_CONTEXT_STATE          \
    GRWL_NSGL_USER_CONTEXT_STATE         \
    GRWL_GLX_USER_CONTEXT_STATE

#if defined(_WIN32)
    #define GRWL_BUILD_WIN32_THREAD
#else
    #define GRWL_BUILD_POSIX_THREAD
#endif

#if defined(GRWL_BUILD_WIN32_THREAD)
    #include "win32/win32_thread.hpp"
    #define GRWL_PLATFORM_TLS_STATE GRWL_WIN32_TLS_STATE
    #define GRWL_PLATFORM_MUTEX_STATE GRWL_WIN32_MUTEX_STATE
#elif defined(GRWL_BUILD_POSIX_THREAD)
    #include "posix_thread.hpp"
    #define GRWL_PLATFORM_TLS_STATE GRWL_POSIX_TLS_STATE
    #define GRWL_PLATFORM_MUTEX_STATE GRWL_POSIX_MUTEX_STATE
#endif

#if defined(_WIN32)
    #define GRWL_BUILD_WIN32_TIMER
#elif defined(__APPLE__)
    #define GRWL_BUILD_COCOA_TIMER
#else
    #define _GNU_SOURCE
    #define GRWL_BUILD_POSIX_TIMER
    #define GRWL_BUILD_POSIX_DBUS
#endif

#if defined(GRWL_BUILD_WIN32_TIMER)
    #include "win32/win32_time.hpp"
    #define GRWL_PLATFORM_LIBRARY_TIMER_STATE GRWL_WIN32_LIBRARY_TIMER_STATE
#elif defined(GRWL_BUILD_COCOA_TIMER)
    #include "mac/cocoa_time.hpp"
    #define GRWL_PLATFORM_LIBRARY_TIMER_STATE GRWL_COCOA_LIBRARY_TIMER_STATE
#elif defined(GRWL_BUILD_POSIX_TIMER)
    #include "linux/posix_time.hpp"
    #define GRWL_PLATFORM_LIBRARY_TIMER_STATE GRWL_POSIX_LIBRARY_TIMER_STATE
#endif

#if defined(GRWL_BUILD_POSIX_DBUS)
    #include "linux/posix_dbus.hpp"
    #define GRWL_PLATFORM_LIBRARY_DBUS_STATE GRWL_POSIX_LIBRARY_DBUS_STATE
#else
    #define GRWL_PLATFORM_LIBRARY_DBUS_STATE
#endif

#if defined(_WIN32)
    #define GRWL_BUILD_WIN32_MODULE
#else
    #define GRWL_BUILD_POSIX_MODULE
#endif

#if defined(_GRWL_WAYLAND) || defined(_GRWL_X11)
    #define GRWL_BUILD_POSIX_POLL
#endif
