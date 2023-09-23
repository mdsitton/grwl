//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cfloat>

//////////////////////////////////////////////////////////////////////////
//////                         GRWL event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code that a window has lost or received input focus
//
void _grwlInputWindowFocus(_GRWLwindow* window, GRWLbool focused)
{
    assert(window != NULL);
    assert(focused == GRWL_TRUE || focused == GRWL_FALSE);

    if (window->callbacks.focus)
    {
        window->callbacks.focus((GRWLwindow*)window, focused);
    }

    if (!focused)
    {
        int key, button;

        for (key = 0; key <= GRWL_KEY_LAST; key++)
        {
            if (window->keys[key] == GRWL_PRESS)
            {
                const int scancode = _grwl.platform.getKeyScancode(key);
                _grwlInputKey(window, key, scancode, GRWL_RELEASE, 0);
            }
        }

        for (button = 0; button <= GRWL_MOUSE_BUTTON_LAST; button++)
        {
            if (window->mouseButtons[button] == GRWL_PRESS)
            {
                _grwlInputMouseClick(window, button, GRWL_RELEASE, 0);
            }
        }
    }
}

// Notifies shared code that a window has moved
// The position is specified in content area relative screen coordinates
//
void _grwlInputWindowPos(_GRWLwindow* window, int x, int y)
{
    assert(window != NULL);

    if (window->callbacks.pos)
    {
        window->callbacks.pos((GRWLwindow*)window, x, y);
    }
}

// Notifies shared code that a window has been resized
// The size is specified in screen coordinates
//
void _grwlInputWindowSize(_GRWLwindow* window, int width, int height)
{
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    if (window->callbacks.size)
    {
        window->callbacks.size((GRWLwindow*)window, width, height);
    }
}

// Notifies shared code that a window has been iconified or restored
//
void _grwlInputWindowIconify(_GRWLwindow* window, GRWLbool iconified)
{
    assert(window != NULL);
    assert(iconified == GRWL_TRUE || iconified == GRWL_FALSE);

    if (window->callbacks.iconify)
    {
        window->callbacks.iconify((GRWLwindow*)window, iconified);
    }
}

// Notifies shared code that a window has been maximized or restored
//
void _grwlInputWindowMaximize(_GRWLwindow* window, GRWLbool maximized)
{
    assert(window != NULL);
    assert(maximized == GRWL_TRUE || maximized == GRWL_FALSE);

    if (window->callbacks.maximize)
    {
        window->callbacks.maximize((GRWLwindow*)window, maximized);
    }
}

// Notifies shared code that a window framebuffer has been resized
// The size is specified in pixels
//
void _grwlInputFramebufferSize(_GRWLwindow* window, int width, int height)
{
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    if (window->callbacks.fbsize)
    {
        window->callbacks.fbsize((GRWLwindow*)window, width, height);
    }
}

// Notifies shared code that a window content scale has changed
// The scale is specified as the ratio between the current and default DPI
//
void _grwlInputWindowContentScale(_GRWLwindow* window, float xscale, float yscale)
{
    assert(window != NULL);
    assert(xscale > 0.f);
    assert(xscale < FLT_MAX);
    assert(yscale > 0.f);
    assert(yscale < FLT_MAX);

    if (window->callbacks.scale)
    {
        window->callbacks.scale((GRWLwindow*)window, xscale, yscale);
    }
}

// Notifies shared code that the window contents needs updating
//
void _grwlInputWindowDamage(_GRWLwindow* window)
{
    assert(window != NULL);

    if (window->callbacks.refresh)
    {
        window->callbacks.refresh((GRWLwindow*)window);
    }
}

// Notifies shared code that the user wishes to close a window
//
void _grwlInputWindowCloseRequest(_GRWLwindow* window)
{
    assert(window != NULL);

    window->shouldClose = GRWL_TRUE;

    if (window->callbacks.close)
    {
        window->callbacks.close((GRWLwindow*)window);
    }
}

