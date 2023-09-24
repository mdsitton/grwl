//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"
#include "mappings.hpp"

#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstring>

// Internal key state used for sticky keys
#define _GRWL_STICK 3

// Internal constants for gamepad mapping source types
#define _GRWL_JOYSTICK_AXIS 1
#define _GRWL_JOYSTICK_BUTTON 2
#define _GRWL_JOYSTICK_HATBIT 3

#define GRWL_MOD_MASK \
    (GRWL_MOD_SHIFT | GRWL_MOD_CONTROL | GRWL_MOD_ALT | GRWL_MOD_SUPER | GRWL_MOD_CAPS_LOCK | GRWL_MOD_NUM_LOCK)

// Initializes the platform joystick API if it has not been already
//
static bool initJoysticks()
{
    if (!_grwl.joysticksInitialized)
    {
        if (!_grwl.platform.initJoysticks())
        {
            _grwl.platform.terminateJoysticks();
            return false;
        }
    }

    return _grwl.joysticksInitialized = true;
}

#if defined(GRWL_BUILD_LINUX_JOYSTICK)

uint16_t parseHexDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    assert(false);
}

struct vendor_product
{
    uint16_t vendor;
    uint16_t product;
};

static struct vendor_product parseGUID(const char* guid)
{
    struct vendor_product result;
    result.vendor = parseHexDigit(guid[8]) | (parseHexDigit(guid[9]) << 8);
    result.product = parseHexDigit(guid[16]) | (parseHexDigit(guid[17]) << 8);
    return result;
}

static _GRWLmapping* findMappingUSBVendorProduct(const char* guid)
{

    struct vendor_product startingItem;
    startingItem = parseGUID(guid);

    for (int i = 0; i < _grwl.mappingCount; i++)
    {
        struct vendor_product testItem = parseGUID(_grwl.mappings[i].guid);

        if (memcmp(&startingItem, &testItem, sizeof(struct vendor_product)) == 0)
        {
            return _grwl.mappings + i;
        }
    }

    return NULL;
}

#endif

// Finds a mapping based on joystick GUID
//
static _GRWLmapping* findMapping(const char* guid)
{
    // exact match
    for (int i = 0; i < _grwl.mappingCount; i++)
    {
        if (strncmp(_grwl.mappings[i].guid, guid, 32) == 0)
        {
            return _grwl.mappings + i;
        }
    }

#if defined(GRWL_BUILD_LINUX_JOYSTICK)
    // only match vendor id, product id
    return findMappingUSBVendorProduct(guid);
#else
    return NULL;
#endif
}

// Checks whether a gamepad mapping element is present in the hardware
//
static bool isValidElementForJoystick(const _GRWLmapelement* e, const _GRWLjoystick* js)
{
    if (e->type == _GRWL_JOYSTICK_HATBIT && (e->index >> 4) >= js->hatCount)
    {
        return false;
    }
    else if (e->type == _GRWL_JOYSTICK_BUTTON && e->index >= js->buttonCount)
    {
        return false;
    }
    else if (e->type == _GRWL_JOYSTICK_AXIS && e->index >= js->axisCount)
    {
        return false;
    }

    return true;
}

// Finds a mapping based on joystick GUID and verifies element indices
//
static _GRWLmapping* findValidMapping(const _GRWLjoystick* js)
{
    _GRWLmapping* mapping = findMapping(js->guid);
    if (mapping)
    {
        int i;

        for (i = 0; i <= GRWL_GAMEPAD_BUTTON_LAST; i++)
        {
            if (!isValidElementForJoystick(mapping->buttons + i, js))
            {
                return NULL;
            }
        }

        for (i = 0; i <= GRWL_GAMEPAD_AXIS_LAST; i++)
        {
            if (!isValidElementForJoystick(mapping->axes + i, js))
            {
                return NULL;
            }
        }
    }

    return mapping;
}

