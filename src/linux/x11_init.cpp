//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_X11)

    #include <cstdlib>
    #include <cstring>
    #include <climits>
    #include <cstdio>
    #include <clocale>
    #include <unistd.h>
    #include <fcntl.h>
    #include <cerrno>
    #include <cassert>

// Translate the X11 KeySyms for a key to a GRWL key code
// NOTE: This is only used as a fallback, in case the XKB method fails
//       It is layout-dependent and will fail partially on most non-US layouts
//
static int translateKeySyms(const KeySym* keysyms, int width)
{
    if (width > 1)
    {
        switch (keysyms[1])
        {
            case XK_KP_0:
                return GRWL_KEY_KP_0;
            case XK_KP_1:
                return GRWL_KEY_KP_1;
            case XK_KP_2:
                return GRWL_KEY_KP_2;
            case XK_KP_3:
                return GRWL_KEY_KP_3;
            case XK_KP_4:
                return GRWL_KEY_KP_4;
            case XK_KP_5:
                return GRWL_KEY_KP_5;
            case XK_KP_6:
                return GRWL_KEY_KP_6;
            case XK_KP_7:
                return GRWL_KEY_KP_7;
            case XK_KP_8:
                return GRWL_KEY_KP_8;
            case XK_KP_9:
                return GRWL_KEY_KP_9;
            case XK_KP_Separator:
            case XK_KP_Decimal:
                return GRWL_KEY_KP_DECIMAL;
            case XK_KP_Equal:
                return GRWL_KEY_KP_EQUAL;
            case XK_KP_Enter:
                return GRWL_KEY_KP_ENTER;
            default:
                break;
        }
    }

    switch (keysyms[0])
    {
        case XK_Escape:
            return GRWL_KEY_ESCAPE;
        case XK_Tab:
            return GRWL_KEY_TAB;
        case XK_Shift_L:
            return GRWL_KEY_LEFT_SHIFT;
        case XK_Shift_R:
            return GRWL_KEY_RIGHT_SHIFT;
        case XK_Control_L:
            return GRWL_KEY_LEFT_CONTROL;
        case XK_Control_R:
            return GRWL_KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:
            return GRWL_KEY_LEFT_ALT;
        case XK_Mode_switch:      // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:
            return GRWL_KEY_RIGHT_ALT;
        case XK_Super_L:
            return GRWL_KEY_LEFT_SUPER;
        case XK_Super_R:
            return GRWL_KEY_RIGHT_SUPER;
        case XK_Menu:
            return GRWL_KEY_MENU;
        case XK_Num_Lock:
            return GRWL_KEY_NUM_LOCK;
        case XK_Caps_Lock:
            return GRWL_KEY_CAPS_LOCK;
        case XK_Print:
            return GRWL_KEY_PRINT_SCREEN;
        case XK_Scroll_Lock:
            return GRWL_KEY_SCROLL_LOCK;
        case XK_Pause:
            return GRWL_KEY_PAUSE;
        case XK_Delete:
            return GRWL_KEY_DELETE;
        case XK_BackSpace:
            return GRWL_KEY_BACKSPACE;
        case XK_Return:
            return GRWL_KEY_ENTER;
        case XK_Home:
            return GRWL_KEY_HOME;
        case XK_End:
            return GRWL_KEY_END;
        case XK_Page_Up:
            return GRWL_KEY_PAGE_UP;
        case XK_Page_Down:
            return GRWL_KEY_PAGE_DOWN;
        case XK_Insert:
            return GRWL_KEY_INSERT;
        case XK_Left:
            return GRWL_KEY_LEFT;
        case XK_Right:
            return GRWL_KEY_RIGHT;
        case XK_Down:
            return GRWL_KEY_DOWN;
        case XK_Up:
            return GRWL_KEY_UP;
        case XK_F1:
            return GRWL_KEY_F1;
        case XK_F2:
            return GRWL_KEY_F2;
        case XK_F3:
            return GRWL_KEY_F3;
        case XK_F4:
            return GRWL_KEY_F4;
        case XK_F5:
            return GRWL_KEY_F5;
        case XK_F6:
            return GRWL_KEY_F6;
        case XK_F7:
            return GRWL_KEY_F7;
        case XK_F8:
            return GRWL_KEY_F8;
        case XK_F9:
            return GRWL_KEY_F9;
        case XK_F10:
            return GRWL_KEY_F10;
        case XK_F11:
            return GRWL_KEY_F11;
        case XK_F12:
            return GRWL_KEY_F12;
        case XK_F13:
            return GRWL_KEY_F13;
        case XK_F14:
            return GRWL_KEY_F14;
        case XK_F15:
            return GRWL_KEY_F15;
        case XK_F16:
            return GRWL_KEY_F16;
        case XK_F17:
            return GRWL_KEY_F17;
        case XK_F18:
            return GRWL_KEY_F18;
        case XK_F19:
            return GRWL_KEY_F19;
        case XK_F20:
            return GRWL_KEY_F20;
        case XK_F21:
            return GRWL_KEY_F21;
        case XK_F22:
            return GRWL_KEY_F22;
        case XK_F23:
            return GRWL_KEY_F23;
        case XK_F24:
            return GRWL_KEY_F24;
        case XK_F25:
            return GRWL_KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:
            return GRWL_KEY_KP_DIVIDE;
        case XK_KP_Multiply:
            return GRWL_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:
            return GRWL_KEY_KP_SUBTRACT;
        case XK_KP_Add:
            return GRWL_KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:
            return GRWL_KEY_KP_0;
        case XK_KP_End:
            return GRWL_KEY_KP_1;
        case XK_KP_Down:
            return GRWL_KEY_KP_2;
        case XK_KP_Page_Down:
            return GRWL_KEY_KP_3;
        case XK_KP_Left:
            return GRWL_KEY_KP_4;
        case XK_KP_Right:
            return GRWL_KEY_KP_6;
        case XK_KP_Home:
            return GRWL_KEY_KP_7;
        case XK_KP_Up:
            return GRWL_KEY_KP_8;
        case XK_KP_Page_Up:
            return GRWL_KEY_KP_9;
        case XK_KP_Delete:
            return GRWL_KEY_KP_DECIMAL;
        case XK_KP_Equal:
            return GRWL_KEY_KP_EQUAL;
        case XK_KP_Enter:
            return GRWL_KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:
            return GRWL_KEY_A;
        case XK_b:
            return GRWL_KEY_B;
        case XK_c:
            return GRWL_KEY_C;
        case XK_d:
            return GRWL_KEY_D;
        case XK_e:
            return GRWL_KEY_E;
        case XK_f:
            return GRWL_KEY_F;
        case XK_g:
            return GRWL_KEY_G;
        case XK_h:
            return GRWL_KEY_H;
        case XK_i:
            return GRWL_KEY_I;
        case XK_j:
            return GRWL_KEY_J;
        case XK_k:
            return GRWL_KEY_K;
        case XK_l:
            return GRWL_KEY_L;
        case XK_m:
            return GRWL_KEY_M;
        case XK_n:
            return GRWL_KEY_N;
        case XK_o:
            return GRWL_KEY_O;
        case XK_p:
            return GRWL_KEY_P;
        case XK_q:
            return GRWL_KEY_Q;
        case XK_r:
            return GRWL_KEY_R;
        case XK_s:
            return GRWL_KEY_S;
        case XK_t:
            return GRWL_KEY_T;
        case XK_u:
            return GRWL_KEY_U;
        case XK_v:
            return GRWL_KEY_V;
        case XK_w:
            return GRWL_KEY_W;
        case XK_x:
            return GRWL_KEY_X;
        case XK_y:
            return GRWL_KEY_Y;
        case XK_z:
            return GRWL_KEY_Z;
        case XK_1:
            return GRWL_KEY_1;
        case XK_2:
            return GRWL_KEY_2;
        case XK_3:
            return GRWL_KEY_3;
        case XK_4:
            return GRWL_KEY_4;
        case XK_5:
            return GRWL_KEY_5;
        case XK_6:
            return GRWL_KEY_6;
        case XK_7:
            return GRWL_KEY_7;
        case XK_8:
            return GRWL_KEY_8;
        case XK_9:
            return GRWL_KEY_9;
        case XK_0:
            return GRWL_KEY_0;
        case XK_space:
            return GRWL_KEY_SPACE;
        case XK_minus:
            return GRWL_KEY_MINUS;
        case XK_equal:
            return GRWL_KEY_EQUAL;
        case XK_bracketleft:
            return GRWL_KEY_LEFT_BRACKET;
        case XK_bracketright:
            return GRWL_KEY_RIGHT_BRACKET;
        case XK_backslash:
            return GRWL_KEY_BACKSLASH;
        case XK_semicolon:
            return GRWL_KEY_SEMICOLON;
        case XK_apostrophe:
            return GRWL_KEY_APOSTROPHE;
        case XK_grave:
            return GRWL_KEY_GRAVE_ACCENT;
        case XK_comma:
            return GRWL_KEY_COMMA;
        case XK_period:
            return GRWL_KEY_PERIOD;
        case XK_slash:
            return GRWL_KEY_SLASH;
        case XK_less:
            return GRWL_KEY_WORLD_1; // At least in some layouts...
        default:
            break;
    }

    // No matching translation was found
    return GRWL_KEY_UNKNOWN;
}

