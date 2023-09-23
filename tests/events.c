//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// This test hooks every available callback and outputs their arguments
//
// Log messages go to stdout, error messages to stderr
//
// Every event also gets a (sequential) number to aid discussion of logs

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include "getopt.h"

// Event index
static unsigned int counter = 0;

typedef struct
{
    GRWLwindow* window;
    int number;
    int closeable;
} Slot;

static void usage(void)
{
    printf("Usage: events [-f] [-h] [-n WINDOWS]\n");
    printf("Options:\n");
    printf("  -f use full screen\n");
    printf("  -h show this help\n");
    printf("  -n the number of windows to create\n");
}

static const char* get_key_name(int key)
{
    switch (key)
    {
        // Printable keys
        case GRWL_KEY_A:
            return "A";
        case GRWL_KEY_B:
            return "B";
        case GRWL_KEY_C:
            return "C";
        case GRWL_KEY_D:
            return "D";
        case GRWL_KEY_E:
            return "E";
        case GRWL_KEY_F:
            return "F";
        case GRWL_KEY_G:
            return "G";
        case GRWL_KEY_H:
            return "H";
        case GRWL_KEY_I:
            return "I";
        case GRWL_KEY_J:
            return "J";
        case GRWL_KEY_K:
            return "K";
        case GRWL_KEY_L:
            return "L";
        case GRWL_KEY_M:
            return "M";
        case GRWL_KEY_N:
            return "N";
        case GRWL_KEY_O:
            return "O";
        case GRWL_KEY_P:
            return "P";
        case GRWL_KEY_Q:
            return "Q";
        case GRWL_KEY_R:
            return "R";
        case GRWL_KEY_S:
            return "S";
        case GRWL_KEY_T:
            return "T";
        case GRWL_KEY_U:
            return "U";
        case GRWL_KEY_V:
            return "V";
        case GRWL_KEY_W:
            return "W";
        case GRWL_KEY_X:
            return "X";
        case GRWL_KEY_Y:
            return "Y";
        case GRWL_KEY_Z:
            return "Z";
        case GRWL_KEY_1:
            return "1";
        case GRWL_KEY_2:
            return "2";
        case GRWL_KEY_3:
            return "3";
        case GRWL_KEY_4:
            return "4";
        case GRWL_KEY_5:
            return "5";
        case GRWL_KEY_6:
            return "6";
        case GRWL_KEY_7:
            return "7";
        case GRWL_KEY_8:
            return "8";
        case GRWL_KEY_9:
            return "9";
        case GRWL_KEY_0:
            return "0";
        case GRWL_KEY_SPACE:
            return "SPACE";
        case GRWL_KEY_MINUS:
            return "MINUS";
        case GRWL_KEY_EQUAL:
            return "EQUAL";
        case GRWL_KEY_LEFT_BRACKET:
            return "LEFT BRACKET";
        case GRWL_KEY_RIGHT_BRACKET:
            return "RIGHT BRACKET";
        case GRWL_KEY_BACKSLASH:
            return "BACKSLASH";
        case GRWL_KEY_SEMICOLON:
            return "SEMICOLON";
        case GRWL_KEY_APOSTROPHE:
            return "APOSTROPHE";
        case GRWL_KEY_GRAVE_ACCENT:
            return "GRAVE ACCENT";
        case GRWL_KEY_COMMA:
            return "COMMA";
        case GRWL_KEY_PERIOD:
            return "PERIOD";
        case GRWL_KEY_SLASH:
            return "SLASH";
        case GRWL_KEY_WORLD_1:
            return "WORLD 1";
        case GRWL_KEY_WORLD_2:
            return "WORLD 2";

        // Function keys
        case GRWL_KEY_ESCAPE:
            return "ESCAPE";
        case GRWL_KEY_F1:
            return "F1";
        case GRWL_KEY_F2:
            return "F2";
        case GRWL_KEY_F3:
            return "F3";
        case GRWL_KEY_F4:
            return "F4";
        case GRWL_KEY_F5:
            return "F5";
        case GRWL_KEY_F6:
            return "F6";
        case GRWL_KEY_F7:
            return "F7";
        case GRWL_KEY_F8:
            return "F8";
        case GRWL_KEY_F9:
            return "F9";
        case GRWL_KEY_F10:
            return "F10";
        case GRWL_KEY_F11:
            return "F11";
        case GRWL_KEY_F12:
            return "F12";
        case GRWL_KEY_F13:
            return "F13";
        case GRWL_KEY_F14:
            return "F14";
        case GRWL_KEY_F15:
            return "F15";
        case GRWL_KEY_F16:
            return "F16";
        case GRWL_KEY_F17:
            return "F17";
        case GRWL_KEY_F18:
            return "F18";
        case GRWL_KEY_F19:
            return "F19";
        case GRWL_KEY_F20:
            return "F20";
        case GRWL_KEY_F21:
            return "F21";
        case GRWL_KEY_F22:
            return "F22";
        case GRWL_KEY_F23:
            return "F23";
        case GRWL_KEY_F24:
            return "F24";
        case GRWL_KEY_F25:
            return "F25";
        case GRWL_KEY_UP:
            return "UP";
        case GRWL_KEY_DOWN:
            return "DOWN";
        case GRWL_KEY_LEFT:
            return "LEFT";
        case GRWL_KEY_RIGHT:
            return "RIGHT";
        case GRWL_KEY_LEFT_SHIFT:
            return "LEFT SHIFT";
        case GRWL_KEY_RIGHT_SHIFT:
            return "RIGHT SHIFT";
        case GRWL_KEY_LEFT_CONTROL:
            return "LEFT CONTROL";
        case GRWL_KEY_RIGHT_CONTROL:
            return "RIGHT CONTROL";
        case GRWL_KEY_LEFT_ALT:
            return "LEFT ALT";
        case GRWL_KEY_RIGHT_ALT:
            return "RIGHT ALT";
        case GRWL_KEY_TAB:
            return "TAB";
        case GRWL_KEY_ENTER:
            return "ENTER";
        case GRWL_KEY_BACKSPACE:
            return "BACKSPACE";
        case GRWL_KEY_INSERT:
            return "INSERT";
        case GRWL_KEY_DELETE:
            return "DELETE";
        case GRWL_KEY_PAGE_UP:
            return "PAGE UP";
        case GRWL_KEY_PAGE_DOWN:
            return "PAGE DOWN";
        case GRWL_KEY_HOME:
            return "HOME";
        case GRWL_KEY_END:
            return "END";
        case GRWL_KEY_KP_0:
            return "KEYPAD 0";
        case GRWL_KEY_KP_1:
            return "KEYPAD 1";
        case GRWL_KEY_KP_2:
            return "KEYPAD 2";
        case GRWL_KEY_KP_3:
            return "KEYPAD 3";
        case GRWL_KEY_KP_4:
            return "KEYPAD 4";
        case GRWL_KEY_KP_5:
            return "KEYPAD 5";
        case GRWL_KEY_KP_6:
            return "KEYPAD 6";
        case GRWL_KEY_KP_7:
            return "KEYPAD 7";
        case GRWL_KEY_KP_8:
            return "KEYPAD 8";
        case GRWL_KEY_KP_9:
            return "KEYPAD 9";
        case GRWL_KEY_KP_DIVIDE:
            return "KEYPAD DIVIDE";
        case GRWL_KEY_KP_MULTIPLY:
            return "KEYPAD MULTIPLY";
        case GRWL_KEY_KP_SUBTRACT:
            return "KEYPAD SUBTRACT";
        case GRWL_KEY_KP_ADD:
            return "KEYPAD ADD";
        case GRWL_KEY_KP_DECIMAL:
            return "KEYPAD DECIMAL";
        case GRWL_KEY_KP_EQUAL:
            return "KEYPAD EQUAL";
        case GRWL_KEY_KP_ENTER:
            return "KEYPAD ENTER";
        case GRWL_KEY_PRINT_SCREEN:
            return "PRINT SCREEN";
        case GRWL_KEY_NUM_LOCK:
            return "NUM LOCK";
        case GRWL_KEY_CAPS_LOCK:
            return "CAPS LOCK";
        case GRWL_KEY_SCROLL_LOCK:
            return "SCROLL LOCK";
        case GRWL_KEY_PAUSE:
            return "PAUSE";
        case GRWL_KEY_LEFT_SUPER:
            return "LEFT SUPER";
        case GRWL_KEY_RIGHT_SUPER:
            return "RIGHT SUPER";
        case GRWL_KEY_MENU:
            return "MENU";

        default:
            return "UNKNOWN";
    }
}

