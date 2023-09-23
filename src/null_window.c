//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#include <stdlib.h>

static void applySizeLimits(_GRWLwindow* window, int* width, int* height)
{
    if (window->numer != GRWL_DONT_CARE && window->denom != GRWL_DONT_CARE)
    {
        const float ratio = (float)window->numer / (float)window->denom;
        *height = (int)(*width / ratio);
    }

    if (window->minwidth != GRWL_DONT_CARE)
    {
        *width = _grwl_max(*width, window->minwidth);
    }
    else if (window->maxwidth != GRWL_DONT_CARE)
    {
        *width = _grwl_min(*width, window->maxwidth);
    }

    if (window->minheight != GRWL_DONT_CARE)
    {
        *height = _grwl_min(*height, window->minheight);
    }
    else if (window->maxheight != GRWL_DONT_CARE)
    {
        *height = _grwl_max(*height, window->maxheight);
    }
}

static void fitToMonitor(_GRWLwindow* window)
{
    GRWLvidmode mode;
    _grwlGetVideoModeNull(window->monitor, &mode);
    _grwlGetMonitorPosNull(window->monitor, &window->null.xpos, &window->null.ypos);
    window->null.width = mode.width;
    window->null.height = mode.height;
}

static void acquireMonitor(_GRWLwindow* window)
{
    _grwlInputMonitorWindow(window->monitor, window);
}

static void releaseMonitor(_GRWLwindow* window)
{
    if (window->monitor->window != window)
    {
        return;
    }

    _grwlInputMonitorWindow(window->monitor, NULL);
}

static int createNativeWindow(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLfbconfig* fbconfig)
{
    if (window->monitor)
    {
        fitToMonitor(window);
    }
    else
    {
        if (wndconfig->xpos == GRWL_ANY_POSITION && wndconfig->ypos == GRWL_ANY_POSITION)
        {
            window->null.xpos = 17;
            window->null.ypos = 17;
        }
        else
        {
            window->null.xpos = wndconfig->xpos;
            window->null.ypos = wndconfig->ypos;
        }

        window->null.width = wndconfig->width;
        window->null.height = wndconfig->height;
    }

    window->null.visible = wndconfig->visible;
    window->null.decorated = wndconfig->decorated;
    window->null.maximized = wndconfig->maximized;
    window->null.floating = wndconfig->floating;
    window->null.transparent = fbconfig->transparent;
    window->null.opacity = 1.f;

    return GRWL_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

GRWLbool _grwlCreateWindowNull(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                               const _GRWLfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig, fbconfig))
    {
        return GRWL_FALSE;
    }

    if (ctxconfig->client != GRWL_NO_API)
    {
        if (ctxconfig->source == GRWL_NATIVE_CONTEXT_API || ctxconfig->source == GRWL_OSMESA_CONTEXT_API)
        {
            if (!_grwlInitOSMesa())
            {
                return GRWL_FALSE;
            }
            if (!_grwlCreateContextOSMesa(window, ctxconfig, fbconfig))
            {
                return GRWL_FALSE;
            }
        }
        else if (ctxconfig->source == GRWL_EGL_CONTEXT_API)
        {
            if (!_grwlInitEGL())
            {
                return GRWL_FALSE;
            }
            if (!_grwlCreateContextEGL(window, ctxconfig, fbconfig))
            {
                return GRWL_FALSE;
            }
        }

        if (!_grwlRefreshContextAttribs(window, ctxconfig))
        {
            return GRWL_FALSE;
        }
    }

    if (wndconfig->mousePassthrough)
    {
        _grwlSetWindowMousePassthroughNull(window, GRWL_TRUE);
    }

    if (window->monitor)
    {
        _grwlShowWindowNull(window);
        _grwlFocusWindowNull(window);
        acquireMonitor(window);

        if (wndconfig->centerCursor)
        {
            _grwlCenterCursorInContentArea(window);
        }
    }
    else
    {
        if (wndconfig->visible)
        {
            _grwlShowWindowNull(window);
            if (wndconfig->focused)
            {
                _grwlFocusWindowNull(window);
            }
        }
    }

    return GRWL_TRUE;
}

