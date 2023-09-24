//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_X11)

    #include <climits>
    #include <cstdlib>
    #include <cstring>
    #include <cmath>

// Check whether the display mode should be included in enumeration
//
static bool modeIsGood(const XRRModeInfo* mi)
{
    return (mi->modeFlags & RR_Interlace) == 0;
}

// Calculates the refresh rate, in Hz, from the specified RandR mode info
//
static int calculateRefreshRate(const XRRModeInfo* mi)
{
    if (mi->hTotal && mi->vTotal)
    {
        return (int)round((double)mi->dotClock / ((double)mi->hTotal * (double)mi->vTotal));
    }
    else
    {
        return 0;
    }
}

// Returns the mode info for a RandR mode XID
//
static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id)
{
    for (int i = 0; i < sr->nmode; i++)
    {
        if (sr->modes[i].id == id)
        {
            return sr->modes + i;
        }
    }

    return nullptr;
}

// Convert RandR mode info to GRWL video mode
//
static GRWLvidmode vidmodeFromModeInfo(const XRRModeInfo* mi, const XRRCrtcInfo* ci)
{
    GRWLvidmode mode;

    if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
    {
        mode.width = mi->height;
        mode.height = mi->width;
    }
    else
    {
        mode.width = mi->width;
        mode.height = mi->height;
    }

    mode.refreshRate = calculateRefreshRate(mi);

    _grwlSplitBPP(DefaultDepth(_grwl.x11.display, _grwl.x11.screen), &mode.redBits, &mode.greenBits, &mode.blueBits);

    return mode;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _grwlPollMonitorsX11()
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        int disconnectedCount, screenCount = 0;
        _GRWLmonitor** disconnected = nullptr;
        XineramaScreenInfo* screens = nullptr;
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        RROutput primary = XRRGetOutputPrimary(_grwl.x11.display, _grwl.x11.root);

        if (_grwl.x11.xinerama.available)
        {
            screens = XineramaQueryScreens(_grwl.x11.display, &screenCount);
        }

        disconnectedCount = _grwl.monitorCount;
        if (disconnectedCount)
        {
            disconnected = (_GRWLmonitor**)_grwl_calloc(_grwl.monitorCount, sizeof(_GRWLmonitor*));
            memcpy(disconnected, _grwl.monitors, _grwl.monitorCount * sizeof(_GRWLmonitor*));
        }

        for (int i = 0; i < sr->noutput; i++)
        {
            int j, type, widthMM, heightMM;

            XRROutputInfo* oi = XRRGetOutputInfo(_grwl.x11.display, sr, sr->outputs[i]);
            if (oi->connection != RR_Connected || oi->crtc == None)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            for (j = 0; j < disconnectedCount; j++)
            {
                if (disconnected[j] && disconnected[j]->x11.output == sr->outputs[i])
                {
                    disconnected[j] = nullptr;
                    break;
                }
            }

            if (j < disconnectedCount)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, oi->crtc);
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
            {
                widthMM = oi->mm_height;
                heightMM = oi->mm_width;
            }
            else
            {
                widthMM = oi->mm_width;
                heightMM = oi->mm_height;
            }

            if (widthMM <= 0 || heightMM <= 0)
            {
                // HACK: If RandR does not provide a physical size, assume the
                //       X11 default 96 DPI and calculate from the CRTC viewport
                // NOTE: These members are affected by rotation, unlike the mode
                //       info and output info members
                widthMM = (int)(ci->width * 25.4f / 96.f);
                heightMM = (int)(ci->height * 25.4f / 96.f);
            }

            _GRWLmonitor* monitor = _grwlAllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i];
            monitor->x11.crtc = oi->crtc;

            for (j = 0; j < screenCount; j++)
            {
                if (screens[j].x_org == ci->x && screens[j].y_org == ci->y && screens[j].width == ci->width &&
                    screens[j].height == ci->height)
                {
                    monitor->x11.index = j;
                    break;
                }
            }

            if (monitor->x11.output == primary)
            {
                type = _GRWL_INSERT_FIRST;
            }
            else
            {
                type = _GRWL_INSERT_LAST;
            }

            _grwlInputMonitor(monitor, GRWL_CONNECTED, type);

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);

        if (screens)
        {
            XFree(screens);
        }

        for (int i = 0; i < disconnectedCount; i++)
        {
            if (disconnected[i])
            {
                _grwlInputMonitor(disconnected[i], GRWL_DISCONNECTED, 0);
            }
        }

        _grwl_free(disconnected);
    }
    else
    {
        const int widthMM = DisplayWidthMM(_grwl.x11.display, _grwl.x11.screen);
        const int heightMM = DisplayHeightMM(_grwl.x11.display, _grwl.x11.screen);

        _grwlInputMonitor(_grwlAllocMonitor("Display", widthMM, heightMM), GRWL_CONNECTED, _GRWL_INSERT_FIRST);
    }
}

