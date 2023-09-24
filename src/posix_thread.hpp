//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <pthread.h>

#define GRWL_POSIX_TLS_STATE _GRWLtlsPOSIX posix;
#define GRWL_POSIX_MUTEX_STATE _GRWLmutexPOSIX posix;

// POSIX-specific thread local storage data
//
typedef struct _GRWLtlsPOSIX
{
    bool allocated;
    pthread_key_t key;
} _GRWLtlsPOSIX;

// POSIX-specific mutex data
//
typedef struct _GRWLmutexPOSIX
{
    bool allocated;
    pthread_mutex_t handle;
} _GRWLmutexPOSIX;
