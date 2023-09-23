//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// Lexically compare video modes, used by qsort
//
static int compareVideoModes(const void* fp, const void* sp)
{
    const GRWLvidmode* fm = fp;
    const GRWLvidmode* sm = sp;
    const int fbpp = fm->redBits + fm->greenBits + fm->blueBits;
    const int sbpp = sm->redBits + sm->greenBits + sm->blueBits;
    const int farea = fm->width * fm->height;
    const int sarea = sm->width * sm->height;

    // First sort on color bits per pixel
    if (fbpp != sbpp)
    {
        return fbpp - sbpp;
    }

    // Then sort on screen area
    if (farea != sarea)
    {
        return farea - sarea;
    }

    // Then sort on width
    if (fm->width != sm->width)
    {
        return fm->width - sm->width;
    }

    // Lastly sort on refresh rate
    return fm->refreshRate - sm->refreshRate;
}

// Retrieves the available modes for the specified monitor
//
static GRWLbool refreshVideoModes(_GRWLmonitor* monitor)
{
    int modeCount;
    GRWLvidmode* modes;

    if (monitor->modes)
    {
        return GRWL_TRUE;
    }

    modes = _grwl.platform.getVideoModes(monitor, &modeCount);
    if (!modes)
    {
        return GRWL_FALSE;
    }

    qsort(modes, modeCount, sizeof(GRWLvidmode), compareVideoModes);

    _grwl_free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;

    return GRWL_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////                         GRWL event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a monitor connection or disconnection
//
void _grwlInputMonitor(_GRWLmonitor* monitor, int action, int placement)
{
    assert(monitor != NULL);
    assert(action == GRWL_CONNECTED || action == GRWL_DISCONNECTED);
    assert(placement == _GRWL_INSERT_FIRST || placement == _GRWL_INSERT_LAST);

    if (action == GRWL_CONNECTED)
    {
        _grwl.monitorCount++;
        _grwl.monitors = _grwl_realloc(_grwl.monitors, sizeof(_GRWLmonitor*) * _grwl.monitorCount);

        if (placement == _GRWL_INSERT_FIRST)
        {
            memmove(_grwl.monitors + 1, _grwl.monitors, ((size_t)_grwl.monitorCount - 1) * sizeof(_GRWLmonitor*));
            _grwl.monitors[0] = monitor;
        }
        else
        {
            _grwl.monitors[_grwl.monitorCount - 1] = monitor;
        }
    }
    else if (action == GRWL_DISCONNECTED)
    {
        int i;
        _GRWLwindow* window;

        for (window = _grwl.windowListHead; window; window = window->next)
        {
            if (window->monitor == monitor)
            {
                int width, height, xoff, yoff;
                _grwl.platform.getWindowSize(window, &width, &height);
                _grwl.platform.setWindowMonitor(window, NULL, 0, 0, width, height, 0);
                _grwl.platform.getWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                _grwl.platform.setWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0; i < _grwl.monitorCount; i++)
        {
            if (_grwl.monitors[i] == monitor)
            {
                _grwl.monitorCount--;
                memmove(_grwl.monitors + i, _grwl.monitors + i + 1,
                        ((size_t)_grwl.monitorCount - i) * sizeof(_GRWLmonitor*));
                break;
            }
        }
    }

    if (_grwl.callbacks.monitor)
    {
        _grwl.callbacks.monitor((GRWLmonitor*)monitor, action);
    }

    if (action == GRWL_DISCONNECTED)
    {
        _grwlFreeMonitor(monitor);
    }
}

// Notifies shared code that a full screen window has acquired or released
// a monitor
//
void _grwlInputMonitorWindow(_GRWLmonitor* monitor, _GRWLwindow* window)
{
    assert(monitor != NULL);
    monitor->window = window;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Allocates and returns a monitor object with the specified name and dimensions
//
_GRWLmonitor* _grwlAllocMonitor(const char* name, int widthMM, int heightMM)
{
    _GRWLmonitor* monitor = _grwl_calloc(1, sizeof(_GRWLmonitor));
    monitor->widthMM = widthMM;
    monitor->heightMM = heightMM;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);

    return monitor;
}

// Frees a monitor object and any data associated with it
//
void _grwlFreeMonitor(_GRWLmonitor* monitor)
{
    if (monitor == NULL)
    {
        return;
    }

    _grwl.platform.freeMonitor(monitor);

    _grwlFreeGammaArrays(&monitor->originalRamp);
    _grwlFreeGammaArrays(&monitor->currentRamp);

    _grwl_free(monitor->modes);
    _grwl_free(monitor);
}

// Allocates red, green and blue value arrays of the specified size
//
void _grwlAllocGammaArrays(GRWLgammaramp* ramp, unsigned int size)
{
    ramp->red = _grwl_calloc(size, sizeof(unsigned short));
    ramp->green = _grwl_calloc(size, sizeof(unsigned short));
    ramp->blue = _grwl_calloc(size, sizeof(unsigned short));
    ramp->size = size;
}

// Frees the red, green and blue value arrays and clears the struct
//
void _grwlFreeGammaArrays(GRWLgammaramp* ramp)
{
    _grwl_free(ramp->red);
    _grwl_free(ramp->green);
    _grwl_free(ramp->blue);

    memset(ramp, 0, sizeof(GRWLgammaramp));
}

// Chooses the video mode most closely matching the desired one
//
const GRWLvidmode* _grwlChooseVideoMode(_GRWLmonitor* monitor, const GRWLvidmode* desired)
{
    int i;
    unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
    unsigned int rateDiff, leastRateDiff = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    const GRWLvidmode* current;
    const GRWLvidmode* closest = NULL;

    if (!refreshVideoModes(monitor))
    {
        return NULL;
    }

    for (i = 0; i < monitor->modeCount; i++)
    {
        current = monitor->modes + i;

        colorDiff = 0;

        if (desired->redBits != GRWL_DONT_CARE)
        {
            colorDiff += abs(current->redBits - desired->redBits);
        }
        if (desired->greenBits != GRWL_DONT_CARE)
        {
            colorDiff += abs(current->greenBits - desired->greenBits);
        }
        if (desired->blueBits != GRWL_DONT_CARE)
        {
            colorDiff += abs(current->blueBits - desired->blueBits);
        }

        sizeDiff = abs((current->width - desired->width) * (current->width - desired->width) +
                       (current->height - desired->height) * (current->height - desired->height));

        if (desired->refreshRate != GRWL_DONT_CARE)
        {
            rateDiff = abs(current->refreshRate - desired->refreshRate);
        }
        else
        {
            rateDiff = UINT_MAX - current->refreshRate;
        }

        if ((colorDiff < leastColorDiff) || (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) ||
            (colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff))
        {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

// Performs lexical comparison between two @ref GRWLvidmode structures
//
int _grwlCompareVideoModes(const GRWLvidmode* fm, const GRWLvidmode* sm)
{
    return compareVideoModes(fm, sm);
}

// Splits a color depth into red, green and blue bit depths
//
void _grwlSplitBPP(int bpp, int* red, int* green, int* blue)
{
    int delta;

    // We assume that by 32 the user really meant 24
    if (bpp == 32)
    {
        bpp = 24;
    }

    // Convert "bits per pixel" to red, green & blue sizes

    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1)
    {
        *green = *green + 1;
    }

    if (delta == 2)
    {
        *red = *red + 1;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI GRWLmonitor** grwlGetMonitors(int* count)
{
    assert(count != NULL);

    *count = 0;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    *count = _grwl.monitorCount;
    return (GRWLmonitor**)_grwl.monitors;
}

GRWLAPI GRWLmonitor* grwlGetPrimaryMonitor(void)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_grwl.monitorCount)
    {
        return NULL;
    }

    return (GRWLmonitor*)_grwl.monitors[0];
}

GRWLAPI void grwlGetMonitorPos(GRWLmonitor* handle, int* xpos, int* ypos)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    if (xpos)
    {
        *xpos = 0;
    }
    if (ypos)
    {
        *ypos = 0;
    }

    _GRWL_REQUIRE_INIT();

    _grwl.platform.getMonitorPos(monitor, xpos, ypos);
}

GRWLAPI void grwlGetMonitorWorkarea(GRWLmonitor* handle, int* xpos, int* ypos, int* width, int* height)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    if (xpos)
    {
        *xpos = 0;
    }
    if (ypos)
    {
        *ypos = 0;
    }
    if (width)
    {
        *width = 0;
    }
    if (height)
    {
        *height = 0;
    }

    _GRWL_REQUIRE_INIT();

    _grwl.platform.getMonitorWorkarea(monitor, xpos, ypos, width, height);
}

GRWLAPI void grwlGetMonitorPhysicalSize(GRWLmonitor* handle, int* widthMM, int* heightMM)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    if (widthMM)
    {
        *widthMM = 0;
    }
    if (heightMM)
    {
        *heightMM = 0;
    }

    _GRWL_REQUIRE_INIT();

    if (widthMM)
    {
        *widthMM = monitor->widthMM;
    }
    if (heightMM)
    {
        *heightMM = monitor->heightMM;
    }
}

GRWLAPI void grwlGetMonitorContentScale(GRWLmonitor* handle, float* xscale, float* yscale)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    if (xscale)
    {
        *xscale = 0.f;
    }
    if (yscale)
    {
        *yscale = 0.f;
    }

    _GRWL_REQUIRE_INIT();
    _grwl.platform.getMonitorContentScale(monitor, xscale, yscale);
}