static const char* get_action_name(int action)
{
    switch (action)
    {
        case GRWL_PRESS:
            return "pressed";
        case GRWL_RELEASE:
            return "released";
        case GRWL_REPEAT:
            return "repeated";
    }

    return "caused unknown action";
}

static const char* get_button_name(int button)
{
    switch (button)
    {
        case GRWL_MOUSE_BUTTON_LEFT:
            return "left";
        case GRWL_MOUSE_BUTTON_RIGHT:
            return "right";
        case GRWL_MOUSE_BUTTON_MIDDLE:
            return "middle";
        default:
        {
            static char name[16];
            snprintf(name, sizeof(name), "%i", button);
            return name;
        }
    }
}

static const char* get_mods_name(int mods)
{
    static char name[512];

    if (mods == 0)
    {
        return " no mods";
    }

    name[0] = '\0';

    if (mods & GRWL_MOD_SHIFT)
    {
        strcat(name, " shift");
    }
    if (mods & GRWL_MOD_CONTROL)
    {
        strcat(name, " control");
    }
    if (mods & GRWL_MOD_ALT)
    {
        strcat(name, " alt");
    }
    if (mods & GRWL_MOD_SUPER)
    {
        strcat(name, " super");
    }
    if (mods & GRWL_MOD_CAPS_LOCK)
    {
        strcat(name, " capslock-on");
    }
    if (mods & GRWL_MOD_NUM_LOCK)
    {
        strcat(name, " numlock-on");
    }

    return name;
}

