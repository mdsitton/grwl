//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
#pragma once

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDKeys.h>

#define GRWL_COCOA_JOYSTICK_STATE _GRWLjoystickNS ns;
#define GRWL_COCOA_LIBRARY_JOYSTICK_STATE

// Cocoa-specific per-joystick data
//
typedef struct _GRWLjoystickNS
{
    IOHIDDeviceRef device;
    CFMutableArrayRef axes;
    CFMutableArrayRef buttons;
    CFMutableArrayRef hats;
} _GRWLjoystickNS;

GRWLbool _grwlInitJoysticksCocoa(void);
void _grwlTerminateJoysticksCocoa(void);
GRWLbool _grwlPollJoystickCocoa(_GRWLjoystick* js, int mode);
const char* _grwlGetMappingNameCocoa(void);
void _grwlUpdateGamepadGUIDCocoa(char* guid);
