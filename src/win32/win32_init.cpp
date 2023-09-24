//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_WIN32)

    #include <stdlib.h>

static const GUID _grwl_GUID_DEVINTERFACE_HID = { 0x4d1e55b2,
                                                  0xf16f,
                                                  0x11cf,
                                                  { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

    #define GUID_DEVINTERFACE_HID _grwl_GUID_DEVINTERFACE_HID

    #if defined(_GRWL_USE_HYBRID_HPG) || defined(_GRWL_USE_OPTIMUS_HPG)

        #if defined(_GRWL_BUILD_DLL)
            #pragma message("These symbols must be exported by the executable and have no effect in a DLL")
        #endif

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on Nvidia Optimus systems
// with up-to-date drivers
//
__declspec(dllexport) DWORD NvOptimusEnablement = 1;

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on AMD PowerXpress systems
// with up-to-date drivers
//
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

    #endif // _GRWL_USE_HYBRID_HPG

    #if defined(_GRWL_BUILD_DLL)

// GRWL DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return TRUE;
}

    #endif // _GRWL_BUILD_DLL

// Load necessary libraries (DLLs)
//
static bool loadLibraries()
{
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (const WCHAR*)&_grwl, (HMODULE*)&_grwl.win32.instance))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to retrieve own module handle");
        return false;
    }

    // ntdll needs to be initialised before user32 because user32 needs IsWindows7OrGreater which depends on ntdll.
    _grwl.win32.ntdll.instance = (HINSTANCE)_grwlPlatformLoadModule("ntdll.dll");
    if (_grwl.win32.ntdll.instance)
    {
        _grwl.win32.ntdll.RtlVerifyVersionInfo_ =
            (PFN_RtlVerifyVersionInfo)_grwlPlatformGetModuleSymbol(_grwl.win32.ntdll.instance, "RtlVerifyVersionInfo");
    }

    _grwl.win32.user32.instance = (HINSTANCE)_grwlPlatformLoadModule("user32.dll");
    if (!_grwl.win32.user32.instance)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to load user32.dll");
        return false;
    }

    _grwl.win32.user32.SetProcessDPIAware_ =
        (PFN_SetProcessDPIAware)_grwlPlatformGetModuleSymbol(_grwl.win32.user32.instance, "SetProcessDPIAware");
    _grwl.win32.user32.ChangeWindowMessageFilterEx_ = (PFN_ChangeWindowMessageFilterEx)_grwlPlatformGetModuleSymbol(
        _grwl.win32.user32.instance, "ChangeWindowMessageFilterEx");
    _grwl.win32.user32.EnableNonClientDpiScaling_ = (PFN_EnableNonClientDpiScaling)_grwlPlatformGetModuleSymbol(
        _grwl.win32.user32.instance, "EnableNonClientDpiScaling");
    _grwl.win32.user32.SetProcessDpiAwarenessContext_ = (PFN_SetProcessDpiAwarenessContext)_grwlPlatformGetModuleSymbol(
        _grwl.win32.user32.instance, "SetProcessDpiAwarenessContext");
    _grwl.win32.user32.GetDpiForWindow_ =
        (PFN_GetDpiForWindow)_grwlPlatformGetModuleSymbol(_grwl.win32.user32.instance, "GetDpiForWindow");
    _grwl.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)_grwlPlatformGetModuleSymbol(
        _grwl.win32.user32.instance, "AdjustWindowRectExForDpi");
    _grwl.win32.user32.GetSystemMetricsForDpi_ =
        (PFN_GetSystemMetricsForDpi)_grwlPlatformGetModuleSymbol(_grwl.win32.user32.instance, "GetSystemMetricsForDpi");

    // Strictly speaking only QueryDisplayConfig requires Windows 7.
    // The others two require Windows Vista but we use them in conjunction so I put them all here.
    // If down the road someone needs them separately feel free to put the others two in if(IsWindowsVistaOrGreater())
    // You will also probably need to move the structures defined in win32_platform.h #ifdef WINVER < 0x0601 to another
    // block with WINVER < 0x0600 aka _WIN32_WINNT_VISTA
    if (IsWindows7OrGreater())
    {
        _grwl.win32.user32.GetDisplayConfigBufferSizes_ = (PFN_GetDisplayConfigBufferSizes)_grwlPlatformGetModuleSymbol(
            _grwl.win32.user32.instance, "GetDisplayConfigBufferSizes");
        _grwl.win32.user32.QueryDisplayConfig_ =
            (PFN_QueryDisplayConfig)_grwlPlatformGetModuleSymbol(_grwl.win32.user32.instance, "QueryDisplayConfig");
        _grwl.win32.user32.DisplayConfigGetDeviceInfo_ = (PFN_DisplayConfigGetDeviceInfo)_grwlPlatformGetModuleSymbol(
            _grwl.win32.user32.instance, "DisplayConfigGetDeviceInfo");
    }
    else
    {
        _grwl.win32.user32.GetDisplayConfigBufferSizes_ = nullptr;
        _grwl.win32.user32.QueryDisplayConfig_ = nullptr;
        _grwl.win32.user32.DisplayConfigGetDeviceInfo_ = nullptr;
    }

    _grwl.win32.dinput8.instance = (HINSTANCE)_grwlPlatformLoadModule("dinput8.dll");
    if (_grwl.win32.dinput8.instance)
    {
        _grwl.win32.dinput8.Create =
            (PFN_DirectInput8Create)_grwlPlatformGetModuleSymbol(_grwl.win32.dinput8.instance, "DirectInput8Create");
    }

    {
        const char* names[] = { "xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll",
                                "xinput1_2.dll", "xinput1_1.dll", nullptr };

        for (int i = 0; names[i]; i++)
        {
            _grwl.win32.xinput.instance = (HINSTANCE)_grwlPlatformLoadModule(names[i]);
            if (_grwl.win32.xinput.instance)
            {
                _grwl.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)_grwlPlatformGetModuleSymbol(
                    _grwl.win32.xinput.instance, "XInputGetCapabilities");
                _grwl.win32.xinput.GetState =
                    (PFN_XInputGetState)_grwlPlatformGetModuleSymbol(_grwl.win32.xinput.instance, "XInputGetState");

                break;
            }
        }
    }

    _grwl.win32.dwmapi.instance = (HINSTANCE)_grwlPlatformLoadModule("dwmapi.dll");
    if (_grwl.win32.dwmapi.instance)
    {
        _grwl.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)_grwlPlatformGetModuleSymbol(
            _grwl.win32.dwmapi.instance, "DwmIsCompositionEnabled");
        _grwl.win32.dwmapi.Flush = (PFN_DwmFlush)_grwlPlatformGetModuleSymbol(_grwl.win32.dwmapi.instance, "DwmFlush");
        _grwl.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)_grwlPlatformGetModuleSymbol(
            _grwl.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
        _grwl.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)_grwlPlatformGetModuleSymbol(
            _grwl.win32.dwmapi.instance, "DwmGetColorizationColor");
        _grwl.win32.dwmapi.SetWindowAttribute = (PFN_DwmSetWindowAttribute)_grwlPlatformGetModuleSymbol(
            _grwl.win32.dwmapi.instance, "DwmSetWindowAttribute");
    }

    _grwl.win32.shcore.instance = (HINSTANCE)_grwlPlatformLoadModule("shcore.dll");
    if (_grwl.win32.shcore.instance)
    {
        _grwl.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)_grwlPlatformGetModuleSymbol(
            _grwl.win32.shcore.instance, "SetProcessDpiAwareness");
        _grwl.win32.shcore.GetDpiForMonitor_ =
            (PFN_GetDpiForMonitor)_grwlPlatformGetModuleSymbol(_grwl.win32.shcore.instance, "GetDpiForMonitor");
    }

    _grwl.win32.ntdll.instance = (HINSTANCE)_grwlPlatformLoadModule("ntdll.dll");
    if (_grwl.win32.ntdll.instance)
    {
        _grwl.win32.ntdll.RtlVerifyVersionInfo_ =
            (PFN_RtlVerifyVersionInfo)_grwlPlatformGetModuleSymbol(_grwl.win32.ntdll.instance, "RtlVerifyVersionInfo");
    }

    _grwl.win32.imm32.instance = (HINSTANCE)_grwlPlatformLoadModule("imm32.dll");
    if (_grwl.win32.imm32.instance)
    {
        _grwl.win32.imm32.ImmGetCandidateListW_ =
            (PFN_ImmGetCandidateListW)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmGetCandidateListW");
        _grwl.win32.imm32.ImmGetCompositionStringW_ = (PFN_ImmGetCompositionStringW)_grwlPlatformGetModuleSymbol(
            _grwl.win32.imm32.instance, "ImmGetCompositionStringW");
        _grwl.win32.imm32.ImmGetContext_ =
            (PFN_ImmGetContext)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmGetContext");
        _grwl.win32.imm32.ImmGetConversionStatus_ = (PFN_ImmGetConversionStatus)_grwlPlatformGetModuleSymbol(
            _grwl.win32.imm32.instance, "ImmGetConversionStatus");
        _grwl.win32.imm32.ImmGetDescriptionW_ =
            (PFN_ImmGetDescriptionW)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmGetDescriptionW");
        _grwl.win32.imm32.ImmGetOpenStatus_ =
            (PFN_ImmGetOpenStatus)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmGetOpenStatus");
        _grwl.win32.imm32.ImmNotifyIME_ =
            (PFN_ImmNotifyIME)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmNotifyIME");
        _grwl.win32.imm32.ImmReleaseContext_ =
            (PFN_ImmReleaseContext)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmReleaseContext");
        _grwl.win32.imm32.ImmSetCandidateWindow_ = (PFN_ImmSetCandidateWindow)_grwlPlatformGetModuleSymbol(
            _grwl.win32.imm32.instance, "ImmSetCandidateWindow");
        _grwl.win32.imm32.ImmSetOpenStatus_ =
            (PFN_ImmSetOpenStatus)_grwlPlatformGetModuleSymbol(_grwl.win32.imm32.instance, "ImmSetOpenStatus");
    }

    _grwl.win32.uxtheme.instance = (HINSTANCE)_grwlPlatformLoadModule("uxtheme.dll");
    if (_grwl.win32.uxtheme.instance)
    {
        _grwl.win32.uxtheme.ShouldAppsUseDarkMode =
            (ShouldAppsUseDarkModePtr)_grwlPlatformGetModuleSymbol(_grwl.win32.uxtheme.instance, MAKEINTRESOURCEA(132));
        _grwl.win32.uxtheme.GetImmersiveColorFromColorSetEx =
            (GetImmersiveColorFromColorSetExPtr)_grwlPlatformGetModuleSymbol(_grwl.win32.uxtheme.instance,
                                                                             MAKEINTRESOURCEA(95));
        _grwl.win32.uxtheme.GetImmersiveColorTypeFromName =
            (GetImmersiveColorTypeFromNamePtr)_grwlPlatformGetModuleSymbol(_grwl.win32.uxtheme.instance,
                                                                           MAKEINTRESOURCEA(96));
        _grwl.win32.uxtheme.GetImmersiveUserColorSetPreference =
            (GetImmersiveUserColorSetPreferencePtr)_grwlPlatformGetModuleSymbol(_grwl.win32.uxtheme.instance,
                                                                                MAKEINTRESOURCEA(98));

        _grwl.win32.uxtheme.uxThemeAvailable =
            _grwl.win32.uxtheme.ShouldAppsUseDarkMode && _grwl.win32.uxtheme.GetImmersiveColorFromColorSetEx &&
            _grwl.win32.uxtheme.GetImmersiveColorTypeFromName && _grwl.win32.uxtheme.GetImmersiveUserColorSetPreference;
        _grwl.win32.uxtheme.darkTitleAvailable = _grwlIsWindows10BuildOrGreaterWin32(22000);
    }

    return true;
}

