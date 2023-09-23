//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// The the sole (fake) video mode of our (sole) fake monitor
//
static GRWLvidmode getVideoMode(void)
{
    GRWLvidmode mode;
    mode.width = 1920;
    mode.height = 1080;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = 60;
    return mode;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlPollMonitorsNull(void)
{
    const float dpi = 141.f;
    const GRWLvidmode mode = getVideoMode();
    _GRWLmonitor* monitor =
        _grwlAllocMonitor("Null SuperNoop 0", (int)(mode.width * 25.4f / dpi), (int)(mode.height * 25.4f / dpi));
    _grwlInputMonitor(monitor, GRWL_CONNECTED, _GRWL_INSERT_FIRST);
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlFreeMonitorNull(_GRWLmonitor* monitor)
{
    _grwlFreeGammaArrays(&monitor->null.ramp);
}

void _grwlGetMonitorPosNull(_GRWLmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
    {
        *xpos = 0;
    }
    if (ypos)
    {
        *ypos = 0;
    }
}

void _grwlGetMonitorContentScaleNull(_GRWLmonitor* monitor, float* xscale, float* yscale)
{
    if (xscale)
    {
        *xscale = 1.f;
    }
    if (yscale)
    {
        *yscale = 1.f;
    }
}

void _grwlGetMonitorWorkareaNull(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height)
{
    const GRWLvidmode mode = getVideoMode();

    if (xpos)
    {
        *xpos = 0;
    }
    if (ypos)
    {
        *ypos = 10;
    }
    if (width)
    {
        *width = mode.width;
    }
    if (height)
    {
        *height = mode.height - 10;
    }
}

GRWLvidmode* _grwlGetVideoModesNull(_GRWLmonitor* monitor, int* found)
{
    GRWLvidmode* mode = _grwl_calloc(1, sizeof(GRWLvidmode));
    *mode = getVideoMode();
    *found = 1;
    return mode;
}

void _grwlGetVideoModeNull(_GRWLmonitor* monitor, GRWLvidmode* mode)
{
    *mode = getVideoMode();
}

GRWLbool _grwlGetGammaRampNull(_GRWLmonitor* monitor, GRWLgammaramp* ramp)
{
    if (!monitor->null.ramp.size)
    {
        unsigned int i;

        _grwlAllocGammaArrays(&monitor->null.ramp, 256);

        for (i = 0; i < monitor->null.ramp.size; i++)
        {
            const float gamma = 2.2f;
            float value;
            value = i / (float)(monitor->null.ramp.size - 1);
            value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
            value = _grwl_fminf(value, 65535.f);

            monitor->null.ramp.red[i] = (unsigned short)value;
            monitor->null.ramp.green[i] = (unsigned short)value;
            monitor->null.ramp.blue[i] = (unsigned short)value;
        }
    }

    _grwlAllocGammaArrays(ramp, monitor->null.ramp.size);
    memcpy(ramp->red, monitor->null.ramp.red, sizeof(short) * ramp->size);
    memcpy(ramp->green, monitor->null.ramp.green, sizeof(short) * ramp->size);
    memcpy(ramp->blue, monitor->null.ramp.blue, sizeof(short) * ramp->size);
    return GRWL_TRUE;
}

void _grwlSetGammaRampNull(_GRWLmonitor* monitor, const GRWLgammaramp* ramp)
{
    if (monitor->null.ramp.size != ramp->size)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Null: Gamma ramp size must match current ramp size");
        return;
    }

    memcpy(monitor->null.ramp.red, ramp->red, sizeof(short) * ramp->size);
    memcpy(monitor->null.ramp.green, ramp->green, sizeof(short) * ramp->size);
    memcpy(monitor->null.ramp.blue, ramp->blue, sizeof(short) * ramp->size);
}