static size_t encode_utf8(char* s, unsigned int ch)
{
    size_t count = 0;

    if (ch < 0x80)
    {
        s[count++] = (char)ch;
    }
    else if (ch < 0x800)
    {
        s[count++] = (ch >> 6) | 0xc0;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x10000)
    {
        s[count++] = (ch >> 12) | 0xe0;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x110000)
    {
        s[count++] = (ch >> 18) | 0xf0;
        s[count++] = ((ch >> 12) & 0x3f) | 0x80;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }

    return count;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_pos_callback(GRWLwindow* window, int x, int y)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window position: %i %i\n", counter++, slot->number, grwlGetTime(), x, y);
}

static void window_size_callback(GRWLwindow* window, int width, int height)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window size: %i %i\n", counter++, slot->number, grwlGetTime(), width, height);
}

static void framebuffer_size_callback(GRWLwindow* window, int width, int height)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Framebuffer size: %i %i\n", counter++, slot->number, grwlGetTime(), width, height);
}

static void window_content_scale_callback(GRWLwindow* window, float xscale, float yscale)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window content scale: %0.3f %0.3f\n", counter++, slot->number, grwlGetTime(), xscale,
           yscale);
}

static void window_close_callback(GRWLwindow* window)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window close\n", counter++, slot->number, grwlGetTime());

    if (!slot->closeable)
    {
        printf("(( closing is disabled, press %s to re-enable )\n", grwlGetKeyName(GRWL_KEY_C, 0));
    }

    grwlSetWindowShouldClose(window, slot->closeable);
}

static void window_refresh_callback(GRWLwindow* window)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window refresh\n", counter++, slot->number, grwlGetTime());

    grwlMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
    grwlSwapBuffers(window);
}

static void window_focus_callback(GRWLwindow* window, int focused)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window %s\n", counter++, slot->number, grwlGetTime(),
           focused ? "focused" : "defocused");
}