// Unload used libraries (DLLs)
//
static void freeLibraries()
{
    if (_grwl.win32.xinput.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.xinput.instance);
    }

    if (_grwl.win32.dinput8.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.dinput8.instance);
    }

    if (_grwl.win32.user32.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.user32.instance);
    }

    if (_grwl.win32.dwmapi.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.dwmapi.instance);
    }

    if (_grwl.win32.shcore.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.shcore.instance);
    }

    if (_grwl.win32.ntdll.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.ntdll.instance);
    }

    if (_grwl.win32.imm32.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.imm32.instance);
    }

    if (_grwl.win32.uxtheme.instance)
    {
        _grwlPlatformFreeModule(_grwl.win32.uxtheme.instance);
    }
}

// Create key code translation tables
//
static void createKeyTables()
{
    memset(_grwl.win32.keycodes, -1, sizeof(_grwl.win32.keycodes));
    memset(_grwl.win32.scancodes, -1, sizeof(_grwl.win32.scancodes));

    _grwl.win32.keycodes[0x00B] = GRWL_KEY_0;
    _grwl.win32.keycodes[0x002] = GRWL_KEY_1;
    _grwl.win32.keycodes[0x003] = GRWL_KEY_2;
    _grwl.win32.keycodes[0x004] = GRWL_KEY_3;
    _grwl.win32.keycodes[0x005] = GRWL_KEY_4;
    _grwl.win32.keycodes[0x006] = GRWL_KEY_5;
    _grwl.win32.keycodes[0x007] = GRWL_KEY_6;
    _grwl.win32.keycodes[0x008] = GRWL_KEY_7;
    _grwl.win32.keycodes[0x009] = GRWL_KEY_8;
    _grwl.win32.keycodes[0x00A] = GRWL_KEY_9;
    _grwl.win32.keycodes[0x01E] = GRWL_KEY_A;
    _grwl.win32.keycodes[0x030] = GRWL_KEY_B;
    _grwl.win32.keycodes[0x02E] = GRWL_KEY_C;
    _grwl.win32.keycodes[0x020] = GRWL_KEY_D;
    _grwl.win32.keycodes[0x012] = GRWL_KEY_E;
    _grwl.win32.keycodes[0x021] = GRWL_KEY_F;
    _grwl.win32.keycodes[0x022] = GRWL_KEY_G;
    _grwl.win32.keycodes[0x023] = GRWL_KEY_H;
    _grwl.win32.keycodes[0x017] = GRWL_KEY_I;
    _grwl.win32.keycodes[0x024] = GRWL_KEY_J;
    _grwl.win32.keycodes[0x025] = GRWL_KEY_K;
    _grwl.win32.keycodes[0x026] = GRWL_KEY_L;
    _grwl.win32.keycodes[0x032] = GRWL_KEY_M;
    _grwl.win32.keycodes[0x031] = GRWL_KEY_N;
    _grwl.win32.keycodes[0x018] = GRWL_KEY_O;
    _grwl.win32.keycodes[0x019] = GRWL_KEY_P;
    _grwl.win32.keycodes[0x010] = GRWL_KEY_Q;
    _grwl.win32.keycodes[0x013] = GRWL_KEY_R;
    _grwl.win32.keycodes[0x01F] = GRWL_KEY_S;
    _grwl.win32.keycodes[0x014] = GRWL_KEY_T;
    _grwl.win32.keycodes[0x016] = GRWL_KEY_U;
    _grwl.win32.keycodes[0x02F] = GRWL_KEY_V;
    _grwl.win32.keycodes[0x011] = GRWL_KEY_W;
    _grwl.win32.keycodes[0x02D] = GRWL_KEY_X;
    _grwl.win32.keycodes[0x015] = GRWL_KEY_Y;
    _grwl.win32.keycodes[0x02C] = GRWL_KEY_Z;

    _grwl.win32.keycodes[0x028] = GRWL_KEY_APOSTROPHE;
    _grwl.win32.keycodes[0x02B] = GRWL_KEY_BACKSLASH;
    _grwl.win32.keycodes[0x033] = GRWL_KEY_COMMA;
    _grwl.win32.keycodes[0x00D] = GRWL_KEY_EQUAL;
    _grwl.win32.keycodes[0x029] = GRWL_KEY_GRAVE_ACCENT;
    _grwl.win32.keycodes[0x01A] = GRWL_KEY_LEFT_BRACKET;
    _grwl.win32.keycodes[0x00C] = GRWL_KEY_MINUS;
    _grwl.win32.keycodes[0x034] = GRWL_KEY_PERIOD;
    _grwl.win32.keycodes[0x01B] = GRWL_KEY_RIGHT_BRACKET;
    _grwl.win32.keycodes[0x027] = GRWL_KEY_SEMICOLON;
    _grwl.win32.keycodes[0x035] = GRWL_KEY_SLASH;
    _grwl.win32.keycodes[0x056] = GRWL_KEY_WORLD_2;

    _grwl.win32.keycodes[0x00E] = GRWL_KEY_BACKSPACE;
    _grwl.win32.keycodes[0x153] = GRWL_KEY_DELETE;
    _grwl.win32.keycodes[0x14F] = GRWL_KEY_END;
    _grwl.win32.keycodes[0x01C] = GRWL_KEY_ENTER;
    _grwl.win32.keycodes[0x001] = GRWL_KEY_ESCAPE;
    _grwl.win32.keycodes[0x147] = GRWL_KEY_HOME;
    _grwl.win32.keycodes[0x152] = GRWL_KEY_INSERT;
    _grwl.win32.keycodes[0x15D] = GRWL_KEY_MENU;
    _grwl.win32.keycodes[0x151] = GRWL_KEY_PAGE_DOWN;
    _grwl.win32.keycodes[0x149] = GRWL_KEY_PAGE_UP;
    _grwl.win32.keycodes[0x045] = GRWL_KEY_PAUSE;
    _grwl.win32.keycodes[0x039] = GRWL_KEY_SPACE;
    _grwl.win32.keycodes[0x00F] = GRWL_KEY_TAB;
    _grwl.win32.keycodes[0x03A] = GRWL_KEY_CAPS_LOCK;
    _grwl.win32.keycodes[0x145] = GRWL_KEY_NUM_LOCK;
    _grwl.win32.keycodes[0x046] = GRWL_KEY_SCROLL_LOCK;
    _grwl.win32.keycodes[0x03B] = GRWL_KEY_F1;
    _grwl.win32.keycodes[0x03C] = GRWL_KEY_F2;
    _grwl.win32.keycodes[0x03D] = GRWL_KEY_F3;
    _grwl.win32.keycodes[0x03E] = GRWL_KEY_F4;
    _grwl.win32.keycodes[0x03F] = GRWL_KEY_F5;
    _grwl.win32.keycodes[0x040] = GRWL_KEY_F6;
    _grwl.win32.keycodes[0x041] = GRWL_KEY_F7;
    _grwl.win32.keycodes[0x042] = GRWL_KEY_F8;
    _grwl.win32.keycodes[0x043] = GRWL_KEY_F9;
    _grwl.win32.keycodes[0x044] = GRWL_KEY_F10;
    _grwl.win32.keycodes[0x057] = GRWL_KEY_F11;
    _grwl.win32.keycodes[0x058] = GRWL_KEY_F12;
    _grwl.win32.keycodes[0x064] = GRWL_KEY_F13;
    _grwl.win32.keycodes[0x065] = GRWL_KEY_F14;
    _grwl.win32.keycodes[0x066] = GRWL_KEY_F15;
    _grwl.win32.keycodes[0x067] = GRWL_KEY_F16;
    _grwl.win32.keycodes[0x068] = GRWL_KEY_F17;
    _grwl.win32.keycodes[0x069] = GRWL_KEY_F18;
    _grwl.win32.keycodes[0x06A] = GRWL_KEY_F19;
    _grwl.win32.keycodes[0x06B] = GRWL_KEY_F20;
    _grwl.win32.keycodes[0x06C] = GRWL_KEY_F21;
    _grwl.win32.keycodes[0x06D] = GRWL_KEY_F22;
    _grwl.win32.keycodes[0x06E] = GRWL_KEY_F23;
    _grwl.win32.keycodes[0x076] = GRWL_KEY_F24;
    _grwl.win32.keycodes[0x038] = GRWL_KEY_LEFT_ALT;
    _grwl.win32.keycodes[0x01D] = GRWL_KEY_LEFT_CONTROL;
    _grwl.win32.keycodes[0x02A] = GRWL_KEY_LEFT_SHIFT;
    _grwl.win32.keycodes[0x15B] = GRWL_KEY_LEFT_SUPER;
    _grwl.win32.keycodes[0x137] = GRWL_KEY_PRINT_SCREEN;
    _grwl.win32.keycodes[0x138] = GRWL_KEY_RIGHT_ALT;
    _grwl.win32.keycodes[0x11D] = GRWL_KEY_RIGHT_CONTROL;
    _grwl.win32.keycodes[0x036] = GRWL_KEY_RIGHT_SHIFT;
    _grwl.win32.keycodes[0x15C] = GRWL_KEY_RIGHT_SUPER;
    _grwl.win32.keycodes[0x150] = GRWL_KEY_DOWN;
    _grwl.win32.keycodes[0x14B] = GRWL_KEY_LEFT;
    _grwl.win32.keycodes[0x14D] = GRWL_KEY_RIGHT;
    _grwl.win32.keycodes[0x148] = GRWL_KEY_UP;

    _grwl.win32.keycodes[0x052] = GRWL_KEY_KP_0;
    _grwl.win32.keycodes[0x04F] = GRWL_KEY_KP_1;
    _grwl.win32.keycodes[0x050] = GRWL_KEY_KP_2;
    _grwl.win32.keycodes[0x051] = GRWL_KEY_KP_3;
    _grwl.win32.keycodes[0x04B] = GRWL_KEY_KP_4;
    _grwl.win32.keycodes[0x04C] = GRWL_KEY_KP_5;
    _grwl.win32.keycodes[0x04D] = GRWL_KEY_KP_6;
    _grwl.win32.keycodes[0x047] = GRWL_KEY_KP_7;
    _grwl.win32.keycodes[0x048] = GRWL_KEY_KP_8;
    _grwl.win32.keycodes[0x049] = GRWL_KEY_KP_9;
    _grwl.win32.keycodes[0x04E] = GRWL_KEY_KP_ADD;
    _grwl.win32.keycodes[0x053] = GRWL_KEY_KP_DECIMAL;
    _grwl.win32.keycodes[0x135] = GRWL_KEY_KP_DIVIDE;
    _grwl.win32.keycodes[0x11C] = GRWL_KEY_KP_ENTER;
    _grwl.win32.keycodes[0x059] = GRWL_KEY_KP_EQUAL;
    _grwl.win32.keycodes[0x037] = GRWL_KEY_KP_MULTIPLY;
    _grwl.win32.keycodes[0x04A] = GRWL_KEY_KP_SUBTRACT;

    for (int scancode = 0; scancode < 512; scancode++)
    {
        if (_grwl.win32.keycodes[scancode] > 0)
        {
            _grwl.win32.scancodes[_grwl.win32.keycodes[scancode]] = scancode;
        }
    }
}

