//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <windows.h>

#define GRWL_WIN32_TLS_STATE _GRWLtlsWin32 win32;
#define GRWL_WIN32_MUTEX_STATE _GRWLmutexWin32 win32;

// Win32-specific thread local storage data
//
typedef struct _GRWLtlsWin32
{
    GRWLbool allocated;
    DWORD index;
} _GRWLtlsWin32;

// Win32-specific mutex data
//
typedef struct _GRWLmutexWin32
{
    GRWLbool allocated;
    CRITICAL_SECTION section;
} _GRWLmutexWin32;
