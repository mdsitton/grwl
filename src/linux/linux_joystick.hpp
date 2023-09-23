//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <linux/input.h>
#include <linux/limits.h>
#include <regex.h>

#define GRWL_LINUX_JOYSTICK_STATE _GRWLjoystickLinux linjs;
#define GRWL_LINUX_LIBRARY_JOYSTICK_STATE _GRWLlibraryLinux linjs;

// Linux-specific joystick data
//
typedef struct _GRWLjoystickLinux
{
    int fd;
    char path[PATH_MAX];
    int keyMap[KEY_CNT - BTN_MISC];
    int absMap[ABS_CNT];
    struct input_absinfo absInfo[ABS_CNT];
    int hats[4][2];
} _GRWLjoystickLinux;

// Linux-specific joystick API data
//
typedef struct _GRWLlibraryLinux
{
    int inotify;
    int watch;
    regex_t regex;
    GRWLbool dropped;
} _GRWLlibraryLinux;

void _grwlDetectJoystickConnectionLinux(void);

GRWLbool _grwlInitJoysticksLinux(void);
void _grwlTerminateJoysticksLinux(void);
GRWLbool _grwlPollJoystickLinux(_GRWLjoystick* js, int mode);
const char* _grwlGetMappingNameLinux(void);
void _grwlUpdateGamepadGUIDLinux(char* guid);
