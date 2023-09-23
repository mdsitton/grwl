//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <windows.h>

#define GLFW_WIN32_TLS_STATE _GLFWtlsWin32 win32;
#define GLFW_WIN32_MUTEX_STATE _GLFWmutexWin32 win32;

// Win32-specific thread local storage data
//
typedef struct _GLFWtlsWin32
{
    GLFWbool allocated;
    DWORD index;
} _GLFWtlsWin32;

// Win32-specific mutex data
//
typedef struct _GLFWmutexWin32
{
    GLFWbool allocated;
    CRITICAL_SECTION section;
} _GLFWmutexWin32;