// Create key code translation tables
//
static void createKeyTables()
{
    int scancodeMin, scancodeMax;

    memset(_grwl.x11.keycodes, -1, sizeof(_grwl.x11.keycodes));
    memset(_grwl.x11.scancodes, -1, sizeof(_grwl.x11.scancodes));

    if (_grwl.x11.xkb.available)
    {
        // Use XKB to determine physical key locations independently of the
        // current keyboard layout

        XkbDescPtr desc = XkbGetMap(_grwl.x11.display, 0, XkbUseCoreKbd);
        XkbGetNames(_grwl.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);

        scancodeMin = desc->min_key_code;
        scancodeMax = desc->max_key_code;

        const struct
        {
            int key;
            char* name;
        } keymap[] = { { GRWL_KEY_GRAVE_ACCENT, "TLDE" },
                       { GRWL_KEY_1, "AE01" },
                       { GRWL_KEY_2, "AE02" },
                       { GRWL_KEY_3, "AE03" },
                       { GRWL_KEY_4, "AE04" },
                       { GRWL_KEY_5, "AE05" },
                       { GRWL_KEY_6, "AE06" },
                       { GRWL_KEY_7, "AE07" },
                       { GRWL_KEY_8, "AE08" },
                       { GRWL_KEY_9, "AE09" },
                       { GRWL_KEY_0, "AE10" },
                       { GRWL_KEY_MINUS, "AE11" },
                       { GRWL_KEY_EQUAL, "AE12" },
                       { GRWL_KEY_Q, "AD01" },
                       { GRWL_KEY_W, "AD02" },
                       { GRWL_KEY_E, "AD03" },
                       { GRWL_KEY_R, "AD04" },
                       { GRWL_KEY_T, "AD05" },
                       { GRWL_KEY_Y, "AD06" },
                       { GRWL_KEY_U, "AD07" },
                       { GRWL_KEY_I, "AD08" },
                       { GRWL_KEY_O, "AD09" },
                       { GRWL_KEY_P, "AD10" },
                       { GRWL_KEY_LEFT_BRACKET, "AD11" },
                       { GRWL_KEY_RIGHT_BRACKET, "AD12" },
                       { GRWL_KEY_A, "AC01" },
                       { GRWL_KEY_S, "AC02" },
                       { GRWL_KEY_D, "AC03" },
                       { GRWL_KEY_F, "AC04" },
                       { GRWL_KEY_G, "AC05" },
                       { GRWL_KEY_H, "AC06" },
                       { GRWL_KEY_J, "AC07" },
                       { GRWL_KEY_K, "AC08" },
                       { GRWL_KEY_L, "AC09" },
                       { GRWL_KEY_SEMICOLON, "AC10" },
                       { GRWL_KEY_APOSTROPHE, "AC11" },
                       { GRWL_KEY_Z, "AB01" },
                       { GRWL_KEY_X, "AB02" },
                       { GRWL_KEY_C, "AB03" },
                       { GRWL_KEY_V, "AB04" },
                       { GRWL_KEY_B, "AB05" },
                       { GRWL_KEY_N, "AB06" },
                       { GRWL_KEY_M, "AB07" },
                       { GRWL_KEY_COMMA, "AB08" },
                       { GRWL_KEY_PERIOD, "AB09" },
                       { GRWL_KEY_SLASH, "AB10" },
                       { GRWL_KEY_BACKSLASH, "BKSL" },
                       { GRWL_KEY_WORLD_1, "LSGT" },
                       { GRWL_KEY_SPACE, "SPCE" },
                       { GRWL_KEY_ESCAPE, "ESC" },
                       { GRWL_KEY_ENTER, "RTRN" },
                       { GRWL_KEY_TAB, "TAB" },
                       { GRWL_KEY_BACKSPACE, "BKSP" },
                       { GRWL_KEY_INSERT, "INS" },
                       { GRWL_KEY_DELETE, "DELE" },
                       { GRWL_KEY_RIGHT, "RGHT" },
                       { GRWL_KEY_LEFT, "LEFT" },
                       { GRWL_KEY_DOWN, "DOWN" },
                       { GRWL_KEY_UP, "UP" },
                       { GRWL_KEY_PAGE_UP, "PGUP" },
                       { GRWL_KEY_PAGE_DOWN, "PGDN" },
                       { GRWL_KEY_HOME, "HOME" },
                       { GRWL_KEY_END, "END" },
                       { GRWL_KEY_CAPS_LOCK, "CAPS" },
                       { GRWL_KEY_SCROLL_LOCK, "SCLK" },
                       { GRWL_KEY_NUM_LOCK, "NMLK" },
                       { GRWL_KEY_PRINT_SCREEN, "PRSC" },
                       { GRWL_KEY_PAUSE, "PAUS" },
                       { GRWL_KEY_F1, "FK01" },
                       { GRWL_KEY_F2, "FK02" },
                       { GRWL_KEY_F3, "FK03" },
                       { GRWL_KEY_F4, "FK04" },
                       { GRWL_KEY_F5, "FK05" },
                       { GRWL_KEY_F6, "FK06" },
                       { GRWL_KEY_F7, "FK07" },
                       { GRWL_KEY_F8, "FK08" },
                       { GRWL_KEY_F9, "FK09" },
                       { GRWL_KEY_F10, "FK10" },
                       { GRWL_KEY_F11, "FK11" },
                       { GRWL_KEY_F12, "FK12" },
                       { GRWL_KEY_F13, "FK13" },
                       { GRWL_KEY_F14, "FK14" },
                       { GRWL_KEY_F15, "FK15" },
                       { GRWL_KEY_F16, "FK16" },
                       { GRWL_KEY_F17, "FK17" },
                       { GRWL_KEY_F18, "FK18" },
                       { GRWL_KEY_F19, "FK19" },
                       { GRWL_KEY_F20, "FK20" },
                       { GRWL_KEY_F21, "FK21" },
                       { GRWL_KEY_F22, "FK22" },
                       { GRWL_KEY_F23, "FK23" },
                       { GRWL_KEY_F24, "FK24" },
                       { GRWL_KEY_F25, "FK25" },
                       { GRWL_KEY_KP_0, "KP0" },
                       { GRWL_KEY_KP_1, "KP1" },
                       { GRWL_KEY_KP_2, "KP2" },
                       { GRWL_KEY_KP_3, "KP3" },
                       { GRWL_KEY_KP_4, "KP4" },
                       { GRWL_KEY_KP_5, "KP5" },
                       { GRWL_KEY_KP_6, "KP6" },
                       { GRWL_KEY_KP_7, "KP7" },
                       { GRWL_KEY_KP_8, "KP8" },
                       { GRWL_KEY_KP_9, "KP9" },
                       { GRWL_KEY_KP_DECIMAL, "KPDL" },
                       { GRWL_KEY_KP_DIVIDE, "KPDV" },
                       { GRWL_KEY_KP_MULTIPLY, "KPMU" },
                       { GRWL_KEY_KP_SUBTRACT, "KPSU" },
                       { GRWL_KEY_KP_ADD, "KPAD" },
                       { GRWL_KEY_KP_ENTER, "KPEN" },
                       { GRWL_KEY_KP_EQUAL, "KPEQ" },
                       { GRWL_KEY_LEFT_SHIFT, "LFSH" },
                       { GRWL_KEY_LEFT_CONTROL, "LCTL" },
                       { GRWL_KEY_LEFT_ALT, "LALT" },
                       { GRWL_KEY_LEFT_SUPER, "LWIN" },
                       { GRWL_KEY_RIGHT_SHIFT, "RTSH" },
                       { GRWL_KEY_RIGHT_CONTROL, "RCTL" },
                       { GRWL_KEY_RIGHT_ALT, "RALT" },
                       { GRWL_KEY_RIGHT_ALT, "LVL3" },
                       { GRWL_KEY_RIGHT_ALT, "MDSW" },
                       { GRWL_KEY_RIGHT_SUPER, "RWIN" },
                       { GRWL_KEY_MENU, "MENU" } };

        // Find the X11 key code -> GRWL key code mapping
        for (int scancode = scancodeMin; scancode <= scancodeMax; scancode++)
        {
            int key = GRWL_KEY_UNKNOWN;

            // Map the key name to a GRWL key code. Note: We use the US
            // keyboard layout. Because function keys aren't mapped correctly
            // when using traditional KeySym translations, they are mapped
            // here instead.
            for (int i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++)
            {
                if (strncmp(desc->names->keys[scancode].name, keymap[i].name, XkbKeyNameLength) == 0)
                {
                    key = keymap[i].key;
                    break;
                }
            }

            // Fall back to key aliases in case the key name did not match
            for (int i = 0; i < desc->names->num_key_aliases; i++)
            {
                if (key != GRWL_KEY_UNKNOWN)
                {
                    break;
                }

                if (strncmp(desc->names->key_aliases[i].real, desc->names->keys[scancode].name, XkbKeyNameLength) != 0)
                {
                    continue;
                }

                for (int j = 0; j < sizeof(keymap) / sizeof(keymap[0]); j++)
                {
                    if (strncmp(desc->names->key_aliases[i].alias, keymap[j].name, XkbKeyNameLength) == 0)
                    {
                        key = keymap[j].key;
                        break;
                    }
                }
            }

            _grwl.x11.keycodes[scancode] = key;
        }

        XkbFreeNames(desc, XkbKeyNamesMask, True);
        XkbFreeKeyboard(desc, 0, True);
    }
    else
    {
        XDisplayKeycodes(_grwl.x11.display, &scancodeMin, &scancodeMax);
    }

    int width;
    KeySym* keysyms = XGetKeyboardMapping(_grwl.x11.display, scancodeMin, scancodeMax - scancodeMin + 1, &width);

    for (int scancode = scancodeMin; scancode <= scancodeMax; scancode++)
    {
        // Translate the un-translated key codes using traditional X11 KeySym
        // lookups
        if (_grwl.x11.keycodes[scancode] < 0)
        {
            const size_t base = (scancode - scancodeMin) * width;
            _grwl.x11.keycodes[scancode] = translateKeySyms(&keysyms[base], width);
        }

        // Store the reverse translation for faster key name lookup
        if (_grwl.x11.keycodes[scancode] > 0)
        {
            _grwl.x11.scancodes[_grwl.x11.keycodes[scancode]] = scancode;
        }
    }

    XFree(keysyms);
}

