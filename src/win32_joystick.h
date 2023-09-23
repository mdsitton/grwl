//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#define GLFW_WIN32_JOYSTICK_STATE _GLFWjoystickWin32 win32;
#define GLFW_WIN32_LIBRARY_JOYSTICK_STATE

// Joystick element (axis, button or slider)
//
typedef struct _GLFWjoyobjectWin32
{
    int offset;
    int type;
} _GLFWjoyobjectWin32;

// Win32-specific per-joystick data
//
typedef struct _GLFWjoystickWin32
{
    _GLFWjoyobjectWin32* objects;
    int objectCount;
    IDirectInputDevice8W* device;
    DWORD index;
    GUID guid;
} _GLFWjoystickWin32;

void _glfwDetectJoystickConnectionWin32(void);
void _glfwDetectJoystickDisconnectionWin32(void);