// Notifies shared code that a window has changed its desired monitor
//
void _grwlInputWindowMonitor(_GRWLwindow* window, _GRWLmonitor* monitor)
{
    assert(window != NULL);
    window->monitor = monitor;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI GRWLwindow* grwlCreateWindow(int width, int height, const char* title, GRWLmonitor* monitor, GRWLwindow* share)
{
    _GRWLfbconfig fbconfig;
    _GRWLctxconfig ctxconfig;
    _GRWLwndconfig wndconfig;
    _GRWLwindow* window;

    assert(title != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (width <= 0 || height <= 0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid window size %ix%i", width, height);

        return NULL;
    }

    fbconfig = _grwl.hints.framebuffer;
    ctxconfig = _grwl.hints.context;
    wndconfig = _grwl.hints.window;

    wndconfig.width = width;
    wndconfig.height = height;
    wndconfig.title = title;
    ctxconfig.share = (_GRWLwindow*)share;

    if (!_grwlIsValidContextConfig(&ctxconfig))
    {
        return NULL;
    }

    window = (_GRWLwindow*)_grwl_calloc(1, sizeof(_GRWLwindow));
    window->next = _grwl.windowListHead;
    _grwl.windowListHead = window;

    window->videoMode.width = width;
    window->videoMode.height = height;
    window->videoMode.redBits = fbconfig.redBits;
    window->videoMode.greenBits = fbconfig.greenBits;
    window->videoMode.blueBits = fbconfig.blueBits;
    window->videoMode.refreshRate = _grwl.hints.refreshRate;

    window->monitor = (_GRWLmonitor*)monitor;
    window->resizable = wndconfig.resizable;
    window->decorated = wndconfig.decorated;
    window->autoIconify = wndconfig.autoIconify;
    window->floating = wndconfig.floating;
    window->focusOnShow = wndconfig.focusOnShow;
    window->mousePassthrough = wndconfig.mousePassthrough;
    window->cursorMode = GRWL_CURSOR_NORMAL;

    window->doublebuffer = fbconfig.doublebuffer;

    window->minwidth = GRWL_DONT_CARE;
    window->minheight = GRWL_DONT_CARE;
    window->maxwidth = GRWL_DONT_CARE;
    window->maxheight = GRWL_DONT_CARE;
    window->numer = GRWL_DONT_CARE;
    window->denom = GRWL_DONT_CARE;

    window->preedit.cursorPosX = 0;
    window->preedit.cursorPosY = height;
    window->preedit.cursorWidth = 0;
    window->preedit.cursorHeight = 0;

    if (!_grwl.platform.createWindow(window, &wndconfig, &ctxconfig, &fbconfig))
    {
        grwlDestroyWindow((GRWLwindow*)window);
        return NULL;
    }

    return (GRWLwindow*)window;
}

void grwlDefaultWindowHints(void)
{
    _GRWL_REQUIRE_INIT();

    // The default is OpenGL with minimum version 1.0
    memset(&_grwl.hints.context, 0, sizeof(_grwl.hints.context));
    _grwl.hints.context.client = GRWL_OPENGL_API;
    _grwl.hints.context.source = GRWL_NATIVE_CONTEXT_API;
    _grwl.hints.context.major = 1;
    _grwl.hints.context.minor = 0;

    // The default is a focused, visible, resizable window with decorations
    memset(&_grwl.hints.window, 0, sizeof(_grwl.hints.window));
    _grwl.hints.window.resizable = GRWL_TRUE;
    _grwl.hints.window.visible = GRWL_TRUE;
    _grwl.hints.window.decorated = GRWL_TRUE;
    _grwl.hints.window.focused = GRWL_TRUE;
    _grwl.hints.window.autoIconify = GRWL_TRUE;
    _grwl.hints.window.centerCursor = GRWL_TRUE;
    _grwl.hints.window.focusOnShow = GRWL_TRUE;
    _grwl.hints.window.xpos = GRWL_ANY_POSITION;
    _grwl.hints.window.ypos = GRWL_ANY_POSITION;
    // The default is hard-fullscreen, which is exclusive.
    // Soft-fullscreen is not exclusive and is suitable for applications such as text-editors.
    _grwl.hints.window.softFullscreen = GRWL_FALSE;

    // The default is 24 bits of color, 24 bits of depth and 8 bits of stencil,
    // double buffered
    memset(&_grwl.hints.framebuffer, 0, sizeof(_grwl.hints.framebuffer));
    _grwl.hints.framebuffer.redBits = 8;
    _grwl.hints.framebuffer.greenBits = 8;
    _grwl.hints.framebuffer.blueBits = 8;
    _grwl.hints.framebuffer.alphaBits = 8;
    _grwl.hints.framebuffer.depthBits = 24;
    _grwl.hints.framebuffer.stencilBits = 8;
    _grwl.hints.framebuffer.doublebuffer = GRWL_TRUE;

    // The default is to select the highest available refresh rate
    _grwl.hints.refreshRate = GRWL_DONT_CARE;

    // The default is to use full Retina resolution framebuffers
    _grwl.hints.window.ns.retina = GRWL_TRUE;
}

GRWLAPI void grwlWindowHint(int hint, int value)
{
    _GRWL_REQUIRE_INIT();

    switch (hint)
    {
        case GRWL_RED_BITS:
            _grwl.hints.framebuffer.redBits = value;
            return;
        case GRWL_GREEN_BITS:
            _grwl.hints.framebuffer.greenBits = value;
            return;
        case GRWL_BLUE_BITS:
            _grwl.hints.framebuffer.blueBits = value;
            return;
        case GRWL_ALPHA_BITS:
            _grwl.hints.framebuffer.alphaBits = value;
            return;
        case GRWL_DEPTH_BITS:
            _grwl.hints.framebuffer.depthBits = value;
            return;
        case GRWL_STENCIL_BITS:
            _grwl.hints.framebuffer.stencilBits = value;
            return;
        case GRWL_ACCUM_RED_BITS:
            _grwl.hints.framebuffer.accumRedBits = value;
            return;
        case GRWL_ACCUM_GREEN_BITS:
            _grwl.hints.framebuffer.accumGreenBits = value;
            return;
        case GRWL_ACCUM_BLUE_BITS:
            _grwl.hints.framebuffer.accumBlueBits = value;
            return;
        case GRWL_ACCUM_ALPHA_BITS:
            _grwl.hints.framebuffer.accumAlphaBits = value;
            return;
        case GRWL_AUX_BUFFERS:
            _grwl.hints.framebuffer.auxBuffers = value;
            return;
        case GRWL_STEREO:
            _grwl.hints.framebuffer.stereo = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_DOUBLEBUFFER:
            _grwl.hints.framebuffer.doublebuffer = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_TRANSPARENT_FRAMEBUFFER:
            _grwl.hints.framebuffer.transparent = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_SAMPLES:
            _grwl.hints.framebuffer.samples = value;
            return;
        case GRWL_SRGB_CAPABLE:
            _grwl.hints.framebuffer.sRGB = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_RESIZABLE:
            _grwl.hints.window.resizable = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_DECORATED:
            _grwl.hints.window.decorated = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_FOCUSED:
            _grwl.hints.window.focused = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_AUTO_ICONIFY:
            _grwl.hints.window.autoIconify = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_FLOATING:
            _grwl.hints.window.floating = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_MAXIMIZED:
            _grwl.hints.window.maximized = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_VISIBLE:
            _grwl.hints.window.visible = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_POSITION_X:
            _grwl.hints.window.xpos = value;
            return;
        case GRWL_POSITION_Y:
            _grwl.hints.window.ypos = value;
            return;
        case GRWL_COCOA_RETINA_FRAMEBUFFER:
            _grwl.hints.window.ns.retina = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_WIN32_KEYBOARD_MENU:
            _grwl.hints.window.win32.keymenu = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_WIN32_GENERIC_BADGE:
            _grwl.hints.window.win32.genericBadge = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_COCOA_GRAPHICS_SWITCHING:
            _grwl.hints.context.nsgl.offline = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_SCALE_TO_MONITOR:
            _grwl.hints.window.scaleToMonitor = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_CENTER_CURSOR:
            _grwl.hints.window.centerCursor = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_FOCUS_ON_SHOW:
            _grwl.hints.window.focusOnShow = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_MOUSE_PASSTHROUGH:
            _grwl.hints.window.mousePassthrough = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_SOFT_FULLSCREEN:
            _grwl.hints.window.softFullscreen = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_CLIENT_API:
            _grwl.hints.context.client = value;
            return;
        case GRWL_CONTEXT_CREATION_API:
            _grwl.hints.context.source = value;
            return;
        case GRWL_CONTEXT_VERSION_MAJOR:
            _grwl.hints.context.major = value;
            return;
        case GRWL_CONTEXT_VERSION_MINOR:
            _grwl.hints.context.minor = value;
            return;
        case GRWL_CONTEXT_ROBUSTNESS:
            _grwl.hints.context.robustness = value;
            return;
        case GRWL_OPENGL_FORWARD_COMPAT:
            _grwl.hints.context.forward = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_CONTEXT_DEBUG:
            _grwl.hints.context.debug = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_CONTEXT_NO_ERROR:
            _grwl.hints.context.noerror = value ? GRWL_TRUE : GRWL_FALSE;
            return;
        case GRWL_OPENGL_PROFILE:
            _grwl.hints.context.profile = value;
            return;
        case GRWL_CONTEXT_RELEASE_BEHAVIOR:
            _grwl.hints.context.release = value;
            return;
        case GRWL_REFRESH_RATE:
            _grwl.hints.refreshRate = value;
            return;
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid window hint 0x%08X", hint);
}

GRWLAPI void grwlWindowHintString(int hint, const char* value)
{
    assert(value != NULL);

    _GRWL_REQUIRE_INIT();

    switch (hint)
    {
        case GRWL_COCOA_FRAME_NAME:
            strncpy(_grwl.hints.window.ns.frameName, value, sizeof(_grwl.hints.window.ns.frameName) - 1);
            return;
        case GRWL_X11_CLASS_NAME:
            strncpy(_grwl.hints.window.x11.className, value, sizeof(_grwl.hints.window.x11.className) - 1);
            return;
        case GRWL_X11_INSTANCE_NAME:
            strncpy(_grwl.hints.window.x11.instanceName, value, sizeof(_grwl.hints.window.x11.instanceName) - 1);
            return;
        case GRWL_WAYLAND_APP_ID:
            strncpy(_grwl.hints.window.wl.appId, value, sizeof(_grwl.hints.window.wl.appId) - 1);
            return;
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid window hint string 0x%08X", hint);
}

GRWLAPI void grwlDestroyWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;

    _GRWL_REQUIRE_INIT();

    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
    {
        return;
    }

    // Clear all callbacks to avoid exposing a half torn-down window object
    memset(&window->callbacks, 0, sizeof(window->callbacks));

    // The window's context must not be current on another thread when the
    // window is destroyed
    if (window == _grwlPlatformGetTls(&_grwl.contextSlot))
    {
        grwlMakeContextCurrent(NULL);
    }

    _grwl.platform.destroyWindow(window);

    // Unlink window from global linked list
    {
        _GRWLwindow** prev = &_grwl.windowListHead;

        while (*prev != window)
        {
            prev = &((*prev)->next);
        }

        *prev = window->next;
    }

    // Clear memory for preedit text
    if (window->preedit.text)
    {
        _grwl_free(window->preedit.text);
    }
    if (window->preedit.blockSizes)
    {
        _grwl_free(window->preedit.blockSizes);
    }
    _grwl_free(window);
}

GRWLAPI int grwlWindowShouldClose(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(0);
    return window->shouldClose;
}

GRWLAPI void grwlSetWindowShouldClose(GRWLwindow* handle, int value)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();
    window->shouldClose = value;
}

GRWLAPI void grwlSetWindowTitle(GRWLwindow* handle, const char* title)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);
    assert(title != NULL);

    _GRWL_REQUIRE_INIT();
    _grwl.platform.setWindowTitle(window, title);
}