// Check whether the IM has a usable style
//
static bool hasUsableInputMethodStyle()
{
    bool found = false;
    XIMStyles* styles = NULL;

    if (XGetIMValues(_grwl.x11.im, XNQueryInputStyle, &styles, NULL) != NULL)
    {
        return false;
    }

    if (_grwl.hints.init.x11.onTheSpotIMStyle)
    {
        _grwl.x11.imStyle = STYLE_ONTHESPOT;
    }
    else
    {
        _grwl.x11.imStyle = STYLE_OVERTHESPOT;
    }

    for (unsigned int i = 0; i < styles->count_styles; i++)
    {
        if (styles->supported_styles[i] == _grwl.x11.imStyle)
        {
            found = true;
            break;
        }
    }

    XFree(styles);
    return found;
}

static void inputMethodDestroyCallback(XIM im, XPointer clientData, XPointer callData)
{
    _grwl.x11.im = NULL;
}

static void inputMethodInstantiateCallback(Display* display, XPointer clientData, XPointer callData)
{
    if (_grwl.x11.im)
    {
        return;
    }

    _grwl.x11.im = XOpenIM(_grwl.x11.display, 0, NULL, NULL);
    if (_grwl.x11.im)
    {
        if (!hasUsableInputMethodStyle())
        {
            XCloseIM(_grwl.x11.im);
            _grwl.x11.im = NULL;
        }
    }

    if (_grwl.x11.im)
    {
        XIMCallback callback;
        callback.callback = (XIMProc)inputMethodDestroyCallback;
        callback.client_data = NULL;
        XSetIMValues(_grwl.x11.im, XNDestroyCallback, &callback, NULL);

        for (_GRWLwindow* window = _grwl.windowListHead; window; window = window->next)
        {
            _grwlCreateInputContextX11(window);
        }
    }
}

// Return the atom ID only if it is listed in the specified array
//
static Atom getAtomIfSupported(Atom* supportedAtoms, unsigned long atomCount, const char* atomName)
{
    const Atom atom = XInternAtom(_grwl.x11.display, atomName, False);

    for (unsigned long i = 0; i < atomCount; i++)
    {
        if (supportedAtoms[i] == atom)
        {
            return atom;
        }
    }

    return None;
}