void _grwlDestroyWindowNull(_GRWLwindow* window)
{
    if (window->monitor)
    {
        releaseMonitor(window);
    }

    if (_grwl.null.focusedWindow == window)
    {
        _grwl.null.focusedWindow = NULL;
    }

    if (window->context.destroy)
    {
        window->context.destroy(window);
    }
}

void _grwlSetWindowTitleNull(_GRWLwindow* window, const char* title)
{
}

void _grwlSetWindowIconNull(_GRWLwindow* window, int count, const GRWLimage* images)
{
}

void _grwlSetWindowProgressIndicatorNull(_GRWLwindow* window, int progressState, double value)
{
}

void _grwlSetWindowBadgeNull(_GRWLwindow* window, int count)
{
}

void _grwlSetWindowBadgeStringNull(_GRWLwindow* window, const char* string)
{
}

void _grwlSetWindowMonitorNull(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                               int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
        {
            _grwlSetWindowPosNull(window, xpos, ypos);
            _grwlSetWindowSizeNull(window, width, height);
        }

        return;
    }

    if (window->monitor)
    {
        releaseMonitor(window);
    }

    _grwlInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        window->null.visible = GRWL_TRUE;
        acquireMonitor(window);
        fitToMonitor(window);
    }
    else
    {
        _grwlSetWindowPosNull(window, xpos, ypos);
        _grwlSetWindowSizeNull(window, width, height);
    }
}

void _grwlGetWindowPosNull(_GRWLwindow* window, int* xpos, int* ypos)
{
    if (xpos)
    {
        *xpos = window->null.xpos;
    }
    if (ypos)
    {
        *ypos = window->null.ypos;
    }
}

void _grwlSetWindowPosNull(_GRWLwindow* window, int xpos, int ypos)
{
    if (window->monitor)
    {
        return;
    }

    if (window->null.xpos != xpos || window->null.ypos != ypos)
    {
        window->null.xpos = xpos;
        window->null.ypos = ypos;
        _grwlInputWindowPos(window, xpos, ypos);
    }
}

void _grwlGetWindowSizeNull(_GRWLwindow* window, int* width, int* height)
{
    if (width)
    {
        *width = window->null.width;
    }
    if (height)
    {
        *height = window->null.height;
    }
}

void _grwlSetWindowSizeNull(_GRWLwindow* window, int width, int height)
{
    if (window->monitor)
    {
        return;
    }

    if (window->null.width != width || window->null.height != height)
    {
        window->null.width = width;
        window->null.height = height;
        _grwlInputWindowSize(window, width, height);
        _grwlInputFramebufferSize(window, width, height);
    }
}

void _grwlSetWindowSizeLimitsNull(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight)
{
    int width = window->null.width;
    int height = window->null.height;
    applySizeLimits(window, &width, &height);
    _grwlSetWindowSizeNull(window, width, height);
}

void _grwlSetWindowAspectRatioNull(_GRWLwindow* window, int n, int d)
{
    int width = window->null.width;
    int height = window->null.height;
    applySizeLimits(window, &width, &height);
    _grwlSetWindowSizeNull(window, width, height);
}

void _grwlGetFramebufferSizeNull(_GRWLwindow* window, int* width, int* height)
{
    if (width)
    {
        *width = window->null.width;
    }
    if (height)
    {
        *height = window->null.height;
    }
}

void _grwlGetWindowFrameSizeNull(_GRWLwindow* window, int* left, int* top, int* right, int* bottom)
{
    if (window->null.decorated && !window->monitor)
    {
        if (left)
        {
            *left = 1;
        }
        if (top)
        {
            *top = 10;
        }
        if (right)
        {
            *right = 1;
        }
        if (bottom)
        {
            *bottom = 1;
        }
    }
    else
    {
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
    }
}