GRWLAPI void grwlSetWindowIcon(GRWLwindow* handle, int count, const GRWLimage* images)
{
    int i;
    _GRWLwindow* window = (_GRWLwindow*)handle;

    assert(window != NULL);
    assert(count >= 0);
    assert(count == 0 || images != NULL);

    _GRWL_REQUIRE_INIT();

    if (count < 0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid image count for window icon");
        return;
    }

    for (i = 0; i < count; i++)
    {
        assert(images[i].pixels != NULL);

        if (images[i].width <= 0 || images[i].height <= 0)
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Invalid image dimensions for window icon");
            return;
        }
    }

    _grwl.platform.setWindowIcon(window, count, images);
}

GRWLAPI void grwlSetWindowProgressIndicator(GRWLwindow* handle, int progressState, double value)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;

    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (value < 0.0 || value > 1.0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid progress amount for window progress indicator");
        return;
    }

    if (progressState != GRWL_PROGRESS_INDICATOR_DISABLED && progressState != GRWL_PROGRESS_INDICATOR_INDETERMINATE &&
        progressState != GRWL_PROGRESS_INDICATOR_NORMAL && progressState != GRWL_PROGRESS_INDICATOR_ERROR &&
        progressState != GRWL_PROGRESS_INDICATOR_PAUSED)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid progress state 0x%08X", progressState);
        return;
    }

    _grwl.platform.setWindowProgressIndicator(window, progressState, value);
}