// Check whether the running window manager is EWMH-compliant
//
static void detectEWMH()
{
    // First we read the _NET_SUPPORTING_WM_CHECK property on the root window

    Window* windowFromRoot = NULL;
    if (!_grwlGetWindowPropertyX11(_grwl.x11.root, _grwl.x11.NET_SUPPORTING_WM_CHECK, XA_WINDOW,
                                   (unsigned char**)&windowFromRoot))
    {
        return;
    }

    _grwlGrabErrorHandlerX11();

    // If it exists, it should be the XID of a top-level window
    // Then we look for the same property on that window

    Window* windowFromChild = NULL;
    if (!_grwlGetWindowPropertyX11(*windowFromRoot, _grwl.x11.NET_SUPPORTING_WM_CHECK, XA_WINDOW,
                                   (unsigned char**)&windowFromChild))
    {
        XFree(windowFromRoot);
        return;
    }

    _grwlReleaseErrorHandlerX11();

    // If the property exists, it should contain the XID of the window

    if (*windowFromRoot != *windowFromChild)
    {
        XFree(windowFromRoot);
        XFree(windowFromChild);
        return;
    }

    XFree(windowFromRoot);
    XFree(windowFromChild);

    // We are now fairly sure that an EWMH-compliant WM is currently running
    // We can now start querying the WM about what features it supports by
    // looking in the _NET_SUPPORTED property on the root window
    // It should contain a list of supported EWMH protocol and state atoms

    Atom* supportedAtoms = NULL;
    const unsigned long atomCount =
        _grwlGetWindowPropertyX11(_grwl.x11.root, _grwl.x11.NET_SUPPORTED, XA_ATOM, (unsigned char**)&supportedAtoms);

    // See which of the atoms we support that are supported by the WM

    _grwl.x11.NET_WM_STATE = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE");
    _grwl.x11.NET_WM_STATE_ABOVE = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_ABOVE");
    _grwl.x11.NET_WM_STATE_FULLSCREEN = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");
    _grwl.x11.NET_WM_STATE_MAXIMIZED_VERT =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_VERT");
    _grwl.x11.NET_WM_STATE_MAXIMIZED_HORZ =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_HORZ");
    _grwl.x11.NET_WM_STATE_DEMANDS_ATTENTION =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_DEMANDS_ATTENTION");
    _grwl.x11.NET_WM_FULLSCREEN_MONITORS = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_FULLSCREEN_MONITORS");
    _grwl.x11.NET_WM_WINDOW_TYPE = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE");
    _grwl.x11.NET_WM_WINDOW_TYPE_NORMAL = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_NORMAL");
    _grwl.x11.NET_WORKAREA = getAtomIfSupported(supportedAtoms, atomCount, "_NET_WORKAREA");
    _grwl.x11.NET_CURRENT_DESKTOP = getAtomIfSupported(supportedAtoms, atomCount, "_NET_CURRENT_DESKTOP");
    _grwl.x11.NET_ACTIVE_WINDOW = getAtomIfSupported(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");
    _grwl.x11.NET_FRAME_EXTENTS = getAtomIfSupported(supportedAtoms, atomCount, "_NET_FRAME_EXTENTS");
    _grwl.x11.NET_REQUEST_FRAME_EXTENTS = getAtomIfSupported(supportedAtoms, atomCount, "_NET_REQUEST_FRAME_EXTENTS");

    if (supportedAtoms)
    {
        XFree(supportedAtoms);
    }
}

// Look for and initialize supported X11 extensions
//
static bool initExtensions()
{
    #if defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.vidmode.handle = _grwlPlatformLoadModule("libXxf86vm.so");
    #else
    _grwl.x11.vidmode.handle = _grwlPlatformLoadModule("libXxf86vm.so.1");
    #endif
    if (_grwl.x11.vidmode.handle)
    {
        _grwl.x11.vidmode.QueryExtension = (PFN_XF86VidModeQueryExtension)_grwlPlatformGetModuleSymbol(
            _grwl.x11.vidmode.handle, "XF86VidModeQueryExtension");
        _grwl.x11.vidmode.GetGammaRamp = (PFN_XF86VidModeGetGammaRamp)_grwlPlatformGetModuleSymbol(
            _grwl.x11.vidmode.handle, "XF86VidModeGetGammaRamp");
        _grwl.x11.vidmode.SetGammaRamp = (PFN_XF86VidModeSetGammaRamp)_grwlPlatformGetModuleSymbol(
            _grwl.x11.vidmode.handle, "XF86VidModeSetGammaRamp");
        _grwl.x11.vidmode.GetGammaRampSize = (PFN_XF86VidModeGetGammaRampSize)_grwlPlatformGetModuleSymbol(
            _grwl.x11.vidmode.handle, "XF86VidModeGetGammaRampSize");

        _grwl.x11.vidmode.available =
            XF86VidModeQueryExtension(_grwl.x11.display, &_grwl.x11.vidmode.eventBase, &_grwl.x11.vidmode.errorBase);
    }

    #if defined(__CYGWIN__)
    _grwl.x11.xi.handle = _grwlPlatformLoadModule("libXi-6.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.xi.handle = _grwlPlatformLoadModule("libXi.so");
    #else
    _grwl.x11.xi.handle = _grwlPlatformLoadModule("libXi.so.6");
    #endif
    if (_grwl.x11.xi.handle)
    {
        _grwl.x11.xi.QueryVersion =
            (PFN_XIQueryVersion)_grwlPlatformGetModuleSymbol(_grwl.x11.xi.handle, "XIQueryVersion");
        _grwl.x11.xi.SelectEvents =
            (PFN_XISelectEvents)_grwlPlatformGetModuleSymbol(_grwl.x11.xi.handle, "XISelectEvents");

        if (XQueryExtension(_grwl.x11.display, "XInputExtension", &_grwl.x11.xi.majorOpcode, &_grwl.x11.xi.eventBase,
                            &_grwl.x11.xi.errorBase))
        {
            _grwl.x11.xi.major = 2;
            _grwl.x11.xi.minor = 0;

            if (XIQueryVersion(_grwl.x11.display, &_grwl.x11.xi.major, &_grwl.x11.xi.minor) == Success)
            {
                _grwl.x11.xi.available = true;
            }
        }
    }

    #if defined(__CYGWIN__)
    _grwl.x11.randr.handle = _grwlPlatformLoadModule("libXrandr-2.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.randr.handle = _grwlPlatformLoadModule("libXrandr.so");
    #else
    _grwl.x11.randr.handle = _grwlPlatformLoadModule("libXrandr.so.2");
    #endif
    if (_grwl.x11.randr.handle)
    {
        _grwl.x11.randr.AllocGamma =
            (PFN_XRRAllocGamma)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRAllocGamma");
        _grwl.x11.randr.FreeGamma =
            (PFN_XRRFreeGamma)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRFreeGamma");
        _grwl.x11.randr.FreeCrtcInfo =
            (PFN_XRRFreeCrtcInfo)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRFreeCrtcInfo");
        _grwl.x11.randr.FreeGamma =
            (PFN_XRRFreeGamma)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRFreeGamma");
        _grwl.x11.randr.FreeOutputInfo =
            (PFN_XRRFreeOutputInfo)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRFreeOutputInfo");
        _grwl.x11.randr.FreeScreenResources =
            (PFN_XRRFreeScreenResources)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRFreeScreenResources");
        _grwl.x11.randr.GetCrtcGamma =
            (PFN_XRRGetCrtcGamma)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRGetCrtcGamma");
        _grwl.x11.randr.GetCrtcGammaSize =
            (PFN_XRRGetCrtcGammaSize)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRGetCrtcGammaSize");
        _grwl.x11.randr.GetCrtcInfo =
            (PFN_XRRGetCrtcInfo)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRGetCrtcInfo");
        _grwl.x11.randr.GetOutputInfo =
            (PFN_XRRGetOutputInfo)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRGetOutputInfo");
        _grwl.x11.randr.GetOutputPrimary =
            (PFN_XRRGetOutputPrimary)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRGetOutputPrimary");
        _grwl.x11.randr.GetScreenResourcesCurrent = (PFN_XRRGetScreenResourcesCurrent)_grwlPlatformGetModuleSymbol(
            _grwl.x11.randr.handle, "XRRGetScreenResourcesCurrent");
        _grwl.x11.randr.QueryExtension =
            (PFN_XRRQueryExtension)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRQueryExtension");
        _grwl.x11.randr.QueryVersion =
            (PFN_XRRQueryVersion)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRQueryVersion");
        _grwl.x11.randr.SelectInput =
            (PFN_XRRSelectInput)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRSelectInput");
        _grwl.x11.randr.SetCrtcConfig =
            (PFN_XRRSetCrtcConfig)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRSetCrtcConfig");
        _grwl.x11.randr.SetCrtcGamma =
            (PFN_XRRSetCrtcGamma)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRSetCrtcGamma");
        _grwl.x11.randr.UpdateConfiguration =
            (PFN_XRRUpdateConfiguration)_grwlPlatformGetModuleSymbol(_grwl.x11.randr.handle, "XRRUpdateConfiguration");

        if (XRRQueryExtension(_grwl.x11.display, &_grwl.x11.randr.eventBase, &_grwl.x11.randr.errorBase))
        {
            if (XRRQueryVersion(_grwl.x11.display, &_grwl.x11.randr.major, &_grwl.x11.randr.minor))
            {
                // The GRWL RandR path requires at least version 1.3
                if (_grwl.x11.randr.major > 1 || _grwl.x11.randr.minor >= 3)
                {
                    _grwl.x11.randr.available = true;
                }
            }
            else
            {
                _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to query RandR version");
            }
        }
    }

    if (_grwl.x11.randr.available)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_grwl.x11.display, _grwl.x11.root);

        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_grwl.x11.display, sr->crtcs[0]))
        {
            // This is likely an older Nvidia driver with broken gamma support
            // Flag it as useless and fall back to xf86vm gamma, if available
            _grwl.x11.randr.gammaBroken = true;
        }

        if (!sr->ncrtc)
        {
            // A system without CRTCs is likely a system with broken RandR
            // Disable the RandR monitor path and fall back to core functions
            _grwl.x11.randr.monitorBroken = true;
        }

        XRRFreeScreenResources(sr);
    }

    if (_grwl.x11.randr.available && !_grwl.x11.randr.monitorBroken)
    {
        XRRSelectInput(_grwl.x11.display, _grwl.x11.root, RROutputChangeNotifyMask);
    }

    #if defined(__CYGWIN__)
    _grwl.x11.xcursor.handle = _grwlPlatformLoadModule("libXcursor-1.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.xcursor.handle = _grwlPlatformLoadModule("libXcursor.so");
    #else
    _grwl.x11.xcursor.handle = _grwlPlatformLoadModule("libXcursor.so.1");
    #endif
    if (_grwl.x11.xcursor.handle)
    {
        _grwl.x11.xcursor.ImageCreate =
            (PFN_XcursorImageCreate)_grwlPlatformGetModuleSymbol(_grwl.x11.xcursor.handle, "XcursorImageCreate");
        _grwl.x11.xcursor.ImageDestroy =
            (PFN_XcursorImageDestroy)_grwlPlatformGetModuleSymbol(_grwl.x11.xcursor.handle, "XcursorImageDestroy");
        _grwl.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)_grwlPlatformGetModuleSymbol(
            _grwl.x11.xcursor.handle, "XcursorImageLoadCursor");
        _grwl.x11.xcursor.GetTheme =
            (PFN_XcursorGetTheme)_grwlPlatformGetModuleSymbol(_grwl.x11.xcursor.handle, "XcursorGetTheme");
        _grwl.x11.xcursor.GetDefaultSize =
            (PFN_XcursorGetDefaultSize)_grwlPlatformGetModuleSymbol(_grwl.x11.xcursor.handle, "XcursorGetDefaultSize");
        _grwl.x11.xcursor.LibraryLoadImage = (PFN_XcursorLibraryLoadImage)_grwlPlatformGetModuleSymbol(
            _grwl.x11.xcursor.handle, "XcursorLibraryLoadImage");
    }

    #if defined(__CYGWIN__)
    _grwl.x11.xinerama.handle = _grwlPlatformLoadModule("libXinerama-1.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.xinerama.handle = _grwlPlatformLoadModule("libXinerama.so");
    #else
    _grwl.x11.xinerama.handle = _grwlPlatformLoadModule("libXinerama.so.1");
    #endif
    if (_grwl.x11.xinerama.handle)
    {
        _grwl.x11.xinerama.IsActive =
            (PFN_XineramaIsActive)_grwlPlatformGetModuleSymbol(_grwl.x11.xinerama.handle, "XineramaIsActive");
        _grwl.x11.xinerama.QueryExtension = (PFN_XineramaQueryExtension)_grwlPlatformGetModuleSymbol(
            _grwl.x11.xinerama.handle, "XineramaQueryExtension");
        _grwl.x11.xinerama.QueryScreens =
            (PFN_XineramaQueryScreens)_grwlPlatformGetModuleSymbol(_grwl.x11.xinerama.handle, "XineramaQueryScreens");

        if (XineramaQueryExtension(_grwl.x11.display, &_grwl.x11.xinerama.major, &_grwl.x11.xinerama.minor))
        {
            if (XineramaIsActive(_grwl.x11.display))
            {
                _grwl.x11.xinerama.available = true;
            }
        }
    }

    _grwl.x11.xkb.major = 1;
    _grwl.x11.xkb.minor = 0;
    _grwl.x11.xkb.available = XkbQueryExtension(_grwl.x11.display, &_grwl.x11.xkb.majorOpcode, &_grwl.x11.xkb.eventBase,
                                                &_grwl.x11.xkb.errorBase, &_grwl.x11.xkb.major, &_grwl.x11.xkb.minor);

    if (_grwl.x11.xkb.available)
    {
        Bool supported;

        if (XkbSetDetectableAutoRepeat(_grwl.x11.display, True, &supported))
        {
            if (supported)
            {
                _grwl.x11.xkb.detectable = true;
            }
        }

        XkbStateRec state;
        if (XkbGetState(_grwl.x11.display, XkbUseCoreKbd, &state) == Success)
        {
            _grwl.x11.xkb.group = (unsigned int)state.group;
        }

        XkbSelectEventDetails(_grwl.x11.display, XkbUseCoreKbd, XkbStateNotify, XkbGroupStateMask, XkbGroupStateMask);
    }

    if (_grwl.hints.init.x11.xcbVulkanSurface)
    {
    #if defined(__CYGWIN__)
        _grwl.x11.x11xcb.handle = _grwlPlatformLoadModule("libX11-xcb-1.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
        _grwl.x11.x11xcb.handle = _grwlPlatformLoadModule("libX11-xcb.so");
    #else
        _grwl.x11.x11xcb.handle = _grwlPlatformLoadModule("libX11-xcb.so.1");
    #endif
    }

    if (_grwl.x11.x11xcb.handle)
    {
        _grwl.x11.x11xcb.GetXCBConnection =
            (PFN_XGetXCBConnection)_grwlPlatformGetModuleSymbol(_grwl.x11.x11xcb.handle, "XGetXCBConnection");
    }

    #if defined(__CYGWIN__)
    _grwl.x11.xrender.handle = _grwlPlatformLoadModule("libXrender-1.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.xrender.handle = _grwlPlatformLoadModule("libXrender.so");
    #else
    _grwl.x11.xrender.handle = _grwlPlatformLoadModule("libXrender.so.1");
    #endif
    if (_grwl.x11.xrender.handle)
    {
        _grwl.x11.xrender.QueryExtension =
            (PFN_XRenderQueryExtension)_grwlPlatformGetModuleSymbol(_grwl.x11.xrender.handle, "XRenderQueryExtension");
        _grwl.x11.xrender.QueryVersion =
            (PFN_XRenderQueryVersion)_grwlPlatformGetModuleSymbol(_grwl.x11.xrender.handle, "XRenderQueryVersion");
        _grwl.x11.xrender.FindVisualFormat = (PFN_XRenderFindVisualFormat)_grwlPlatformGetModuleSymbol(
            _grwl.x11.xrender.handle, "XRenderFindVisualFormat");

        if (XRenderQueryExtension(_grwl.x11.display, &_grwl.x11.xrender.errorBase, &_grwl.x11.xrender.eventBase))
        {
            if (XRenderQueryVersion(_grwl.x11.display, &_grwl.x11.xrender.major, &_grwl.x11.xrender.minor))
            {
                _grwl.x11.xrender.available = true;
            }
        }
    }

    #if defined(__CYGWIN__)
    _grwl.x11.xshape.handle = _grwlPlatformLoadModule("libXext-6.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    _grwl.x11.xshape.handle = _grwlPlatformLoadModule("libXext.so");
    #else
    _grwl.x11.xshape.handle = _grwlPlatformLoadModule("libXext.so.6");
    #endif
    if (_grwl.x11.xshape.handle)
    {
        _grwl.x11.xshape.QueryExtension =
            (PFN_XShapeQueryExtension)_grwlPlatformGetModuleSymbol(_grwl.x11.xshape.handle, "XShapeQueryExtension");
        _grwl.x11.xshape.ShapeCombineRegion =
            (PFN_XShapeCombineRegion)_grwlPlatformGetModuleSymbol(_grwl.x11.xshape.handle, "XShapeCombineRegion");
        _grwl.x11.xshape.QueryVersion =
            (PFN_XShapeQueryVersion)_grwlPlatformGetModuleSymbol(_grwl.x11.xshape.handle, "XShapeQueryVersion");
        _grwl.x11.xshape.ShapeCombineMask =
            (PFN_XShapeCombineMask)_grwlPlatformGetModuleSymbol(_grwl.x11.xshape.handle, "XShapeCombineMask");

        if (XShapeQueryExtension(_grwl.x11.display, &_grwl.x11.xshape.errorBase, &_grwl.x11.xshape.eventBase))
        {
            if (XShapeQueryVersion(_grwl.x11.display, &_grwl.x11.xshape.major, &_grwl.x11.xshape.minor))
            {
                _grwl.x11.xshape.available = true;
            }
        }
    }

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    createKeyTables();

    // String format atoms
    _grwl.x11.NULL_ = XInternAtom(_grwl.x11.display, "NULL", False);
    _grwl.x11.UTF8_STRING = XInternAtom(_grwl.x11.display, "UTF8_STRING", False);
    _grwl.x11.ATOM_PAIR = XInternAtom(_grwl.x11.display, "ATOM_PAIR", False);

    // Custom selection property atom
    _grwl.x11.GRWL_SELECTION = XInternAtom(_grwl.x11.display, "GRWL_SELECTION", False);

    // ICCCM standard clipboard atoms
    _grwl.x11.TARGETS = XInternAtom(_grwl.x11.display, "TARGETS", False);
    _grwl.x11.MULTIPLE = XInternAtom(_grwl.x11.display, "MULTIPLE", False);
    _grwl.x11.PRIMARY = XInternAtom(_grwl.x11.display, "PRIMARY", False);
    _grwl.x11.INCR = XInternAtom(_grwl.x11.display, "INCR", False);
    _grwl.x11.CLIPBOARD = XInternAtom(_grwl.x11.display, "CLIPBOARD", False);

    // Clipboard manager atoms
    _grwl.x11.CLIPBOARD_MANAGER = XInternAtom(_grwl.x11.display, "CLIPBOARD_MANAGER", False);
    _grwl.x11.SAVE_TARGETS = XInternAtom(_grwl.x11.display, "SAVE_TARGETS", False);

    // Xdnd (drag and drop) atoms
    _grwl.x11.XdndAware = XInternAtom(_grwl.x11.display, "XdndAware", False);
    _grwl.x11.XdndEnter = XInternAtom(_grwl.x11.display, "XdndEnter", False);
    _grwl.x11.XdndPosition = XInternAtom(_grwl.x11.display, "XdndPosition", False);
    _grwl.x11.XdndStatus = XInternAtom(_grwl.x11.display, "XdndStatus", False);
    _grwl.x11.XdndActionCopy = XInternAtom(_grwl.x11.display, "XdndActionCopy", False);
    _grwl.x11.XdndDrop = XInternAtom(_grwl.x11.display, "XdndDrop", False);
    _grwl.x11.XdndFinished = XInternAtom(_grwl.x11.display, "XdndFinished", False);
    _grwl.x11.XdndSelection = XInternAtom(_grwl.x11.display, "XdndSelection", False);
    _grwl.x11.XdndTypeList = XInternAtom(_grwl.x11.display, "XdndTypeList", False);
    _grwl.x11.text_uri_list = XInternAtom(_grwl.x11.display, "text/uri-list", False);

    // ICCCM, EWMH and Motif window property atoms
    // These can be set safely even without WM support
    // The EWMH atoms that require WM support are handled in detectEWMH
    _grwl.x11.WM_PROTOCOLS = XInternAtom(_grwl.x11.display, "WM_PROTOCOLS", False);
    _grwl.x11.WM_STATE = XInternAtom(_grwl.x11.display, "WM_STATE", False);
    _grwl.x11.WM_DELETE_WINDOW = XInternAtom(_grwl.x11.display, "WM_DELETE_WINDOW", False);
    _grwl.x11.NET_SUPPORTED = XInternAtom(_grwl.x11.display, "_NET_SUPPORTED", False);
    _grwl.x11.NET_SUPPORTING_WM_CHECK = XInternAtom(_grwl.x11.display, "_NET_SUPPORTING_WM_CHECK", False);
    _grwl.x11.NET_WM_ICON = XInternAtom(_grwl.x11.display, "_NET_WM_ICON", False);
    _grwl.x11.NET_WM_PING = XInternAtom(_grwl.x11.display, "_NET_WM_PING", False);
    _grwl.x11.NET_WM_PID = XInternAtom(_grwl.x11.display, "_NET_WM_PID", False);
    _grwl.x11.NET_WM_NAME = XInternAtom(_grwl.x11.display, "_NET_WM_NAME", False);
    _grwl.x11.NET_WM_ICON_NAME = XInternAtom(_grwl.x11.display, "_NET_WM_ICON_NAME", False);
    _grwl.x11.NET_WM_BYPASS_COMPOSITOR = XInternAtom(_grwl.x11.display, "_NET_WM_BYPASS_COMPOSITOR", False);
    _grwl.x11.NET_WM_WINDOW_OPACITY = XInternAtom(_grwl.x11.display, "_NET_WM_WINDOW_OPACITY", False);
    _grwl.x11.MOTIF_WM_HINTS = XInternAtom(_grwl.x11.display, "_MOTIF_WM_HINTS", False);

    // The compositing manager selection name contains the screen number
    {
        char name[32];
        snprintf(name, sizeof(name), "_NET_WM_CM_S%u", _grwl.x11.screen);
        _grwl.x11.NET_WM_CM_Sx = XInternAtom(_grwl.x11.display, name, False);
    }

    // Detect whether an EWMH-conformant window manager is running
    detectEWMH();

    return true;
}

// Retrieve system content scale via folklore heuristics
//
static void getSystemContentScale(float* xscale, float* yscale)
{
    // Start by assuming the default X11 DPI
    // NOTE: Some desktop environments (KDE) may remove the Xft.dpi field when it
    //       would be set to 96, so assume that is the case if we cannot find it
    float xdpi = 96.f, ydpi = 96.f;

    // NOTE: Basing the scale on Xft.dpi where available should provide the most
    //       consistent user experience (matches Qt, Gtk, etc), although not
    //       always the most accurate one
    char* rms = XResourceManagerString(_grwl.x11.display);
    if (rms)
    {
        XrmDatabase db = XrmGetStringDatabase(rms);
        if (db)
        {
            XrmValue value;
            char* type = NULL;

            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value))
            {
                if (type && strcmp(type, "String") == 0)
                {
                    xdpi = ydpi = atof(value.addr);
                }
            }

            XrmDestroyDatabase(db);
        }
    }

    *xscale = xdpi / 96.f;
    *yscale = ydpi / 96.f;
}

// Create a blank cursor for hidden and disabled cursor modes
//
static Cursor createHiddenCursor()
{
    unsigned char pixels[16 * 16 * 4] = { 0 };
    GRWLimage image = { 16, 16, pixels };
    return _grwlCreateNativeCursorX11(&image, 0, 0);
}

// Create a helper window for IPC
//
static Window createHelperWindow()
{
    XSetWindowAttributes wa;
    wa.event_mask = PropertyChangeMask;

    return XCreateWindow(_grwl.x11.display, _grwl.x11.root, 0, 0, 1, 1, 0, 0, InputOnly,
                         DefaultVisual(_grwl.x11.display, _grwl.x11.screen), CWEventMask, &wa);
}

// Create the pipe for empty events without assumuing the OS has pipe2(2)
//
static bool createEmptyEventPipe()
{
    if (pipe(_grwl.x11.emptyEventPipe) != 0)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to create empty event pipe: %s", strerror(errno));
        return false;
    }

    for (int i = 0; i < 2; i++)
    {
        const int sf = fcntl(_grwl.x11.emptyEventPipe[i], F_GETFL, 0);
        const int df = fcntl(_grwl.x11.emptyEventPipe[i], F_GETFD, 0);

        if (sf == -1 || df == -1 || fcntl(_grwl.x11.emptyEventPipe[i], F_SETFL, sf | O_NONBLOCK) == -1 ||
            fcntl(_grwl.x11.emptyEventPipe[i], F_SETFD, df | FD_CLOEXEC) == -1)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to set flags for empty event pipe: %s", strerror(errno));
            return false;
        }
    }

    return true;
}

