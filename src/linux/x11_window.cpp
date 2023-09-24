//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_X11)

    #include <X11/cursorfont.h>
    #include <X11/Xmd.h>

    #include <poll.h>

    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <limits.h>
    #include <errno.h>
    #include <assert.h>

    // Action for EWMH client messages
    #define _NET_WM_STATE_REMOVE 0
    #define _NET_WM_STATE_ADD 1
    #define _NET_WM_STATE_TOGGLE 2

    // Additional mouse button names for XButtonEvent
    #define Button6 6
    #define Button7 7

    // Motif WM hints flags
    #define MWM_HINTS_DECORATIONS 2
    #define MWM_DECOR_ALL 1

    #define _GRWL_XDND_VERSION 5

// Wait for event data to arrive on the X11 display socket
// This avoids blocking other threads via the per-display Xlib lock that also
// covers GLX functions
//
static bool waitForX11Event(double* timeout)
{
    struct pollfd fd = { ConnectionNumber(_grwl.x11.display), POLLIN };

    while (!XPending(_grwl.x11.display))
    {
        if (!_grwlPollPOSIX(&fd, 1, timeout))
        {
            return false;
        }
    }

    return true;
}

// Wait for event data to arrive on any event file descriptor
// This avoids blocking other threads via the per-display Xlib lock that also
// covers GLX functions
//
static bool waitForAnyEvent(double* timeout)
{
    nfds_t count = 2;
    struct pollfd fds[3] = { { ConnectionNumber(_grwl.x11.display), POLLIN }, { _grwl.x11.emptyEventPipe[0], POLLIN } };

    #if defined(GRWL_BUILD_LINUX_JOYSTICK)
    if (_grwl.joysticksInitialized)
    {
        fds[count++] = (struct pollfd) { _grwl.linjs.inotify, POLLIN };
    }
    #endif

    while (!XPending(_grwl.x11.display))
    {
        if (!_grwlPollPOSIX(fds, count, timeout))
        {
            return false;
        }

        for (int i = 1; i < count; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                return true;
            }
        }
    }

    return true;
}

// Writes a byte to the empty event pipe
//
static void writeEmptyEvent()
{
    for (;;)
    {
        const char byte = 0;
        const ssize_t result = write(_grwl.x11.emptyEventPipe[1], &byte, 1);
        if (result == 1 || (result == -1 && errno != EINTR))
        {
            break;
        }
    }
}

// Drains available data from the empty event pipe
//
static void drainEmptyEvents()
{
    for (;;)
    {
        char dummy[64];
        const ssize_t result = read(_grwl.x11.emptyEventPipe[0], dummy, sizeof(dummy));
        if (result == -1 && errno != EINTR)
        {
            break;
        }
    }
}

// Waits until a VisibilityNotify event arrives for the specified window or the
// timeout period elapses (ICCCM section 4.2.2)
//
static bool waitForVisibilityNotify(_GRWLwindow* window)
{
    XEvent dummy;
    double timeout = 0.1;

    while (!XCheckTypedWindowEvent(_grwl.x11.display, window->x11.handle, VisibilityNotify, &dummy))
    {
        if (!waitForX11Event(&timeout))
        {
            return false;
        }
    }

    return true;
}

// Returns whether the window is iconified
//
static int getWindowState(_GRWLwindow* window)
{
    int result = WithdrawnState;

    struct
    {
        CARD32 state;
        Window icon;
    }* state = nullptr;

    if (_grwlGetWindowPropertyX11(window->x11.handle, _grwl.x11.WM_STATE, _grwl.x11.WM_STATE,
                                  (unsigned char**)&state) >= 2)
    {
        result = state->state;
    }

    if (state)
    {
        XFree(state);
    }

    return result;
}

// Returns whether the event is a selection event
//
static Bool isSelectionEvent(Display* display, XEvent* event, XPointer pointer)
{
    if (event->xany.window != _grwl.x11.helperWindowHandle)
    {
        return False;
    }

    return event->type == SelectionRequest || event->type == SelectionNotify || event->type == SelectionClear;
}

// Returns whether it is a _NET_FRAME_EXTENTS event for the specified window
//
static Bool isFrameExtentsEvent(Display* display, XEvent* event, XPointer pointer)
{
    _GRWLwindow* window = (_GRWLwindow*)pointer;
    return event->type == PropertyNotify && event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == window->x11.handle && event->xproperty.atom == _grwl.x11.NET_FRAME_EXTENTS;
}

// Returns whether it is a property event for the specified selection transfer
//
static Bool isSelPropNewValueNotify(Display* display, XEvent* event, XPointer pointer)
{
    XEvent* notification = (XEvent*)pointer;
    return event->type == PropertyNotify && event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == notification->xselection.requestor &&
           event->xproperty.atom == notification->xselection.property;
}

// Translates an X event modifier state mask
//
static int translateState(int state)
{
    int mods = 0;

    if (state & ShiftMask)
    {
        mods |= GRWL_MOD_SHIFT;
    }
    if (state & ControlMask)
    {
        mods |= GRWL_MOD_CONTROL;
    }
    if (state & Mod1Mask)
    {
        mods |= GRWL_MOD_ALT;
    }
    if (state & Mod4Mask)
    {
        mods |= GRWL_MOD_SUPER;
    }
    if (state & LockMask)
    {
        mods |= GRWL_MOD_CAPS_LOCK;
    }
    if (state & Mod2Mask)
    {
        mods |= GRWL_MOD_NUM_LOCK;
    }

    return mods;
}

// Translates an X11 key code to a GRWL key token
//
static int translateKey(int scancode)
{
    // Use the pre-filled LUT (see createKeyTables() in x11_init.c)
    if (scancode < 0 || scancode > 255)
    {
        return GRWL_KEY_UNKNOWN;
    }

    return _grwl.x11.keycodes[scancode];
}

// Sends an EWMH or ICCCM event to the window manager
//
static void sendEventToWM(_GRWLwindow* window, Atom type, long a, long b, long c, long d, long e)
{
    XEvent event = { ClientMessage };
    event.xclient.window = window->x11.handle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = type;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(_grwl.x11.display, _grwl.x11.root, False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
}

// Updates the normal hints according to the window settings
//
static void updateNormalHints(_GRWLwindow* window, int width, int height)
{
    XSizeHints* hints = XAllocSizeHints();

    long supplied;
    XGetWMNormalHints(_grwl.x11.display, window->x11.handle, hints, &supplied);

    hints->flags &= ~(PMinSize | PMaxSize | PAspect);

    if (!window->monitor)
    {
        if (window->resizable)
        {
            if (window->minwidth != GRWL_DONT_CARE && window->minheight != GRWL_DONT_CARE)
            {
                hints->flags |= PMinSize;
                hints->min_width = window->minwidth;
                hints->min_height = window->minheight;
            }

            if (window->maxwidth != GRWL_DONT_CARE && window->maxheight != GRWL_DONT_CARE)
            {
                hints->flags |= PMaxSize;
                hints->max_width = window->maxwidth;
                hints->max_height = window->maxheight;
            }

            if (window->numer != GRWL_DONT_CARE && window->denom != GRWL_DONT_CARE)
            {
                hints->flags |= PAspect;
                hints->min_aspect.x = hints->max_aspect.x = window->numer;
                hints->min_aspect.y = hints->max_aspect.y = window->denom;
            }
        }
        else
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width = hints->max_width = width;
            hints->min_height = hints->max_height = height;
        }
    }

    XSetWMNormalHints(_grwl.x11.display, window->x11.handle, hints);
    XFree(hints);
}

// Updates the full screen status of the window
//
static void updateWindowMode(_GRWLwindow* window)
{
    if (window->monitor)
    {
        if (_grwl.x11.xinerama.available && _grwl.x11.NET_WM_FULLSCREEN_MONITORS)
        {
            sendEventToWM(window, _grwl.x11.NET_WM_FULLSCREEN_MONITORS, window->monitor->x11.index,
                          window->monitor->x11.index, window->monitor->x11.index, window->monitor->x11.index, 0);
        }

        if (_grwl.x11.NET_WM_STATE && _grwl.x11.NET_WM_STATE_FULLSCREEN)
        {
            sendEventToWM(window, _grwl.x11.NET_WM_STATE, _NET_WM_STATE_ADD, _grwl.x11.NET_WM_STATE_FULLSCREEN, 0, 1,
                          0);
        }
        else
        {
            // This is the butcher's way of removing window decorations
            // Setting the override-redirect attribute on a window makes the
            // window manager ignore the window completely (ICCCM, section 4)
            // The good thing is that this makes undecorated full screen windows
            // easy to do; the bad thing is that we have to do everything
            // manually and some things (like iconify/restore) won't work at
            // all, as those are tasks usually performed by the window manager

            XSetWindowAttributes attributes;
            attributes.override_redirect = True;
            XChangeWindowAttributes(_grwl.x11.display, window->x11.handle, CWOverrideRedirect, &attributes);

            window->x11.overrideRedirect = true;
        }

        // Enable compositor bypass
        if (!window->x11.transparent)
        {
            const unsigned long value = _grwl.hints.window.softFullscreen ? 2 : 1;

            XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
                            PropModeReplace, (unsigned char*)&value, 1);
        }
    }
    else
    {
        if (_grwl.x11.xinerama.available && _grwl.x11.NET_WM_FULLSCREEN_MONITORS)
        {
            XDeleteProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_FULLSCREEN_MONITORS);
        }

        if (_grwl.x11.NET_WM_STATE && _grwl.x11.NET_WM_STATE_FULLSCREEN)
        {
            sendEventToWM(window, _grwl.x11.NET_WM_STATE, _NET_WM_STATE_REMOVE, _grwl.x11.NET_WM_STATE_FULLSCREEN, 0, 1,
                          0);
        }
        else
        {
            XSetWindowAttributes attributes;
            attributes.override_redirect = False;
            XChangeWindowAttributes(_grwl.x11.display, window->x11.handle, CWOverrideRedirect, &attributes);

            window->x11.overrideRedirect = false;
        }

        // Disable compositor bypass
        if (!window->x11.transparent)
        {
            XDeleteProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_BYPASS_COMPOSITOR);
        }
    }
}

// Convert the specified Latin-1 string to UTF-8
//
static char* convertLatin1toUTF8(const char* source)
{
    size_t size = 1;

    for (const char* sp = source; *sp; sp++)
    {
        size += (*sp & 0x80) ? 2 : 1;
    }

    char* target = (char*)_grwl_calloc(size, 1);
    char* tp = target;

    for (const char* sp = source; *sp; sp++)
    {
        tp += _grwlEncodeUTF8(tp, *sp);
    }

    return target;
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_GRWLwindow* window)
{
    if (window->cursorMode == GRWL_CURSOR_NORMAL || window->cursorMode == GRWL_CURSOR_CAPTURED)
    {
        if (window->cursor)
        {
            XDefineCursor(_grwl.x11.display, window->x11.handle, window->cursor->x11.handle);
        }
        else
        {
            XUndefineCursor(_grwl.x11.display, window->x11.handle);
        }
    }
    else
    {
        XDefineCursor(_grwl.x11.display, window->x11.handle, _grwl.x11.hiddenCursorHandle);
    }
}

// Grabs the cursor and confines it to the window
//
static void captureCursor(_GRWLwindow* window)
{
    XGrabPointer(_grwl.x11.display, window->x11.handle, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, window->x11.handle, None, CurrentTime);
}

// Ungrabs the cursor
//
static void releaseCursor()
{
    XUngrabPointer(_grwl.x11.display, CurrentTime);
}

// Enable XI2 raw mouse motion events
//
static void enableRawMouseMotion(_GRWLwindow* window)
{
    XIEventMask em;
    unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };

    em.deviceid = XIAllMasterDevices;
    em.mask_len = sizeof(mask);
    em.mask = mask;
    XISetMask(mask, XI_RawMotion);

    XISelectEvents(_grwl.x11.display, _grwl.x11.root, &em, 1);
}

// Disable XI2 raw mouse motion events
//
static void disableRawMouseMotion(_GRWLwindow* window)
{
    XIEventMask em;
    unsigned char mask[] = { 0 };

    em.deviceid = XIAllMasterDevices;
    em.mask_len = sizeof(mask);
    em.mask = mask;

    XISelectEvents(_grwl.x11.display, _grwl.x11.root, &em, 1);
}

