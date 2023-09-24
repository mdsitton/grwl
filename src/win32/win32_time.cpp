//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(GRWL_BUILD_WIN32_TIMER)

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlPlatformInitTimer()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&_grwl.timer.win32.frequency);
}

uint64_t _grwlPlatformGetTimerValue()
{
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*)&value);
    return value;
}

uint64_t _grwlPlatformGetTimerFrequency()
{
    return _grwl.timer.win32.frequency;
}

#endif // GRWL_BUILD_WIN32_TIMER