static void window_iconify_callback(GRWLwindow* window, int iconified)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window was %s\n", counter++, slot->number, grwlGetTime(),
           iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(GRWLwindow* window, int maximized)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window was %s\n", counter++, slot->number, grwlGetTime(),
           maximized ? "maximized" : "unmaximized");
}

static void mouse_button_callback(GRWLwindow* window, int button, int action, int mods)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Mouse button %i (%s) (with%s) was %s\n", counter++, slot->number, grwlGetTime(),
           button, get_button_name(button), get_mods_name(mods), get_action_name(action));
}

static void cursor_position_callback(GRWLwindow* window, double x, double y)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Cursor position: %f %f\n", counter++, slot->number, grwlGetTime(), x, y);
}

static void cursor_enter_callback(GRWLwindow* window, int entered)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Cursor %s window\n", counter++, slot->number, grwlGetTime(),
           entered ? "entered" : "left");
}

static void scroll_callback(GRWLwindow* window, double x, double y)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Scroll: %0.3f %0.3f\n", counter++, slot->number, grwlGetTime(), x, y);
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    const char* name = grwlGetKeyName(key, scancode);

    if (name)
    {
        printf("%08x to %i at %0.3f: Key 0x%04x Scancode 0x%04x (%s) (%s) (with%s) was %s\n", counter++, slot->number,
               grwlGetTime(), key, scancode, get_key_name(key), name, get_mods_name(mods), get_action_name(action));
    }
    else
    {
        printf("%08x to %i at %0.3f: Key 0x%04x Scancode 0x%04x (%s) (with%s) was %s\n", counter++, slot->number,
               grwlGetTime(), key, scancode, get_key_name(key), get_mods_name(mods), get_action_name(action));
    }

    if (action != GRWL_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GRWL_KEY_C:
        {
            slot->closeable = !slot->closeable;

            printf("(( closing %s ))\n", slot->closeable ? "enabled" : "disabled");
            break;
        }

        case GRWL_KEY_L:
        {
            const int state = grwlGetInputMode(window, GRWL_LOCK_KEY_MODS);
            grwlSetInputMode(window, GRWL_LOCK_KEY_MODS, !state);

            printf("(( lock key mods %s ))\n", !state ? "enabled" : "disabled");
            break;
        }
    }
}

static void char_callback(GRWLwindow* window, unsigned int codepoint)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    char string[5] = "";

    encode_utf8(string, codepoint);
    printf("%08x to %i at %0.3f: Character 0x%08x (%s) input\n", counter++, slot->number, grwlGetTime(), codepoint,
           string);
}

static void preedit_callback(GRWLwindow* window, int preeditCount, unsigned int* preeditString, int blockCount,
                             int* blockSizes, int focusedBlock, int caret)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    int i, blockIndex = -1, remainingBlockSize = 0;
    int width, height;
    char encoded[5] = "";
    size_t encodedCount = 0;
    printf("%08x to %i at %0.3f: Preedit text ", counter++, slot->number, grwlGetTime());
    if (preeditCount == 0 || blockCount == 0)
    {
        printf("(empty)\n");
    }
    else
    {
        for (i = 0; i < preeditCount; i++)
        {
            if (remainingBlockSize == 0)
            {
                if (blockIndex == focusedBlock)
                {
                    printf("]");
                }
                blockIndex++;
                remainingBlockSize = blockSizes[blockIndex];
                printf("\n   block %d: ", blockIndex);
                if (blockIndex == focusedBlock)
                {
                    printf("[");
                }
            }
            if (i == caret)
            {
                printf("|");
            }
            encodedCount = encode_utf8(encoded, preeditString[i]);
            encoded[encodedCount] = '\0';
            printf("%s", encoded);
            remainingBlockSize--;
        }
        if (blockIndex == focusedBlock)
        {
            printf("]");
        }
        if (caret == preeditCount)
        {
            printf("|");
        }
        printf("\n");
        grwlGetWindowSize(window, &width, &height);
        grwlSetPreeditCursorRectangle(window, width / 2, height / 2, 1, 20);
    }
}

