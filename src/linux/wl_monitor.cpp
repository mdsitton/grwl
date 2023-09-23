//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_WAYLAND)

    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #include <cerrno>
    #include <cmath>

    #include "wayland-client-protocol.h"

static void outputHandleGeometry(void* userData, struct wl_output* output, int32_t x, int32_t y, int32_t physicalWidth,
                                 int32_t physicalHeight, int32_t subpixel, const char* make, const char* model,
                                 int32_t transform)
{
    struct _GRWLmonitor* monitor = userData;

    monitor->wl.x = x;
    monitor->wl.y = y;
    monitor->widthMM = physicalWidth;
    monitor->heightMM = physicalHeight;

    if (strlen(monitor->name) == 0)
    {
        snprintf(monitor->name, sizeof(monitor->name), "%s %s", make, model);
    }
}

static void outputHandleMode(void* userData, struct wl_output* output, uint32_t flags, int32_t width, int32_t height,
                             int32_t refresh)
{
    struct _GRWLmonitor* monitor = userData;
    GRWLvidmode mode;

    mode.width = width;
    mode.height = height;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = (int)round(refresh / 1000.0);

    monitor->modeCount++;
    monitor->modes = _grwl_realloc(monitor->modes, monitor->modeCount * sizeof(GRWLvidmode));
    monitor->modes[monitor->modeCount - 1] = mode;

    if (flags & WL_OUTPUT_MODE_CURRENT)
    {
        monitor->wl.currentMode = monitor->modeCount - 1;
    }
}

static void outputHandleDone(void* userData, struct wl_output* output)
{
    struct _GRWLmonitor* monitor = userData;

    if (monitor->widthMM <= 0 || monitor->heightMM <= 0)
    {
        // If Wayland does not provide a physical size, assume the default 96 DPI
        const GRWLvidmode* mode = &monitor->modes[monitor->wl.currentMode];
        monitor->widthMM = (int)(mode->width * 25.4f / 96.f);
        monitor->heightMM = (int)(mode->height * 25.4f / 96.f);
    }

    for (int i = 0; i < _grwl.monitorCount; i++)
    {
        if (_grwl.monitors[i] == monitor)
        {
            return;
        }
    }

    _grwlInputMonitor(monitor, GRWL_CONNECTED, _GRWL_INSERT_LAST);
}

static void outputHandleScale(void* userData, struct wl_output* output, int32_t factor)
{
    struct _GRWLmonitor* monitor = userData;

    monitor->wl.contentScale = factor;

    for (_GRWLwindow* window = _grwl.windowListHead; window; window = window->next)
    {
        for (int i = 0; i < window->wl.scaleCount; i++)
        {
            if (window->wl.scales[i].output == monitor->wl.output)
            {
                window->wl.scales[i].factor = monitor->wl.contentScale;
                _grwlUpdateContentScaleWayland(window);
                break;
            }
        }
    }
}

    #ifdef WL_OUTPUT_NAME_SINCE_VERSION

void outputHandleName(void* userData, struct wl_output* wl_output, const char* name)
{
    struct _GRWLmonitor* monitor = userData;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);
}

void outputHandleDescription(void* userData, struct wl_output* wl_output, const char* description)
{
}

    #endif // WL_OUTPUT_NAME_SINCE_VERSION

static const struct wl_output_listener outputListener = {
    outputHandleGeometry, outputHandleMode,        outputHandleDone, outputHandleScale,
    #ifdef WL_OUTPUT_NAME_SINCE_VERSION
    outputHandleName,     outputHandleDescription,
    #endif
};

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlAddOutputWayland(uint32_t name, uint32_t version)
{
    if (version < 2)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Unsupported output interface version");
        return;
    }

    #ifdef WL_OUTPUT_NAME_SINCE_VERSION
    version = _grwl_min(version, WL_OUTPUT_NAME_SINCE_VERSION);
    #else
    version = 2;
    #endif

    struct wl_output* output = wl_registry_bind(_grwl.wl.registry, name, &wl_output_interface, version);
    if (!output)
    {
        return;
    }

    // The actual name of this output will be set in the geometry handler
    _GRWLmonitor* monitor = _grwlAllocMonitor("", 0, 0);
    monitor->wl.contentScale = 1;
    monitor->wl.output = output;
    monitor->wl.name = name;

    wl_proxy_set_tag((struct wl_proxy*)output, &_grwl.wl.tag);
    wl_output_add_listener(output, &outputListener, monitor);
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlFreeMonitorWayland(_GRWLmonitor* monitor)
{
    if (monitor->wl.output)
    {
        wl_output_destroy(monitor->wl.output);
    }
}

void _grwlGetMonitorPosWayland(_GRWLmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
    {
        *xpos = monitor->wl.x;
    }
    if (ypos)
    {
        *ypos = monitor->wl.y;
    }
}

void _grwlGetMonitorContentScaleWayland(_GRWLmonitor* monitor, float* xscale, float* yscale)
{
    if (xscale)
    {
        *xscale = (float)monitor->wl.contentScale;
    }
    if (yscale)
    {
        *yscale = (float)monitor->wl.contentScale;
    }
}

void _grwlGetMonitorWorkareaWayland(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height)
{
    if (xpos)
    {
        *xpos = monitor->wl.x;
    }
    if (ypos)
    {
        *ypos = monitor->wl.y;
    }
    if (width)
    {
        *width = monitor->modes[monitor->wl.currentMode].width;
    }
    if (height)
    {
        *height = monitor->modes[monitor->wl.currentMode].height;
    }
}

GRWLvidmode* _grwlGetVideoModesWayland(_GRWLmonitor* monitor, int* found)
{
    *found = monitor->modeCount;
    return monitor->modes;
}

void _grwlGetVideoModeWayland(_GRWLmonitor* monitor, GRWLvidmode* mode)
{
    *mode = monitor->modes[monitor->wl.currentMode];
}

GRWLbool _grwlGetGammaRampWayland(_GRWLmonitor* monitor, GRWLgammaramp* ramp)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: Gamma ramp access is not available");
    return GRWL_FALSE;
}

void _grwlSetGammaRampWayland(_GRWLmonitor* monitor, const GRWLgammaramp* ramp)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: Gamma ramp access is not available");
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI struct wl_output* grwlGetWaylandMonitor(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->wl.output;
}

#endif // _GRWL_WAYLAND
