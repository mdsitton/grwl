//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDKeys.h>

#define GLFW_COCOA_JOYSTICK_STATE _GLFWjoystickNS ns;
#define GLFW_COCOA_LIBRARY_JOYSTICK_STATE

// Cocoa-specific per-joystick data
//
typedef struct _GLFWjoystickNS
{
    IOHIDDeviceRef device;
    CFMutableArrayRef axes;
    CFMutableArrayRef buttons;
    CFMutableArrayRef hats;
} _GLFWjoystickNS;

GLFWbool _glfwInitJoysticksCocoa(void);
void _glfwTerminateJoysticksCocoa(void);
GLFWbool _glfwPollJoystickCocoa(_GLFWjoystick* js, int mode);
const char* _glfwGetMappingNameCocoa(void);
void _glfwUpdateGamepadGUIDCocoa(char* guid);