static void ime_callback(GRWLwindow* window)
{
    Slot* slot = grwlGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: IME switched\n", counter++, slot->number, grwlGetTime());
}

static void drop_callback(GRWLwindow* window, int count, const char* paths[])
{
    int i;
    Slot* slot = grwlGetWindowUserPointer(window);

    printf("%08x to %i at %0.3f: Drop input\n", counter++, slot->number, grwlGetTime());

    for (i = 0; i < count; i++)
    {
        printf("  %i: \"%s\"\n", i, paths[i]);
    }
}

static void keyboard_layout_callback(void)
{
    printf("%08x at %0.3f: Keyboard layout changed to \'%s\'\n", counter++, grwlGetTime(), grwlGetKeyboardLayoutName());
}

static void monitor_callback(GRWLmonitor* monitor, int event)
{
    if (event == GRWL_CONNECTED)
    {
        int x, y, widthMM, heightMM;
        const GRWLvidmode* mode = grwlGetVideoMode(monitor);

        grwlGetMonitorPos(monitor, &x, &y);
        grwlGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

        printf("%08x at %0.3f: Monitor %s (%ix%i at %ix%i, %ix%i mm) was connected\n", counter++, grwlGetTime(),
               grwlGetMonitorName(monitor), mode->width, mode->height, x, y, widthMM, heightMM);
    }
    else if (event == GRWL_DISCONNECTED)
    {
        printf("%08x at %0.3f: Monitor %s was disconnected\n", counter++, grwlGetTime(), grwlGetMonitorName(monitor));
    }
}

static void joystick_callback(int jid, int event)
{
    if (event == GRWL_CONNECTED)
    {
        int axisCount, buttonCount, hatCount;

        grwlGetJoystickAxes(jid, &axisCount);
        grwlGetJoystickButtons(jid, &buttonCount);
        grwlGetJoystickHats(jid, &hatCount);

        printf("%08x at %0.3f: Joystick %i (%s) was connected with %i axes, %i buttons, and %i hats\n", counter++,
               grwlGetTime(), jid, grwlGetJoystickName(jid), axisCount, buttonCount, hatCount);

        if (grwlJoystickIsGamepad(jid))
        {
            printf("  Joystick %i (%s) has a gamepad mapping (%s)\n", jid, grwlGetJoystickGUID(jid),
                   grwlGetGamepadName(jid));
        }
        else
        {
            printf("  Joystick %i (%s) has no gamepad mapping\n", jid, grwlGetJoystickGUID(jid));
        }
    }
    else
    {
        printf("%08x at %0.3f: Joystick %i was disconnected\n", counter++, grwlGetTime(), jid);
    }
}

static void joystick_button_callback(int jid, int button, int state)
{
    printf("%08x at %0.3f: Joystick %i (%s) button %d state %d\n", counter++, grwlGetTime(), jid,
           grwlGetJoystickName(jid), button, state);
}

static void joystick_axis_callback(int jid, int axis, float value)
{
    printf("%08x at %0.3f: Joystick %i (%s) axis %d value %0.4f\n", counter++, grwlGetTime(), jid,
           grwlGetJoystickName(jid), axis, value);
}

static void joystick_hat_callback(int jid, int hat, int value)
{
    printf("%08x at %0.3f: Joystick %i (%s) hat %d value %d\n", counter++, grwlGetTime(), jid, grwlGetJoystickName(jid),
           hat, value);
}

