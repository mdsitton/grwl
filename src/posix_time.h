//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#define GLFW_POSIX_LIBRARY_TIMER_STATE _GLFWtimerPOSIX posix;

#include <stdint.h>
#include <time.h>

// POSIX-specific global timer data
//
typedef struct _GLFWtimerPOSIX
{
    clockid_t clock;
    uint64_t frequency;
} _GLFWtimerPOSIX;
