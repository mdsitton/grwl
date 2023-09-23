//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(GRWL_BUILD_COCOA_TIMER)

    #include <mach/mach_time.h>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlPlatformInitTimer(void)
{
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);

    _grwl.timer.ns.frequency = (info.denom * 1e9) / info.numer;
}

uint64_t _grwlPlatformGetTimerValue(void)
{
    return mach_absolute_time();
}

uint64_t _grwlPlatformGetTimerFrequency(void)
{
    return _grwl.timer.ns.frequency;
}

#endif // GRWL_BUILD_COCOA_TIMER