static void gamepad_state_callback(int jid, unsigned char buttons[15], float axes[6])
{
    int i = 0;
    printf("%08x at %0.3f: Gamepad %i (%s) state:", counter++, grwlGetTime(), jid, grwlGetJoystickName(jid));

    printf("Buttons: ");
    for (i = 0; i < 15; i++)
    {
        printf(" %d:%d", i, buttons[i]);
    }
    printf("Axes: ");
    for (i = 0; i < 6; i++)
    {
        printf(" %d:%0.4f", i, axes[i]);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    Slot* slots;
    GRWLmonitor* monitor = NULL;
    int ch, i, width, height, count = 1;

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    printf("Library initialized\n");

    grwlSetMonitorCallback(monitor_callback);
    grwlSetJoystickCallback(joystick_callback);
    grwlSetKeyboardLayoutCallback(keyboard_layout_callback);
    grwlSetJoystickAxisCallback(joystick_axis_callback);
    grwlSetJoystickButtonCallback(joystick_button_callback);
    grwlSetJoystickHatCallback(joystick_hat_callback);
    grwlSetGamepadStateCallback(gamepad_state_callback);

    while ((ch = getopt(argc, argv, "hfn:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                monitor = grwlGetPrimaryMonitor();
                break;

            case 'n':
                count = (int)strtoul(optarg, NULL, 10);
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (monitor)
    {
        const GRWLvidmode* mode = grwlGetVideoMode(monitor);

        grwlWindowHint(GRWL_REFRESH_RATE, mode->refreshRate);
        grwlWindowHint(GRWL_RED_BITS, mode->redBits);
        grwlWindowHint(GRWL_GREEN_BITS, mode->greenBits);
        grwlWindowHint(GRWL_BLUE_BITS, mode->blueBits);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = 640;
        height = 480;
    }

    slots = calloc(count, sizeof(Slot));

    for (i = 0; i < count; i++)
    {
        char title[128];

        slots[i].closeable = GRWL_TRUE;
        slots[i].number = i + 1;

        snprintf(title, sizeof(title), "Event Linter (Window %i)", slots[i].number);

        if (monitor)
        {
            printf("Creating full screen window %i (%ix%i on %s)\n", slots[i].number, width, height,
                   grwlGetMonitorName(monitor));
        }
        else
        {
            printf("Creating windowed mode window %i (%ix%i)\n", slots[i].number, width, height);
        }

        slots[i].window = grwlCreateWindow(width, height, title, monitor, NULL);
        if (!slots[i].window)
        {
            free(slots);
            grwlTerminate();
            exit(EXIT_FAILURE);
        }

        grwlSetWindowUserPointer(slots[i].window, slots + i);

        grwlSetWindowPosCallback(slots[i].window, window_pos_callback);
        grwlSetWindowSizeCallback(slots[i].window, window_size_callback);
        grwlSetFramebufferSizeCallback(slots[i].window, framebuffer_size_callback);
        grwlSetWindowContentScaleCallback(slots[i].window, window_content_scale_callback);
        grwlSetWindowCloseCallback(slots[i].window, window_close_callback);
        grwlSetWindowRefreshCallback(slots[i].window, window_refresh_callback);
        grwlSetWindowFocusCallback(slots[i].window, window_focus_callback);
        grwlSetWindowIconifyCallback(slots[i].window, window_iconify_callback);
        grwlSetWindowMaximizeCallback(slots[i].window, window_maximize_callback);
        grwlSetMouseButtonCallback(slots[i].window, mouse_button_callback);
        grwlSetCursorPosCallback(slots[i].window, cursor_position_callback);
        grwlSetCursorEnterCallback(slots[i].window, cursor_enter_callback);
        grwlSetScrollCallback(slots[i].window, scroll_callback);
        grwlSetKeyCallback(slots[i].window, key_callback);
        grwlSetCharCallback(slots[i].window, char_callback);
        grwlSetPreeditCallback(slots[i].window, preedit_callback);
        grwlSetIMEStatusCallback(slots[i].window, ime_callback);
        grwlSetDropCallback(slots[i].window, drop_callback);

        grwlMakeContextCurrent(slots[i].window);
        gladLoadGL(grwlGetProcAddress);
        grwlSwapBuffers(slots[i].window);
    }

    printf("Main loop starting\n");

    for (;;)
    {
        for (i = 0; i < count; i++)
        {
            if (grwlWindowShouldClose(slots[i].window))
            {
                break;
            }
        }

        if (i < count)
        {
            break;
        }

        grwlWaitEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    free(slots);
    grwlTerminate();
    exit(EXIT_SUCCESS);
}
