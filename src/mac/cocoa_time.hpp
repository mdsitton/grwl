//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
#pragma once

#define GRWL_COCOA_LIBRARY_TIMER_STATE _GRWLtimerNS ns;

// Cocoa-specific global timer data
typedef struct _GRWLtimerNS
{
    uint64_t frequency;
} _GRWLtimerNS;