void _grwlGetWindowContentScaleNull(_GRWLwindow* window, float* xscale, float* yscale)
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

void _grwlIconifyWindowNull(_GRWLwindow* window)
{
    if (_grwl.null.focusedWindow == window)
    {
        _grwl.null.focusedWindow = NULL;
        _grwlInputWindowFocus(window, GRWL_FALSE);
    }

    if (!window->null.iconified)
    {
        window->null.iconified = GRWL_TRUE;
        _grwlInputWindowIconify(window, GRWL_TRUE);

        if (window->monitor)
        {
            releaseMonitor(window);
        }
    }
}

void _grwlRestoreWindowNull(_GRWLwindow* window)
{
    if (window->null.iconified)
    {
        window->null.iconified = GRWL_FALSE;
        _grwlInputWindowIconify(window, GRWL_FALSE);

        if (window->monitor)
        {
            acquireMonitor(window);
        }
    }
    else if (window->null.maximized)
    {
        window->null.maximized = GRWL_FALSE;
        _grwlInputWindowMaximize(window, GRWL_FALSE);
    }
}

void _grwlMaximizeWindowNull(_GRWLwindow* window)
{
    if (!window->null.maximized)
    {
        window->null.maximized = GRWL_TRUE;
        _grwlInputWindowMaximize(window, GRWL_TRUE);
    }
}

GRWLbool _grwlWindowMaximizedNull(_GRWLwindow* window)
{
    return window->null.maximized;
}

GRWLbool _grwlWindowHoveredNull(_GRWLwindow* window)
{
    return _grwl.null.xcursor >= window->null.xpos && _grwl.null.ycursor >= window->null.ypos &&
           _grwl.null.xcursor <= window->null.xpos + window->null.width - 1 &&
           _grwl.null.ycursor <= window->null.ypos + window->null.height - 1;
}

GRWLbool _grwlFramebufferTransparentNull(_GRWLwindow* window)
{
    return window->null.transparent;
}

void _grwlSetWindowResizableNull(_GRWLwindow* window, GRWLbool enabled)
{
    window->null.resizable = enabled;
}

void _grwlSetWindowDecoratedNull(_GRWLwindow* window, GRWLbool enabled)
{
    window->null.decorated = enabled;
}

void _grwlSetWindowFloatingNull(_GRWLwindow* window, GRWLbool enabled)
{
    window->null.floating = enabled;
}

void _grwlSetWindowMousePassthroughNull(_GRWLwindow* window, GRWLbool enabled)
{
}

float _grwlGetWindowOpacityNull(_GRWLwindow* window)
{
    return window->null.opacity;
}

void _grwlSetWindowOpacityNull(_GRWLwindow* window, float opacity)
{
    window->null.opacity = opacity;
}

void _grwlSetRawMouseMotionNull(_GRWLwindow* window, GRWLbool enabled)
{
}

GRWLbool _grwlRawMouseMotionSupportedNull(void)
{
    return GRWL_TRUE;
}

void _grwlShowWindowNull(_GRWLwindow* window)
{
    window->null.visible = GRWL_TRUE;
}

void _grwlRequestWindowAttentionNull(_GRWLwindow* window)
{
}

void _grwlHideWindowNull(_GRWLwindow* window)
{
    if (_grwl.null.focusedWindow == window)
    {
        _grwl.null.focusedWindow = NULL;
        _grwlInputWindowFocus(window, GRWL_FALSE);
    }

    window->null.visible = GRWL_FALSE;
}

void _grwlFocusWindowNull(_GRWLwindow* window)
{
    _GRWLwindow* previous;

    if (_grwl.null.focusedWindow == window)
    {
        return;
    }

    if (!window->null.visible)
    {
        return;
    }

    previous = _grwl.null.focusedWindow;
    _grwl.null.focusedWindow = window;

    if (previous)
    {
        _grwlInputWindowFocus(previous, GRWL_FALSE);
        if (previous->monitor && previous->autoIconify)
        {
            _grwlIconifyWindowNull(previous);
        }
    }

    _grwlInputWindowFocus(window, GRWL_TRUE);
}