// X error handler
//
static int errorHandler(Display* display, XErrorEvent* event)
{
    if (_grwl.x11.display != display)
    {
        return 0;
    }

    _grwl.x11.errorCode = event->error_code;
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Sets the X error handler callback
//
void _grwlGrabErrorHandlerX11()
{
    assert(_grwl.x11.errorHandler == NULL);
    _grwl.x11.errorCode = Success;
    _grwl.x11.errorHandler = XSetErrorHandler(errorHandler);
}

// Clears the X error handler callback
//
void _grwlReleaseErrorHandlerX11()
{
    // Synchronize to make sure all commands are processed
    XSync(_grwl.x11.display, False);
    XSetErrorHandler(_grwl.x11.errorHandler);
    _grwl.x11.errorHandler = NULL;
}

// Reports the specified error, appending information about the last X error
//
void _grwlInputErrorX11(int error, const char* message)
{
    char buffer[_GRWL_MESSAGE_SIZE];
    XGetErrorText(_grwl.x11.display, _grwl.x11.errorCode, buffer, sizeof(buffer));

    _grwlInputError(error, "%s: %s", message, buffer);
}

// Creates a native cursor object from the specified image and hotspot
//
Cursor _grwlCreateNativeCursorX11(const GRWLimage* image, int xhot, int yhot)
{
    Cursor cursor;

    if (!_grwl.x11.xcursor.handle)
    {
        return None;
    }

    XcursorImage* native = XcursorImageCreate(image->width, image->height);
    if (native == NULL)
    {
        return None;
    }

    native->xhot = xhot;
    native->yhot = yhot;

    unsigned char* source = (unsigned char*)image->pixels;
    XcursorPixel* target = native->pixels;

    for (int i = 0; i < image->width * image->height; i++, target++, source += 4)
    {
        unsigned int alpha = source[3];

        *target = (alpha << 24) | ((unsigned char)((source[0] * alpha) / 255) << 16) |
                  ((unsigned char)((source[1] * alpha) / 255) << 8) | ((unsigned char)((source[2] * alpha) / 255) << 0);
    }

    cursor = XcursorImageLoadCursor(_grwl.x11.display, native);
    XcursorImageDestroy(native);

    return cursor;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlConnectX11(int platformID, _GRWLplatform* platform)
{
    const _GRWLplatform x11 = {
        GRWL_PLATFORM_X11,
        _grwlInitX11,
        _grwlTerminateX11,
        _grwlGetCursorPosX11,
        _grwlSetCursorPosX11,
        _grwlSetCursorModeX11,
        _grwlSetRawMouseMotionX11,
        _grwlRawMouseMotionSupportedX11,
        _grwlCreateCursorX11,
        _grwlCreateStandardCursorX11,
        _grwlDestroyCursorX11,
        _grwlSetCursorX11,
        _grwlGetScancodeNameX11,
        _grwlGetKeyScancodeX11,
        _grwlGetKeyboardLayoutNameX11,
        _grwlSetClipboardStringX11,
        _grwlGetClipboardStringX11,
        _grwlUpdatePreeditCursorRectangleX11,
        _grwlResetPreeditTextX11,
        _grwlSetIMEStatusX11,
        _grwlGetIMEStatusX11,
    #if defined(GRWL_BUILD_LINUX_JOYSTICK)
        _grwlInitJoysticksLinux,
        _grwlTerminateJoysticksLinux,
        _grwlPollJoystickLinux,
        _grwlGetMappingNameLinux,
        _grwlUpdateGamepadGUIDLinux,
    #else
        _grwlInitJoysticksNull,
        _grwlTerminateJoysticksNull,
        _grwlPollJoystickNull,
        _grwlGetMappingNameNull,
        _grwlUpdateGamepadGUIDNull,
    #endif
        _grwlFreeMonitorX11,
        _grwlGetMonitorPosX11,
        _grwlGetMonitorContentScaleX11,
        _grwlGetMonitorWorkareaX11,
        _grwlGetVideoModesX11,
        _grwlGetVideoModeX11,
        _grwlGetGammaRampX11,
        _grwlSetGammaRampX11,
        _grwlCreateWindowX11,
        _grwlDestroyWindowX11,
        _grwlSetWindowTitleX11,
        _grwlSetWindowIconX11,
        _grwlSetWindowProgressIndicatorX11,
        _grwlSetWindowBadgeX11,
        _grwlSetWindowBadgeStringX11,
        _grwlGetWindowPosX11,
        _grwlSetWindowPosX11,
        _grwlGetWindowSizeX11,
        _grwlSetWindowSizeX11,
        _grwlSetWindowSizeLimitsX11,
        _grwlSetWindowAspectRatioX11,
        _grwlGetFramebufferSizeX11,
        _grwlGetWindowFrameSizeX11,
        _grwlGetWindowContentScaleX11,
        _grwlIconifyWindowX11,
        _grwlRestoreWindowX11,
        _grwlMaximizeWindowX11,
        _grwlShowWindowX11,
        _grwlHideWindowX11,
        _grwlRequestWindowAttentionX11,
        _grwlFocusWindowX11,
        _grwlSetWindowMonitorX11,
        _grwlWindowFocusedX11,
        _grwlWindowIconifiedX11,
        _grwlWindowVisibleX11,
        _grwlWindowMaximizedX11,
        _grwlWindowHoveredX11,
        _grwlFramebufferTransparentX11,
        _grwlGetWindowOpacityX11,
        _grwlSetWindowResizableX11,
        _grwlSetWindowDecoratedX11,
        _grwlSetWindowFloatingX11,
        _grwlSetWindowOpacityX11,
        _grwlSetWindowMousePassthroughX11,
        _grwlPollEventsX11,
        _grwlWaitEventsX11,
        _grwlWaitEventsTimeoutX11,
        _grwlPostEmptyEventX11,
        _grwlCreateUserContextX11,
        _grwlGetEGLPlatformX11,
        _grwlGetEGLNativeDisplayX11,
        _grwlGetEGLNativeWindowX11,
        _grwlGetRequiredInstanceExtensionsX11,
        _grwlGetPhysicalDevicePresentationSupportX11,
        _grwlCreateWindowSurfaceX11,
    };

    // HACK: If the application has left the locale as "C" then both wide
    //       character text input and explicit UTF-8 input via XIM will break
    //       This sets the CTYPE part of the current locale from the environment
    //       in the hope that it is set to something more sane than "C"
    if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0)
    {
        setlocale(LC_CTYPE, "");
    }

    #if defined(__CYGWIN__)
    void* module = _grwlPlatformLoadModule("libX11-6.so");
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
    void* module = _grwlPlatformLoadModule("libX11.so");
    #else
    void* module = _grwlPlatformLoadModule("libX11.so.6");
    #endif
    if (!module)
    {
        if (platformID == GRWL_PLATFORM_X11)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to load Xlib");
        }

        return false;
    }

    PFN_XInitThreads XInitThreads = (PFN_XInitThreads)_grwlPlatformGetModuleSymbol(module, "XInitThreads");
    PFN_XrmInitialize XrmInitialize = (PFN_XrmInitialize)_grwlPlatformGetModuleSymbol(module, "XrmInitialize");
    PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)_grwlPlatformGetModuleSymbol(module, "XOpenDisplay");
    if (!XInitThreads || !XrmInitialize || !XOpenDisplay)
    {
        if (platformID == GRWL_PLATFORM_X11)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "X11: Failed to load Xlib entry point");
        }

        _grwlPlatformFreeModule(module);
        return false;
    }

    XInitThreads();
    XrmInitialize();

    Display* display = XOpenDisplay(NULL);
    if (!display)
    {
        if (platformID == GRWL_PLATFORM_X11)
        {
            const char* name = getenv("DISPLAY");
            if (name)
            {
                _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "X11: Failed to open display %s", name);
            }
            else
            {
                _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "X11: The DISPLAY environment variable is missing");
            }
        }

        _grwlPlatformFreeModule(module);
        return false;
    }

    _grwl.x11.display = display;
    _grwl.x11.xlib.handle = module;

    *platform = x11;
    return true;
}

