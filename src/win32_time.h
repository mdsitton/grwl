//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <windows.h>

#define GRWL_WIN32_LIBRARY_TIMER_STATE _GRWLtimerWin32 win32;

// Win32-specific global timer data
//
typedef struct _GRWLtimerWin32
{
    uint64_t frequency;
} _GRWLtimerWin32;
