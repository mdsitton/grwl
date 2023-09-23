//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <linux/input.h>
#include <linux/limits.h>
#include <regex.h>

#define GLFW_LINUX_JOYSTICK_STATE _GLFWjoystickLinux linjs;
#define GLFW_LINUX_LIBRARY_JOYSTICK_STATE _GLFWlibraryLinux linjs;

// Linux-specific joystick data
//
typedef struct _GLFWjoystickLinux
{
    int fd;
    char path[PATH_MAX];
    int keyMap[KEY_CNT - BTN_MISC];
    int absMap[ABS_CNT];
    struct input_absinfo absInfo[ABS_CNT];
    int hats[4][2];
} _GLFWjoystickLinux;

// Linux-specific joystick API data
//
typedef struct _GLFWlibraryLinux
{
    int inotify;
    int watch;
    regex_t regex;
    GLFWbool dropped;
} _GLFWlibraryLinux;

void _glfwDetectJoystickConnectionLinux(void);

GLFWbool _glfwInitJoysticksLinux(void);
void _glfwTerminateJoysticksLinux(void);
GLFWbool _glfwPollJoystickLinux(_GLFWjoystick* js, int mode);
const char* _glfwGetMappingNameLinux(void);
void _glfwUpdateGamepadGUIDLinux(char* guid);