GRWLbool _grwlWindowFocusedNull(_GRWLwindow* window)
{
    return _grwl.null.focusedWindow == window;
}

GRWLbool _grwlWindowIconifiedNull(_GRWLwindow* window)
{
    return window->null.iconified;
}

GRWLbool _grwlWindowVisibleNull(_GRWLwindow* window)
{
    return window->null.visible;
}

void _grwlPollEventsNull(void)
{
}

void _grwlWaitEventsNull(void)
{
}

void _grwlWaitEventsTimeoutNull(double timeout)
{
}

void _grwlPostEmptyEventNull(void)
{
}

void _grwlGetCursorPosNull(_GRWLwindow* window, double* xpos, double* ypos)
{
    if (xpos)
    {
        *xpos = _grwl.null.xcursor - window->null.xpos;
    }
    if (ypos)
    {
        *ypos = _grwl.null.ycursor - window->null.ypos;
    }
}

void _grwlSetCursorPosNull(_GRWLwindow* window, double x, double y)
{
    _grwl.null.xcursor = window->null.xpos + (int)x;
    _grwl.null.ycursor = window->null.ypos + (int)y;
}

void _grwlSetCursorModeNull(_GRWLwindow* window, int mode)
{
}

GRWLbool _grwlCreateCursorNull(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot)
{
    return GRWL_TRUE;
}

GRWLbool _grwlCreateStandardCursorNull(_GRWLcursor* cursor, int shape)
{
    return GRWL_TRUE;
}

void _grwlDestroyCursorNull(_GRWLcursor* cursor)
{
}

void _grwlSetCursorNull(_GRWLwindow* window, _GRWLcursor* cursor)
{
}

void _grwlSetClipboardStringNull(const char* string)
{
    char* copy = _grwl_strdup(string);
    _grwl_free(_grwl.null.clipboardString);
    _grwl.null.clipboardString = copy;
}

const char* _grwlGetClipboardStringNull(void)
{
    return _grwl.null.clipboardString;
}

void _grwlUpdatePreeditCursorRectangleNull(_GRWLwindow* window)
{
}

void _grwlResetPreeditTextNull(_GRWLwindow* window)
{
}

void _grwlSetIMEStatusNull(_GRWLwindow* window, int active)
{
}

int _grwlGetIMEStatusNull(_GRWLwindow* window)
{
    return GRWL_FALSE;
}

EGLenum _grwlGetEGLPlatformNull(EGLint** attribs)
{
    return 0;
}

EGLNativeDisplayType _grwlGetEGLNativeDisplayNull(void)
{
    return 0;
}

EGLNativeWindowType _grwlGetEGLNativeWindowNull(_GRWLwindow* window)
{
    return 0;
}

