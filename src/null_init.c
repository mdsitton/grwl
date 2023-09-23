//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

GRWLbool _grwlConnectNull(int platformID, _GRWLplatform* platform)
{
    const _GRWLplatform null = {
        GRWL_PLATFORM_NULL,
        _grwlInitNull,
        _grwlTerminateNull,
        _grwlGetCursorPosNull,
        _grwlSetCursorPosNull,
        _grwlSetCursorModeNull,
        _grwlSetRawMouseMotionNull,
        _grwlRawMouseMotionSupportedNull,
        _grwlCreateCursorNull,
        _grwlCreateStandardCursorNull,
        _grwlDestroyCursorNull,
        _grwlSetCursorNull,
        _grwlGetScancodeNameNull,
        _grwlGetKeyScancodeNull,
        _grwlGetKeyboardLayoutNameNull,
        _grwlSetClipboardStringNull,
        _grwlGetClipboardStringNull,
        _grwlUpdatePreeditCursorRectangleNull,
        _grwlResetPreeditTextNull,
        _grwlSetIMEStatusNull,
        _grwlGetIMEStatusNull,
        _grwlInitJoysticksNull,
        _grwlTerminateJoysticksNull,
        _grwlPollJoystickNull,
        _grwlGetMappingNameNull,
        _grwlUpdateGamepadGUIDNull,
        _grwlFreeMonitorNull,
        _grwlGetMonitorPosNull,
        _grwlGetMonitorContentScaleNull,
        _grwlGetMonitorWorkareaNull,
        _grwlGetVideoModesNull,
        _grwlGetVideoModeNull,
        _grwlGetGammaRampNull,
        _grwlSetGammaRampNull,
        _grwlCreateWindowNull,
        _grwlDestroyWindowNull,
        _grwlSetWindowTitleNull,
        _grwlSetWindowIconNull,
        _grwlSetWindowProgressIndicatorNull,
        _grwlSetWindowBadgeNull,
        _grwlSetWindowBadgeStringNull,
        _grwlGetWindowPosNull,
        _grwlSetWindowPosNull,
        _grwlGetWindowSizeNull,
        _grwlSetWindowSizeNull,
        _grwlSetWindowSizeLimitsNull,
        _grwlSetWindowAspectRatioNull,
        _grwlGetFramebufferSizeNull,
        _grwlGetWindowFrameSizeNull,
        _grwlGetWindowContentScaleNull,
        _grwlIconifyWindowNull,
        _grwlRestoreWindowNull,
        _grwlMaximizeWindowNull,
        _grwlShowWindowNull,
        _grwlHideWindowNull,
        _grwlRequestWindowAttentionNull,
        _grwlFocusWindowNull,
        _grwlSetWindowMonitorNull,
        _grwlWindowFocusedNull,
        _grwlWindowIconifiedNull,
        _grwlWindowVisibleNull,
        _grwlWindowMaximizedNull,
        _grwlWindowHoveredNull,
        _grwlFramebufferTransparentNull,
        _grwlGetWindowOpacityNull,
        _grwlSetWindowResizableNull,
        _grwlSetWindowDecoratedNull,
        _grwlSetWindowFloatingNull,
        _grwlSetWindowOpacityNull,
        _grwlSetWindowMousePassthroughNull,
        _grwlPollEventsNull,
        _grwlWaitEventsNull,
        _grwlWaitEventsTimeoutNull,
        _grwlPostEmptyEventNull,
        _grwlCreateUserContextNull,
        _grwlGetEGLPlatformNull,
        _grwlGetEGLNativeDisplayNull,
        _grwlGetEGLNativeWindowNull,
        _grwlGetRequiredInstanceExtensionsNull,
        _grwlGetPhysicalDevicePresentationSupportNull,
        _grwlCreateWindowSurfaceNull,
    };

    *platform = null;
    return GRWL_TRUE;
}