// Parses an SDL_GameControllerDB line and adds it to the mapping list
//
static bool parseMapping(_GRWLmapping* mapping, const char* string)
{
    const char* c = string;
    size_t i, length;

    struct
    {
        const char* name;
        _GRWLmapelement* element;
    } fields[] = { { "platform", NULL },
                   { "a", mapping->buttons + GRWL_GAMEPAD_BUTTON_A },
                   { "b", mapping->buttons + GRWL_GAMEPAD_BUTTON_B },
                   { "x", mapping->buttons + GRWL_GAMEPAD_BUTTON_X },
                   { "y", mapping->buttons + GRWL_GAMEPAD_BUTTON_Y },
                   { "back", mapping->buttons + GRWL_GAMEPAD_BUTTON_BACK },
                   { "start", mapping->buttons + GRWL_GAMEPAD_BUTTON_START },
                   { "guide", mapping->buttons + GRWL_GAMEPAD_BUTTON_GUIDE },
                   { "leftshoulder", mapping->buttons + GRWL_GAMEPAD_BUTTON_LEFT_BUMPER },
                   { "rightshoulder", mapping->buttons + GRWL_GAMEPAD_BUTTON_RIGHT_BUMPER },
                   { "leftstick", mapping->buttons + GRWL_GAMEPAD_BUTTON_LEFT_THUMB },
                   { "rightstick", mapping->buttons + GRWL_GAMEPAD_BUTTON_RIGHT_THUMB },
                   { "dpup", mapping->buttons + GRWL_GAMEPAD_BUTTON_DPAD_UP },
                   { "dpright", mapping->buttons + GRWL_GAMEPAD_BUTTON_DPAD_RIGHT },
                   { "dpdown", mapping->buttons + GRWL_GAMEPAD_BUTTON_DPAD_DOWN },
                   { "dpleft", mapping->buttons + GRWL_GAMEPAD_BUTTON_DPAD_LEFT },
                   { "lefttrigger", mapping->axes + GRWL_GAMEPAD_AXIS_LEFT_TRIGGER },
                   { "righttrigger", mapping->axes + GRWL_GAMEPAD_AXIS_RIGHT_TRIGGER },
                   { "leftx", mapping->axes + GRWL_GAMEPAD_AXIS_LEFT_X },
                   { "lefty", mapping->axes + GRWL_GAMEPAD_AXIS_LEFT_Y },
                   { "rightx", mapping->axes + GRWL_GAMEPAD_AXIS_RIGHT_X },
                   { "righty", mapping->axes + GRWL_GAMEPAD_AXIS_RIGHT_Y } };

    length = strcspn(c, ",");
    if (length != 32 || c[length] != ',')
    {
        _grwlInputError(GRWL_INVALID_VALUE, NULL);
        return false;
    }

    memcpy(mapping->guid, c, length);
    c += length + 1;

    length = strcspn(c, ",");
    if (length >= sizeof(mapping->name) || c[length] != ',')
    {
        _grwlInputError(GRWL_INVALID_VALUE, NULL);
        return false;
    }

    memcpy(mapping->name, c, length);
    c += length + 1;

    while (*c)
    {
        // TODO: Implement output modifiers
        if (*c == '+' || *c == '-')
        {
            return false;
        }

        for (i = 0; i < sizeof(fields) / sizeof(fields[0]); i++)
        {
            length = strlen(fields[i].name);
            if (strncmp(c, fields[i].name, length) != 0 || c[length] != ':')
            {
                continue;
            }

            c += length + 1;

            if (fields[i].element)
            {
                _GRWLmapelement* e = fields[i].element;
                int8_t minimum = -1;
                int8_t maximum = 1;

                if (*c == '+')
                {
                    minimum = 0;
                    c += 1;
                }
                else if (*c == '-')
                {
                    maximum = 0;
                    c += 1;
                }

                if (*c == 'a')
                {
                    e->type = _GRWL_JOYSTICK_AXIS;
                }
                else if (*c == 'b')
                {
                    e->type = _GRWL_JOYSTICK_BUTTON;
                }
                else if (*c == 'h')
                {
                    e->type = _GRWL_JOYSTICK_HATBIT;
                }
                else
                {
                    break;
                }

                if (e->type == _GRWL_JOYSTICK_HATBIT)
                {
                    const unsigned long hat = strtoul(c + 1, (char**)&c, 10);
                    const unsigned long bit = strtoul(c + 1, (char**)&c, 10);
                    e->index = (uint8_t)((hat << 4) | bit);
                }
                else
                {
                    e->index = (uint8_t)strtoul(c + 1, (char**)&c, 10);
                }

                if (e->type == _GRWL_JOYSTICK_AXIS)
                {
                    e->axisScale = 2 / (maximum - minimum);
                    e->axisOffset = -(maximum + minimum);

                    if (*c == '~')
                    {
                        e->axisScale = -e->axisScale;
                        e->axisOffset = -e->axisOffset;
                    }
                }
            }
            else
            {
                const char* name = _grwl.platform.getMappingName();
                length = strlen(name);
                if (strncmp(c, name, length) != 0)
                {
                    return false;
                }
            }

            break;
        }

        c += strcspn(c, ",");
        c += strspn(c, ",");
    }

    for (i = 0; i < 32; i++)
    {
        if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F')
        {
            mapping->guid[i] += 'a' - 'A';
        }
    }

    _grwl.platform.updateGamepadGUID(mapping->guid);
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////                         GRWL event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a keyboard layout change event
//
void _grwlInputKeyboardLayout()
{
    if (_grwl.callbacks.layout)
    {
        _grwl.callbacks.layout();
    }
}

// Notifies shared code of a physical key event
//
void _grwlInputKey(_GRWLwindow* window, int key, int scancode, int action, int mods)
{
    assert(window != NULL);
    assert(key >= 0 || key == GRWL_KEY_UNKNOWN);
    assert(key <= GRWL_KEY_LAST);
    assert(action == GRWL_PRESS || action == GRWL_RELEASE);
    assert(mods == (mods & GRWL_MOD_MASK));

    if (key >= 0 && key <= GRWL_KEY_LAST)
    {
        bool repeated = false;

        if (action == GRWL_RELEASE && window->keys[key] == GRWL_RELEASE)
        {
            return;
        }

        if (action == GRWL_PRESS && window->keys[key] == GRWL_PRESS)
        {
            repeated = true;
        }

        if (action == GRWL_RELEASE && window->stickyKeys)
        {
            window->keys[key] = _GRWL_STICK;
        }
        else
        {
            window->keys[key] = (char)action;
        }

        if (repeated)
        {
            action = GRWL_REPEAT;
        }
    }

    if (!window->lockKeyMods)
    {
        mods &= ~(GRWL_MOD_CAPS_LOCK | GRWL_MOD_NUM_LOCK);
    }

    if (window->callbacks.key)
    {
        window->callbacks.key((GRWLwindow*)window, key, scancode, action, mods);
    }
}

// Notifies shared code of a Unicode codepoint input event
// The 'plain' parameter determines whether to emit a regular character event
//
void _grwlInputChar(_GRWLwindow* window, uint32_t codepoint, int mods, bool plain)
{
    assert(window != NULL);
    assert(mods == (mods & GRWL_MOD_MASK));
    assert(plain == true || plain == false);

    if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
    {
        return;
    }

    if (!window->lockKeyMods)
    {
        mods &= ~(GRWL_MOD_CAPS_LOCK | GRWL_MOD_NUM_LOCK);
    }

    if (window->callbacks.charmods)
    {
        window->callbacks.charmods((GRWLwindow*)window, codepoint, mods);
    }

    if (plain)
    {
        if (window->callbacks.character)
        {
            window->callbacks.character((GRWLwindow*)window, codepoint);
        }
    }
}

// Notifies shared code of a preedit event
//
void _grwlInputPreedit(_GRWLwindow* window)
{
    if (window->callbacks.preedit)
    {
        _GRWLpreedit* preedit = &window->preedit;
        window->callbacks.preedit((GRWLwindow*)window, preedit->textCount, preedit->text, preedit->blockSizesCount,
                                  preedit->blockSizes, preedit->focusedBlockIndex, preedit->caretIndex);
    }
}

// Notifies shared code of a IME status event
//
void _grwlInputIMEStatus(_GRWLwindow* window)
{
    if (window->callbacks.imestatus)
    {
        window->callbacks.imestatus((GRWLwindow*)window);
    }
}

// Notifies shared code of a preedit candidate event
//
void _grwlInputPreeditCandidate(_GRWLwindow* window)
{
    if (window->callbacks.preeditCandidate)
    {
        _GRWLpreedit* preedit = &window->preedit;
        window->callbacks.preeditCandidate((GRWLwindow*)window, preedit->candidateCount, preedit->candidateSelection,
                                           preedit->candidatePageStart, preedit->candidatePageSize);
    }
}

// Notifies shared code of a scroll event
//
void _grwlInputScroll(_GRWLwindow* window, double xoffset, double yoffset)
{
    assert(window != NULL);
    assert(xoffset > -FLT_MAX);
    assert(xoffset < FLT_MAX);
    assert(yoffset > -FLT_MAX);
    assert(yoffset < FLT_MAX);

    if (window->callbacks.scroll)
    {
        window->callbacks.scroll((GRWLwindow*)window, xoffset, yoffset);
    }
}

// Notifies shared code of a mouse button click event
//
void _grwlInputMouseClick(_GRWLwindow* window, int button, int action, int mods)
{
    assert(window != NULL);
    assert(button >= 0);
    assert(button <= GRWL_MOUSE_BUTTON_LAST);
    assert(action == GRWL_PRESS || action == GRWL_RELEASE);
    assert(mods == (mods & GRWL_MOD_MASK));

    if (button < 0 || button > GRWL_MOUSE_BUTTON_LAST)
    {
        return;
    }

    if (!window->lockKeyMods)
    {
        mods &= ~(GRWL_MOD_CAPS_LOCK | GRWL_MOD_NUM_LOCK);
    }

    if (action == GRWL_RELEASE && window->stickyMouseButtons)
    {
        window->mouseButtons[button] = _GRWL_STICK;
    }
    else
    {
        window->mouseButtons[button] = (char)action;
    }

    if (window->callbacks.mouseButton)
    {
        window->callbacks.mouseButton((GRWLwindow*)window, button, action, mods);
    }
}

// Notifies shared code of a cursor motion event
// The position is specified in content area relative screen coordinates
//
void _grwlInputCursorPos(_GRWLwindow* window, double xpos, double ypos)
{
    assert(window != NULL);
    assert(xpos > -FLT_MAX);
    assert(xpos < FLT_MAX);
    assert(ypos > -FLT_MAX);
    assert(ypos < FLT_MAX);

    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos)
    {
        return;
    }

    window->virtualCursorPosX = xpos;
    window->virtualCursorPosY = ypos;

    if (window->callbacks.cursorPos)
    {
        window->callbacks.cursorPos((GRWLwindow*)window, xpos, ypos);
    }
}