// Window procedure for the hidden helper window
//
static LRESULT CALLBACK helperWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DISPLAYCHANGE:
            _grwlPollMonitorsWin32();
            break;

        case WM_DEVICECHANGE:
        {
            if (!_grwl.joysticksInitialized)
            {
                break;
            }

            if (wParam == DBT_DEVICEARRIVAL)
            {
                DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*)lParam;
                if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    _grwlDetectJoystickConnectionWin32();
                }
            }
            else if (wParam == DBT_DEVICEREMOVECOMPLETE)
            {
                DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*)lParam;
                if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    _grwlDetectJoystickDisconnectionWin32();
                }
            }

            break;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// Creates a dummy window for behind-the-scenes work
//
static bool createHelperWindow()
{
    MSG msg;
    WNDCLASSEXW wc = { sizeof(wc) };

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)helperWindowProc;
    wc.hInstance = _grwl.win32.instance;
    wc.lpszClassName = L"GRWL Helper";

    _grwl.win32.helperWindowClass = RegisterClassExW(&wc);
    if (!_grwl.win32.helperWindowClass)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to register helper window class");
        return false;
    }

    _grwl.win32.helperWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, MAKEINTATOM(_grwl.win32.helperWindowClass),
                                                     L"GRWL message window", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1,
                                                     1, nullptr, nullptr, _grwl.win32.instance, nullptr);

    if (!_grwl.win32.helperWindowHandle)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create helper window");
        return false;
    }

    // HACK: The command to the first ShowWindow call is ignored if the parent
    //       process passed along a STARTUPINFO, so clear that with a no-op call
    ShowWindow(_grwl.win32.helperWindowHandle, SW_HIDE);

    // Register for HID device notifications
    {
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        ZeroMemory(&dbi, sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;

        _grwl.win32.deviceNotificationHandle = RegisterDeviceNotificationW(
            _grwl.win32.helperWindowHandle, (DEV_BROADCAST_HDR*)&dbi, DEVICE_NOTIFY_WINDOW_HANDLE);
    }

    while (PeekMessageW(&msg, _grwl.win32.helperWindowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Returns a wide string version of the specified UTF-8 string
//
WCHAR* _grwlCreateWideStringFromUTF8Win32(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, nullptr, 0);
    if (!count)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to convert string from UTF-8");
        return nullptr;
    }

    target = (WCHAR*)_grwl_calloc(count, sizeof(WCHAR));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to convert string from UTF-8");
        _grwl_free(target);
        return nullptr;
    }

    return target;
}

