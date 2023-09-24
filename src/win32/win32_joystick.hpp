//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
#pragma once

#define GRWL_WIN32_JOYSTICK_STATE _GRWLjoystickWin32 win32;
#define GRWL_WIN32_LIBRARY_JOYSTICK_STATE

// Joystick element (axis, button or slider)
//
typedef struct _GRWLjoyobjectWin32
{
    int offset;
    int type;
} _GRWLjoyobjectWin32;

// Win32-specific per-joystick data
//
typedef struct _GRWLjoystickWin32
{
    _GRWLjoyobjectWin32* objects;
    int objectCount;
    IDirectInputDevice8W* device;
    uint32_t index;
    GUID guid;
} _GRWLjoystickWin32;

void _grwlDetectJoystickConnectionWin32();
void _grwlDetectJoystickDisconnectionWin32();