// Notifies shared code of a cursor enter/leave event
//
void _grwlInputCursorEnter(_GRWLwindow* window, bool entered)
{
    assert(window != NULL);
    assert(entered == true || entered == false);

    if (window->callbacks.cursorEnter)
    {
        window->callbacks.cursorEnter((GRWLwindow*)window, entered);
    }
}

// Notifies shared code of files or directories dropped on a window
//
void _grwlInputDrop(_GRWLwindow* window, int count, const char** paths)
{
    assert(window != NULL);
    assert(count > 0);
    assert(paths != NULL);

    if (window->callbacks.drop)
    {
        window->callbacks.drop((GRWLwindow*)window, count, paths);
    }
}

// Notifies shared code of a change in the gamepad state.
// Automatically recalculates the state if the gamepad callback is installed.
void _grwlInputGamepad(_GRWLjoystick* js)
{
    if (!_grwl.initialized)
    {
        return;
    }

    if ((js->mapping != NULL) && (_grwl.callbacks.gamepad_state))
    {
        GRWLgamepadstate state;
        const int jid = (int)(js - _grwl.joysticks);
        if (grwlGetGamepadState(jid, &state))
        {
            _grwl.callbacks.gamepad_state(jid, state.buttons, state.axes);
        }
    }
}

// Notifies shared code of a joystick connection or disconnection
//
void _grwlInputJoystick(_GRWLjoystick* js, int event)
{
    assert(js != NULL);
    assert(event == GRWL_CONNECTED || event == GRWL_DISCONNECTED);

    if (event == GRWL_CONNECTED)
    {
        js->connected = true;
    }
    else if (event == GRWL_DISCONNECTED)
    {
        js->connected = false;
    }

    if (_grwl.callbacks.joystick)
    {
        _grwl.callbacks.joystick((int)(js - _grwl.joysticks), event);
    }
}