// Set the current video mode for the specified monitor
//
void _grwlSetVideoModeX11(_GRWLmonitor* monitor, const GRWLvidmode* desired)
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        GRWLvidmode current;
        RRMode native = None;

        const GRWLvidmode* best = _grwlChooseVideoMode(monitor, desired);
        _grwlGetVideoModeX11(monitor, &current);
        if (_grwlCompareVideoModes(&current, best) == 0)
        {
            return;
        }

        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_grwl.x11.display, sr, monitor->x11.output);

        for (int i = 0; i < oi->nmode; i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
            {
                continue;
            }

            const GRWLvidmode mode = vidmodeFromModeInfo(mi, ci);
            if (_grwlCompareVideoModes(best, &mode) == 0)
            {
                native = mi->id;
                break;
            }
        }

        if (native)
        {
            if (monitor->x11.oldMode == None)
            {
                monitor->x11.oldMode = ci->mode;
            }

            XRRSetCrtcConfig(_grwl.x11.display, sr, monitor->x11.crtc, CurrentTime, ci->x, ci->y, native, ci->rotation,
                             ci->outputs, ci->noutput);
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
}

// Restore the saved (original) video mode for the specified monitor
//
void _grwlRestoreVideoModeX11(_GRWLmonitor* monitor)
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        if (monitor->x11.oldMode == None)
        {
            return;
        }

        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, monitor->x11.crtc);

        XRRSetCrtcConfig(_grwl.x11.display, sr, monitor->x11.crtc, CurrentTime, ci->x, ci->y, monitor->x11.oldMode,
                         ci->rotation, ci->outputs, ci->noutput);

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);

        monitor->x11.oldMode = None;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlFreeMonitorX11(_GRWLmonitor* monitor)
{
}

void _grwlGetMonitorPosX11(_GRWLmonitor* monitor, int* xpos, int* ypos)
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, monitor->x11.crtc);

        if (ci)
        {
            if (xpos)
            {
                *xpos = ci->x;
            }
            if (ypos)
            {
                *ypos = ci->y;
            }

            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
    }
}

void _grwlGetMonitorContentScaleX11(_GRWLmonitor* monitor, float* xscale, float* yscale)
{
    if (xscale)
    {
        *xscale = _grwl.x11.contentScaleX;
    }
    if (yscale)
    {
        *yscale = _grwl.x11.contentScaleY;
    }
}

void _grwlGetMonitorWorkareaX11(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height)
{
    int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;

    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, monitor->x11.crtc);

        areaX = ci->x;
        areaY = ci->y;

        const XRRModeInfo* mi = getModeInfo(sr, ci->mode);

        if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
        {
            areaWidth = mi->height;
            areaHeight = mi->width;
        }
        else
        {
            areaWidth = mi->width;
            areaHeight = mi->height;
        }

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        areaWidth = DisplayWidth(_grwl.x11.display, _grwl.x11.screen);
        areaHeight = DisplayHeight(_grwl.x11.display, _grwl.x11.screen);
    }

    if (_grwl.x11.NET_WORKAREA && _grwl.x11.NET_CURRENT_DESKTOP)
    {
        Atom* extents = nullptr;
        Atom* desktop = nullptr;
        const unsigned long extentCount =
            _grwlGetWindowPropertyX11(_grwl.x11.root, _grwl.x11.NET_WORKAREA, XA_CARDINAL, (unsigned char**)&extents);

        if (_grwlGetWindowPropertyX11(_grwl.x11.root, _grwl.x11.NET_CURRENT_DESKTOP, XA_CARDINAL,
                                      (unsigned char**)&desktop) > 0)
        {
            if (extentCount >= 4 && *desktop < extentCount / 4)
            {
                const int globalX = extents[*desktop * 4 + 0];
                const int globalY = extents[*desktop * 4 + 1];
                const int globalWidth = extents[*desktop * 4 + 2];
                const int globalHeight = extents[*desktop * 4 + 3];

                if (areaX < globalX)
                {
                    areaWidth -= globalX - areaX;
                    areaX = globalX;
                }

                if (areaY < globalY)
                {
                    areaHeight -= globalY - areaY;
                    areaY = globalY;
                }

                if (areaX + areaWidth > globalX + globalWidth)
                {
                    areaWidth = globalX - areaX + globalWidth;
                }
                if (areaY + areaHeight > globalY + globalHeight)
                {
                    areaHeight = globalY - areaY + globalHeight;
                }
            }
        }

        if (extents)
        {
            XFree(extents);
        }
        if (desktop)
        {
            XFree(desktop);
        }
    }

    if (xpos)
    {
        *xpos = areaX;
    }
    if (ypos)
    {
        *ypos = areaY;
    }
    if (width)
    {
        *width = areaWidth;
    }
    if (height)
    {
        *height = areaHeight;
    }
}