// Apply disabled cursor mode to a focused window
//
static void disableCursor(_GRWLwindow* window)
{
    if (window->rawMouseMotion)
    {
        enableRawMouseMotion(window);
    }

    _grwl.x11.disabledCursorWindow = window;
    _grwlGetCursorPosX11(window, &_grwl.x11.restoreCursorPosX, &_grwl.x11.restoreCursorPosY);
    updateCursorImage(window);
    _grwlCenterCursorInContentArea(window);
    captureCursor(window);
}

// Exit disabled cursor mode for the specified window
//
static void enableCursor(_GRWLwindow* window)
{
    if (window->rawMouseMotion)
    {
        disableRawMouseMotion(window);
    }

    _grwl.x11.disabledCursorWindow = nullptr;
    releaseCursor();
    _grwlSetCursorPosX11(window, _grwl.x11.restoreCursorPosX, _grwl.x11.restoreCursorPosY);
    updateCursorImage(window);
}

// Clear its handle when the input context has been destroyed
//
static void inputContextDestroyCallback(XIC ic, XPointer clientData, XPointer callData)
{
    _GRWLwindow* window = (_GRWLwindow*)clientData;
    window->x11.ic = nullptr;
}

// IME Start callback (do nothing)
//
static void _ximPreeditStartCallback(XIC xic, XPointer clientData, XPointer callData)
{
}

// IME Done callback (do nothing)
//
static void _ximPreeditDoneCallback(XIC xic, XPointer clientData, XPointer callData)
{
}

// IME Draw callback
// When using the dafault style: STYLE_OVERTHESPOT, this is not used since applications
// don't need to display preedit texts.
//
static void _ximPreeditDrawCallback(XIC xic, XPointer clientData, XIMPreeditDrawCallbackStruct* callData)
{
    _GRWLwindow* window = (_GRWLwindow*)clientData;
    _GRWLpreedit* preedit = &window->preedit;

    if (!callData->text)
    {
        // preedit text is empty
        preedit->textCount = 0;
        preedit->blockSizesCount = 0;
        preedit->focusedBlockIndex = 0;
        preedit->caretIndex = 0;
        _grwlInputPreedit(window);
        return;
    }
    else if (callData->text->encoding_is_wchar)
    {
        // wchar is not supported
        return;
    }
    else
    {
        XIMText* text = callData->text;
        int textLen = preedit->textCount + text->length - callData->chg_length;
        int textBufferCount = preedit->textBufferCount;
        int rstart, rend;
        const char* src;

        // realloc preedit text
        while (textBufferCount < textLen + 1)
        {
            textBufferCount = (textBufferCount == 0) ? 1 : textBufferCount * 2;
        }
        if (textBufferCount != preedit->textBufferCount)
        {
            unsigned int* preeditText =
                (unsigned int*)_grwl_realloc(preedit->text, sizeof(unsigned int) * textBufferCount);
            if (preeditText == nullptr)
            {
                return;
            }

            preedit->text = preeditText;
            preedit->textBufferCount = textBufferCount;
        }
        preedit->textCount = textLen;
        preedit->text[textLen] = 0;

        // realloc block sizes
        if (preedit->blockSizesBufferCount == 0)
        {
            preedit->blockSizes = (int*)_grwl_calloc(4, sizeof(int));
            preedit->blockSizesBufferCount = 4;
        }

        // store preedit text
        src = text->string.multi_byte;
        rend = 0;
        rstart = textLen;
        for (int i = 0, j = callData->chg_first; i < text->length; i++)
        {
            if (i < callData->chg_first || callData->chg_first + textLen < i)
            {
                continue;
            }

            preedit->text[j++] = _grwlDecodeUTF8(&src);
            XIMFeedback f = text->feedback[i];
            if ((f & XIMReverse) || (f & XIMHighlight))
            {
                rend = i;
                if (i < rstart)
                {
                    rstart = i;
                }
            }
        }

        // store block sizes
        // TODO: It doesn't care callData->chg_first != 0 case although it's quite rare.
        if (rstart == textLen)
        {
            preedit->blockSizesCount = 1;
            preedit->blockSizes[0] = textLen;
            preedit->blockSizes[1] = 0;
            preedit->focusedBlockIndex = 0;
            preedit->caretIndex = callData->caret;
            _grwlInputPreedit(window);
        }
        else if (rstart == 0)
        {
            if (rend == textLen - 1)
            {
                preedit->blockSizesCount = 1;
                preedit->blockSizes[0] = textLen;
                preedit->blockSizes[1] = 0;
                preedit->focusedBlockIndex = 0;
                preedit->caretIndex = callData->caret;
                _grwlInputPreedit(window);
            }
            else
            {
                preedit->blockSizesCount = 2;
                preedit->blockSizes[0] = rend + 1;
                preedit->blockSizes[1] = textLen - rend - 1;
                preedit->blockSizes[2] = 0;
                preedit->focusedBlockIndex = 0;
                preedit->caretIndex = callData->caret;
                _grwlInputPreedit(window);
            }
        }
        else if (rend == textLen - 1)
        {
            preedit->blockSizesCount = 2;
            preedit->blockSizes[0] = rstart;
            preedit->blockSizes[1] = textLen - rstart;
            preedit->blockSizes[2] = 0;
            preedit->focusedBlockIndex = 1;
            preedit->caretIndex = callData->caret;
            _grwlInputPreedit(window);
        }
        else
        {
            preedit->blockSizesCount = 3;
            preedit->blockSizes[0] = rstart;
            preedit->blockSizes[1] = rend - rstart + 1;
            preedit->blockSizes[2] = textLen - rend - 1;
            preedit->blockSizes[3] = 0;
            preedit->focusedBlockIndex = 1;
            preedit->caretIndex = callData->caret;
            _grwlInputPreedit(window);
        }
    }
}

// IME Caret callback (do nothing)
//
static void _ximPreeditCaretCallback(XIC xic, XPointer clientData, XPointer callData)
{
}

// IME Status Start callback
// When using the dafault style: STYLE_OVERTHESPOT, this is not used and the IME status
// can not be taken.
//
static void _ximStatusStartCallback(XIC xic, XPointer clientData, XPointer callData)
{
    _GRWLwindow* window = (_GRWLwindow*)clientData;
    window->x11.imeFocus = true;
}

// IME Status Done callback
// When using the dafault style: STYLE_OVERTHESPOT, this is not used and the IME status
// can not be taken.
//
static void _ximStatusDoneCallback(XIC xic, XPointer clientData, XPointer callData)
{
    _GRWLwindow* window = (_GRWLwindow*)clientData;
    window->x11.imeFocus = false;
}

// IME Status Draw callback
// When using the dafault style: STYLE_OVERTHESPOT, this is not used and the IME status
// can not be taken.
//
static void _ximStatusDrawCallback(XIC xic, XPointer clientData, XIMStatusDrawCallbackStruct* callData)
{
    _GRWLwindow* window = (_GRWLwindow*)clientData;
    _grwlInputIMEStatus(window);
}

// Create XIM Preedit callback
// When using the dafault style: STYLE_OVERTHESPOT, this is not used since applications
// don't need to display preedit texts.
//
static XVaNestedList _createXIMPreeditCallbacks(_GRWLwindow* window)
{
    window->x11.preeditStartCallback.client_data = (XPointer)window;
    window->x11.preeditStartCallback.callback = (XIMProc)_ximPreeditStartCallback;
    window->x11.preeditDoneCallback.client_data = (XPointer)window;
    window->x11.preeditDoneCallback.callback = (XIMProc)_ximPreeditDoneCallback;
    window->x11.preeditDrawCallback.client_data = (XPointer)window;
    window->x11.preeditDrawCallback.callback = (XIMProc)_ximPreeditDrawCallback;
    window->x11.preeditCaretCallback.client_data = (XPointer)window;
    window->x11.preeditCaretCallback.callback = (XIMProc)_ximPreeditCaretCallback;
    return XVaCreateNestedList(0, XNPreeditStartCallback, &window->x11.preeditStartCallback.client_data,
                               XNPreeditDoneCallback, &window->x11.preeditDoneCallback.client_data,
                               XNPreeditDrawCallback, &window->x11.preeditDrawCallback.client_data,
                               XNPreeditCaretCallback, &window->x11.preeditCaretCallback.client_data, nullptr);
}

// Create XIM status callback
// When using the dafault style: STYLE_OVERTHESPOT, this is not used and the IME status
// can not be taken.
//
static XVaNestedList _createXIMStatusCallbacks(_GRWLwindow* window)
{
    window->x11.statusStartCallback.client_data = (XPointer)window;
    window->x11.statusStartCallback.callback = (XIMProc)_ximStatusStartCallback;
    window->x11.statusDoneCallback.client_data = (XPointer)window;
    window->x11.statusDoneCallback.callback = (XIMProc)_ximStatusDoneCallback;
    window->x11.statusDrawCallback.client_data = (XPointer)window;
    window->x11.statusDrawCallback.callback = (XIMProc)_ximStatusDrawCallback;
    return XVaCreateNestedList(0, XNStatusStartCallback, &window->x11.statusStartCallback.client_data,
                               XNStatusDoneCallback, &window->x11.statusDoneCallback.client_data, XNStatusDrawCallback,
                               &window->x11.statusDrawCallback.client_data, nullptr);
}

// Create the X11 window (and its colormap)
//
static bool createNativeWindow(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, Visual* visual, int depth)
{
    int width = wndconfig->width;
    int height = wndconfig->height;

    if (wndconfig->scaleToMonitor)
    {
        width *= _grwl.x11.contentScaleX;
        height *= _grwl.x11.contentScaleY;
    }

    int xpos = 0, ypos = 0;

    if (wndconfig->xpos != GRWL_ANY_POSITION && wndconfig->ypos != GRWL_ANY_POSITION)
    {
        xpos = wndconfig->xpos;
        ypos = wndconfig->ypos;
    }

    // Create a colormap based on the visual used by the current context
    window->x11.colormap = XCreateColormap(_grwl.x11.display, _grwl.x11.root, visual, AllocNone);

    window->x11.transparent = _grwlIsVisualTransparentX11(visual);

    XSetWindowAttributes wa = { 0 };
    wa.colormap = window->x11.colormap;
    wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask |
                    ButtonReleaseMask | ExposureMask | FocusChangeMask | VisibilityChangeMask | EnterWindowMask |
                    LeaveWindowMask | PropertyChangeMask;

    _grwlGrabErrorHandlerX11();

    window->x11.parent = _grwl.x11.root;
    window->x11.handle = XCreateWindow(_grwl.x11.display, _grwl.x11.root, xpos, ypos, width, height,
                                       0,     // Border width
                                       depth, // Color depth
                                       InputOutput, visual, CWBorderPixel | CWColormap | CWEventMask, &wa);

    _grwlReleaseErrorHandlerX11();

    if (!window->x11.handle)
    {
        _grwlInputErrorX11(GRWL_PLATFORM_ERROR, "X11: Failed to create window");
        return false;
    }

    XSaveContext(_grwl.x11.display, window->x11.handle, _grwl.x11.context, (XPointer)window);

    if (!wndconfig->decorated)
    {
        _grwlSetWindowDecoratedX11(window, false);
    }

    if (_grwl.x11.NET_WM_STATE && !window->monitor)
    {
        Atom states[3];
        int count = 0;

        if (wndconfig->floating)
        {
            if (_grwl.x11.NET_WM_STATE_ABOVE)
            {
                states[count++] = _grwl.x11.NET_WM_STATE_ABOVE;
            }
        }

        if (wndconfig->maximized)
        {
            if (_grwl.x11.NET_WM_STATE_MAXIMIZED_VERT && _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ)
            {
                states[count++] = _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT;
                states[count++] = _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ;
                window->x11.maximized = true;
            }
        }

        if (count)
        {
            XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, 32, PropModeReplace,
                            (unsigned char*)states, count);
        }
    }

    // Declare the WM protocols supported by GRWL
    {
        Atom protocols[] = { _grwl.x11.WM_DELETE_WINDOW, _grwl.x11.NET_WM_PING };

        XSetWMProtocols(_grwl.x11.display, window->x11.handle, protocols, sizeof(protocols) / sizeof(Atom));
    }

    // Declare our PID
    {
        const long pid = getpid();

        XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_PID, XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char*)&pid, 1);
    }

    if (_grwl.x11.NET_WM_WINDOW_TYPE && _grwl.x11.NET_WM_WINDOW_TYPE_NORMAL)
    {
        Atom type = _grwl.x11.NET_WM_WINDOW_TYPE_NORMAL;
        XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&type, 1);
    }

    // Set ICCCM WM_HINTS property
    {
        XWMHints* hints = XAllocWMHints();
        if (!hints)
        {
            _grwlInputError(GRWL_OUT_OF_MEMORY, "X11: Failed to allocate WM hints");
            return false;
        }

        hints->flags = StateHint;
        hints->initial_state = NormalState;

        XSetWMHints(_grwl.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    // Set ICCCM WM_NORMAL_HINTS property
    {
        XSizeHints* hints = XAllocSizeHints();
        if (!hints)
        {
            _grwlInputError(GRWL_OUT_OF_MEMORY, "X11: Failed to allocate size hints");
            return false;
        }

        if (!wndconfig->resizable)
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width = hints->max_width = width;
            hints->min_height = hints->max_height = height;
        }

        // HACK: Explicitly setting PPosition to any value causes some WMs, notably
        //       Compiz and Metacity, to honor the position of unmapped windows
        if (wndconfig->xpos != GRWL_ANY_POSITION && wndconfig->ypos != GRWL_ANY_POSITION)
        {
            hints->flags |= PPosition;
            hints->x = 0;
            hints->y = 0;
        }

        hints->flags |= PWinGravity;
        hints->win_gravity = StaticGravity;

        XSetWMNormalHints(_grwl.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    // Set ICCCM WM_CLASS property
    {
        XClassHint* hint = XAllocClassHint();

        if (strlen(wndconfig->x11.instanceName) && strlen(wndconfig->x11.className))
        {
            hint->res_name = (char*)wndconfig->x11.instanceName;
            hint->res_class = (char*)wndconfig->x11.className;
        }
        else
        {
            const char* resourceName = getenv("RESOURCE_NAME");
            if (resourceName && strlen(resourceName))
            {
                hint->res_name = (char*)resourceName;
            }
            else if (strlen(wndconfig->title))
            {
                hint->res_name = (char*)wndconfig->title;
            }
            else
            {
                hint->res_name = (char*)"grwl-application";
            }

            if (strlen(wndconfig->title))
            {
                hint->res_class = (char*)wndconfig->title;
            }
            else
            {
                hint->res_class = (char*)"GRWL-Application";
            }
        }

        XSetClassHint(_grwl.x11.display, window->x11.handle, hint);
        XFree(hint);
    }

    // Announce support for Xdnd (drag and drop)
    {
        const Atom version = _GRWL_XDND_VERSION;
        XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.XdndAware, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)&version, 1);
    }

    if (_grwl.x11.im)
    {
        _grwlCreateInputContextX11(window);
    }

    _grwlSetWindowTitleX11(window, wndconfig->title);
    _grwlGetWindowPosX11(window, &window->x11.xpos, &window->x11.ypos);
    _grwlGetWindowSizeX11(window, &window->x11.width, &window->x11.height);

    return true;
}