GRWLAPI const char* grwlGetMonitorName(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->name;
}

GRWLAPI void grwlSetMonitorUserPointer(GRWLmonitor* handle, void* pointer)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    _GRWL_REQUIRE_INIT();
    monitor->userPointer = pointer;
}

GRWLAPI void* grwlGetMonitorUserPointer(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->userPointer;
}

GRWLAPI GRWLmonitorfun grwlSetMonitorCallback(GRWLmonitorfun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLmonitorfun, _grwl.callbacks.monitor, cbfun);
    return cbfun;
}

GRWLAPI const GRWLvidmode* grwlGetVideoModes(GRWLmonitor* handle, int* count)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);
    assert(count != NULL);

    *count = 0;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (!refreshVideoModes(monitor))
    {
        return NULL;
    }

    *count = monitor->modeCount;
    return monitor->modes;
}

GRWLAPI const GRWLvidmode* grwlGetVideoMode(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    _grwl.platform.getVideoMode(monitor, &monitor->currentMode);
    return &monitor->currentMode;
}

GRWLAPI void grwlSetGamma(GRWLmonitor* handle, float gamma)
{
    unsigned int i;
    unsigned short* values;
    GRWLgammaramp ramp;
    const GRWLgammaramp* original;
    assert(handle != NULL);
    assert(gamma > 0.f);
    assert(gamma <= FLT_MAX);

    _GRWL_REQUIRE_INIT();

    if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid gamma value %f", gamma);
        return;
    }

    original = grwlGetGammaRamp(handle);
    if (!original)
    {
        return;
    }

    values = _grwl_calloc(original->size, sizeof(unsigned short));

    for (i = 0; i < original->size; i++)
    {
        float value;

        // Calculate intensity
        value = i / (float)(original->size - 1);
        // Apply gamma curve
        value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
        // Clamp to value range
        value = _grwl_fminf(value, 65535.f);

        values[i] = (unsigned short)value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = original->size;

    grwlSetGammaRamp(handle, &ramp);
    _grwl_free(values);
}

GRWLAPI const GRWLgammaramp* grwlGetGammaRamp(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    _grwlFreeGammaArrays(&monitor->currentRamp);
    if (!_grwl.platform.getGammaRamp(monitor, &monitor->currentRamp))
    {
        return NULL;
    }

    return &monitor->currentRamp;
}

GRWLAPI void grwlSetGammaRamp(GRWLmonitor* handle, const GRWLgammaramp* ramp)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    assert(monitor != NULL);
    assert(ramp != NULL);
    assert(ramp->size > 0);
    assert(ramp->red != NULL);
    assert(ramp->green != NULL);
    assert(ramp->blue != NULL);

    _GRWL_REQUIRE_INIT();

    if (ramp->size <= 0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid gamma ramp size %i", ramp->size);
        return;
    }

    if (!monitor->originalRamp.size)
    {
        if (!_grwl.platform.getGammaRamp(monitor, &monitor->originalRamp))
        {
            return;
        }
    }

    _grwl.platform.setGammaRamp(monitor, ramp);
}