// Notifies shared code of the new value of a joystick axis
//
void _grwlInputJoystickAxis(_GRWLjoystick* js, int axis, float value)
{
    assert(js != NULL);
    assert(axis >= 0);
    assert(axis < js->axisCount);

    if (js->axes[axis] != value)
    {

        js->axes[axis] = value;
        if (_grwl.callbacks.joystick_axis)
        {
            _grwl.callbacks.joystick_axis((int)(js - _grwl.joysticks), axis, value);
        }
    }
}

// Notifies shared code of the new value of a joystick button
//
void _grwlInputJoystickButton(_GRWLjoystick* js, int button, char value)
{
    assert(js != NULL);
    assert(button >= 0);
    assert(button < js->buttonCount);
    assert(value == GRWL_PRESS || value == GRWL_RELEASE);

    if (js->buttons[button] != value)
    {
        js->buttons[button] = value;
        if (_grwl.callbacks.joystick_button)
        {
            _grwl.callbacks.joystick_button((int)(js - _grwl.joysticks), button, value);
        }
    }
}

// Notifies shared code of the new value of a joystick hat
//
void _grwlInputJoystickHat(_GRWLjoystick* js, int hat, char value)
{
    int base;

    assert(js != NULL);
    assert(hat >= 0);
    assert(hat < js->hatCount);

    // Valid hat values only use the least significant nibble and have at most two bits
    // set, which can be considered adjacent plus an arbitrary rotation within the nibble
    assert((value & 0xf0) == 0);
    assert((value & ((value << 2) | (value >> 2))) == 0);

    base = js->buttonCount + hat * 4;

    js->buttons[base + 0] = (value & 0x01) ? GRWL_PRESS : GRWL_RELEASE;
    js->buttons[base + 1] = (value & 0x02) ? GRWL_PRESS : GRWL_RELEASE;
    js->buttons[base + 2] = (value & 0x04) ? GRWL_PRESS : GRWL_RELEASE;
    js->buttons[base + 3] = (value & 0x08) ? GRWL_PRESS : GRWL_RELEASE;

    if (js->hats[hat] != value)
    {
        js->hats[hat] = value;
        if (_grwl.callbacks.joystick_hat)
        {
            _grwl.callbacks.joystick_hat((int)(js - _grwl.joysticks), hat, value);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Adds the built-in set of gamepad mappings
//
void _grwlInitGamepadMappings()
{
    size_t i;
    const size_t count = sizeof(_grwlDefaultMappings) / sizeof(char*);
    _grwl.mappings = (_GRWLmapping*)_grwl_calloc(count, sizeof(_GRWLmapping));

    for (i = 0; i < count; i++)
    {
        if (parseMapping(&_grwl.mappings[_grwl.mappingCount], _grwlDefaultMappings[i]))
        {
            _grwl.mappingCount++;
        }
    }
}

// Returns an available joystick object with arrays and name allocated
//
_GRWLjoystick* _grwlAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount)
{
    int jid;
    _GRWLjoystick* js;

    for (jid = 0; jid <= GRWL_JOYSTICK_LAST; jid++)
    {
        if (!_grwl.joysticks[jid].allocated)
        {
            break;
        }
    }

    if (jid > GRWL_JOYSTICK_LAST)
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    js->allocated = true;
    js->axes = (float*)_grwl_calloc(axisCount, sizeof(float));
    js->buttons = (unsigned char*)_grwl_calloc(buttonCount + (size_t)hatCount * 4, 1);
    js->hats = (unsigned char*)_grwl_calloc(hatCount, 1);
    js->axisCount = axisCount;
    js->buttonCount = buttonCount;
    js->hatCount = hatCount;

    strncpy(js->name, name, sizeof(js->name) - 1);
    strncpy(js->guid, guid, sizeof(js->guid) - 1);
    js->mapping = findValidMapping(js);

    return js;
}

// Frees arrays and name and flags the joystick object as unused
//
void _grwlFreeJoystick(_GRWLjoystick* js)
{
    _grwl_free(js->axes);
    _grwl_free(js->buttons);
    _grwl_free(js->hats);
    memset(js, 0, sizeof(_GRWLjoystick));
}

// Center the cursor in the content area of the specified window
//
void _grwlCenterCursorInContentArea(_GRWLwindow* window)
{
    int width, height;

    _grwl.platform.getWindowSize(window, &width, &height);
    _grwl.platform.setCursorPos(window, width / 2.0, height / 2.0);
}

void _grwlPollAllJoysticks()
{
    int jid;

    for (jid = 0; jid <= GRWL_JOYSTICK_LAST; jid++)
    {
        if (_grwl.joysticks[jid].connected == true)
        {
            _grwl.platform.pollJoystick(_grwl.joysticks + jid, _GRWL_POLL_ALL);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI int grwlGetInputMode(GRWLwindow* handle, int mode)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(0);

    switch (mode)
    {
        case GRWL_CURSOR:
            return window->cursorMode;
        case GRWL_STICKY_KEYS:
            return window->stickyKeys;
        case GRWL_STICKY_MOUSE_BUTTONS:
            return window->stickyMouseButtons;
        case GRWL_LOCK_KEY_MODS:
            return window->lockKeyMods;
        case GRWL_RAW_MOUSE_MOTION:
            return window->rawMouseMotion;
        case GRWL_IME:
            return _grwl.platform.getIMEStatus(window);
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
    return 0;
}

GRWLAPI void grwlSetInputMode(GRWLwindow* handle, int mode, int value)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    switch (mode)
    {
        case GRWL_CURSOR:
        {
            if (value != GRWL_CURSOR_NORMAL && value != GRWL_CURSOR_HIDDEN && value != GRWL_CURSOR_DISABLED &&
                value != GRWL_CURSOR_CAPTURED)
            {
                _grwlInputError(GRWL_INVALID_ENUM, "Invalid cursor mode 0x%08X", value);
                return;
            }

            if (window->cursorMode == value)
            {
                return;
            }

            window->cursorMode = value;

            _grwl.platform.getCursorPos(window, &window->virtualCursorPosX, &window->virtualCursorPosY);
            _grwl.platform.setCursorMode(window, value);
            return;
        }

        case GRWL_STICKY_KEYS:
        {
            bool bvalue = value ? true : false;
            if (window->stickyKeys == bvalue)
            {
                return;
            }

            if (!bvalue)
            {
                int i;

                // Release all sticky keys
                for (i = 0; i <= GRWL_KEY_LAST; i++)
                {
                    if (window->keys[i] == _GRWL_STICK)
                    {
                        window->keys[i] = GRWL_RELEASE;
                    }
                }
            }

            window->stickyKeys = bvalue;
            return;
        }

        case GRWL_STICKY_MOUSE_BUTTONS:
        {
            bool bvalue = value ? true : false;
            if (window->stickyMouseButtons == bvalue)
            {
                return;
            }

            if (!bvalue)
            {
                int i;

                // Release all sticky mouse buttons
                for (i = 0; i <= GRWL_MOUSE_BUTTON_LAST; i++)
                {
                    if (window->mouseButtons[i] == _GRWL_STICK)
                    {
                        window->mouseButtons[i] = GRWL_RELEASE;
                    }
                }
            }

            window->stickyMouseButtons = bvalue;
            return;
        }

        case GRWL_LOCK_KEY_MODS:
        {
            window->lockKeyMods = value ? true : false;
            return;
        }

        case GRWL_RAW_MOUSE_MOTION:
        {
            if (!_grwl.platform.rawMouseMotionSupported())
            {
                _grwlInputError(GRWL_PLATFORM_ERROR, "Raw mouse motion is not supported on this system");
                return;
            }

            bool bvalue = value ? true : false;
            if (window->rawMouseMotion == bvalue)
            {
                return;
            }

            window->rawMouseMotion = bvalue;
            _grwl.platform.setRawMouseMotion(window, bvalue);
            return;
        }

        case GRWL_IME:
        {
            _grwl.platform.setIMEStatus(window, value ? true : false);
            return;
        }
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
}

GRWLAPI int grwlRawMouseMotionSupported()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(false);
    return _grwl.platform.rawMouseMotionSupported();
}

GRWLAPI const char* grwlGetKeyName(int key, int scancode)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (key != GRWL_KEY_UNKNOWN)
    {
        if (key != GRWL_KEY_KP_EQUAL && (key < GRWL_KEY_KP_0 || key > GRWL_KEY_KP_ADD) &&
            (key < GRWL_KEY_APOSTROPHE || key > GRWL_KEY_WORLD_2))
        {
            return NULL;
        }

        scancode = _grwl.platform.getKeyScancode(key);
    }

    return _grwl.platform.getScancodeName(scancode);
}

GRWLAPI int grwlGetKeyScancode(int key)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(-1);

    if (key < GRWL_KEY_SPACE || key > GRWL_KEY_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid key %i", key);
        return GRWL_RELEASE;
    }

    return _grwl.platform.getKeyScancode(key);
}

GRWLAPI const char* grwlGetKeyboardLayoutName()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return _grwl.platform.getKeyboardLayoutName();
}

GRWLAPI GRWLkeyboardlayoutfun grwlSetKeyboardLayoutCallback(GRWLkeyboardlayoutfun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLkeyboardlayoutfun, _grwl.callbacks.layout, cbfun);
    return cbfun;
}