// Set the specified property to the selection converted to the requested target
//
static Atom writeTargetToProperty(const XSelectionRequestEvent* request)
{
    char* selectionString = nullptr;
    const Atom formats[] = { _grwl.x11.UTF8_STRING, XA_STRING };
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->selection == _grwl.x11.PRIMARY)
    {
        selectionString = _grwl.x11.primarySelectionString;
    }
    else
    {
        selectionString = _grwl.x11.clipboardString;
    }

    if (request->property == None)
    {
        // The requester is a legacy client (ICCCM section 2.2)
        // We don't support legacy clients, so fail here
        return None;
    }

    if (request->target == _grwl.x11.TARGETS)
    {
        // The list of supported targets was requested

        const Atom targets[] = { _grwl.x11.TARGETS, _grwl.x11.MULTIPLE, _grwl.x11.UTF8_STRING, XA_STRING };

        XChangeProperty(_grwl.x11.display, request->requestor, request->property, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)targets, sizeof(targets) / sizeof(targets[0]));

        return request->property;
    }

    if (request->target == _grwl.x11.MULTIPLE)
    {
        // Multiple conversions were requested

        Atom* targets;
        const unsigned long count = _grwlGetWindowPropertyX11(request->requestor, request->property,
                                                              _grwl.x11.ATOM_PAIR, (unsigned char**)&targets);

        for (unsigned long i = 0; i < count; i += 2)
        {
            int j;

            for (j = 0; j < formatCount; j++)
            {
                if (targets[i] == formats[j])
                {
                    break;
                }
            }

            if (j < formatCount)
            {
                XChangeProperty(_grwl.x11.display, request->requestor, targets[i + 1], targets[i], 8, PropModeReplace,
                                (unsigned char*)selectionString, strlen(selectionString));
            }
            else
            {
                targets[i + 1] = None;
            }
        }

        XChangeProperty(_grwl.x11.display, request->requestor, request->property, _grwl.x11.ATOM_PAIR, 32,
                        PropModeReplace, (unsigned char*)targets, count);

        XFree(targets);

        return request->property;
    }

    if (request->target == _grwl.x11.SAVE_TARGETS)
    {
        // The request is a check whether we support SAVE_TARGETS
        // It should be handled as a no-op side effect target

        XChangeProperty(_grwl.x11.display, request->requestor, request->property, _grwl.x11.NULL_, 32, PropModeReplace,
                        nullptr, 0);

        return request->property;
    }

    // Conversion to a data target was requested

    for (int i = 0; i < formatCount; i++)
    {
        if (request->target == formats[i])
        {
            // The requested target is one we support

            XChangeProperty(_grwl.x11.display, request->requestor, request->property, request->target, 8,
                            PropModeReplace, (unsigned char*)selectionString, strlen(selectionString));

            return request->property;
        }
    }

    // The requested target is not supported

    return None;
}

static void handleSelectionRequest(XEvent* event)
{
    const XSelectionRequestEvent* request = &event->xselectionrequest;

    XEvent reply = { SelectionNotify };
    reply.xselection.property = writeTargetToProperty(request);
    reply.xselection.display = request->display;
    reply.xselection.requestor = request->requestor;
    reply.xselection.selection = request->selection;
    reply.xselection.target = request->target;
    reply.xselection.time = request->time;

    XSendEvent(_grwl.x11.display, request->requestor, False, 0, &reply);
}

static const char* getSelectionString(Atom selection)
{
    char** selectionString = nullptr;
    const Atom targets[] = { _grwl.x11.UTF8_STRING, XA_STRING };
    const size_t targetCount = sizeof(targets) / sizeof(targets[0]);

    if (selection == _grwl.x11.PRIMARY)
    {
        selectionString = &_grwl.x11.primarySelectionString;
    }
    else
    {
        selectionString = &_grwl.x11.clipboardString;
    }

    if (XGetSelectionOwner(_grwl.x11.display, selection) == _grwl.x11.helperWindowHandle)
    {
        // Instead of doing a large number of X round-trips just to put this
        // string into a window property and then read it back, just return it
        return *selectionString;
    }

    _grwl_free(*selectionString);
    *selectionString = nullptr;

    for (size_t i = 0; i < targetCount; i++)
    {
        char* data;
        Atom actualType;
        int actualFormat;
        unsigned long itemCount, bytesAfter;
        XEvent notification, dummy;

        XConvertSelection(_grwl.x11.display, selection, targets[i], _grwl.x11.GRWL_SELECTION,
                          _grwl.x11.helperWindowHandle, CurrentTime);

        while (!XCheckTypedWindowEvent(_grwl.x11.display, _grwl.x11.helperWindowHandle, SelectionNotify, &notification))
        {
            waitForX11Event(nullptr);
        }

        if (notification.xselection.property == None)
        {
            continue;
        }

        XCheckIfEvent(_grwl.x11.display, &dummy, isSelPropNewValueNotify, (XPointer)&notification);

        XGetWindowProperty(_grwl.x11.display, notification.xselection.requestor, notification.xselection.property, 0,
                           LONG_MAX, True, AnyPropertyType, &actualType, &actualFormat, &itemCount, &bytesAfter,
                           (unsigned char**)&data);

        if (actualType == _grwl.x11.INCR)
        {
            size_t size = 1;
            char* string = nullptr;

            for (;;)
            {
                while (!XCheckIfEvent(_grwl.x11.display, &dummy, isSelPropNewValueNotify, (XPointer)&notification))
                {
                    waitForX11Event(nullptr);
                }

                XFree(data);
                XGetWindowProperty(_grwl.x11.display, notification.xselection.requestor,
                                   notification.xselection.property, 0, LONG_MAX, True, AnyPropertyType, &actualType,
                                   &actualFormat, &itemCount, &bytesAfter, (unsigned char**)&data);

                if (itemCount)
                {
                    size += itemCount;
                    string = (char*)_grwl_realloc(string, size);
                    string[size - itemCount - 1] = '\0';
                    strcat(string, data);
                }

                if (!itemCount)
                {
                    if (string)
                    {
                        if (targets[i] == XA_STRING)
                        {
                            *selectionString = convertLatin1toUTF8(string);
                            _grwl_free(string);
                        }
                        else
                        {
                            *selectionString = string;
                        }
                    }

                    break;
                }
            }
        }
        else if (actualType == targets[i])
        {
            if (targets[i] == XA_STRING)
            {
                *selectionString = convertLatin1toUTF8(data);
            }
            else
            {
                *selectionString = _grwl_strdup(data);
            }
        }

        XFree(data);

        if (*selectionString)
        {
            break;
        }
    }

    if (!*selectionString)
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "X11: Failed to convert selection to string");
    }

    return *selectionString;
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_GRWLwindow* window)
{
    if (_grwl.x11.saver.count == 0)
    {
        // Remember old screen saver settings
        XGetScreenSaver(_grwl.x11.display, &_grwl.x11.saver.timeout, &_grwl.x11.saver.interval,
                        &_grwl.x11.saver.blanking, &_grwl.x11.saver.exposure);

        // Disable screen saver
        XSetScreenSaver(_grwl.x11.display, 0, 0, DontPreferBlanking, DefaultExposures);
    }

    if (!window->monitor->window)
    {
        _grwl.x11.saver.count++;
    }

    _grwlSetVideoModeX11(window->monitor, &window->videoMode);

    if (window->x11.overrideRedirect)
    {
        int xpos, ypos;
        GRWLvidmode mode;

        // Manually position the window over its monitor
        _grwlGetMonitorPosX11(window->monitor, &xpos, &ypos);
        _grwlGetVideoModeX11(window->monitor, &mode);

        XMoveResizeWindow(_grwl.x11.display, window->x11.handle, xpos, ypos, mode.width, mode.height);
    }

    _grwlInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_GRWLwindow* window)
{
    if (window->monitor->window != window)
    {
        return;
    }

    _grwlInputMonitorWindow(window->monitor, nullptr);
    _grwlRestoreVideoModeX11(window->monitor);

    _grwl.x11.saver.count--;

    if (_grwl.x11.saver.count == 0)
    {
        // Restore old screen saver settings
        XSetScreenSaver(_grwl.x11.display, _grwl.x11.saver.timeout, _grwl.x11.saver.interval, _grwl.x11.saver.blanking,
                        _grwl.x11.saver.exposure);
    }
}