const char* _grwlGetScancodeNameNull(int scancode)
{
    if (scancode < GRWL_NULL_SC_FIRST || scancode > GRWL_NULL_SC_LAST)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    switch (scancode)
    {
        case GRWL_NULL_SC_APOSTROPHE:
            return "'";
        case GRWL_NULL_SC_COMMA:
            return ",";
        case GRWL_NULL_SC_MINUS:
        case GRWL_NULL_SC_KP_SUBTRACT:
            return "-";
        case GRWL_NULL_SC_PERIOD:
        case GRWL_NULL_SC_KP_DECIMAL:
            return ".";
        case GRWL_NULL_SC_SLASH:
        case GRWL_NULL_SC_KP_DIVIDE:
            return "/";
        case GRWL_NULL_SC_SEMICOLON:
            return ";";
        case GRWL_NULL_SC_EQUAL:
        case GRWL_NULL_SC_KP_EQUAL:
            return "=";
        case GRWL_NULL_SC_LEFT_BRACKET:
            return "[";
        case GRWL_NULL_SC_RIGHT_BRACKET:
            return "]";
        case GRWL_NULL_SC_KP_MULTIPLY:
            return "*";
        case GRWL_NULL_SC_KP_ADD:
            return "+";
        case GRWL_NULL_SC_BACKSLASH:
        case GRWL_NULL_SC_WORLD_1:
        case GRWL_NULL_SC_WORLD_2:
            return "\\";
        case GRWL_NULL_SC_0:
        case GRWL_NULL_SC_KP_0:
            return "0";
        case GRWL_NULL_SC_1:
        case GRWL_NULL_SC_KP_1:
            return "1";
        case GRWL_NULL_SC_2:
        case GRWL_NULL_SC_KP_2:
            return "2";
        case GRWL_NULL_SC_3:
        case GRWL_NULL_SC_KP_3:
            return "3";
        case GRWL_NULL_SC_4:
        case GRWL_NULL_SC_KP_4:
            return "4";
        case GRWL_NULL_SC_5:
        case GRWL_NULL_SC_KP_5:
            return "5";
        case GRWL_NULL_SC_6:
        case GRWL_NULL_SC_KP_6:
            return "6";
        case GRWL_NULL_SC_7:
        case GRWL_NULL_SC_KP_7:
            return "7";
        case GRWL_NULL_SC_8:
        case GRWL_NULL_SC_KP_8:
            return "8";
        case GRWL_NULL_SC_9:
        case GRWL_NULL_SC_KP_9:
            return "9";
        case GRWL_NULL_SC_A:
            return "a";
        case GRWL_NULL_SC_B:
            return "b";
        case GRWL_NULL_SC_C:
            return "c";
        case GRWL_NULL_SC_D:
            return "d";
        case GRWL_NULL_SC_E:
            return "e";
        case GRWL_NULL_SC_F:
            return "f";
        case GRWL_NULL_SC_G:
            return "g";
        case GRWL_NULL_SC_H:
            return "h";
        case GRWL_NULL_SC_I:
            return "i";
        case GRWL_NULL_SC_J:
            return "j";
        case GRWL_NULL_SC_K:
            return "k";
        case GRWL_NULL_SC_L:
            return "l";
        case GRWL_NULL_SC_M:
            return "m";
        case GRWL_NULL_SC_N:
            return "n";
        case GRWL_NULL_SC_O:
            return "o";
        case GRWL_NULL_SC_P:
            return "p";
        case GRWL_NULL_SC_Q:
            return "q";
        case GRWL_NULL_SC_R:
            return "r";
        case GRWL_NULL_SC_S:
            return "s";
        case GRWL_NULL_SC_T:
            return "t";
        case GRWL_NULL_SC_U:
            return "u";
        case GRWL_NULL_SC_V:
            return "v";
        case GRWL_NULL_SC_W:
            return "w";
        case GRWL_NULL_SC_X:
            return "x";
        case GRWL_NULL_SC_Y:
            return "y";
        case GRWL_NULL_SC_Z:
            return "z";
    }

    return NULL;
}

int _grwlGetKeyScancodeNull(int key)
{
    return _grwl.null.scancodes[key];
}

const char* _grwlGetKeyboardLayoutNameNull(void)
{
    return "";
}

void _grwlGetRequiredInstanceExtensionsNull(char** extensions)
{
}

GRWLbool _grwlGetPhysicalDevicePresentationSupportNull(VkInstance instance, VkPhysicalDevice device,
                                                       uint32_t queuefamily)
{
    return GRWL_FALSE;
}

VkResult _grwlCreateWindowSurfaceNull(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                      VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

_GRWLusercontext* _grwlCreateUserContextNull(_GRWLwindow* window)
{
    if (window->context.osmesa.handle)
    {
        return _grwlCreateUserContextOSMesa(window);
    }

    return NULL;
}