GRWLAPI int grwlGetKey(GRWLwindow* handle, int key)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(GRWL_RELEASE);

    if (key < GRWL_KEY_SPACE || key > GRWL_KEY_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid key %i", key);
        return GRWL_RELEASE;
    }

    if (window->keys[key] == _GRWL_STICK)
    {
        // Sticky mode: release key now
        window->keys[key] = GRWL_RELEASE;
        return GRWL_PRESS;
    }

    return (int)window->keys[key];
}

GRWLAPI int grwlGetMouseButton(GRWLwindow* handle, int button)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(GRWL_RELEASE);

    if (button < GRWL_MOUSE_BUTTON_1 || button > GRWL_MOUSE_BUTTON_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid mouse button %i", button);
        return GRWL_RELEASE;
    }

    if (window->mouseButtons[button] == _GRWL_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButtons[button] = GRWL_RELEASE;
        return GRWL_PRESS;
    }

    return (int)window->mouseButtons[button];
}

GRWLAPI void grwlGetCursorPos(GRWLwindow* handle, double* xpos, double* ypos)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    if (xpos)
    {
        *xpos = 0;
    }
    if (ypos)
    {
        *ypos = 0;
    }

    _GRWL_REQUIRE_INIT();

    if (window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        if (xpos)
        {
            *xpos = window->virtualCursorPosX;
        }
        if (ypos)
        {
            *ypos = window->virtualCursorPosY;
        }
    }
    else
    {
        _grwl.platform.getCursorPos(window, xpos, ypos);
    }
}