// Process the specified X event
//
static void processEvent(XEvent* event)
{
    int keycode = 0;

    // HACK: Save scancode as some IMs clear the field in XFilterEvent
    if (event->type == KeyPress || event->type == KeyRelease)
    {
        keycode = event->xkey.keycode;
    }

    if (XFilterEvent(event, None))
    {
        return;
    }

    if (_grwl.x11.randr.available)
    {
        if (event->type == _grwl.x11.randr.eventBase + RRNotify)
        {
            XRRUpdateConfiguration(event);
            _grwlPollMonitorsX11();
            return;
        }
    }

    if (_grwl.x11.xkb.available)
    {
        if (event->type == _grwl.x11.xkb.eventBase + XkbEventCode)
        {
            if (((XkbEvent*)event)->any.xkb_type == XkbStateNotify &&
                (((XkbEvent*)event)->state.changed & XkbGroupStateMask))
            {
                _grwl.x11.xkb.group = ((XkbEvent*)event)->state.group;
                _grwlInputKeyboardLayout();
            }

            return;
        }
    }

    if (event->type == GenericEvent)
    {
        if (_grwl.x11.xi.available)
        {
            _GRWLwindow* window = _grwl.x11.disabledCursorWindow;

            if (window && window->rawMouseMotion && event->xcookie.extension == _grwl.x11.xi.majorOpcode &&
                XGetEventData(_grwl.x11.display, &event->xcookie) && event->xcookie.evtype == XI_RawMotion)
            {
                XIRawEvent* re = (XIRawEvent*)event->xcookie.data;
                if (re->valuators.mask_len)
                {
                    const double* values = re->raw_values;
                    double xpos = window->virtualCursorPosX;
                    double ypos = window->virtualCursorPosY;

                    if (XIMaskIsSet(re->valuators.mask, 0))
                    {
                        xpos += *values;
                        values++;
                    }

                    if (XIMaskIsSet(re->valuators.mask, 1))
                    {
                        ypos += *values;
                    }

                    _grwlInputCursorPos(window, xpos, ypos);
                }
            }

            XFreeEventData(_grwl.x11.display, &event->xcookie);
        }

        return;
    }

    if (event->type == SelectionRequest)
    {
        handleSelectionRequest(event);
        return;
    }

    _GRWLwindow* window = nullptr;
    if (XFindContext(_grwl.x11.display, event->xany.window, _grwl.x11.context, (XPointer*)&window) != 0)
    {
        // This is an event for a window that has already been destroyed
        return;
    }

    switch (event->type)
    {
        case ReparentNotify:
        {
            window->x11.parent = event->xreparent.parent;
            return;
        }

        case KeyPress:
        {
            const int key = translateKey(keycode);
            const int mods = translateState(event->xkey.state);
            const int plain = !(mods & (GRWL_MOD_CONTROL | GRWL_MOD_ALT));

            if (window->x11.ic)
            {
                // HACK: Do not report the key press events duplicated by XIM
                //       Duplicate key releases are filtered out implicitly by
                //       the GRWL key repeat logic in _grwlInputKey
                //       A timestamp per key is used to handle simultaneous keys
                // NOTE: Always allow the first event for each key through
                //       (the server never sends a timestamp of zero)
                // NOTE: Timestamp difference is compared to handle wrap-around
                Time diff = event->xkey.time - window->x11.keyPressTimes[keycode];
                if (diff == event->xkey.time || (diff > 0 && diff < ((Time)1 << 31)))
                {
                    if (keycode)
                    {
                        _grwlInputKey(window, key, keycode, GRWL_PRESS, mods);
                    }

                    window->x11.keyPressTimes[keycode] = event->xkey.time;
                }

                int count;
                Status status;
                char buffer[100];
                char* chars = buffer;

                count = Xutf8LookupString(window->x11.ic, &event->xkey, buffer, sizeof(buffer) - 1, nullptr, &status);

                if (status == XBufferOverflow)
                {
                    chars = (char*)_grwl_calloc(count + 1, 1);
                    count = Xutf8LookupString(window->x11.ic, &event->xkey, chars, count, nullptr, &status);
                }

                if (status == XLookupChars || status == XLookupBoth)
                {
                    const char* c = chars;
                    chars[count] = '\0';
                    while (c - chars < count)
                    {
                        _grwlInputChar(window, _grwlDecodeUTF8(&c), mods, plain);
                    }
                }

                if (chars != buffer)
                {
                    _grwl_free(chars);
                }
            }
            else
            {
                KeySym keysym;
                XLookupString(&event->xkey, nullptr, 0, &keysym, nullptr);

                _grwlInputKey(window, key, keycode, GRWL_PRESS, mods);

                const uint32_t codepoint = _grwlKeySym2Unicode(keysym);
                if (codepoint != GRWL_INVALID_CODEPOINT)
                {
                    _grwlInputChar(window, codepoint, mods, plain);
                }
            }

            return;
        }

        case KeyRelease:
        {
            const int key = translateKey(keycode);
            const int mods = translateState(event->xkey.state);

            if (!_grwl.x11.xkb.detectable)
            {
                // HACK: Key repeat events will arrive as KeyRelease/KeyPress
                //       pairs with similar or identical time stamps
                //       The key repeat logic in _grwlInputKey expects only key
                //       presses to repeat, so detect and discard release events
                if (XEventsQueued(_grwl.x11.display, QueuedAfterReading))
                {
                    XEvent next;
                    XPeekEvent(_grwl.x11.display, &next);

                    if (next.type == KeyPress && next.xkey.window == event->xkey.window && next.xkey.keycode == keycode)
                    {
                        // HACK: The time of repeat events sometimes doesn't
                        //       match that of the press event, so add an
                        //       epsilon
                        //       Toshiyuki Takahashi can press a button
                        //       16 times per second so it's fairly safe to
                        //       assume that no human is pressing the key 50
                        //       times per second (value is ms)
                        if ((next.xkey.time - event->xkey.time) < 20)
                        {
                            // This is very likely a server-generated key repeat
                            // event, so ignore it
                            return;
                        }
                    }
                }
            }

            _grwlInputKey(window, key, keycode, GRWL_RELEASE, mods);
            return;
        }

        case ButtonPress:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
            {
                _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_LEFT, GRWL_PRESS, mods);
            }
            else if (event->xbutton.button == Button2)
            {
                _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_MIDDLE, GRWL_PRESS, mods);
            }
            else if (event->xbutton.button == Button3)
            {
                _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_RIGHT, GRWL_PRESS, mods);
            }

            // Modern X provides scroll events as mouse button presses
            else if (event->xbutton.button == Button4)
            {
                _grwlInputScroll(window, 0.0, 1.0);
            }
            else if (event->xbutton.button == Button5)
            {
                _grwlInputScroll(window, 0.0, -1.0);
            }
            else if (event->xbutton.button == Button6)
            {
                _grwlInputScroll(window, 1.0, 0.0);
            }
            else if (event->xbutton.button == Button7)
            {
                _grwlInputScroll(window, -1.0, 0.0);
            }

            else
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _grwlInputMouseClick(window, event->xbutton.button - Button1 - 4, GRWL_PRESS, mods);
            }

            return;
        }

        case ButtonRelease:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
            {
                _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_LEFT, GRWL_RELEASE, mods);
            }
            else if (event->xbutton.button == Button2)
            {
                _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_MIDDLE, GRWL_RELEASE, mods);
            }
            else if (event->xbutton.button == Button3)
            {
                _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_RIGHT, GRWL_RELEASE, mods);
            }
            else if (event->xbutton.button > Button7)
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _grwlInputMouseClick(window, event->xbutton.button - Button1 - 4, GRWL_RELEASE, mods);
            }

            return;
        }

        case EnterNotify:
        {
            // XEnterWindowEvent is XCrossingEvent
            const int x = event->xcrossing.x;
            const int y = event->xcrossing.y;

            // HACK: This is a workaround for WMs (KWM, Fluxbox) that otherwise
            //       ignore the defined cursor for hidden cursor mode
            if (window->cursorMode == GRWL_CURSOR_HIDDEN)
            {
                updateCursorImage(window);
            }

            _grwlInputCursorEnter(window, true);
            _grwlInputCursorPos(window, x, y);

            window->x11.lastCursorPosX = x;
            window->x11.lastCursorPosY = y;
            return;
        }

        case LeaveNotify:
        {
            _grwlInputCursorEnter(window, false);
            return;
        }

        case MotionNotify:
        {
            const int x = event->xmotion.x;
            const int y = event->xmotion.y;

            if (x != window->x11.warpCursorPosX || y != window->x11.warpCursorPosY)
            {
                // The cursor was moved by something other than GRWL

                if (window->cursorMode == GRWL_CURSOR_DISABLED)
                {
                    if (_grwl.x11.disabledCursorWindow != window)
                    {
                        return;
                    }
                    if (window->rawMouseMotion)
                    {
                        return;
                    }

                    const int dx = x - window->x11.lastCursorPosX;
                    const int dy = y - window->x11.lastCursorPosY;

                    _grwlInputCursorPos(window, window->virtualCursorPosX + dx, window->virtualCursorPosY + dy);
                }
                else
                {
                    _grwlInputCursorPos(window, x, y);
                }
            }

            window->x11.lastCursorPosX = x;
            window->x11.lastCursorPosY = y;
            return;
        }

        case ConfigureNotify:
        {
            if (event->xconfigure.width != window->x11.width || event->xconfigure.height != window->x11.height)
            {
                _grwlInputFramebufferSize(window, event->xconfigure.width, event->xconfigure.height);

                _grwlInputWindowSize(window, event->xconfigure.width, event->xconfigure.height);

                window->x11.width = event->xconfigure.width;
                window->x11.height = event->xconfigure.height;
            }

            int xpos = event->xconfigure.x;
            int ypos = event->xconfigure.y;

            // NOTE: ConfigureNotify events from the server are in local
            //       coordinates, so if we are reparented we need to translate
            //       the position into root (screen) coordinates
            if (!event->xany.send_event && window->x11.parent != _grwl.x11.root)
            {
                _grwlGrabErrorHandlerX11();

                Window dummy;
                XTranslateCoordinates(_grwl.x11.display, window->x11.parent, _grwl.x11.root, xpos, ypos, &xpos, &ypos,
                                      &dummy);

                _grwlReleaseErrorHandlerX11();
                if (_grwl.x11.errorCode == BadWindow)
                {
                    return;
                }
            }

            if (xpos != window->x11.xpos || ypos != window->x11.ypos)
            {
                _grwlInputWindowPos(window, xpos, ypos);
                window->x11.xpos = xpos;
                window->x11.ypos = ypos;
            }

            return;
        }

        case ClientMessage:
        {
            // Custom client message, probably from the window manager

            if (event->xclient.message_type == None)
            {
                return;
            }

            if (event->xclient.message_type == _grwl.x11.WM_PROTOCOLS)
            {
                const Atom protocol = event->xclient.data.l[0];
                if (protocol == None)
                {
                    return;
                }

                if (protocol == _grwl.x11.WM_DELETE_WINDOW)
                {
                    // The window manager was asked to close the window, for
                    // example by the user pressing a 'close' window decoration
                    // button
                    _grwlInputWindowCloseRequest(window);
                }
                else if (protocol == _grwl.x11.NET_WM_PING)
                {
                    // The window manager is pinging the application to ensure
                    // it's still responding to events

                    XEvent reply = *event;
                    reply.xclient.window = _grwl.x11.root;

                    XSendEvent(_grwl.x11.display, _grwl.x11.root, False,
                               SubstructureNotifyMask | SubstructureRedirectMask, &reply);
                }
            }
            else if (event->xclient.message_type == _grwl.x11.XdndEnter)
            {
                // A drag operation has entered the window
                unsigned long count;
                Atom* formats = nullptr;
                const bool list = event->xclient.data.l[1] & 1;

                _grwl.x11.xdnd.source = event->xclient.data.l[0];
                _grwl.x11.xdnd.version = event->xclient.data.l[1] >> 24;
                _grwl.x11.xdnd.format = None;

                if (_grwl.x11.xdnd.version > _GRWL_XDND_VERSION)
                {
                    return;
                }

                if (list)
                {
                    count = _grwlGetWindowPropertyX11(_grwl.x11.xdnd.source, _grwl.x11.XdndTypeList, XA_ATOM,
                                                      (unsigned char**)&formats);
                }
                else
                {
                    count = 3;
                    formats = (Atom*)event->xclient.data.l + 2;
                }

                for (unsigned int i = 0; i < count; i++)
                {
                    if (formats[i] == _grwl.x11.text_uri_list)
                    {
                        _grwl.x11.xdnd.format = _grwl.x11.text_uri_list;
                        break;
                    }
                }

                if (list && formats)
                {
                    XFree(formats);
                }
            }
            else if (event->xclient.message_type == _grwl.x11.XdndDrop)
            {
                // The drag operation has finished by dropping on the window
                Time time = CurrentTime;

                if (_grwl.x11.xdnd.version > _GRWL_XDND_VERSION)
                {
                    return;
                }

                if (_grwl.x11.xdnd.format)
                {
                    if (_grwl.x11.xdnd.version >= 1)
                    {
                        time = event->xclient.data.l[2];
                    }

                    // Request the chosen format from the source window
                    XConvertSelection(_grwl.x11.display, _grwl.x11.XdndSelection, _grwl.x11.xdnd.format,
                                      _grwl.x11.XdndSelection, window->x11.handle, time);
                }
                else if (_grwl.x11.xdnd.version >= 2)
                {
                    XEvent reply = { ClientMessage };
                    reply.xclient.window = _grwl.x11.xdnd.source;
                    reply.xclient.message_type = _grwl.x11.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = window->x11.handle;
                    reply.xclient.data.l[1] = 0; // The drag was rejected
                    reply.xclient.data.l[2] = None;

                    XSendEvent(_grwl.x11.display, _grwl.x11.xdnd.source, False, NoEventMask, &reply);
                    XFlush(_grwl.x11.display);
                }
            }
            else if (event->xclient.message_type == _grwl.x11.XdndPosition)
            {
                // The drag operation has moved over the window
                const int xabs = (event->xclient.data.l[2] >> 16) & 0xffff;
                const int yabs = (event->xclient.data.l[2]) & 0xffff;
                Window dummy;
                int xpos, ypos;

                if (_grwl.x11.xdnd.version > _GRWL_XDND_VERSION)
                {
                    return;
                }

                XTranslateCoordinates(_grwl.x11.display, _grwl.x11.root, window->x11.handle, xabs, yabs, &xpos, &ypos,
                                      &dummy);

                _grwlInputCursorPos(window, xpos, ypos);

                XEvent reply = { ClientMessage };
                reply.xclient.window = _grwl.x11.xdnd.source;
                reply.xclient.message_type = _grwl.x11.XdndStatus;
                reply.xclient.format = 32;
                reply.xclient.data.l[0] = window->x11.handle;
                reply.xclient.data.l[2] = 0; // Specify an empty rectangle
                reply.xclient.data.l[3] = 0;

                if (_grwl.x11.xdnd.format)
                {
                    // Reply that we are ready to copy the dragged data
                    reply.xclient.data.l[1] = 1; // Accept with no rectangle
                    if (_grwl.x11.xdnd.version >= 2)
                    {
                        reply.xclient.data.l[4] = _grwl.x11.XdndActionCopy;
                    }
                }

                XSendEvent(_grwl.x11.display, _grwl.x11.xdnd.source, False, NoEventMask, &reply);
                XFlush(_grwl.x11.display);
            }

            return;
        }

        case SelectionNotify:
        {
            if (event->xselection.property == _grwl.x11.XdndSelection)
            {
                // The converted data from the drag operation has arrived
                char* data;
                const unsigned long result =
                    _grwlGetWindowPropertyX11(event->xselection.requestor, event->xselection.property,
                                              event->xselection.target, (unsigned char**)&data);

                if (result)
                {
                    int count;
                    char** paths = _grwlParseUriList(data, &count);

                    _grwlInputDrop(window, count, (const char**)paths);

                    for (int i = 0; i < count; i++)
                    {
                        _grwl_free(paths[i]);
                    }
                    _grwl_free(paths);
                }

                if (data)
                {
                    XFree(data);
                }

                if (_grwl.x11.xdnd.version >= 2)
                {
                    XEvent reply = { ClientMessage };
                    reply.xclient.window = _grwl.x11.xdnd.source;
                    reply.xclient.message_type = _grwl.x11.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = window->x11.handle;
                    reply.xclient.data.l[1] = result;
                    reply.xclient.data.l[2] = _grwl.x11.XdndActionCopy;

                    XSendEvent(_grwl.x11.display, _grwl.x11.xdnd.source, False, NoEventMask, &reply);
                    XFlush(_grwl.x11.display);
                }
            }

            return;
        }

        case FocusIn:
        {
            if (event->xfocus.mode == NotifyGrab || event->xfocus.mode == NotifyUngrab)
            {
                // Ignore focus events from popup indicator windows, window menu
                // key chords and window dragging
                return;
            }

            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                disableCursor(window);
            }
            else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
            {
                captureCursor(window);
            }

            if (window->x11.ic)
            {
                XSetICFocus(window->x11.ic);
            }

            _grwlInputWindowFocus(window, true);
            return;
        }

        case FocusOut:
        {
            if (event->xfocus.mode == NotifyGrab || event->xfocus.mode == NotifyUngrab)
            {
                // Ignore focus events from popup indicator windows, window menu
                // key chords and window dragging
                return;
            }

            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                enableCursor(window);
            }
            else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
            {
                releaseCursor();
            }

            if (window->x11.ic)
            {
                XUnsetICFocus(window->x11.ic);
            }

            if (window->monitor && window->autoIconify)
            {
                _grwlIconifyWindowX11(window);
            }

            _grwlInputWindowFocus(window, false);
            return;
        }

        case Expose:
        {
            _grwlInputWindowDamage(window);
            return;
        }

        case PropertyNotify:
        {
            if (event->xproperty.state != PropertyNewValue)
            {
                return;
            }

            if (event->xproperty.atom == _grwl.x11.WM_STATE)
            {
                const int state = getWindowState(window);
                if (state != IconicState && state != NormalState)
                {
                    return;
                }

                const bool iconified = (state == IconicState);
                if (window->x11.iconified != iconified)
                {
                    if (window->monitor)
                    {
                        if (iconified)
                        {
                            releaseMonitor(window);
                        }
                        else
                        {
                            acquireMonitor(window);
                        }
                    }

                    window->x11.iconified = iconified;
                    _grwlInputWindowIconify(window, iconified);
                }
            }
            else if (event->xproperty.atom == _grwl.x11.NET_WM_STATE)
            {
                const bool maximized = _grwlWindowMaximizedX11(window);
                if (window->x11.maximized != maximized)
                {
                    window->x11.maximized = maximized;
                    _grwlInputWindowMaximize(window, maximized);
                }
            }

            return;
        }

        case DestroyNotify:
            return;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Retrieve a single window property of the specified type
