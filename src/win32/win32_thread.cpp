//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(GRWL_BUILD_WIN32_THREAD)

    #include <cassert>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlPlatformCreateTls(_GRWLtls* tls)
{
    assert(tls->win32.allocated == false);

    tls->win32.index = TlsAlloc();
    if (tls->win32.index == TLS_OUT_OF_INDEXES)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to allocate TLS index");
        return false;
    }

    tls->win32.allocated = true;
    return true;
}

void _grwlPlatformDestroyTls(_GRWLtls* tls)
{
    if (tls->win32.allocated)
    {
        TlsFree(tls->win32.index);
    }
    memset(tls, 0, sizeof(_GRWLtls));
}

void* _grwlPlatformGetTls(_GRWLtls* tls)
{
    assert(tls->win32.allocated == true);
    return TlsGetValue(tls->win32.index);
}

void _grwlPlatformSetTls(_GRWLtls* tls, void* value)
{
    assert(tls->win32.allocated == true);
    TlsSetValue(tls->win32.index, value);
}

bool _grwlPlatformCreateMutex(_GRWLmutex* mutex)
{
    assert(mutex->win32.allocated == false);
    InitializeCriticalSection(&mutex->win32.section);
    return mutex->win32.allocated = true;
}

void _grwlPlatformDestroyMutex(_GRWLmutex* mutex)
{
    if (mutex->win32.allocated)
    {
        DeleteCriticalSection(&mutex->win32.section);
    }
    memset(mutex, 0, sizeof(_GRWLmutex));
}

void _grwlPlatformLockMutex(_GRWLmutex* mutex)
{
    assert(mutex->win32.allocated == true);
    EnterCriticalSection(&mutex->win32.section);
}

void _grwlPlatformUnlockMutex(_GRWLmutex* mutex)
{
    assert(mutex->win32.allocated == true);
    LeaveCriticalSection(&mutex->win32.section);
}

#endif // GRWL_BUILD_WIN32_THREAD