GRWLAPI void grwlSetWindowBadge(GRWLwindow* handle, int count)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;

    _GRWL_REQUIRE_INIT();

    if (count < 0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid badge count %d", count);
        return;
    }

    _grwl.platform.setWindowBadge(window, count);
}

GRWLAPI void grwlSetWindowBadgeString(GRWLwindow* handle, const char* string)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;

    _GRWL_REQUIRE_INIT();

    _grwl.platform.setWindowBadgeString(window, string);
}

GRWLAPI void grwlGetWindowPos(GRWLwindow* handle, int* xpos, int* ypos)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    if (xpos)
    {
        *xpos = 0;
    }
    if (ypos)
    {
        *ypos = 0;
    }

    _GRWL_REQUIRE_INIT();
    _grwl.platform.getWindowPos(window, xpos, ypos);
}

GRWLAPI void grwlSetWindowPos(GRWLwindow* handle, int xpos, int ypos)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (window->monitor)
    {
        return;
    }

    _grwl.platform.setWindowPos(window, xpos, ypos);
}

GRWLAPI void grwlGetWindowSize(GRWLwindow* handle, int* width, int* height)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    if (width)
    {
        *width = 0;
    }
    if (height)
    {
        *height = 0;
    }

    _GRWL_REQUIRE_INIT();
    _grwl.platform.getWindowSize(window, width, height);
}