// Inspired by fghGetWindowProperty from freeglut
//
unsigned long _grwlGetWindowPropertyX11(Window window, Atom property, Atom type, unsigned char** value)
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(_grwl.x11.display, window, property, 0, LONG_MAX, False, type, &actualType, &actualFormat,
                       &itemCount, &bytesAfter, value);

    return itemCount;
}

bool _grwlIsVisualTransparentX11(Visual* visual)
{
    if (!_grwl.x11.xrender.available)
    {
        return false;
    }

    XRenderPictFormat* pf = XRenderFindVisualFormat(_grwl.x11.display, visual);
    return pf && pf->direct.alphaMask;
}

// Push contents of our selection to clipboard manager
//
void _grwlPushSelectionToManagerX11()
{
    XConvertSelection(_grwl.x11.display, _grwl.x11.CLIPBOARD_MANAGER, _grwl.x11.SAVE_TARGETS, None,
                      _grwl.x11.helperWindowHandle, CurrentTime);

    for (;;)
    {
        XEvent event;

        while (XCheckIfEvent(_grwl.x11.display, &event, isSelectionEvent, nullptr))
        {
            switch (event.type)
            {
                case SelectionRequest:
                    handleSelectionRequest(&event);
                    break;

                case SelectionNotify:
                {
                    if (event.xselection.target == _grwl.x11.SAVE_TARGETS)
                    {
                        // This means one of two things; either the selection
                        // was not owned, which means there is no clipboard
                        // manager, or the transfer to the clipboard manager has
                        // completed
                        // In either case, it means we are done here
                        return;
                    }

                    break;
                }
            }
        }

        waitForX11Event(nullptr);
    }
}

void _grwlCreateInputContextX11(_GRWLwindow* window)
{
    XIMCallback callback;
    callback.callback = (XIMProc)inputContextDestroyCallback;
    callback.client_data = (XPointer)window;

    window->x11.imeFocus = false;

    if (_grwl.x11.imStyle == STYLE_ONTHESPOT)
    {
        // On X11, on-the-spot style is unstable.
        // Status callbacks are not called and the preedit cursor position
        // can not be changed.
        XVaNestedList preeditList = _createXIMPreeditCallbacks(window);
        XVaNestedList statusList = _createXIMStatusCallbacks(window);

        window->x11.ic = XCreateIC(_grwl.x11.im, XNInputStyle, _grwl.x11.imStyle, XNClientWindow, window->x11.handle,
                                   XNFocusWindow, window->x11.handle, XNPreeditAttributes, preeditList,
                                   XNStatusAttributes, statusList, XNDestroyCallback, &callback, nullptr);

        XFree(preeditList);
        XFree(statusList);
    }
    else if (_grwl.x11.imStyle == STYLE_OVERTHESPOT)
    {
        window->x11.ic = XCreateIC(_grwl.x11.im, XNInputStyle, _grwl.x11.imStyle, XNClientWindow, window->x11.handle,
                                   XNFocusWindow, window->x11.handle, XNDestroyCallback, &callback, nullptr);
    }
    else
    {
        // (XIMPreeditNothing | XIMStatusNothing) is considered as STYLE_OVERTHESPOT.
        // So this branch should not be used now.
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to create input context.");
        return;
    }

    if (window->x11.ic)
    {
        XWindowAttributes attribs;
        XGetWindowAttributes(_grwl.x11.display, window->x11.handle, &attribs);

        unsigned long filter = 0;
        if (XGetICValues(window->x11.ic, XNFilterEvents, &filter, nullptr) == nullptr)
        {
            XSelectInput(_grwl.x11.display, window->x11.handle, attribs.your_event_mask | filter);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlCreateWindowX11(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                          const _GRWLfbconfig* fbconfig)
{
    Visual* visual = nullptr;
    int depth;

    if (ctxconfig->client != GRWL_NO_API)
    {
        if (ctxconfig->source == GRWL_NATIVE_CONTEXT_API)
        {
            if (!_grwlInitGLX())
            {
                return false;
            }
            if (!_grwlChooseVisualGLX(wndconfig, ctxconfig, fbconfig, &visual, &depth))
            {
                return false;
            }
        }
        else if (ctxconfig->source == GRWL_EGL_CONTEXT_API)
        {
            if (!_grwlInitEGL())
            {
                return false;
            }
            if (!_grwlChooseVisualEGL(wndconfig, ctxconfig, fbconfig, &visual, &depth))
            {
                return false;
            }
        }
        else if (ctxconfig->source == GRWL_OSMESA_CONTEXT_API)
        {
            if (!_grwlInitOSMesa())
            {
                return false;
            }
        }
    }

    if (!visual)
    {
        visual = DefaultVisual(_grwl.x11.display, _grwl.x11.screen);
        depth = DefaultDepth(_grwl.x11.display, _grwl.x11.screen);
    }

    if (!createNativeWindow(window, wndconfig, visual, depth))
    {
        return false;
    }

    if (ctxconfig->client != GRWL_NO_API)
    {
        if (ctxconfig->source == GRWL_NATIVE_CONTEXT_API)
        {
            if (!_grwlCreateContextGLX(window, ctxconfig, fbconfig))
            {
                return false;
            }
        }
        else if (ctxconfig->source == GRWL_EGL_CONTEXT_API)
        {
            if (!_grwlCreateContextEGL(window, ctxconfig, fbconfig))
            {
                return false;
            }
        }
        else if (ctxconfig->source == GRWL_OSMESA_CONTEXT_API)
        {
            if (!_grwlCreateContextOSMesa(window, ctxconfig, fbconfig))
            {
                return false;
            }
        }

        if (!_grwlRefreshContextAttribs(window, ctxconfig))
        {
            return false;
        }
    }

    if (wndconfig->mousePassthrough)
    {
        _grwlSetWindowMousePassthroughX11(window, true);
    }

    if (window->monitor)
    {
        _grwlShowWindowX11(window);
        updateWindowMode(window);
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
            _grwlShowWindowX11(window);
            if (wndconfig->focused)
            {
                _grwlFocusWindowX11(window);
            }
        }
    }

    // Reset progress state as it gets saved between application runs
    if (_grwl.dbus.connection)
    {
        // Window nullptr is safe here because it won't get
        // used inside the SetWindowTaskbarProgress function
        _grwlSetWindowProgressIndicatorX11(nullptr, GRWL_PROGRESS_INDICATOR_DISABLED, 0.0);
    }

    XFlush(_grwl.x11.display);
    return true;
}

void _grwlDestroyWindowX11(_GRWLwindow* window)
{
    if (_grwl.x11.disabledCursorWindow == window)
    {
        enableCursor(window);
    }

    if (window->monitor)
    {
        releaseMonitor(window);
    }

    if (window->x11.ic)
    {
        XDestroyIC(window->x11.ic);
        window->x11.ic = nullptr;
    }

    if (window->context.destroy)
    {
        window->context.destroy(window);
    }

    if (window->x11.handle)
    {
        XDeleteContext(_grwl.x11.display, window->x11.handle, _grwl.x11.context);
        XUnmapWindow(_grwl.x11.display, window->x11.handle);
        XDestroyWindow(_grwl.x11.display, window->x11.handle);
        window->x11.handle = (Window)0;
    }

    if (window->x11.colormap)
    {
        XFreeColormap(_grwl.x11.display, window->x11.colormap);
        window->x11.colormap = (Colormap)0;
    }

    XFlush(_grwl.x11.display);
}

void _grwlSetWindowTitleX11(_GRWLwindow* window, const char* title)
{
    if (_grwl.x11.xlib.utf8)
    {
        Xutf8SetWMProperties(_grwl.x11.display, window->x11.handle, title, title, nullptr, 0, nullptr, nullptr,
                             nullptr);
    }

    XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_NAME, _grwl.x11.UTF8_STRING, 8,
                    PropModeReplace, (unsigned char*)title, strlen(title));

    XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_ICON_NAME, _grwl.x11.UTF8_STRING, 8,
                    PropModeReplace, (unsigned char*)title, strlen(title));

    XFlush(_grwl.x11.display);
}