GRWLAPI void grwlSetCursorPos(GRWLwindow* handle, double xpos, double ypos)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    if (xpos != xpos || xpos < -DBL_MAX || xpos > DBL_MAX || ypos != ypos || ypos < -DBL_MAX || ypos > DBL_MAX)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid cursor position %f %f", xpos, ypos);
        return;
    }

    if (!_grwl.platform.windowFocused(window))
    {
        return;
    }

    if (window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        // Only update the accumulated position if the cursor is disabled
        window->virtualCursorPosX = xpos;
        window->virtualCursorPosY = ypos;
    }
    else
    {
        // Update system cursor position
        _grwl.platform.setCursorPos(window, xpos, ypos);
    }
}

GRWLAPI GRWLcursor* grwlCreateCursor(const GRWLimage* image, int xhot, int yhot)
{
    _GRWLcursor* cursor;

    assert(image != NULL);
    assert(image->pixels != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (image->width <= 0 || image->height <= 0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid image dimensions for cursor");
        return NULL;
    }

    cursor = (_GRWLcursor*)_grwl_calloc(1, sizeof(_GRWLcursor));
    cursor->next = _grwl.cursorListHead;
    _grwl.cursorListHead = cursor;

    if (!_grwl.platform.createCursor(cursor, image, xhot, yhot))
    {
        grwlDestroyCursor((GRWLcursor*)cursor);
        return NULL;
    }

    return (GRWLcursor*)cursor;
}

GRWLAPI GRWLcursor* grwlCreateStandardCursor(int shape)
{
    _GRWLcursor* cursor;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (shape != GRWL_ARROW_CURSOR && shape != GRWL_IBEAM_CURSOR && shape != GRWL_CROSSHAIR_CURSOR &&
        shape != GRWL_POINTING_HAND_CURSOR && shape != GRWL_RESIZE_EW_CURSOR && shape != GRWL_RESIZE_NS_CURSOR &&
        shape != GRWL_RESIZE_NWSE_CURSOR && shape != GRWL_RESIZE_NESW_CURSOR && shape != GRWL_RESIZE_ALL_CURSOR &&
        shape != GRWL_NOT_ALLOWED_CURSOR)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid standard cursor 0x%08X", shape);
        return NULL;
    }

    cursor = (_GRWLcursor*)_grwl_calloc(1, sizeof(_GRWLcursor));
    cursor->next = _grwl.cursorListHead;
    _grwl.cursorListHead = cursor;

    if (!_grwl.platform.createStandardCursor(cursor, shape))
    {
        grwlDestroyCursor((GRWLcursor*)cursor);
        return NULL;
    }

    return (GRWLcursor*)cursor;
}

GRWLAPI void grwlDestroyCursor(GRWLcursor* handle)
{
    _GRWLcursor* cursor = (_GRWLcursor*)handle;

    _GRWL_REQUIRE_INIT();

    if (cursor == NULL)
    {
        return;
    }

    // Make sure the cursor is not being used by any window
    {
        _GRWLwindow* window;

        for (window = _grwl.windowListHead; window; window = window->next)
        {
            if (window->cursor == cursor)
            {
                grwlSetCursor((GRWLwindow*)window, NULL);
            }
        }
    }

    _grwl.platform.destroyCursor(cursor);

    // Unlink cursor from global linked list
    {
        _GRWLcursor** prev = &_grwl.cursorListHead;

        while (*prev != cursor)
        {
            prev = &((*prev)->next);
        }

        *prev = cursor->next;
    }

    _grwl_free(cursor);
}

GRWLAPI void grwlSetCursor(GRWLwindow* windowHandle, GRWLcursor* cursorHandle)
{
    _GRWLwindow* window = (_GRWLwindow*)windowHandle;
    _GRWLcursor* cursor = (_GRWLcursor*)cursorHandle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT();

    window->cursor = cursor;

    _grwl.platform.setCursor(window, cursor);
}

GRWLAPI void grwlGetPreeditCursorRectangle(GRWLwindow* handle, int* x, int* y, int* w, int* h)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWLpreedit* preedit = &window->preedit;
    if (x)
    {
        *x = preedit->cursorPosX;
    }
    if (y)
    {
        *y = preedit->cursorPosY;
    }
    if (w)
    {
        *w = preedit->cursorWidth;
    }
    if (h)
    {
        *h = preedit->cursorHeight;
    }
}

GRWLAPI void grwlSetPreeditCursorRectangle(GRWLwindow* handle, int x, int y, int w, int h)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWLpreedit* preedit = &window->preedit;

    if (x == preedit->cursorPosX && y == preedit->cursorPosY && w == preedit->cursorWidth && h == preedit->cursorHeight)
    {
        return;
    }

    preedit->cursorPosX = x;
    preedit->cursorPosY = y;
    preedit->cursorWidth = w;
    preedit->cursorHeight = h;

    _grwl.platform.updatePreeditCursorRectangle(window);
}

GRWLAPI void grwlResetPreeditText(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _grwl.platform.resetPreeditText(window);
}

GRWLAPI unsigned int* grwlGetPreeditCandidate(GRWLwindow* handle, int index, int* textCount)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWLpreedit* preedit = &window->preedit;

    if (preedit->candidateCount <= index)
    {
        return NULL;
    }

    if (textCount)
    {
        *textCount = preedit->candidates[index].textCount;
    }

    return preedit->candidates[index].text;
}