GRWLAPI void grwlSetWindowSize(GRWLwindow* handle, int width, int height)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _GRWL_REQUIRE_INIT();

    window->videoMode.width = width;
    window->videoMode.height = height;

    _grwl.platform.setWindowSize(window, width, height);
}

GRWLAPI void grwlSetWindowSizeLimits(GRWLwindow* handle, int minwidth, int minheight, int maxwidth, int maxheight)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (minwidth != GRWL_DONT_CARE && minheight != GRWL_DONT_CARE)
    {
        if (minwidth < 0 || minheight < 0)
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Invalid window minimum size %ix%i", minwidth, minheight);
            return;
        }
    }

    if (maxwidth != GRWL_DONT_CARE && maxheight != GRWL_DONT_CARE)
    {
        if (maxwidth < 0 || maxheight < 0 || maxwidth < minwidth || maxheight < minheight)
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Invalid window maximum size %ix%i", maxwidth, maxheight);
            return;
        }
    }

    window->minwidth = minwidth;
    window->minheight = minheight;
    window->maxwidth = maxwidth;
    window->maxheight = maxheight;

    if (window->monitor || !window->resizable)
    {
        return;
    }

    _grwl.platform.setWindowSizeLimits(window, minwidth, minheight, maxwidth, maxheight);
}

GRWLAPI void grwlSetWindowAspectRatio(GRWLwindow* handle, int numer, int denom)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);
    assert(numer != 0);
    assert(denom != 0);

    _GRWL_REQUIRE_INIT();

    if (numer != GRWL_DONT_CARE && denom != GRWL_DONT_CARE)
    {
        if (numer <= 0 || denom <= 0)
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Invalid window aspect ratio %i:%i", numer, denom);
            return;
        }
    }

    window->numer = numer;
    window->denom = denom;

    if (window->monitor || !window->resizable)
    {
        return;
    }

    _grwl.platform.setWindowAspectRatio(window, numer, denom);
}

GRWLAPI void grwlGetFramebufferSize(GRWLwindow* handle, int* width, int* height)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    if (width)
    {
        *width = 0;
    }
    if (height)
    {
        *height = 0;
    }

    _GRWL_REQUIRE_INIT();
    _grwl.platform.getFramebufferSize(window, width, height);
}

GRWLAPI void grwlGetWindowFrameSize(GRWLwindow* handle, int* left, int* top, int* right, int* bottom)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    if (left)
    {
        *left = 0;
    }
    if (top)
    {
        *top = 0;
    }
    if (right)
    {
        *right = 0;
    }
    if (bottom)
    {
        *bottom = 0;
    }

    _GRWL_REQUIRE_INIT();
    _grwl.platform.getWindowFrameSize(window, left, top, right, bottom);
}