void _grwlSetWindowIconX11(_GRWLwindow* window, int count, const GRWLimage* images)
{
    if (count)
    {
        int longCount = 0;

        for (int i = 0; i < count; i++)
        {
            longCount += 2 + images[i].width * images[i].height;
        }

        unsigned long* icon = (unsigned long*)_grwl_calloc(longCount, sizeof(unsigned long));
        unsigned long* target = icon;

        for (int i = 0; i < count; i++)
        {
            *target++ = images[i].width;
            *target++ = images[i].height;

            for (int j = 0; j < images[i].width * images[i].height; j++)
            {
                *target++ = (((unsigned long)images[i].pixels[j * 4 + 0]) << 16) |
                            (((unsigned long)images[i].pixels[j * 4 + 1]) << 8) |
                            (((unsigned long)images[i].pixels[j * 4 + 2]) << 0) |
                            (((unsigned long)images[i].pixels[j * 4 + 3]) << 24);
            }
        }

        // NOTE: XChangeProperty expects 32-bit values like the image data above to be
        //       placed in the 32 least significant bits of individual longs.  This is
        //       true even if long is 64-bit and a WM protocol calls for "packed" data.
        //       This is because of a historical mistake that then became part of the Xlib
        //       ABI.  Xlib will pack these values into a regular array of 32-bit values
        //       before sending it over the wire.
        XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_ICON, XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char*)icon, longCount);

        _grwl_free(icon);
    }
    else
    {
        XDeleteProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_ICON);
    }

    XFlush(_grwl.x11.display);
}

void _grwlSetWindowProgressIndicatorX11(_GRWLwindow* window, int progressState, double value)
{
    () window;

    const dbus_bool_t progressVisible = (progressState != GRWL_PROGRESS_INDICATOR_DISABLED);

    _grwlUpdateTaskbarProgressDBusPOSIX(progressVisible, value);
}

void _grwlSetWindowBadgeX11(_GRWLwindow* window, int count)
{
    if (window != nullptr)
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE,
                        "X11: Cannot set a badge for a window. Pass nullptr to set the application's shared badge.");
        return;
    }

    const dbus_bool_t badgeVisible = (count > 0);

    _grwlUpdateBadgeDBusPOSIX(badgeVisible, count);
}

void _grwlSetWindowBadgeStringX11(_GRWLwindow* window, const char* string)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "X11: Unable to set a string badge. Only integer badges are supported.");
}

void _grwlGetWindowPosX11(_GRWLwindow* window, int* xpos, int* ypos)
{
    Window dummy;
    int x, y;

    XTranslateCoordinates(_grwl.x11.display, window->x11.handle, _grwl.x11.root, 0, 0, &x, &y, &dummy);

    if (xpos)
    {
        *xpos = x;
    }
    if (ypos)
    {
        *ypos = y;
    }
}

void _grwlSetWindowPosX11(_GRWLwindow* window, int xpos, int ypos)
{
    // HACK: Explicitly setting PPosition to any value causes some WMs, notably
    //       Compiz and Metacity, to honor the position of unmapped windows
    if (!_grwlWindowVisibleX11(window))
    {
        long supplied;
        XSizeHints* hints = XAllocSizeHints();

        if (XGetWMNormalHints(_grwl.x11.display, window->x11.handle, hints, &supplied))
        {
            hints->flags |= PPosition;
            hints->x = hints->y = 0;

            XSetWMNormalHints(_grwl.x11.display, window->x11.handle, hints);
        }

        XFree(hints);
    }

    XMoveWindow(_grwl.x11.display, window->x11.handle, xpos, ypos);
    XFlush(_grwl.x11.display);
}

void _grwlGetWindowSizeX11(_GRWLwindow* window, int* width, int* height)
{
    XWindowAttributes attribs;
    XGetWindowAttributes(_grwl.x11.display, window->x11.handle, &attribs);

    if (width)
    {
        *width = attribs.width;
    }
    if (height)
    {
        *height = attribs.height;
    }
}

void _grwlSetWindowSizeX11(_GRWLwindow* window, int width, int height)
{
    if (window->monitor)
    {
        if (window->monitor->window == window)
        {
            acquireMonitor(window);
        }
    }
    else
    {
        if (!window->resizable)
        {
            updateNormalHints(window, width, height);
        }

        XResizeWindow(_grwl.x11.display, window->x11.handle, width, height);
    }

    XFlush(_grwl.x11.display);
}

void _grwlSetWindowSizeLimitsX11(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight)
{
    int width, height;
    _grwlGetWindowSizeX11(window, &width, &height);
    updateNormalHints(window, width, height);
    XFlush(_grwl.x11.display);
}

void _grwlSetWindowAspectRatioX11(_GRWLwindow* window, int numer, int denom)
{
    int width, height;
    _grwlGetWindowSizeX11(window, &width, &height);
    updateNormalHints(window, width, height);
    XFlush(_grwl.x11.display);
}

void _grwlGetFramebufferSizeX11(_GRWLwindow* window, int* width, int* height)
{
    _grwlGetWindowSizeX11(window, width, height);
}

void _grwlGetWindowFrameSizeX11(_GRWLwindow* window, int* left, int* top, int* right, int* bottom)
{
    long* extents = nullptr;

    if (window->monitor || !window->decorated)
    {
        return;
    }

    if (_grwl.x11.NET_FRAME_EXTENTS == None)
    {
        return;
    }

    if (!_grwlWindowVisibleX11(window) && _grwl.x11.NET_REQUEST_FRAME_EXTENTS)
    {
        XEvent event;
        double timeout = 0.5;

        // Ensure _NET_FRAME_EXTENTS is set, allowing grwlGetWindowFrameSize to
        // function before the window is mapped
        sendEventToWM(window, _grwl.x11.NET_REQUEST_FRAME_EXTENTS, 0, 0, 0, 0, 0);

        // HACK: Use a timeout because earlier versions of some window managers
        //       (at least Unity, Fluxbox and Xfwm) failed to send the reply
        //       They have been fixed but broken versions are still in the wild
        //       If you are affected by this and your window manager is NOT
        //       listed above, PLEASE report it to their and our issue trackers
        while (!XCheckIfEvent(_grwl.x11.display, &event, isFrameExtentsEvent, (XPointer)window))
        {
            if (!waitForX11Event(&timeout))
            {
                _grwlInputError(
                    GRWL_PLATFORM_ERROR,
                    "X11: The window manager has a broken _NET_REQUEST_FRAME_EXTENTS implementation; please report this issue");
                return;
            }
        }
    }

    if (_grwlGetWindowPropertyX11(window->x11.handle, _grwl.x11.NET_FRAME_EXTENTS, XA_CARDINAL,
                                  (unsigned char**)&extents) == 4)
    {
        if (left)
        {
            *left = extents[0];
        }
        if (top)
        {
            *top = extents[2];
        }
        if (right)
        {
            *right = extents[1];
        }
        if (bottom)
        {
            *bottom = extents[3];
        }
    }

    if (extents)
    {
        XFree(extents);
    }
}

void _grwlGetWindowContentScaleX11(_GRWLwindow* window, float* xscale, float* yscale)
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

void _grwlIconifyWindowX11(_GRWLwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _grwlInputError(GRWL_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH full screen");
        return;
    }

    XIconifyWindow(_grwl.x11.display, window->x11.handle, _grwl.x11.screen);
    XFlush(_grwl.x11.display);
}

void _grwlRestoreWindowX11(_GRWLwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _grwlInputError(GRWL_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH full screen");
        return;
    }

    if (_grwlWindowIconifiedX11(window))
    {
        XMapWindow(_grwl.x11.display, window->x11.handle);
        waitForVisibilityNotify(window);
    }
    else if (_grwlWindowVisibleX11(window))
    {
        if (_grwl.x11.NET_WM_STATE && _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT && _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ)
        {
            sendEventToWM(window, _grwl.x11.NET_WM_STATE, _NET_WM_STATE_REMOVE, _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT,
                          _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ, 1, 0);
        }
    }

    XFlush(_grwl.x11.display);
}

void _grwlMaximizeWindowX11(_GRWLwindow* window)
{
    if (!_grwl.x11.NET_WM_STATE || !_grwl.x11.NET_WM_STATE_MAXIMIZED_VERT || !_grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ)
    {
        return;
    }

    if (_grwlWindowVisibleX11(window))
    {
        sendEventToWM(window, _grwl.x11.NET_WM_STATE, _NET_WM_STATE_ADD, _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT,
                      _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ, 1, 0);
    }
    else
    {
        Atom* states = nullptr;
        unsigned long count =
            _grwlGetWindowPropertyX11(window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, (unsigned char**)&states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] = { _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT, _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ };
        unsigned long missingCount = 2;

        for (unsigned long i = 0; i < count; i++)
        {
            for (unsigned long j = 0; j < missingCount; j++)
            {
                if (states[i] == missing[j])
                {
                    missing[j] = missing[missingCount - 1];
                    missingCount--;
                }
            }
        }

        if (states)
        {
            XFree(states);
        }

        if (!missingCount)
        {
            return;
        }

        XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, 32, PropModeAppend,
                        (unsigned char*)missing, missingCount);
    }

    XFlush(_grwl.x11.display);
}

void _grwlShowWindowX11(_GRWLwindow* window)
{
    if (_grwlWindowVisibleX11(window))
    {
        return;
    }

    XMapWindow(_grwl.x11.display, window->x11.handle);
    waitForVisibilityNotify(window);
}

void _grwlHideWindowX11(_GRWLwindow* window)
{
    XUnmapWindow(_grwl.x11.display, window->x11.handle);
    XFlush(_grwl.x11.display);
}

void _grwlRequestWindowAttentionX11(_GRWLwindow* window)
{
    if (!_grwl.x11.NET_WM_STATE || !_grwl.x11.NET_WM_STATE_DEMANDS_ATTENTION)
    {
        return;
    }

    sendEventToWM(window, _grwl.x11.NET_WM_STATE, _NET_WM_STATE_ADD, _grwl.x11.NET_WM_STATE_DEMANDS_ATTENTION, 0, 1, 0);
}

void _grwlFocusWindowX11(_GRWLwindow* window)
{
    if (_grwl.x11.NET_ACTIVE_WINDOW)
    {
        sendEventToWM(window, _grwl.x11.NET_ACTIVE_WINDOW, 1, 0, 0, 0, 0);
    }
    else if (_grwlWindowVisibleX11(window))
    {
        XRaiseWindow(_grwl.x11.display, window->x11.handle);
        XSetInputFocus(_grwl.x11.display, window->x11.handle, RevertToParent, CurrentTime);
    }

    XFlush(_grwl.x11.display);
}

void _grwlSetWindowMonitorX11(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                              int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
            {
                acquireMonitor(window);
            }
        }
        else
        {
            if (!window->resizable)
            {
                updateNormalHints(window, width, height);
            }

            XMoveResizeWindow(_grwl.x11.display, window->x11.handle, xpos, ypos, width, height);
        }

        XFlush(_grwl.x11.display);
        return;
    }

    if (window->monitor)
    {
        _grwlSetWindowDecoratedX11(window, window->decorated);
        _grwlSetWindowFloatingX11(window, window->floating);
        releaseMonitor(window);
    }

    _grwlInputWindowMonitor(window, monitor);
    updateNormalHints(window, width, height);

    if (window->monitor)
    {
        if (!_grwlWindowVisibleX11(window))
        {
            XMapRaised(_grwl.x11.display, window->x11.handle);
            waitForVisibilityNotify(window);
        }

        updateWindowMode(window);
        acquireMonitor(window);
    }
    else
    {
        updateWindowMode(window);
        XMoveResizeWindow(_grwl.x11.display, window->x11.handle, xpos, ypos, width, height);
    }

    XFlush(_grwl.x11.display);
}