GRWLAPI GRWLkeyfun grwlSetKeyCallback(GRWLwindow* handle, GRWLkeyfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLkeyfun, window->callbacks.key, cbfun);
    return cbfun;
}

GRWLAPI GRWLcharfun grwlSetCharCallback(GRWLwindow* handle, GRWLcharfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLcharfun, window->callbacks.character, cbfun);
    return cbfun;
}

GRWLAPI GRWLcharmodsfun grwlSetCharModsCallback(GRWLwindow* handle, GRWLcharmodsfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLcharmodsfun, window->callbacks.charmods, cbfun);
    return cbfun;
}

GRWLAPI GRWLpreeditfun grwlSetPreeditCallback(GRWLwindow* handle, GRWLpreeditfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLpreeditfun, window->callbacks.preedit, cbfun);
    return cbfun;
}

GRWLAPI GRWLimestatusfun grwlSetIMEStatusCallback(GRWLwindow* handle, GRWLimestatusfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLimestatusfun, window->callbacks.imestatus, cbfun);
    return cbfun;
}

GRWLAPI GRWLpreeditcandidatefun grwlSetPreeditCandidateCallback(GRWLwindow* handle, GRWLpreeditcandidatefun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLpreeditcandidatefun, window->callbacks.preeditCandidate, cbfun);
    return cbfun;
}

GRWLAPI GRWLmousebuttonfun grwlSetMouseButtonCallback(GRWLwindow* handle, GRWLmousebuttonfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLmousebuttonfun, window->callbacks.mouseButton, cbfun);
    return cbfun;
}

GRWLAPI GRWLcursorposfun grwlSetCursorPosCallback(GRWLwindow* handle, GRWLcursorposfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLcursorposfun, window->callbacks.cursorPos, cbfun);
    return cbfun;
}

GRWLAPI GRWLcursorenterfun grwlSetCursorEnterCallback(GRWLwindow* handle, GRWLcursorenterfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLcursorenterfun, window->callbacks.cursorEnter, cbfun);
    return cbfun;
}

GRWLAPI GRWLscrollfun grwlSetScrollCallback(GRWLwindow* handle, GRWLscrollfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLscrollfun, window->callbacks.scroll, cbfun);
    return cbfun;
}

GRWLAPI GRWLdropfun grwlSetDropCallback(GRWLwindow* handle, GRWLdropfun cbfun)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLdropfun, window->callbacks.drop, cbfun);
    return cbfun;
}

GRWLAPI int grwlJoystickPresent(int jid)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT_OR_RETURN(false);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return false;
    }

    if (!initJoysticks())
    {
        return false;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return false;
    }

    return _grwl.platform.pollJoystick(js, _GRWL_POLL_PRESENCE);
}

GRWLAPI const float* grwlGetJoystickAxes(int jid, int* count)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return NULL;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_AXES))
    {
        return NULL;
    }

    *count = js->axisCount;
    return js->axes;
}

GRWLAPI const unsigned char* grwlGetJoystickButtons(int jid, int* count)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return NULL;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_BUTTONS))
    {
        return NULL;
    }

    if (_grwl.hints.init.hatButtons)
    {
        *count = js->buttonCount + js->hatCount * 4;
    }
    else
    {
        *count = js->buttonCount;
    }

    return js->buttons;
}

GRWLAPI const unsigned char* grwlGetJoystickHats(int jid, int* count)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return NULL;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_BUTTONS))
    {
        return NULL;
    }

    *count = js->hatCount;
    return js->hats;
}

GRWLAPI const char* grwlGetJoystickName(int jid)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return NULL;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_PRESENCE))
    {
        return NULL;
    }

    return js->name;
}

GRWLAPI const char* grwlGetJoystickGUID(int jid)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return NULL;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_PRESENCE))
    {
        return NULL;
    }

    return js->guid;
}

GRWLAPI void grwlSetJoystickUserPointer(int jid, void* pointer)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT();

    js = _grwl.joysticks + jid;
    if (!js->allocated)
    {
        return;
    }

    js->userPointer = pointer;
}

GRWLAPI void* grwlGetJoystickUserPointer(int jid)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    js = _grwl.joysticks + jid;
    if (!js->allocated)
    {
        return NULL;
    }

    return js->userPointer;
}

GRWLAPI GRWLjoystickfun grwlSetJoystickCallback(GRWLjoystickfun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (!initJoysticks())
    {
        return NULL;
    }

    _GRWL_SWAP(GRWLjoystickfun, _grwl.callbacks.joystick, cbfun);
    return cbfun;
}

GRWLAPI GRWLgamepadstatefun grwlSetGamepadStateCallback(GRWLgamepadstatefun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLgamepadstatefun, _grwl.callbacks.gamepad_state, cbfun);
    return cbfun;
}

GRWLAPI GRWLjoystickbuttonfun grwlSetJoystickButtonCallback(GRWLjoystickbuttonfun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLjoystickbuttonfun, _grwl.callbacks.joystick_button, cbfun);
    return cbfun;
}

GRWLAPI GRWLjoystickaxisfun grwlSetJoystickAxisCallback(GRWLjoystickaxisfun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLjoystickaxisfun, _grwl.callbacks.joystick_axis, cbfun);
    return cbfun;
}

GRWLAPI GRWLjoystickhatfun grwlSetJoystickHatCallback(GRWLjoystickhatfun cbfun)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    _GRWL_SWAP(GRWLjoystickhatfun, _grwl.callbacks.joystick_hat, cbfun);
    return cbfun;
}