// Returns a UTF-8 string version of the specified wide string
//
char* _grwlCreateUTF8FromWideStringWin32(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, nullptr, 0, nullptr, nullptr);
    if (!size)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to convert string to UTF-8");
        return nullptr;
    }

    target = (char*)_grwl_calloc(size, 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, nullptr, nullptr))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to convert string to UTF-8");
        _grwl_free(target);
        return nullptr;
    }

    return target;
}

// Reports the specified error, appending information about the last Win32 error
//
void _grwlInputErrorWin32(int error, const char* description)
{
    WCHAR buffer[_GRWL_MESSAGE_SIZE] = L"";
    char message[_GRWL_MESSAGE_SIZE] = "";

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr,
                   GetLastError() & 0xffff, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer,
                   sizeof(buffer) / sizeof(WCHAR), nullptr);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), nullptr, nullptr);

    _grwlInputError(error, "%s: %s", description, message);
}

// Updates key names according to the current keyboard layout
//
void _grwlUpdateKeyNamesWin32()
{
    BYTE state[256] = { 0 };

    memset(_grwl.win32.keynames, 0, sizeof(_grwl.win32.keynames));

    for (int key = GRWL_KEY_SPACE; key <= GRWL_KEY_LAST; key++)
    {
        UINT vk;
        int scancode, length;
        WCHAR chars[16];

        scancode = _grwl.win32.scancodes[key];
        if (scancode == -1)
        {
            continue;
        }

        if (key >= GRWL_KEY_KP_0 && key <= GRWL_KEY_KP_ADD)
        {
            const UINT vks[] = { VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2,  VK_NUMPAD3,  VK_NUMPAD4,
                                 VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7,  VK_NUMPAD8,  VK_NUMPAD9,
                                 VK_DECIMAL, VK_DIVIDE,  VK_MULTIPLY, VK_SUBTRACT, VK_ADD };

            vk = vks[key - GRWL_KEY_KP_0];
        }
        else
        {
            vk = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK);
        }

        length = ToUnicode(vk, scancode, state, chars, sizeof(chars) / sizeof(WCHAR), 0);

        if (length == -1)
        {
            // This is a dead key, so we need a second simulated key press
            // to make it output its own character (usually a diacritic)
            length = ToUnicode(vk, scancode, state, chars, sizeof(chars) / sizeof(WCHAR), 0);
        }

        if (length < 1)
        {
            continue;
        }

        WideCharToMultiByte(CP_UTF8, 0, chars, 1, _grwl.win32.keynames[key], sizeof(_grwl.win32.keynames[key]), nullptr,
                            nullptr);
    }
}