int _grwlInitX11()
{
    _grwlInitDBusPOSIX();

    _grwl.x11.xlib.AllocClassHint =
        (PFN_XAllocClassHint)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XAllocClassHint");
    _grwl.x11.xlib.AllocSizeHints =
        (PFN_XAllocSizeHints)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XAllocSizeHints");
    _grwl.x11.xlib.AllocWMHints =
        (PFN_XAllocWMHints)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XAllocWMHints");
    _grwl.x11.xlib.ChangeProperty =
        (PFN_XChangeProperty)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XChangeProperty");
    _grwl.x11.xlib.ChangeWindowAttributes =
        (PFN_XChangeWindowAttributes)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XChangeWindowAttributes");
    _grwl.x11.xlib.CheckIfEvent =
        (PFN_XCheckIfEvent)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCheckIfEvent");
    _grwl.x11.xlib.CheckTypedWindowEvent =
        (PFN_XCheckTypedWindowEvent)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCheckTypedWindowEvent");
    _grwl.x11.xlib.CloseDisplay =
        (PFN_XCloseDisplay)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCloseDisplay");
    _grwl.x11.xlib.CloseIM = (PFN_XCloseIM)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCloseIM");
    _grwl.x11.xlib.ConvertSelection =
        (PFN_XConvertSelection)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XConvertSelection");
    _grwl.x11.xlib.CreateColormap =
        (PFN_XCreateColormap)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCreateColormap");
    _grwl.x11.xlib.CreateFontCursor =
        (PFN_XCreateFontCursor)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCreateFontCursor");
    _grwl.x11.xlib.CreateIC = (PFN_XCreateIC)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCreateIC");
    _grwl.x11.xlib.CreateRegion =
        (PFN_XCreateRegion)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCreateRegion");
    _grwl.x11.xlib.CreateWindow =
        (PFN_XCreateWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XCreateWindow");
    _grwl.x11.xlib.DefineCursor =
        (PFN_XDefineCursor)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDefineCursor");
    _grwl.x11.xlib.DeleteContext =
        (PFN_XDeleteContext)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDeleteContext");
    _grwl.x11.xlib.DeleteProperty =
        (PFN_XDeleteProperty)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDeleteProperty");
    _grwl.x11.xlib.DestroyIC = (PFN_XDestroyIC)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDestroyIC");
    _grwl.x11.xlib.DestroyRegion =
        (PFN_XDestroyRegion)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDestroyRegion");
    _grwl.x11.xlib.DestroyWindow =
        (PFN_XDestroyWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDestroyWindow");
    _grwl.x11.xlib.DisplayKeycodes =
        (PFN_XDisplayKeycodes)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XDisplayKeycodes");
    _grwl.x11.xlib.EventsQueued =
        (PFN_XEventsQueued)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XEventsQueued");
    _grwl.x11.xlib.FilterEvent = (PFN_XFilterEvent)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFilterEvent");
    _grwl.x11.xlib.FindContext = (PFN_XFindContext)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFindContext");
    _grwl.x11.xlib.Flush = (PFN_XFlush)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFlush");
    _grwl.x11.xlib.Free = (PFN_XFree)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFree");
    _grwl.x11.xlib.FreeColormap =
        (PFN_XFreeColormap)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFreeColormap");
    _grwl.x11.xlib.FreeCursor = (PFN_XFreeCursor)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFreeCursor");
    _grwl.x11.xlib.FreeEventData =
        (PFN_XFreeEventData)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XFreeEventData");
    _grwl.x11.xlib.GetAtomName = (PFN_XGetAtomName)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetAtomName");
    _grwl.x11.xlib.GetErrorText =
        (PFN_XGetErrorText)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetErrorText");
    _grwl.x11.xlib.GetEventData =
        (PFN_XGetEventData)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetEventData");
    _grwl.x11.xlib.GetICValues = (PFN_XGetICValues)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetICValues");
    _grwl.x11.xlib.GetIMValues = (PFN_XGetIMValues)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetIMValues");
    _grwl.x11.xlib.GetInputFocus =
        (PFN_XGetInputFocus)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetInputFocus");
    _grwl.x11.xlib.GetKeyboardMapping =
        (PFN_XGetKeyboardMapping)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetKeyboardMapping");
    _grwl.x11.xlib.GetScreenSaver =
        (PFN_XGetScreenSaver)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetScreenSaver");
    _grwl.x11.xlib.GetSelectionOwner =
        (PFN_XGetSelectionOwner)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetSelectionOwner");
    _grwl.x11.xlib.GetVisualInfo =
        (PFN_XGetVisualInfo)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetVisualInfo");
    _grwl.x11.xlib.GetWMNormalHints =
        (PFN_XGetWMNormalHints)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetWMNormalHints");
    _grwl.x11.xlib.GetWindowAttributes =
        (PFN_XGetWindowAttributes)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetWindowAttributes");
    _grwl.x11.xlib.GetWindowProperty =
        (PFN_XGetWindowProperty)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGetWindowProperty");
    _grwl.x11.xlib.GrabPointer = (PFN_XGrabPointer)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XGrabPointer");
    _grwl.x11.xlib.IconifyWindow =
        (PFN_XIconifyWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XIconifyWindow");
    _grwl.x11.xlib.InternAtom = (PFN_XInternAtom)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XInternAtom");
    _grwl.x11.xlib.LookupString =
        (PFN_XLookupString)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XLookupString");
    _grwl.x11.xlib.MapRaised = (PFN_XMapRaised)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XMapRaised");
    _grwl.x11.xlib.MapWindow = (PFN_XMapWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XMapWindow");
    _grwl.x11.xlib.MoveResizeWindow =
        (PFN_XMoveResizeWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XMoveResizeWindow");
    _grwl.x11.xlib.MoveWindow = (PFN_XMoveWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XMoveWindow");
    _grwl.x11.xlib.NextEvent = (PFN_XNextEvent)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XNextEvent");
    _grwl.x11.xlib.OpenIM = (PFN_XOpenIM)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XOpenIM");
    _grwl.x11.xlib.PeekEvent = (PFN_XPeekEvent)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XPeekEvent");
    _grwl.x11.xlib.Pending = (PFN_XPending)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XPending");
    _grwl.x11.xlib.QueryExtension =
        (PFN_XQueryExtension)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XQueryExtension");
    _grwl.x11.xlib.QueryPointer =
        (PFN_XQueryPointer)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XQueryPointer");
    _grwl.x11.xlib.RaiseWindow = (PFN_XRaiseWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XRaiseWindow");
    _grwl.x11.xlib.RegisterIMInstantiateCallback = (PFN_XRegisterIMInstantiateCallback)_grwlPlatformGetModuleSymbol(
        _grwl.x11.xlib.handle, "XRegisterIMInstantiateCallback");
    _grwl.x11.xlib.ResizeWindow =
        (PFN_XResizeWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XResizeWindow");
    _grwl.x11.xlib.ResourceManagerString =
        (PFN_XResourceManagerString)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XResourceManagerString");
    _grwl.x11.xlib.SaveContext = (PFN_XSaveContext)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSaveContext");
    _grwl.x11.xlib.SelectInput = (PFN_XSelectInput)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSelectInput");
    _grwl.x11.xlib.SendEvent = (PFN_XSendEvent)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSendEvent");
    _grwl.x11.xlib.SetClassHint =
        (PFN_XSetClassHint)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetClassHint");
    _grwl.x11.xlib.SetErrorHandler =
        (PFN_XSetErrorHandler)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetErrorHandler");
    _grwl.x11.xlib.SetICFocus = (PFN_XSetICFocus)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetICFocus");
    _grwl.x11.xlib.SetICValues = (PFN_XSetICValues)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetICValues");
    _grwl.x11.xlib.SetIMValues = (PFN_XSetIMValues)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetIMValues");
    _grwl.x11.xlib.SetInputFocus =
        (PFN_XSetInputFocus)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetInputFocus");
    _grwl.x11.xlib.SetLocaleModifiers =
        (PFN_XSetLocaleModifiers)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetLocaleModifiers");
    _grwl.x11.xlib.SetScreenSaver =
        (PFN_XSetScreenSaver)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetScreenSaver");
    _grwl.x11.xlib.SetSelectionOwner =
        (PFN_XSetSelectionOwner)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetSelectionOwner");
    _grwl.x11.xlib.SetWMHints = (PFN_XSetWMHints)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetWMHints");
    _grwl.x11.xlib.SetWMNormalHints =
        (PFN_XSetWMNormalHints)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetWMNormalHints");
    _grwl.x11.xlib.SetWMProtocols =
        (PFN_XSetWMProtocols)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSetWMProtocols");
    _grwl.x11.xlib.SupportsLocale =
        (PFN_XSupportsLocale)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSupportsLocale");
    _grwl.x11.xlib.Sync = (PFN_XSync)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XSync");
    _grwl.x11.xlib.TranslateCoordinates =
        (PFN_XTranslateCoordinates)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XTranslateCoordinates");
    _grwl.x11.xlib.UndefineCursor =
        (PFN_XUndefineCursor)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XUndefineCursor");
    _grwl.x11.xlib.UngrabPointer =
        (PFN_XUngrabPointer)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XUngrabPointer");
    _grwl.x11.xlib.UnmapWindow = (PFN_XUnmapWindow)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XUnmapWindow");
    _grwl.x11.xlib.UnsetICFocus =
        (PFN_XUnsetICFocus)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XUnsetICFocus");
    _grwl.x11.xlib.VaCreateNestedList =
        (PFN_XVaCreateNestedList)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XVaCreateNestedList");
    _grwl.x11.xlib.VisualIDFromVisual =
        (PFN_XVisualIDFromVisual)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XVisualIDFromVisual");
    _grwl.x11.xlib.WarpPointer = (PFN_XWarpPointer)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XWarpPointer");
    _grwl.x11.xkb.AllocKeyboard =
        (PFN_XkbAllocKeyboard)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbAllocKeyboard");
    _grwl.x11.xkb.FreeKeyboard =
        (PFN_XkbFreeKeyboard)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbFreeKeyboard");
    _grwl.x11.xkb.FreeNames = (PFN_XkbFreeNames)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbFreeNames");
    _grwl.x11.xkb.GetMap = (PFN_XkbGetMap)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbGetMap");
    _grwl.x11.xkb.GetNames = (PFN_XkbGetNames)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbGetNames");
    _grwl.x11.xkb.GetState = (PFN_XkbGetState)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbGetState");
    _grwl.x11.xkb.KeycodeToKeysym =
        (PFN_XkbKeycodeToKeysym)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbKeycodeToKeysym");
    _grwl.x11.xkb.QueryExtension =
        (PFN_XkbQueryExtension)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbQueryExtension");
    _grwl.x11.xkb.SelectEventDetails =
        (PFN_XkbSelectEventDetails)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XkbSelectEventDetails");
    _grwl.x11.xkb.SetDetectableAutoRepeat = (PFN_XkbSetDetectableAutoRepeat)_grwlPlatformGetModuleSymbol(
        _grwl.x11.xlib.handle, "XkbSetDetectableAutoRepeat");
    _grwl.x11.xrm.DestroyDatabase =
        (PFN_XrmDestroyDatabase)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XrmDestroyDatabase");
    _grwl.x11.xrm.GetResource =
        (PFN_XrmGetResource)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XrmGetResource");
    _grwl.x11.xrm.GetStringDatabase =
        (PFN_XrmGetStringDatabase)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XrmGetStringDatabase");
    _grwl.x11.xrm.UniqueQuark =
        (PFN_XrmUniqueQuark)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XrmUniqueQuark");
    _grwl.x11.xlib.UnregisterIMInstantiateCallback = (PFN_XUnregisterIMInstantiateCallback)_grwlPlatformGetModuleSymbol(
        _grwl.x11.xlib.handle, "XUnregisterIMInstantiateCallback");
    _grwl.x11.xlib.mbResetIC = (PFN_XmbResetIC)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "XmbResetIC");
    _grwl.x11.xlib.utf8LookupString =
        (PFN_Xutf8LookupString)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "Xutf8LookupString");
    _grwl.x11.xlib.utf8SetWMProperties =
        (PFN_Xutf8SetWMProperties)_grwlPlatformGetModuleSymbol(_grwl.x11.xlib.handle, "Xutf8SetWMProperties");

    if (_grwl.x11.xlib.utf8LookupString && _grwl.x11.xlib.utf8SetWMProperties)
    {
        _grwl.x11.xlib.utf8 = true;
    }

    _grwl.x11.screen = DefaultScreen(_grwl.x11.display);
    _grwl.x11.root = RootWindow(_grwl.x11.display, _grwl.x11.screen);
    _grwl.x11.context = XUniqueContext();

    getSystemContentScale(&_grwl.x11.contentScaleX, &_grwl.x11.contentScaleY);

    if (!createEmptyEventPipe())
    {
        return false;
    }

    if (!initExtensions())
    {
        return false;
    }

    _grwl.x11.helperWindowHandle = createHelperWindow();
    _grwl.x11.hiddenCursorHandle = createHiddenCursor();

    if (XSupportsLocale() && _grwl.x11.xlib.utf8)
    {
        XSetLocaleModifiers("");

        // If an IM is already present our callback will be called right away
        XRegisterIMInstantiateCallback(_grwl.x11.display, NULL, NULL, NULL, inputMethodInstantiateCallback, NULL);
    }

    _grwlPollMonitorsX11();
    return true;
}

void _grwlTerminateX11()
{
    if (_grwl.x11.helperWindowHandle)
    {
        if (XGetSelectionOwner(_grwl.x11.display, _grwl.x11.CLIPBOARD) == _grwl.x11.helperWindowHandle)
        {
            _grwlPushSelectionToManagerX11();
        }

        XDestroyWindow(_grwl.x11.display, _grwl.x11.helperWindowHandle);
        _grwl.x11.helperWindowHandle = None;
    }

    if (_grwl.x11.hiddenCursorHandle)
    {
        XFreeCursor(_grwl.x11.display, _grwl.x11.hiddenCursorHandle);
        _grwl.x11.hiddenCursorHandle = (Cursor)0;
    }

    _grwl_free(_grwl.x11.primarySelectionString);
    _grwl_free(_grwl.x11.clipboardString);

    if (_grwl.x11.keyboardLayoutName)
    {
        XFree(_grwl.x11.keyboardLayoutName);
    }

    XUnregisterIMInstantiateCallback(_grwl.x11.display, NULL, NULL, NULL, inputMethodInstantiateCallback, NULL);

    if (_grwl.x11.im)
    {
        XCloseIM(_grwl.x11.im);
        _grwl.x11.im = NULL;
    }

    if (_grwl.x11.display)
    {
        XCloseDisplay(_grwl.x11.display);
        _grwl.x11.display = NULL;
    }

    if (_grwl.x11.x11xcb.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.x11xcb.handle);
        _grwl.x11.x11xcb.handle = NULL;
    }

    if (_grwl.x11.xcursor.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.xcursor.handle);
        _grwl.x11.xcursor.handle = NULL;
    }

    if (_grwl.x11.randr.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.randr.handle);
        _grwl.x11.randr.handle = NULL;
    }

    if (_grwl.x11.xinerama.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.xinerama.handle);
        _grwl.x11.xinerama.handle = NULL;
    }

    if (_grwl.x11.xrender.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.xrender.handle);
        _grwl.x11.xrender.handle = NULL;
    }

    if (_grwl.x11.vidmode.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.vidmode.handle);
        _grwl.x11.vidmode.handle = NULL;
    }

    if (_grwl.x11.xi.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.xi.handle);
        _grwl.x11.xi.handle = NULL;
    }

    _grwlTerminateOSMesa();
    // NOTE: These need to be unloaded after XCloseDisplay, as they register
    //       cleanup callbacks that get called by that function
    _grwlTerminateEGL();
    _grwlTerminateGLX();

    if (_grwl.x11.xlib.handle)
    {
        _grwlPlatformFreeModule(_grwl.x11.xlib.handle);
        _grwl.x11.xlib.handle = NULL;
    }

    if (_grwl.x11.emptyEventPipe[0] || _grwl.x11.emptyEventPipe[1])
    {
        close(_grwl.x11.emptyEventPipe[0]);
        close(_grwl.x11.emptyEventPipe[1]);
    }

    _grwlTerminateDBusPOSIX();
}

#endif // _GRWL_X11
