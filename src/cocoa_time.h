//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#define GLFW_COCOA_LIBRARY_TIMER_STATE _GLFWtimerNS ns;

// Cocoa-specific global timer data
typedef struct _GLFWtimerNS
{
    uint64_t frequency;
} _GLFWtimerNS;