GRWLAPI int grwlUpdateGamepadMappings(const char* string)
{
    int jid;
    const char* c = string;

    assert(string != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(false);

    while (*c)
    {
        if ((*c >= '0' && *c <= '9') || (*c >= 'a' && *c <= 'f') || (*c >= 'A' && *c <= 'F'))
        {
            char line[1024];

            const size_t length = strcspn(c, "\r\n");
            if (length < sizeof(line))
            {
                _GRWLmapping mapping = { { 0 } };

                memcpy(line, c, length);
                line[length] = '\0';

                if (parseMapping(&mapping, line))
                {
                    _GRWLmapping* previous = findMapping(mapping.guid);
                    if (previous)
                    {
                        *previous = mapping;
                    }
                    else
                    {
                        _grwl.mappingCount++;
                        _grwl.mappings =
                            (_GRWLmapping*)_grwl_realloc(_grwl.mappings, sizeof(_GRWLmapping) * _grwl.mappingCount);
                        _grwl.mappings[_grwl.mappingCount - 1] = mapping;
                    }
                }
            }

            c += length;
        }
        else
        {
            c += strcspn(c, "\r\n");
            c += strspn(c, "\r\n");
        }
    }

    for (jid = 0; jid <= GRWL_JOYSTICK_LAST; jid++)
    {
        _GRWLjoystick* js = _grwl.joysticks + jid;
        if (js->connected)
        {
            js->mapping = findValidMapping(js);
        }
    }

    return true;
}

GRWLAPI int grwlJoystickIsGamepad(int jid)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT_OR_RETURN(false);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return false;
    }

    if (!initJoysticks())
    {
        return false;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return false;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_PRESENCE))
    {
        return false;
    }

    return js->mapping != NULL;
}

GRWLAPI const char* grwlGetGamepadName(int jid)
{
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
    {
        return NULL;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return NULL;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_PRESENCE))
    {
        return NULL;
    }

    if (!js->mapping)
    {
        return NULL;
    }

    return js->mapping->name;
}

GRWLAPI int grwlGetGamepadState(int jid, GRWLgamepadstate* state)
{
    int i;
    _GRWLjoystick* js;

    assert(jid >= GRWL_JOYSTICK_1);
    assert(jid <= GRWL_JOYSTICK_LAST);
    assert(state != NULL);

    memset(state, 0, sizeof(GRWLgamepadstate));

    _GRWL_REQUIRE_INIT_OR_RETURN(false);

    if (jid < 0 || jid > GRWL_JOYSTICK_LAST)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return false;
    }

    if (!initJoysticks())
    {
        return false;
    }

    js = _grwl.joysticks + jid;
    if (!js->connected)
    {
        return false;
    }

    if (!_grwl.platform.pollJoystick(js, _GRWL_POLL_ALL))
    {
        return false;
    }

    if (!js->mapping)
    {
        return false;
    }

    for (i = 0; i <= GRWL_GAMEPAD_BUTTON_LAST; i++)
    {
        const _GRWLmapelement* e = js->mapping->buttons + i;
        if (e->type == _GRWL_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            // HACK: This should be baked into the value transform
            // TODO: Bake into transform when implementing output modifiers
            if (e->axisOffset < 0 || (e->axisOffset == 0 && e->axisScale > 0))
            {
                if (value >= 0.f)
                {
                    state->buttons[i] = GRWL_PRESS;
                }
            }
            else
            {
                if (value <= 0.f)
                {
                    state->buttons[i] = GRWL_PRESS;
                }
            }
        }
        else if (e->type == _GRWL_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
            {
                state->buttons[i] = GRWL_PRESS;
            }
        }
        else if (e->type == _GRWL_JOYSTICK_BUTTON)
        {
            state->buttons[i] = js->buttons[e->index];
        }
    }

    for (i = 0; i <= GRWL_GAMEPAD_AXIS_LAST; i++)
    {
        const _GRWLmapelement* e = js->mapping->axes + i;
        if (e->type == _GRWL_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            state->axes[i] = _grwl_fminf(_grwl_fmaxf(value, -1.f), 1.f);
        }
        else if (e->type == _GRWL_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
            {
                state->axes[i] = 1.f;
            }
            else
            {
                state->axes[i] = -1.f;
            }
        }
        else if (e->type == _GRWL_JOYSTICK_BUTTON)
        {
            state->axes[i] = js->buttons[e->index] * 2.f - 1.f;
        }
    }

    return true;
}

GRWLAPI void grwlSetClipboardString(GRWLwindow* handle, const char* string)
{
    assert(string != NULL);

    _GRWL_REQUIRE_INIT();
    _grwl.platform.setClipboardString(string);
}

GRWLAPI const char* grwlGetClipboardString(GRWLwindow* handle)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return _grwl.platform.getClipboardString();
}

GRWLAPI double grwlGetTime()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(0.0);
    return (double)(_grwlPlatformGetTimerValue() - _grwl.timer.offset) / _grwlPlatformGetTimerFrequency();
}

GRWLAPI void grwlSetTime(double time)
{
    _GRWL_REQUIRE_INIT();

    if (time != time || time < 0.0 || time > 18446744073.0)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid time %f", time);
        return;
    }

    _grwl.timer.offset = _grwlPlatformGetTimerValue() - (uint64_t)(time * _grwlPlatformGetTimerFrequency());
}

GRWLAPI uint64_t grwlGetTimerValue()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(0);
    return _grwlPlatformGetTimerValue();
}

GRWLAPI uint64_t grwlGetTimerFrequency()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(0);
    return _grwlPlatformGetTimerFrequency();
}
