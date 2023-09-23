//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <windows.h>

#define GLFW_WIN32_LIBRARY_TIMER_STATE _GLFWtimerWin32 win32;

// Win32-specific global timer data
//
typedef struct _GLFWtimerWin32
{
    uint64_t frequency;
} _GLFWtimerWin32;
