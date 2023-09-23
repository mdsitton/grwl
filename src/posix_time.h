//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#define GRWL_POSIX_LIBRARY_TIMER_STATE _GRWLtimerPOSIX posix;

#include <stdint.h>
#include <time.h>

// POSIX-specific global timer data
//
typedef struct _GRWLtimerPOSIX
{
    clockid_t clock;
    uint64_t frequency;
} _GRWLtimerPOSIX;