GRWLAPI void grwlGetWindowContentScale(GRWLwindow* handle, float* xscale, float* yscale)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    if (xscale)
    {
        *xscale = 0.f;
    }
    if (yscale)
    {
        *yscale = 0.f;
    }

    _GRWL_REQUIRE_INIT();
    _grwl.platform.getWindowContentScale(window, xscale, yscale);
}

GRWLAPI float grwlGetWindowOpacity(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(1.f);
    return _grwl.platform.getWindowOpacity(window);
}

GRWLAPI void grwlSetWindowOpacity(GRWLwindow* handle, float opacity)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);
    assert(opacity == opacity);
    assert(opacity >= 0.f);
    assert(opacity <= 1.f);

    _GRWL_REQUIRE_INIT();

    if (opacity != opacity || opacity < 0.f || opacity > 1.f)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid window opacity %f", opacity);
        return;
    }

    _grwl.platform.setWindowOpacity(window, opacity);
}

GRWLAPI void grwlIconifyWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();
    _grwl.platform.iconifyWindow(window);
}

GRWLAPI void grwlRestoreWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();
    _grwl.platform.restoreWindow(window);
}

GRWLAPI void grwlMaximizeWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (window->monitor)
    {
        return;
    }

    _grwl.platform.maximizeWindow(window);
}

GRWLAPI void grwlShowWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (window->monitor)
    {
        return;
    }

    _grwl.platform.showWindow(window);

    if (window->focusOnShow)
    {
        _grwl.platform.focusWindow(window);
    }
}

GRWLAPI void grwlRequestWindowAttention(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    _grwl.platform.requestWindowAttention(window);
}

GRWLAPI void grwlHideWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (window->monitor)
    {
        return;
    }

    _grwl.platform.hideWindow(window);
}

GRWLAPI void grwlFocusWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    _grwl.platform.focusWindow(window);
}

GRWLAPI int grwlGetWindowAttrib(GRWLwindow* handle, int attrib)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(0);

    switch (attrib)
    {
        case GRWL_FOCUSED:
            return _grwl.platform.windowFocused(window);
        case GRWL_ICONIFIED:
            return _grwl.platform.windowIconified(window);
        case GRWL_VISIBLE:
            return _grwl.platform.windowVisible(window);
        case GRWL_MAXIMIZED:
            return _grwl.platform.windowMaximized(window);
        case GRWL_HOVERED:
            return _grwl.platform.windowHovered(window);
        case GRWL_FOCUS_ON_SHOW:
            return window->focusOnShow;
        case GRWL_MOUSE_PASSTHROUGH:
            return window->mousePassthrough;
        case GRWL_TRANSPARENT_FRAMEBUFFER:
            return _grwl.platform.framebufferTransparent(window);
        case GRWL_RESIZABLE:
            return window->resizable;
        case GRWL_DECORATED:
            return window->decorated;
        case GRWL_FLOATING:
            return window->floating;
        case GRWL_AUTO_ICONIFY:
            return window->autoIconify;
        case GRWL_DOUBLEBUFFER:
            return window->doublebuffer;
        case GRWL_CLIENT_API:
            return window->context.client;
        case GRWL_CONTEXT_CREATION_API:
            return window->context.source;
        case GRWL_CONTEXT_VERSION_MAJOR:
            return window->context.major;
        case GRWL_CONTEXT_VERSION_MINOR:
            return window->context.minor;
        case GRWL_CONTEXT_REVISION:
            return window->context.revision;
        case GRWL_CONTEXT_ROBUSTNESS:
            return window->context.robustness;
        case GRWL_OPENGL_FORWARD_COMPAT:
            return window->context.forward;
        case GRWL_CONTEXT_DEBUG:
            return window->context.debug;
        case GRWL_OPENGL_PROFILE:
            return window->context.profile;
        case GRWL_CONTEXT_RELEASE_BEHAVIOR:
            return window->context.release;
        case GRWL_CONTEXT_NO_ERROR:
            return window->context.noerror;
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
    return 0;
}