bool _grwlWindowFocusedX11(_GRWLwindow* window)
{
    Window focused;
    int state;

    XGetInputFocus(_grwl.x11.display, &focused, &state);
    return window->x11.handle == focused;
}

bool _grwlWindowIconifiedX11(_GRWLwindow* window)
{
    return getWindowState(window) == IconicState;
}

bool _grwlWindowVisibleX11(_GRWLwindow* window)
{
    XWindowAttributes wa;
    XGetWindowAttributes(_grwl.x11.display, window->x11.handle, &wa);
    return wa.map_state == IsViewable;
}

bool _grwlWindowMaximizedX11(_GRWLwindow* window)
{
    Atom* states;
    bool maximized = false;

    if (!_grwl.x11.NET_WM_STATE || !_grwl.x11.NET_WM_STATE_MAXIMIZED_VERT || !_grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ)
    {
        return maximized;
    }

    const unsigned long count =
        _grwlGetWindowPropertyX11(window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, (unsigned char**)&states);

    for (unsigned long i = 0; i < count; i++)
    {
        if (states[i] == _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT || states[i] == _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ)
        {
            maximized = true;
            break;
        }
    }

    if (states)
    {
        XFree(states);
    }

    return maximized;
}

bool _grwlWindowHoveredX11(_GRWLwindow* window)
{
    Window w = _grwl.x11.root;
    while (w)
    {
        Window root;
        int rootX, rootY, childX, childY;
        unsigned int mask;

        _grwlGrabErrorHandlerX11();

        const Bool result = XQueryPointer(_grwl.x11.display, w, &root, &w, &rootX, &rootY, &childX, &childY, &mask);

        _grwlReleaseErrorHandlerX11();

        if (_grwl.x11.errorCode == BadWindow)
        {
            w = _grwl.x11.root;
        }
        else if (!result)
        {
            return false;
        }
        else if (w == window->x11.handle)
        {
            return true;
        }
    }

    return false;
}

bool _grwlFramebufferTransparentX11(_GRWLwindow* window)
{
    if (!window->x11.transparent)
    {
        return false;
    }

    return XGetSelectionOwner(_grwl.x11.display, _grwl.x11.NET_WM_CM_Sx) != None;
}

void _grwlSetWindowResizableX11(_GRWLwindow* window, bool enabled)
{
    int width, height;
    _grwlGetWindowSizeX11(window, &width, &height);
    updateNormalHints(window, width, height);
}

void _grwlSetWindowDecoratedX11(_GRWLwindow* window, bool enabled)
{
    struct
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } hints = { 0 };

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = enabled ? MWM_DECOR_ALL : 0;

    XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.MOTIF_WM_HINTS, _grwl.x11.MOTIF_WM_HINTS, 32,
                    PropModeReplace, (unsigned char*)&hints, sizeof(hints) / sizeof(long));
}

void _grwlSetWindowFloatingX11(_GRWLwindow* window, bool enabled)
{
    if (!_grwl.x11.NET_WM_STATE || !_grwl.x11.NET_WM_STATE_ABOVE)
    {
        return;
    }

    if (_grwlWindowVisibleX11(window))
    {
        const long action = enabled ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
        sendEventToWM(window, _grwl.x11.NET_WM_STATE, action, _grwl.x11.NET_WM_STATE_ABOVE, 0, 1, 0);
    }
    else
    {
        Atom* states = nullptr;
        const unsigned long count =
            _grwlGetWindowPropertyX11(window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, (unsigned char**)&states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        if (enabled)
        {
            unsigned long i;

            for (i = 0; i < count; i++)
            {
                if (states[i] == _grwl.x11.NET_WM_STATE_ABOVE)
                {
                    break;
                }
            }

            if (i == count)
            {
                XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, 32,
                                PropModeAppend, (unsigned char*)&_grwl.x11.NET_WM_STATE_ABOVE, 1);
            }
        }
        else if (states)
        {
            for (unsigned long i = 0; i < count; i++)
            {
                if (states[i] == _grwl.x11.NET_WM_STATE_ABOVE)
                {
                    states[i] = states[count - 1];
                    XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_STATE, XA_ATOM, 32,
                                    PropModeReplace, (unsigned char*)states, count - 1);
                    break;
                }
            }
        }

        if (states)
        {
            XFree(states);
        }
    }

    XFlush(_grwl.x11.display);
}

void _grwlSetWindowMousePassthroughX11(_GRWLwindow* window, bool enabled)
{
    if (!_grwl.x11.xshape.available)
    {
        return;
    }

    if (enabled)
    {
        Region region = XCreateRegion();
        XShapeCombineRegion(_grwl.x11.display, window->x11.handle, ShapeInput, 0, 0, region, ShapeSet);
        XDestroyRegion(region);
    }
    else
    {
        XShapeCombineMask(_grwl.x11.display, window->x11.handle, ShapeInput, 0, 0, None, ShapeSet);
    }
}

float _grwlGetWindowOpacityX11(_GRWLwindow* window)
{
    float opacity = 1.f;

    if (XGetSelectionOwner(_grwl.x11.display, _grwl.x11.NET_WM_CM_Sx))
    {
        CARD32* value = nullptr;

        if (_grwlGetWindowPropertyX11(window->x11.handle, _grwl.x11.NET_WM_WINDOW_OPACITY, XA_CARDINAL,
                                      (unsigned char**)&value))
        {
            opacity = (float)(*value / (double)0xffffffffu);
        }

        if (value)
        {
            XFree(value);
        }
    }

    return opacity;
}

void _grwlSetWindowOpacityX11(_GRWLwindow* window, float opacity)
{
    const CARD32 value = (CARD32)(0xffffffffu * (double)opacity);
    XChangeProperty(_grwl.x11.display, window->x11.handle, _grwl.x11.NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)&value, 1);
}

void _grwlSetRawMouseMotionX11(_GRWLwindow* window, bool enabled)
{
    if (!_grwl.x11.xi.available)
    {
        return;
    }

    if (_grwl.x11.disabledCursorWindow != window)
    {
        return;
    }

    if (enabled)
    {
        enableRawMouseMotion(window);
    }
    else
    {
        disableRawMouseMotion(window);
    }
}

bool _grwlRawMouseMotionSupportedX11()
{
    return _grwl.x11.xi.available;
}

void _grwlPollEventsX11()
{
    drainEmptyEvents();

    #if defined(GRWL_BUILD_LINUX_JOYSTICK)
    if (_grwl.joysticksInitialized)
    {
        _grwlDetectJoystickConnectionLinux();
    }
    #endif
    XPending(_grwl.x11.display);

    while (QLength(_grwl.x11.display))
    {
        XEvent event;
        XNextEvent(_grwl.x11.display, &event);
        processEvent(&event);
    }

    _GRWLwindow* window = _grwl.x11.disabledCursorWindow;
    if (window)
    {
        int width, height;
        _grwlGetWindowSizeX11(window, &width, &height);

        // NOTE: Re-center the cursor only if it has moved since the last call,
        //       to avoid breaking grwlWaitEvents with MotionNotify
        if (window->x11.lastCursorPosX != width / 2 || window->x11.lastCursorPosY != height / 2)
        {
            _grwlSetCursorPosX11(window, width / 2, height / 2);
        }
    }

    XFlush(_grwl.x11.display);
}

void _grwlWaitEventsX11()
{
    waitForAnyEvent(nullptr);
    _grwlPollEventsX11();
}

void _grwlWaitEventsTimeoutX11(double timeout)
{
    waitForAnyEvent(&timeout);
    _grwlPollEventsX11();
}

void _grwlPostEmptyEventX11()
{
    writeEmptyEvent();
}

void _grwlGetCursorPosX11(_GRWLwindow* window, double* xpos, double* ypos)
{
    Window root, child;
    int rootX, rootY, childX, childY;
    unsigned int mask;

    XQueryPointer(_grwl.x11.display, window->x11.handle, &root, &child, &rootX, &rootY, &childX, &childY, &mask);

    if (xpos)
    {
        *xpos = childX;
    }
    if (ypos)
    {
        *ypos = childY;
    }
}

void _grwlSetCursorPosX11(_GRWLwindow* window, double x, double y)
{
    // Store the new position so it can be recognized later
    window->x11.warpCursorPosX = (int)x;
    window->x11.warpCursorPosY = (int)y;

    XWarpPointer(_grwl.x11.display, None, window->x11.handle, 0, 0, 0, 0, (int)x, (int)y);
    XFlush(_grwl.x11.display);
}

void _grwlSetCursorModeX11(_GRWLwindow* window, int mode)
{
    if (_grwlWindowFocusedX11(window))
    {
        if (mode == GRWL_CURSOR_DISABLED)
        {
            _grwlGetCursorPosX11(window, &_grwl.x11.restoreCursorPosX, &_grwl.x11.restoreCursorPosY);
            _grwlCenterCursorInContentArea(window);
            if (window->rawMouseMotion)
            {
                enableRawMouseMotion(window);
            }
        }
        else if (_grwl.x11.disabledCursorWindow == window)
        {
            if (window->rawMouseMotion)
            {
                disableRawMouseMotion(window);
            }
        }

        if (mode == GRWL_CURSOR_DISABLED || mode == GRWL_CURSOR_CAPTURED)
        {
            captureCursor(window);
        }
        else
        {
            releaseCursor();
        }

        if (mode == GRWL_CURSOR_DISABLED)
        {
            _grwl.x11.disabledCursorWindow = window;
        }
        else if (_grwl.x11.disabledCursorWindow == window)
        {
            _grwl.x11.disabledCursorWindow = nullptr;
            _grwlSetCursorPosX11(window, _grwl.x11.restoreCursorPosX, _grwl.x11.restoreCursorPosY);
        }
    }

    updateCursorImage(window);
    XFlush(_grwl.x11.display);
}

const char* _grwlGetScancodeNameX11(int scancode)
{
    if (!_grwl.x11.xkb.available)
    {
        return nullptr;
    }

    if (scancode < 0 || scancode > 0xff || _grwl.x11.keycodes[scancode] == GRWL_KEY_UNKNOWN)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid scancode %i", scancode);
        return nullptr;
    }

    const int key = _grwl.x11.keycodes[scancode];
    const KeySym keysym = XkbKeycodeToKeysym(_grwl.x11.display, scancode, _grwl.x11.xkb.group, 0);
    if (keysym == NoSymbol)
    {
        return nullptr;
    }

    const uint32_t codepoint = _grwlKeySym2Unicode(keysym);
    if (codepoint == GRWL_INVALID_CODEPOINT)
    {
        return nullptr;
    }

    const size_t count = _grwlEncodeUTF8(_grwl.x11.keynames[key], codepoint);
    if (count == 0)
    {
        return nullptr;
    }

    _grwl.x11.keynames[key][count] = '\0';
    return _grwl.x11.keynames[key];
}

int _grwlGetKeyScancodeX11(int key)
{
    return _grwl.x11.scancodes[key];
}

const char* _grwlGetKeyboardLayoutNameX11()
{
    if (!_grwl.x11.xkb.available)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: XKB extension required for keyboard layout names");
        return nullptr;
    }

    XkbStateRec state = { 0 };
    XkbGetState(_grwl.x11.display, XkbUseCoreKbd, &state);

    XkbDescPtr desc = XkbAllocKeyboard();
    if (XkbGetNames(_grwl.x11.display, XkbGroupNamesMask, desc) != Success)
    {
        XkbFreeKeyboard(desc, 0, True);
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to retrieve keyboard layout names");
        return nullptr;
    }

    const Atom atom = desc->names->groups[state.group];
    XkbFreeKeyboard(desc, 0, True);

    if (atom == None)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Name missing for current keyboard layout");
        return nullptr;
    }

    free(_grwl.x11.keyboardLayoutName);
    _grwl.x11.keyboardLayoutName = XGetAtomName(_grwl.x11.display, atom);
    return _grwl.x11.keyboardLayoutName;
}

bool _grwlCreateCursorX11(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot)
{
    cursor->x11.handle = _grwlCreateNativeCursorX11(image, xhot, yhot);
    if (!cursor->x11.handle)
    {
        return false;
    }

    return true;
}