// Replacement for IsWindowsVersionOrGreater, as we cannot rely on the
// application having a correct embedded manifest
//
BOOL _grwlIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, { 0 }, sp };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

// Checks whether we are on at least the specified build of Windows 10
//
BOOL _grwlIsWindows10BuildOrGreaterWin32(WORD build)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 10, 0, build };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

bool _grwlConnectWin32(int platformID, _GRWLplatform* platform)
{
    const _GRWLplatform win32 = {
        GRWL_PLATFORM_WIN32,
        _grwlInitWin32,
        _grwlTerminateWin32,
        _grwlGetCursorPosWin32,
        _grwlSetCursorPosWin32,
        _grwlSetCursorModeWin32,
        _grwlSetRawMouseMotionWin32,
        _grwlRawMouseMotionSupportedWin32,
        _grwlCreateCursorWin32,
        _grwlCreateStandardCursorWin32,
        _grwlDestroyCursorWin32,
        _grwlSetCursorWin32,
        _grwlGetScancodeNameWin32,
        _grwlGetKeyScancodeWin32,
        _grwlGetKeyboardLayoutNameWin32,
        _grwlSetClipboardStringWin32,
        _grwlGetClipboardStringWin32,
        _grwlUpdatePreeditCursorRectangleWin32,
        _grwlResetPreeditTextWin32,
        _grwlSetIMEStatusWin32,
        _grwlGetIMEStatusWin32,
        _grwlInitJoysticksWin32,
        _grwlTerminateJoysticksWin32,
        _grwlPollJoystickWin32,
        _grwlGetMappingNameWin32,
        _grwlUpdateGamepadGUIDWin32,
        _grwlFreeMonitorWin32,
        _grwlGetMonitorPosWin32,
        _grwlGetMonitorContentScaleWin32,
        _grwlGetMonitorWorkareaWin32,
        _grwlGetVideoModesWin32,
        _grwlGetVideoModeWin32,
        _grwlGetGammaRampWin32,
        _grwlSetGammaRampWin32,
        _grwlCreateWindowWin32,
        _grwlDestroyWindowWin32,
        _grwlSetWindowTitleWin32,
        _grwlSetWindowIconWin32,
        _grwlSetWindowProgressIndicatorWin32,
        _grwlSetWindowBadgeWin32,
        _grwlSetWindowBadgeStringWin32,
        _grwlGetWindowPosWin32,
        _grwlSetWindowPosWin32,
        _grwlGetWindowSizeWin32,
        _grwlSetWindowSizeWin32,
        _grwlSetWindowSizeLimitsWin32,
        _grwlSetWindowAspectRatioWin32,
        _grwlGetFramebufferSizeWin32,
        _grwlGetWindowFrameSizeWin32,
        _grwlGetWindowContentScaleWin32,
        _grwlIconifyWindowWin32,
        _grwlRestoreWindowWin32,
        _grwlMaximizeWindowWin32,
        _grwlShowWindowWin32,
        _grwlHideWindowWin32,
        _grwlRequestWindowAttentionWin32,
        _grwlFocusWindowWin32,
        _grwlSetWindowMonitorWin32,
        _grwlWindowFocusedWin32,
        _grwlWindowIconifiedWin32,
        _grwlWindowVisibleWin32,
        _grwlWindowMaximizedWin32,
        _grwlWindowHoveredWin32,
        _grwlFramebufferTransparentWin32,
        _grwlGetWindowOpacityWin32,
        _grwlSetWindowResizableWin32,
        _grwlSetWindowDecoratedWin32,
        _grwlSetWindowFloatingWin32,
        _grwlSetWindowOpacityWin32,
        _grwlSetWindowMousePassthroughWin32,
        _grwlPollEventsWin32,
        _grwlWaitEventsWin32,
        _grwlWaitEventsTimeoutWin32,
        _grwlPostEmptyEventWin32,
        _grwlCreateUserContextWin32,
        _grwlGetEGLPlatformWin32,
        _grwlGetEGLNativeDisplayWin32,
        _grwlGetEGLNativeWindowWin32,
        _grwlGetRequiredInstanceExtensionsWin32,
        _grwlGetPhysicalDevicePresentationSupportWin32,
        _grwlCreateWindowSurfaceWin32,
    };

    *platform = win32;
    return true;
}

