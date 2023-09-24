//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(GRWL_BUILD_POSIX_THREAD)

    #include <cassert>
    #include <cstring>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlPlatformCreateTls(_GRWLtls* tls)
{
    assert(tls->posix.allocated == false);

    if (pthread_key_create(&tls->posix.key, NULL) != 0)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "POSIX: Failed to create context TLS");
        return false;
    }

    tls->posix.allocated = true;
    return true;
}

void _grwlPlatformDestroyTls(_GRWLtls* tls)
{
    if (tls->posix.allocated)
    {
        pthread_key_delete(tls->posix.key);
    }
    memset(tls, 0, sizeof(_GRWLtls));
}

void* _grwlPlatformGetTls(_GRWLtls* tls)
{
    assert(tls->posix.allocated == true);
    return pthread_getspecific(tls->posix.key);
}

void _grwlPlatformSetTls(_GRWLtls* tls, void* value)
{
    assert(tls->posix.allocated == true);
    pthread_setspecific(tls->posix.key, value);
}

bool _grwlPlatformCreateMutex(_GRWLmutex* mutex)
{
    assert(mutex->posix.allocated == false);

    if (pthread_mutex_init(&mutex->posix.handle, NULL) != 0)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "POSIX: Failed to create mutex");
        return false;
    }

    return mutex->posix.allocated = true;
}

void _grwlPlatformDestroyMutex(_GRWLmutex* mutex)
{
    if (mutex->posix.allocated)
    {
        pthread_mutex_destroy(&mutex->posix.handle);
    }
    memset(mutex, 0, sizeof(_GRWLmutex));
}

void _grwlPlatformLockMutex(_GRWLmutex* mutex)
{
    assert(mutex->posix.allocated == true);
    pthread_mutex_lock(&mutex->posix.handle);
}

void _grwlPlatformUnlockMutex(_GRWLmutex* mutex)
{
    assert(mutex->posix.allocated == true);
    pthread_mutex_unlock(&mutex->posix.handle);
}

#endif // GRWL_BUILD_POSIX_THREAD
