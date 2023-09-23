//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(GRWL_BUILD_POSIX_TIMER)

    #include <unistd.h>
    #include <sys/time.h>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlPlatformInitTimer(void)
{
    _grwl.timer.posix.clock = CLOCK_REALTIME;
    _grwl.timer.posix.frequency = 1000000000;

    #if defined(_POSIX_MONOTONIC_CLOCK)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        _grwl.timer.posix.clock = CLOCK_MONOTONIC;
    }
    #endif
}

uint64_t _grwlPlatformGetTimerValue(void)
{
    struct timespec ts;
    clock_gettime(_grwl.timer.posix.clock, &ts);
    return (uint64_t)ts.tv_sec * _grwl.timer.posix.frequency + (uint64_t)ts.tv_nsec;
}

uint64_t _grwlPlatformGetTimerFrequency(void)
{
    return _grwl.timer.posix.frequency;
}

#endif // GRWL_BUILD_POSIX_TIMER