bool _grwlInitWin32()
{
    if (!loadLibraries())
    {
        return false;
    }

    createKeyTables();
    _grwlUpdateKeyNamesWin32();

    if (_grwlIsWindows10Version1703OrGreaterWin32())
    {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }
    else if (IsWindows8Point1OrGreater())
    {
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
    else if (IsWindowsVistaOrGreater())
    {
        SetProcessDPIAware();
    }

    if (!createHelperWindow())
    {
        return false;
    }

    _grwlPollMonitorsWin32();
    return true;
}

void _grwlTerminateWin32()
{
    if (_grwl.win32.deviceNotificationHandle)
    {
        UnregisterDeviceNotification(_grwl.win32.deviceNotificationHandle);
    }

    if (_grwl.win32.helperWindowHandle)
    {
        DestroyWindow(_grwl.win32.helperWindowHandle);
    }
    if (_grwl.win32.helperWindowClass)
    {
        UnregisterClassW(MAKEINTATOM(_grwl.win32.helperWindowClass), _grwl.win32.instance);
    }
    if (_grwl.win32.mainWindowClass)
    {
        UnregisterClassW(MAKEINTATOM(_grwl.win32.mainWindowClass), _grwl.win32.instance);
    }

    _grwl_free(_grwl.win32.clipboardString);
    _grwl_free(_grwl.win32.keyboardLayoutName);
    _grwl_free(_grwl.win32.rawInput);

    _grwlTerminateWGL();
    _grwlTerminateEGL();
    _grwlTerminateOSMesa();

    freeLibraries();
}

#endif // _GRWL_WIN32