GRWLAPI void grwlSetWindowAttrib(GRWLwindow* handle, int attrib, int value)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    value = value ? GRWL_TRUE : GRWL_FALSE;

    switch (attrib)
    {
        case GRWL_AUTO_ICONIFY:
            window->autoIconify = value;
            return;

        case GRWL_RESIZABLE:
            window->resizable = value;
            if (!window->monitor)
            {
                _grwl.platform.setWindowResizable(window, value);
            }
            return;

        case GRWL_DECORATED:
            window->decorated = value;
            if (!window->monitor)
            {
                _grwl.platform.setWindowDecorated(window, value);
            }
            return;

        case GRWL_FLOATING:
            window->floating = value;
            if (!window->monitor)
            {
                _grwl.platform.setWindowFloating(window, value);
            }
            return;

        case GRWL_FOCUS_ON_SHOW:
            window->focusOnShow = value;
            return;

        case GRWL_MOUSE_PASSTHROUGH:
            window->mousePassthrough = value;
            _grwl.platform.setWindowMousePassthrough(window, value);
            return;
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
}

GRWLAPI GRWLmonitor* grwlGetWindowMonitor(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return (GRWLmonitor*)window->monitor;
}

GRWLAPI void grwlSetWindowMonitor(GRWLwindow* wh, GRWLmonitor* mh, int xpos, int ypos, int width, int height,
                                  int refreshRate)
{
    _GRWLwindow* window = (_GRWLwindow*)wh;
    _GRWLmonitor* monitor = (_GRWLmonitor*)mh;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _GRWL_REQUIRE_INIT();

    if (width <= 0 || height <= 0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid window size %ix%i", width, height);
        return;
    }

    if (refreshRate < 0 && refreshRate != GRWL_DONT_CARE)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid refresh rate %i", refreshRate);
        return;
    }

    window->videoMode.width = width;
    window->videoMode.height = height;
    window->videoMode.refreshRate = refreshRate;

    _grwl.platform.setWindowMonitor(window, monitor, xpos, ypos, width, height, refreshRate);
}

GRWLAPI void grwlSetWindowUserPointer(GRWLwindow* handle, void* pointer)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();
    window->userPointer = pointer;
}

GRWLAPI void* grwlGetWindowUserPointer(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return window->userPointer;
}

GRWLAPI GRWLwindowposfun grwlSetWindowPosCallback(GRWLwindow* handle, GRWLwindowposfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowposfun, window->callbacks.pos, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowsizefun grwlSetWindowSizeCallback(GRWLwindow* handle, GRWLwindowsizefun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowsizefun, window->callbacks.size, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowclosefun grwlSetWindowCloseCallback(GRWLwindow* handle, GRWLwindowclosefun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowclosefun, window->callbacks.close, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowrefreshfun grwlSetWindowRefreshCallback(GRWLwindow* handle, GRWLwindowrefreshfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowrefreshfun, window->callbacks.refresh, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowfocusfun grwlSetWindowFocusCallback(GRWLwindow* handle, GRWLwindowfocusfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowfocusfun, window->callbacks.focus, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowiconifyfun grwlSetWindowIconifyCallback(GRWLwindow* handle, GRWLwindowiconifyfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowiconifyfun, window->callbacks.iconify, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowmaximizefun grwlSetWindowMaximizeCallback(GRWLwindow* handle, GRWLwindowmaximizefun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowmaximizefun, window->callbacks.maximize, cbfun);
    return cbfun;
}

GRWLAPI GRWLframebuffersizefun grwlSetFramebufferSizeCallback(GRWLwindow* handle, GRWLframebuffersizefun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLframebuffersizefun, window->callbacks.fbsize, cbfun);
    return cbfun;
}

GRWLAPI GRWLwindowcontentscalefun grwlSetWindowContentScaleCallback(GRWLwindow* handle, GRWLwindowcontentscalefun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLwindowcontentscalefun, window->callbacks.scale, cbfun);
    return cbfun;
}

GRWLAPI void grwlPollEvents(void)
{
    _GRWL_REQUIRE_INIT();
    _grwl.platform.pollEvents();
}

GRWLAPI void grwlWaitEvents(void)
{
    _GRWL_REQUIRE_INIT();
    _grwl.platform.waitEvents();
    _grwlPollAllJoysticks();
}

GRWLAPI void grwlWaitEventsTimeout(double timeout)
{
    _GRWL_REQUIRE_INIT();
    assert(timeout == timeout);
    assert(timeout >= 0.0);
    assert(timeout <= DBL_MAX);

    if (timeout != timeout || timeout < 0.0 || timeout > DBL_MAX)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid time %f", timeout);
        return;
    }

    _grwl.platform.waitEventsTimeout(timeout);
}

GRWLAPI void grwlPostEmptyEvent(void)
{
    _GRWL_REQUIRE_INIT();
    _grwl.platform.postEmptyEvent();
}