GRWLvidmode* _grwlGetVideoModesX11(_GRWLmonitor* monitor, int* count)
{
    GRWLvidmode* result;

    *count = 0;

    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_grwl.x11.display, sr, monitor->x11.output);

        result = (GRWLvidmode*)_grwl_calloc(oi->nmode, sizeof(GRWLvidmode));

        for (int i = 0; i < oi->nmode; i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
            {
                continue;
            }

            const GRWLvidmode mode = vidmodeFromModeInfo(mi, ci);
            int j;

            for (j = 0; j < *count; j++)
            {
                if (_grwlCompareVideoModes(result + j, &mode) == 0)
                {
                    break;
                }
            }

            // Skip duplicate modes
            if (j < *count)
            {
                continue;
            }

            (*count)++;
            result[*count - 1] = mode;
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        *count = 1;
        result = (GRWLvidmode*)_grwl_calloc(1, sizeof(GRWLvidmode));
        _grwlGetVideoModeX11(monitor, result);
    }

    return result;
}

void _grwlGetVideoModeX11(_GRWLmonitor* monitor, GRWLvidmode* mode)
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_grwl.x11.display, sr, monitor->x11.crtc);

        if (ci)
        {
            const XRRModeInfo* mi = getModeInfo(sr, ci->mode);
            if (mi) // mi can be nullptr if the monitor has been disconnected
            {
                *mode = vidmodeFromModeInfo(mi, ci);
            }

            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
    }
    else
    {
        mode->width = DisplayWidth(_grwl.x11.display, _grwl.x11.screen);
        mode->height = DisplayHeight(_grwl.x11.display, _grwl.x11.screen);
        mode->refreshRate = 0;

        _grwlSplitBPP(DefaultDepth(_grwl.x11.display, _grwl.x11.screen), &mode->redBits, &mode->greenBits,
                      &mode->blueBits);
    }
}

bool _grwlGetGammaRampX11(_GRWLmonitor* monitor, GRWLgammaramp* ramp)
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.gammaBroken)
    {
        const size_t size = XRRGetCrtcGammaSize(_grwl.x11.display, monitor->x11.crtc);
        XRRCrtcGamma* gamma = XRRGetCrtcGamma(_grwl.x11.display, monitor->x11.crtc);

        _grwlAllocGammaArrays(ramp, size);

        memcpy(ramp->red, gamma->red, size * sizeof(unsigned short));
        memcpy(ramp->green, gamma->green, size * sizeof(unsigned short));
        memcpy(ramp->blue, gamma->blue, size * sizeof(unsigned short));

        XRRFreeGamma(gamma);
        return true;
    }
    else if (_grwl.x11.vidmode.available)
    {
        int size;
        XF86VidModeGetGammaRampSize(_grwl.x11.display, _grwl.x11.screen, &size);

        _grwlAllocGammaArrays(ramp, size);

        XF86VidModeGetGammaRamp(_grwl.x11.display, _grwl.x11.screen, ramp->size, ramp->red, ramp->green, ramp->blue);
        return true;
    }
    else
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Gamma ramp access not supported by server");
        return false;
    }
}

void _grwlSetGammaRampX11(_GRWLmonitor* monitor, const GRWLgammaramp* ramp)
{
    if (_grwl.x11.randr.available && !_grwl.x11.randr.gammaBroken)
    {
        if (XRRGetCrtcGammaSize(_grwl.x11.display, monitor->x11.crtc) != ramp->size)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Gamma ramp size must match current ramp size");
            return;
        }

        XRRCrtcGamma* gamma = XRRAllocGamma(ramp->size);

        memcpy(gamma->red, ramp->red, ramp->size * sizeof(unsigned short));
        memcpy(gamma->green, ramp->green, ramp->size * sizeof(unsigned short));
        memcpy(gamma->blue, ramp->blue, ramp->size * sizeof(unsigned short));

        XRRSetCrtcGamma(_grwl.x11.display, monitor->x11.crtc, gamma);
        XRRFreeGamma(gamma);
    }
    else if (_grwl.x11.vidmode.available)
    {
        XF86VidModeSetGammaRamp(_grwl.x11.display, _grwl.x11.screen, ramp->size, (unsigned short*)ramp->red,
                                (unsigned short*)ramp->green, (unsigned short*)ramp->blue);
    }
    else
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Gamma ramp access not supported by server");
    }
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI RRCrtc grwlGetX11Adapter(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.crtc;
}

GRWLAPI RROutput grwlGetX11Monitor(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.output;
}

#endif // _GRWL_X11