bool _grwlCreateStandardCursorX11(_GRWLcursor* cursor, int shape)
{
    if (_grwl.x11.xcursor.handle)
    {
        char* theme = XcursorGetTheme(_grwl.x11.display);
        if (theme)
        {
            const int size = XcursorGetDefaultSize(_grwl.x11.display);
            const char* name = nullptr;

            switch (shape)
            {
                case GRWL_ARROW_CURSOR:
                    name = "default";
                    break;
                case GRWL_IBEAM_CURSOR:
                    name = "text";
                    break;
                case GRWL_CROSSHAIR_CURSOR:
                    name = "crosshair";
                    break;
                case GRWL_POINTING_HAND_CURSOR:
                    name = "pointer";
                    break;
                case GRWL_RESIZE_EW_CURSOR:
                    name = "ew-resize";
                    break;
                case GRWL_RESIZE_NS_CURSOR:
                    name = "ns-resize";
                    break;
                case GRWL_RESIZE_NWSE_CURSOR:
                    name = "nwse-resize";
                    break;
                case GRWL_RESIZE_NESW_CURSOR:
                    name = "nesw-resize";
                    break;
                case GRWL_RESIZE_ALL_CURSOR:
                    name = "all-scroll";
                    break;
                case GRWL_NOT_ALLOWED_CURSOR:
                    name = "not-allowed";
                    break;
            }

            XcursorImage* image = XcursorLibraryLoadImage(name, theme, size);
            if (image)
            {
                cursor->x11.handle = XcursorImageLoadCursor(_grwl.x11.display, image);
                XcursorImageDestroy(image);
            }
        }
    }

    if (!cursor->x11.handle)
    {
        unsigned int native = 0;

        switch (shape)
        {
            case GRWL_ARROW_CURSOR:
                native = XC_left_ptr;
                break;
            case GRWL_IBEAM_CURSOR:
                native = XC_xterm;
                break;
            case GRWL_CROSSHAIR_CURSOR:
                native = XC_crosshair;
                break;
            case GRWL_POINTING_HAND_CURSOR:
                native = XC_hand2;
                break;
            case GRWL_RESIZE_EW_CURSOR:
                native = XC_sb_h_double_arrow;
                break;
            case GRWL_RESIZE_NS_CURSOR:
                native = XC_sb_v_double_arrow;
                break;
            case GRWL_RESIZE_ALL_CURSOR:
                native = XC_fleur;
                break;
            default:
                _grwlInputError(GRWL_CURSOR_UNAVAILABLE, "X11: Standard cursor shape unavailable");
                return false;
        }

        cursor->x11.handle = XCreateFontCursor(_grwl.x11.display, native);
        if (!cursor->x11.handle)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to create standard cursor");
            return false;
        }
    }

    return true;
}

void _grwlDestroyCursorX11(_GRWLcursor* cursor)
{
    if (cursor->x11.handle)
    {
        XFreeCursor(_grwl.x11.display, cursor->x11.handle);
    }
}

void _grwlSetCursorX11(_GRWLwindow* window, _GRWLcursor* cursor)
{
    if (window->cursorMode == GRWL_CURSOR_NORMAL || window->cursorMode == GRWL_CURSOR_CAPTURED)
    {
        updateCursorImage(window);
        XFlush(_grwl.x11.display);
    }
}

void _grwlSetClipboardStringX11(const char* string)
{
    char* copy = _grwl_strdup(string);
    _grwl_free(_grwl.x11.clipboardString);
    _grwl.x11.clipboardString = copy;

    XSetSelectionOwner(_grwl.x11.display, _grwl.x11.CLIPBOARD, _grwl.x11.helperWindowHandle, CurrentTime);

    if (XGetSelectionOwner(_grwl.x11.display, _grwl.x11.CLIPBOARD) != _grwl.x11.helperWindowHandle)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to become owner of clipboard selection");
    }
}

const char* _grwlGetClipboardStringX11()
{
    return getSelectionString(_grwl.x11.CLIPBOARD);
}

// When using STYLE_ONTHESPOT, this doesn't work and the cursor position can't be updated
//
void _grwlUpdatePreeditCursorRectangleX11(_GRWLwindow* window)
{
    XVaNestedList preedit_attr;
    XPoint spot;
    _GRWLpreedit* preedit = &window->preedit;

    if (!window->x11.ic)
    {
        return;
    }

    spot.x = preedit->cursorPosX + preedit->cursorWidth;
    spot.y = preedit->cursorPosY + preedit->cursorHeight;
    preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &spot, nullptr);
    XSetICValues(window->x11.ic, XNPreeditAttributes, preedit_attr, nullptr);
    XFree(preedit_attr);
}

void _grwlResetPreeditTextX11(_GRWLwindow* window)
{
    XIC ic = window->x11.ic;
    _GRWLpreedit* preedit = &window->preedit;

    /* restore conversion state after resetting ic later */
    XIMPreeditState preedit_state = XIMPreeditUnKnown;
    XVaNestedList preedit_attr;
    char* result;

    if (!ic)
    {
        return;
    }

    // Can not manage IME in the case of over-the-spot.
    if (_grwl.x11.imStyle == STYLE_OVERTHESPOT)
    {
        return;
    }

    if (preedit->textCount == 0)
    {
        return;
    }

    preedit_attr = XVaCreateNestedList(0, XNPreeditState, &preedit_state, nullptr);
    XGetICValues(ic, XNPreeditAttributes, preedit_attr, nullptr);
    XFree(preedit_attr);

    result = XmbResetIC(ic);

    preedit_attr = XVaCreateNestedList(0, XNPreeditState, preedit_state, nullptr);
    XSetICValues(ic, XNPreeditAttributes, preedit_attr, nullptr);
    XFree(preedit_attr);

    preedit->textCount = 0;
    preedit->blockSizesCount = 0;
    preedit->focusedBlockIndex = 0;
    preedit->caretIndex = 0;
    _grwlInputPreedit(window);

    XFree(result);
}

void _grwlSetIMEStatusX11(_GRWLwindow* window, int active)
{
    XIC ic = window->x11.ic;

    if (!ic)
    {
        return;
    }

    // Can not manage IME in the case of over-the-spot.
    if (_grwl.x11.imStyle == STYLE_OVERTHESPOT)
    {
        return;
    }

    if (active)
    {
        XSetICFocus(ic);
    }
    else
    {
        XUnsetICFocus(ic);
    }
}

int _grwlGetIMEStatusX11(_GRWLwindow* window)
{
    if (!window->x11.ic)
    {
        return false;
    }

    // Can not manage IME in the case of over-the-spot.
    if (_grwl.x11.imStyle == STYLE_OVERTHESPOT)
    {
        return false;
    }

    return window->x11.imeFocus;
}

EGLenum _grwlGetEGLPlatformX11(EGLint** attribs)
{
    if (_grwl.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_grwl.egl.ANGLE_platform_angle_opengl)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_OPENGL)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
            }
        }

        if (_grwl.egl.ANGLE_platform_angle_vulkan)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_VULKAN)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
            }
        }

        if (type)
        {
            *attribs = (EGLint*)_grwl_calloc(5, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE;
            (*attribs)[3] = EGL_PLATFORM_X11_EXT;
            (*attribs)[4] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    if (_grwl.egl.EXT_platform_base && _grwl.egl.EXT_platform_x11)
    {
        return EGL_PLATFORM_X11_EXT;
    }

    return 0;
}

EGLNativeDisplayType _grwlGetEGLNativeDisplayX11()
{
    return _grwl.x11.display;
}

EGLNativeWindowType _grwlGetEGLNativeWindowX11(_GRWLwindow* window)
{
    if (_grwl.egl.platform)
    {
        return &window->x11.handle;
    }
    else
    {
        return (EGLNativeWindowType)window->x11.handle;
    }
}

void _grwlGetRequiredInstanceExtensionsX11(char** extensions)
{
    if (!_grwl.vk.KHR_surface)
    {
        return;
    }

    if (!_grwl.vk.KHR_xcb_surface || !_grwl.x11.x11xcb.handle)
    {
        if (!_grwl.vk.KHR_xlib_surface)
        {
            return;
        }
    }

    extensions[0] = "VK_KHR_surface";

    // NOTE: VK_KHR_xcb_surface is preferred due to some early ICDs exposing but
    //       not correctly implementing VK_KHR_xlib_surface
    if (_grwl.vk.KHR_xcb_surface && _grwl.x11.x11xcb.handle)
    {
        extensions[1] = "VK_KHR_xcb_surface";
    }
    else
    {
        extensions[1] = "VK_KHR_xlib_surface";
    }
}

bool _grwlGetPhysicalDevicePresentationSupportX11(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily)
{
    VisualID visualID = XVisualIDFromVisual(DefaultVisual(_grwl.x11.display, _grwl.x11.screen));

    if (_grwl.vk.KHR_xcb_surface && _grwl.x11.x11xcb.handle)
    {
        PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)vkGetInstanceProcAddr(
                instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
        if (!vkGetPhysicalDeviceXcbPresentationSupportKHR)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "X11: Vulkan instance missing VK_KHR_xcb_surface extension");
            return false;
        }

        xcb_connection_t* connection = XGetXCBConnection(_grwl.x11.display);
        if (!connection)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to retrieve XCB connection");
            return false;
        }

        return vkGetPhysicalDeviceXcbPresentationSupportKHR(device, queuefamily, connection, visualID);
    }
    else
    {
        PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)vkGetInstanceProcAddr(
                instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
        if (!vkGetPhysicalDeviceXlibPresentationSupportKHR)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "X11: Vulkan instance missing VK_KHR_xlib_surface extension");
            return false;
        }

        return vkGetPhysicalDeviceXlibPresentationSupportKHR(device, queuefamily, _grwl.x11.display, visualID);
    }
}

VkResult _grwlCreateWindowSurfaceX11(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                     VkSurfaceKHR* surface)
{
    if (_grwl.vk.KHR_xcb_surface && _grwl.x11.x11xcb.handle)
    {
        VkResult err;
        VkXcbSurfaceCreateInfoKHR sci;
        PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;

        xcb_connection_t* connection = XGetXCBConnection(_grwl.x11.display);
        if (!connection)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to retrieve XCB connection");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
        if (!vkCreateXcbSurfaceKHR)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "X11: Vulkan instance missing VK_KHR_xcb_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        sci.connection = connection;
        sci.window = window->x11.handle;

        err = vkCreateXcbSurfaceKHR(instance, &sci, allocator, surface);
        if (err)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to create Vulkan XCB surface: %s",
                            _grwlGetVulkanResultString(err));
        }

        return err;
    }
    else
    {
        VkResult err;
        VkXlibSurfaceCreateInfoKHR sci;
        PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

        vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
        if (!vkCreateXlibSurfaceKHR)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "X11: Vulkan instance missing VK_KHR_xlib_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        sci.dpy = _grwl.x11.display;
        sci.window = window->x11.handle;

        err = vkCreateXlibSurfaceKHR(instance, &sci, allocator, surface);
        if (err)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to create Vulkan X11 surface: %s",
                            _grwlGetVulkanResultString(err));
        }

        return err;
    }
}

_GRWLusercontext* _grwlCreateUserContextX11(_GRWLwindow* window)
{
    if (window->context.glx.handle)
    {
        return _grwlCreateUserContextGLX(window);
    }
    else if (window->context.egl.handle)
    {
        return _grwlCreateUserContextEGL(window);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI Display* grwlGetX11Display()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    if (_grwl.platform.platformID != GRWL_PLATFORM_X11)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return nullptr;
    }

    return _grwl.x11.display;
}

GRWLAPI Window grwlGetX11Window(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(None);

    if (_grwl.platform.platformID != GRWL_PLATFORM_X11)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return None;
    }

    return window->x11.handle;
}

GRWLAPI void grwlSetX11SelectionString(const char* string)
{
    _GRWL_REQUIRE_INIT();

    if (_grwl.platform.platformID != GRWL_PLATFORM_X11)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return;
    }

    _grwl_free(_grwl.x11.primarySelectionString);
    _grwl.x11.primarySelectionString = _grwl_strdup(string);

    XSetSelectionOwner(_grwl.x11.display, _grwl.x11.PRIMARY, _grwl.x11.helperWindowHandle, CurrentTime);

    if (XGetSelectionOwner(_grwl.x11.display, _grwl.x11.PRIMARY) != _grwl.x11.helperWindowHandle)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to become owner of primary selection");
    }
}

GRWLAPI const char* grwlGetX11SelectionString()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    if (_grwl.platform.platformID != GRWL_PLATFORM_X11)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return nullptr;
    }

    return getSelectionString(_grwl.x11.PRIMARY);
}

#endif // _GRWL_X11