int _grwlInitNull(void)
{
    int scancode;

    memset(_grwl.null.keycodes, -1, sizeof(_grwl.null.keycodes));
    memset(_grwl.null.scancodes, -1, sizeof(_grwl.null.scancodes));

    _grwl.null.keycodes[GRWL_NULL_SC_SPACE] = GRWL_KEY_SPACE;
    _grwl.null.keycodes[GRWL_NULL_SC_APOSTROPHE] = GRWL_KEY_APOSTROPHE;
    _grwl.null.keycodes[GRWL_NULL_SC_COMMA] = GRWL_KEY_COMMA;
    _grwl.null.keycodes[GRWL_NULL_SC_MINUS] = GRWL_KEY_MINUS;
    _grwl.null.keycodes[GRWL_NULL_SC_PERIOD] = GRWL_KEY_PERIOD;
    _grwl.null.keycodes[GRWL_NULL_SC_SLASH] = GRWL_KEY_SLASH;
    _grwl.null.keycodes[GRWL_NULL_SC_0] = GRWL_KEY_0;
    _grwl.null.keycodes[GRWL_NULL_SC_1] = GRWL_KEY_1;
    _grwl.null.keycodes[GRWL_NULL_SC_2] = GRWL_KEY_2;
    _grwl.null.keycodes[GRWL_NULL_SC_3] = GRWL_KEY_3;
    _grwl.null.keycodes[GRWL_NULL_SC_4] = GRWL_KEY_4;
    _grwl.null.keycodes[GRWL_NULL_SC_5] = GRWL_KEY_5;
    _grwl.null.keycodes[GRWL_NULL_SC_6] = GRWL_KEY_6;
    _grwl.null.keycodes[GRWL_NULL_SC_7] = GRWL_KEY_7;
    _grwl.null.keycodes[GRWL_NULL_SC_8] = GRWL_KEY_8;
    _grwl.null.keycodes[GRWL_NULL_SC_9] = GRWL_KEY_9;
    _grwl.null.keycodes[GRWL_NULL_SC_SEMICOLON] = GRWL_KEY_SEMICOLON;
    _grwl.null.keycodes[GRWL_NULL_SC_EQUAL] = GRWL_KEY_EQUAL;
    _grwl.null.keycodes[GRWL_NULL_SC_A] = GRWL_KEY_A;
    _grwl.null.keycodes[GRWL_NULL_SC_B] = GRWL_KEY_B;
    _grwl.null.keycodes[GRWL_NULL_SC_C] = GRWL_KEY_C;
    _grwl.null.keycodes[GRWL_NULL_SC_D] = GRWL_KEY_D;
    _grwl.null.keycodes[GRWL_NULL_SC_E] = GRWL_KEY_E;
    _grwl.null.keycodes[GRWL_NULL_SC_F] = GRWL_KEY_F;
    _grwl.null.keycodes[GRWL_NULL_SC_G] = GRWL_KEY_G;
    _grwl.null.keycodes[GRWL_NULL_SC_H] = GRWL_KEY_H;
    _grwl.null.keycodes[GRWL_NULL_SC_I] = GRWL_KEY_I;
    _grwl.null.keycodes[GRWL_NULL_SC_J] = GRWL_KEY_J;
    _grwl.null.keycodes[GRWL_NULL_SC_K] = GRWL_KEY_K;
    _grwl.null.keycodes[GRWL_NULL_SC_L] = GRWL_KEY_L;
    _grwl.null.keycodes[GRWL_NULL_SC_M] = GRWL_KEY_M;
    _grwl.null.keycodes[GRWL_NULL_SC_N] = GRWL_KEY_N;
    _grwl.null.keycodes[GRWL_NULL_SC_O] = GRWL_KEY_O;
    _grwl.null.keycodes[GRWL_NULL_SC_P] = GRWL_KEY_P;
    _grwl.null.keycodes[GRWL_NULL_SC_Q] = GRWL_KEY_Q;
    _grwl.null.keycodes[GRWL_NULL_SC_R] = GRWL_KEY_R;
    _grwl.null.keycodes[GRWL_NULL_SC_S] = GRWL_KEY_S;
    _grwl.null.keycodes[GRWL_NULL_SC_T] = GRWL_KEY_T;
    _grwl.null.keycodes[GRWL_NULL_SC_U] = GRWL_KEY_U;
    _grwl.null.keycodes[GRWL_NULL_SC_V] = GRWL_KEY_V;
    _grwl.null.keycodes[GRWL_NULL_SC_W] = GRWL_KEY_W;
    _grwl.null.keycodes[GRWL_NULL_SC_X] = GRWL_KEY_X;
    _grwl.null.keycodes[GRWL_NULL_SC_Y] = GRWL_KEY_Y;
    _grwl.null.keycodes[GRWL_NULL_SC_Z] = GRWL_KEY_Z;
    _grwl.null.keycodes[GRWL_NULL_SC_LEFT_BRACKET] = GRWL_KEY_LEFT_BRACKET;
    _grwl.null.keycodes[GRWL_NULL_SC_BACKSLASH] = GRWL_KEY_BACKSLASH;
    _grwl.null.keycodes[GRWL_NULL_SC_RIGHT_BRACKET] = GRWL_KEY_RIGHT_BRACKET;
    _grwl.null.keycodes[GRWL_NULL_SC_GRAVE_ACCENT] = GRWL_KEY_GRAVE_ACCENT;
    _grwl.null.keycodes[GRWL_NULL_SC_WORLD_1] = GRWL_KEY_WORLD_1;
    _grwl.null.keycodes[GRWL_NULL_SC_WORLD_2] = GRWL_KEY_WORLD_2;
    _grwl.null.keycodes[GRWL_NULL_SC_ESCAPE] = GRWL_KEY_ESCAPE;
    _grwl.null.keycodes[GRWL_NULL_SC_ENTER] = GRWL_KEY_ENTER;
    _grwl.null.keycodes[GRWL_NULL_SC_TAB] = GRWL_KEY_TAB;
    _grwl.null.keycodes[GRWL_NULL_SC_BACKSPACE] = GRWL_KEY_BACKSPACE;
    _grwl.null.keycodes[GRWL_NULL_SC_INSERT] = GRWL_KEY_INSERT;
    _grwl.null.keycodes[GRWL_NULL_SC_DELETE] = GRWL_KEY_DELETE;
    _grwl.null.keycodes[GRWL_NULL_SC_RIGHT] = GRWL_KEY_RIGHT;
    _grwl.null.keycodes[GRWL_NULL_SC_LEFT] = GRWL_KEY_LEFT;
    _grwl.null.keycodes[GRWL_NULL_SC_DOWN] = GRWL_KEY_DOWN;
    _grwl.null.keycodes[GRWL_NULL_SC_UP] = GRWL_KEY_UP;
    _grwl.null.keycodes[GRWL_NULL_SC_PAGE_UP] = GRWL_KEY_PAGE_UP;
    _grwl.null.keycodes[GRWL_NULL_SC_PAGE_DOWN] = GRWL_KEY_PAGE_DOWN;
    _grwl.null.keycodes[GRWL_NULL_SC_HOME] = GRWL_KEY_HOME;
    _grwl.null.keycodes[GRWL_NULL_SC_END] = GRWL_KEY_END;
    _grwl.null.keycodes[GRWL_NULL_SC_CAPS_LOCK] = GRWL_KEY_CAPS_LOCK;
    _grwl.null.keycodes[GRWL_NULL_SC_SCROLL_LOCK] = GRWL_KEY_SCROLL_LOCK;
    _grwl.null.keycodes[GRWL_NULL_SC_NUM_LOCK] = GRWL_KEY_NUM_LOCK;
    _grwl.null.keycodes[GRWL_NULL_SC_PRINT_SCREEN] = GRWL_KEY_PRINT_SCREEN;
    _grwl.null.keycodes[GRWL_NULL_SC_PAUSE] = GRWL_KEY_PAUSE;
    _grwl.null.keycodes[GRWL_NULL_SC_F1] = GRWL_KEY_F1;
    _grwl.null.keycodes[GRWL_NULL_SC_F2] = GRWL_KEY_F2;
    _grwl.null.keycodes[GRWL_NULL_SC_F3] = GRWL_KEY_F3;
    _grwl.null.keycodes[GRWL_NULL_SC_F4] = GRWL_KEY_F4;
    _grwl.null.keycodes[GRWL_NULL_SC_F5] = GRWL_KEY_F5;
    _grwl.null.keycodes[GRWL_NULL_SC_F6] = GRWL_KEY_F6;
    _grwl.null.keycodes[GRWL_NULL_SC_F7] = GRWL_KEY_F7;
    _grwl.null.keycodes[GRWL_NULL_SC_F8] = GRWL_KEY_F8;
    _grwl.null.keycodes[GRWL_NULL_SC_F9] = GRWL_KEY_F9;
    _grwl.null.keycodes[GRWL_NULL_SC_F10] = GRWL_KEY_F10;
    _grwl.null.keycodes[GRWL_NULL_SC_F11] = GRWL_KEY_F11;
    _grwl.null.keycodes[GRWL_NULL_SC_F12] = GRWL_KEY_F12;
    _grwl.null.keycodes[GRWL_NULL_SC_F13] = GRWL_KEY_F13;
    _grwl.null.keycodes[GRWL_NULL_SC_F14] = GRWL_KEY_F14;
    _grwl.null.keycodes[GRWL_NULL_SC_F15] = GRWL_KEY_F15;
    _grwl.null.keycodes[GRWL_NULL_SC_F16] = GRWL_KEY_F16;
    _grwl.null.keycodes[GRWL_NULL_SC_F17] = GRWL_KEY_F17;
    _grwl.null.keycodes[GRWL_NULL_SC_F18] = GRWL_KEY_F18;
    _grwl.null.keycodes[GRWL_NULL_SC_F19] = GRWL_KEY_F19;
    _grwl.null.keycodes[GRWL_NULL_SC_F20] = GRWL_KEY_F20;
    _grwl.null.keycodes[GRWL_NULL_SC_F21] = GRWL_KEY_F21;
    _grwl.null.keycodes[GRWL_NULL_SC_F22] = GRWL_KEY_F22;
    _grwl.null.keycodes[GRWL_NULL_SC_F23] = GRWL_KEY_F23;
    _grwl.null.keycodes[GRWL_NULL_SC_F24] = GRWL_KEY_F24;
    _grwl.null.keycodes[GRWL_NULL_SC_F25] = GRWL_KEY_F25;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_0] = GRWL_KEY_KP_0;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_1] = GRWL_KEY_KP_1;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_2] = GRWL_KEY_KP_2;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_3] = GRWL_KEY_KP_3;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_4] = GRWL_KEY_KP_4;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_5] = GRWL_KEY_KP_5;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_6] = GRWL_KEY_KP_6;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_7] = GRWL_KEY_KP_7;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_8] = GRWL_KEY_KP_8;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_9] = GRWL_KEY_KP_9;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_DECIMAL] = GRWL_KEY_KP_DECIMAL;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_DIVIDE] = GRWL_KEY_KP_DIVIDE;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_MULTIPLY] = GRWL_KEY_KP_MULTIPLY;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_SUBTRACT] = GRWL_KEY_KP_SUBTRACT;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_ADD] = GRWL_KEY_KP_ADD;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_ENTER] = GRWL_KEY_KP_ENTER;
    _grwl.null.keycodes[GRWL_NULL_SC_KP_EQUAL] = GRWL_KEY_KP_EQUAL;
    _grwl.null.keycodes[GRWL_NULL_SC_LEFT_SHIFT] = GRWL_KEY_LEFT_SHIFT;
    _grwl.null.keycodes[GRWL_NULL_SC_LEFT_CONTROL] = GRWL_KEY_LEFT_CONTROL;
    _grwl.null.keycodes[GRWL_NULL_SC_LEFT_ALT] = GRWL_KEY_LEFT_ALT;
    _grwl.null.keycodes[GRWL_NULL_SC_LEFT_SUPER] = GRWL_KEY_LEFT_SUPER;
    _grwl.null.keycodes[GRWL_NULL_SC_RIGHT_SHIFT] = GRWL_KEY_RIGHT_SHIFT;
    _grwl.null.keycodes[GRWL_NULL_SC_RIGHT_CONTROL] = GRWL_KEY_RIGHT_CONTROL;
    _grwl.null.keycodes[GRWL_NULL_SC_RIGHT_ALT] = GRWL_KEY_RIGHT_ALT;
    _grwl.null.keycodes[GRWL_NULL_SC_RIGHT_SUPER] = GRWL_KEY_RIGHT_SUPER;
    _grwl.null.keycodes[GRWL_NULL_SC_MENU] = GRWL_KEY_MENU;

    for (scancode = GRWL_NULL_SC_FIRST; scancode < GRWL_NULL_SC_LAST; scancode++)
    {
        if (_grwl.null.keycodes[scancode] > 0)
        {
            _grwl.null.scancodes[_grwl.null.keycodes[scancode]] = scancode;
        }
    }

    _grwlPollMonitorsNull();
    return GRWL_TRUE;
}

void _grwlTerminateNull(void)
{
    free(_grwl.null.clipboardString);
    _grwlTerminateOSMesa();
    _grwlTerminateEGL();
}
