//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

// We don't need all the fancy stuff
#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

// This is a workaround for the fact that grwl.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_KHR_debug callback)
// but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

// GRWL on Windows is Unicode only and does not work in MBCS mode
#ifndef UNICODE
    #define UNICODE
#endif

// GRWL requires Windows XP or later
#if WINVER < 0x0501
    #undef WINVER
    #define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
#endif

// GRWL uses DirectInput8 interfaces
#define DIRECTINPUT_VERSION 0x0800

// GRWL uses OEM cursor resources
#define OEMRESOURCE

#include <cwctype>
#include <windows.h>
#define CINTERFACE
#include <dinput.h>
#include <xinput.h>
#include <dbt.h>
#include <imm.h>

// HACK: Define macros that some windows.h variants don't
#ifndef WM_MOUSEHWHEEL
    #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
    #define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
    #define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif
#ifndef WM_COPYGLOBALDATA
    #define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_UNICHAR
    #define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
    #define UNICODE_NOCHAR 0xFFFF
#endif
#ifndef WM_DPICHANGED
    #define WM_DPICHANGED 0x02E0
#endif
#ifndef GET_XBUTTON_WPARAM
    #define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef EDS_ROTATEDMODE
    #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
    #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif
#ifndef _WIN32_WINNT_WINBLUE
    #define _WIN32_WINNT_WINBLUE 0x0603
#endif
#ifndef _WIN32_WINNT_WIN8
    #define _WIN32_WINNT_WIN8 0x0602
#endif
#ifndef WM_GETDPISCALEDSIZE
    #define WM_GETDPISCALEDSIZE 0x02e4
#endif
#ifndef USER_DEFAULT_SCREEN_DPI
    #define USER_DEFAULT_SCREEN_DPI 96
#endif
#ifndef OCR_HAND
    #define OCR_HAND 32649
#endif

#if WINVER < 0x0601
typedef struct
{
    DWORD cbSize;
    DWORD ExtStatus;
} CHANGEFILTERSTRUCT;
    #ifndef MSGFLT_ALLOW
        #define MSGFLT_ALLOW 1
    #endif

typedef struct DISPLAYCONFIG_PATH_SOURCE_INFO
{
    LUID adapterId;
    UINT32 id;

    union
    {
        UINT32 modeInfoIdx;

        struct
        {
            UINT32 cloneGroupId : 16;
            UINT32 sourceModeInfoIdx : 16;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    UINT32 statusFlags;
} DISPLAYCONFIG_PATH_SOURCE_INFO;

typedef enum
{
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER = -1,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15 = 0,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO = 1,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO = 2,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO = 3,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI = 4,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI = 5,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS = 6,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN = 8,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI = 9,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL = 10,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED = 11,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL = 12,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED = 13,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE = 14,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_MIRACAST = 15,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_WIRED = 16,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_VIRTUAL = 17,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_USB_TUNNEL,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL = (int)0x80000000,    // Cast required to enforce 4 byte enum.
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_FORCE_UINT32 = (int)0xFFFFFFFF // Cast required to enforce 4 byte enum.
} DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY;

typedef enum
{
    DISPLAYCONFIG_ROTATION_IDENTITY = 1,
    DISPLAYCONFIG_ROTATION_ROTATE90 = 2,
    DISPLAYCONFIG_ROTATION_ROTATE180 = 3,
    DISPLAYCONFIG_ROTATION_ROTATE270 = 4,
    DISPLAYCONFIG_ROTATION_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_ROTATION;

typedef enum
{
    DISPLAYCONFIG_SCALING_IDENTITY = 1,
    DISPLAYCONFIG_SCALING_CENTERED = 2,
    DISPLAYCONFIG_SCALING_STRETCHED = 3,
    DISPLAYCONFIG_SCALING_ASPECTRATIOCENTEREDMAX = 4,
    DISPLAYCONFIG_SCALING_CUSTOM = 5,
    DISPLAYCONFIG_SCALING_PREFERRED = 128,
    DISPLAYCONFIG_SCALING_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_SCALING;

typedef struct DISPLAYCONFIG_RATIONAL
{
    UINT32 Numerator;
    UINT32 Denominator;
} DISPLAYCONFIG_RATIONAL;

typedef enum
{
    DISPLAYCONFIG_SCANLINE_ORDERING_UNSPECIFIED = 0,
    DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE = 1,
    DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED = 2,
    DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_UPPERFIELDFIRST,
    DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_LOWERFIELDFIRST = 3,
    DISPLAYCONFIG_SCANLINE_ORDERING_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_SCANLINE_ORDERING;

typedef struct DISPLAYCONFIG_PATH_TARGET_INFO
{
    LUID adapterId;
    UINT32 id;

    union
    {
        UINT32 modeInfoIdx;

        struct
        {
            UINT32 desktopModeInfoIdx : 16;
            UINT32 targetModeInfoIdx : 16;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
    DISPLAYCONFIG_ROTATION rotation;
    DISPLAYCONFIG_SCALING scaling;
    DISPLAYCONFIG_RATIONAL refreshRate;
    DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
    BOOL targetAvailable;
    UINT32 statusFlags;
} DISPLAYCONFIG_PATH_TARGET_INFO;

typedef struct DISPLAYCONFIG_PATH_INFO
{
    DISPLAYCONFIG_PATH_SOURCE_INFO sourceInfo;
    DISPLAYCONFIG_PATH_TARGET_INFO targetInfo;
    UINT32 flags;
} DISPLAYCONFIG_PATH_INFO;

typedef enum
{
    DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE = 1,
    DISPLAYCONFIG_MODE_INFO_TYPE_TARGET = 2,
    DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE = 3,
    DISPLAYCONFIG_MODE_INFO_TYPE_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_MODE_INFO_TYPE;

typedef struct DISPLAYCONFIG_2DREGION
{
    UINT32 cx;
    UINT32 cy;
} DISPLAYCONFIG_2DREGION;

typedef struct DISPLAYCONFIG_VIDEO_SIGNAL_INFO
{
    UINT64 pixelRate;
    DISPLAYCONFIG_RATIONAL hSyncFreq;
    DISPLAYCONFIG_RATIONAL vSyncFreq;
    DISPLAYCONFIG_2DREGION activeSize;
    DISPLAYCONFIG_2DREGION totalSize;

    union
    {
        struct
        {
            UINT32 videoStandard : 16;
            UINT32 vSyncFreqDivider : 6;
            UINT32 reserved : 10;
        } AdditionalSignalInfo;

        UINT32 videoStandard;
    } DUMMYUNIONNAME;

    DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
} DISPLAYCONFIG_VIDEO_SIGNAL_INFO;

typedef struct DISPLAYCONFIG_TARGET_MODE
{
    DISPLAYCONFIG_VIDEO_SIGNAL_INFO targetVideoSignalInfo;
} DISPLAYCONFIG_TARGET_MODE;

typedef enum
{
    DISPLAYCONFIG_PIXELFORMAT_8BPP = 1,
    DISPLAYCONFIG_PIXELFORMAT_16BPP = 2,
    DISPLAYCONFIG_PIXELFORMAT_24BPP = 3,
    DISPLAYCONFIG_PIXELFORMAT_32BPP = 4,
    DISPLAYCONFIG_PIXELFORMAT_NONGDI = 5,
    DISPLAYCONFIG_PIXELFORMAT_FORCE_UINT32 = 0xffffffff
} DISPLAYCONFIG_PIXELFORMAT;

typedef struct DISPLAYCONFIG_SOURCE_MODE
{
    UINT32 width;
    UINT32 height;
    DISPLAYCONFIG_PIXELFORMAT pixelFormat;
    POINTL position;
} DISPLAYCONFIG_SOURCE_MODE;

typedef struct DISPLAYCONFIG_DESKTOP_IMAGE_INFO
{
    POINTL PathSourceSize;
    RECTL DesktopImageRegion;
    RECTL DesktopImageClip;
} DISPLAYCONFIG_DESKTOP_IMAGE_INFO;

typedef struct DISPLAYCONFIG_MODE_INFO
{
    DISPLAYCONFIG_MODE_INFO_TYPE infoType;
    UINT32 id;
    LUID adapterId;

    union
    {
        DISPLAYCONFIG_TARGET_MODE targetMode;
        DISPLAYCONFIG_SOURCE_MODE sourceMode;
        DISPLAYCONFIG_DESKTOP_IMAGE_INFO desktopImageInfo;
    } DUMMYUNIONNAME;
} DISPLAYCONFIG_MODE_INFO;

extern const UINT32 QDC_ONLY_ACTIVE_PATHS;

typedef enum
{
    DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME = 1,
    DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME = 2,
    DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE = 3,
    DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME = 4,
    DISPLAYCONFIG_DEVICE_INFO_SET_TARGET_PERSISTENCE = 5,
    DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_BASE_TYPE = 6,
    DISPLAYCONFIG_DEVICE_INFO_GET_SUPPORT_VIRTUAL_RESOLUTION = 7,
    DISPLAYCONFIG_DEVICE_INFO_SET_SUPPORT_VIRTUAL_RESOLUTION = 8,
    DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO = 9,
    DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE = 10,
    DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL = 11,
    DISPLAYCONFIG_DEVICE_INFO_GET_MONITOR_SPECIALIZATION,
    DISPLAYCONFIG_DEVICE_INFO_SET_MONITOR_SPECIALIZATION,
    DISPLAYCONFIG_DEVICE_INFO_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_DEVICE_INFO_TYPE;

typedef struct DISPLAYCONFIG_DEVICE_INFO_HEADER
{
    DISPLAYCONFIG_DEVICE_INFO_TYPE type;
    UINT32 size;
    LUID adapterId;
    UINT32 id;
} DISPLAYCONFIG_DEVICE_INFO_HEADER;

typedef struct DISPLAYCONFIG_SOURCE_DEVICE_NAME
{
    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
    WCHAR viewGdiDeviceName[CCHDEVICENAME];
} DISPLAYCONFIG_SOURCE_DEVICE_NAME;

typedef struct DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS
{
    union
    {
        struct
        {
            UINT32 friendlyNameFromEdid : 1;
            UINT32 friendlyNameForced : 1;
            UINT32 edidIdsValid : 1;
            UINT32 reserved : 29;
        } DUMMYSTRUCTNAME;

        UINT32 value;
    } DUMMYUNIONNAME;
} DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS;

typedef struct DISPLAYCONFIG_TARGET_DEVICE_NAME
{
    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
    DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS flags;
    DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
    UINT16 edidManufactureId;
    UINT16 edidProductCodeId;
    UINT32 connectorInstance;
    WCHAR monitorFriendlyDeviceName[64];
    WCHAR monitorDevicePath[128];
} DISPLAYCONFIG_TARGET_DEVICE_NAME;

typedef enum DISPLAYCONFIG_TOPOLOGY_ID
{
    DISPLAYCONFIG_TOPOLOGY_INTERNAL = 0x00000001,
    DISPLAYCONFIG_TOPOLOGY_CLONE = 0x00000002,
    DISPLAYCONFIG_TOPOLOGY_EXTEND = 0x00000004,
    DISPLAYCONFIG_TOPOLOGY_EXTERNAL = 0x00000008,
    DISPLAYCONFIG_TOPOLOGY_FORCE_UINT32 = 0xFFFFFFFF
} DISPLAYCONFIG_TOPOLOGY_ID;

#endif /*Windows 7*/

#if WINVER < 0x0600
    #define DWM_BB_ENABLE 0x00000001
    #define DWM_BB_BLURREGION 0x00000002

typedef struct
{
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;
#else
    #include <dwmapi.h>
#endif /*Windows Vista*/

#ifndef DPI_ENUMS_DECLARED
typedef enum
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum
{
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif /*DPI_ENUMS_DECLARED*/

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)
#endif /*DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2*/

// Replacement for versionhelpers.h macros, as we cannot rely on the
// application having a correct embedded manifest
//
#define IsWindowsVistaOrGreater() \
    _grwlIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0)
#define IsWindows7OrGreater() \
    _grwlIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0)
#define IsWindows8OrGreater() \
    _grwlIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0)
#define IsWindows8Point1OrGreater() \
    _grwlIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0)

// Windows 10 Anniversary Update
#define _grwlIsWindows10Version1607OrGreaterWin32() _grwlIsWindows10BuildOrGreaterWin32(14393)
// Windows 10 Creators Update
#define _grwlIsWindows10Version1703OrGreaterWin32() _grwlIsWindows10BuildOrGreaterWin32(15063)

// HACK: Define macros that some xinput.h variants don't
#ifndef XINPUT_CAPS_WIRELESS
    #define XINPUT_CAPS_WIRELESS 0x0002
#endif
#ifndef XINPUT_DEVSUBTYPE_WHEEL
    #define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_STICK
    #define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
    #define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#ifndef XINPUT_DEVSUBTYPE_DANCE_PAD
    #define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR
    #define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#ifndef XINPUT_DEVSUBTYPE_DRUM_KIT
    #define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_PAD
    #define XINPUT_DEVSUBTYPE_ARCADE_PAD 0x13
#endif
#ifndef XUSER_MAX_COUNT
    #define XUSER_MAX_COUNT 4
#endif

// HACK: Define macros that some dinput.h variants don't
#ifndef DIDFT_OPTIONAL
    #define DIDFT_OPTIONAL 0x80000000
#endif

#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202b
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201a
#define WGL_ALPHA_BITS_ARB 0x201b
#define WGL_ALPHA_SHIFT_ARB 0x201c
#define WGL_ACCUM_BITS_ARB 0x201d
#define WGL_ACCUM_RED_BITS_ARB 0x201e
#define WGL_ACCUM_GREEN_BITS_ARB 0x201f
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_STEREO_ARB 0x2012
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_SAMPLES_ARB 0x2042
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20a9
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define WGL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define WGL_NO_RESET_NOTIFICATION_ARB 0x8261
#define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
#define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3
#define WGL_COLORSPACE_EXT 0x309d
#define WGL_COLORSPACE_SRGB_EXT 0x3089

#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096
#define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054

// xinput.dll function pointer typedefs
typedef DWORD(WINAPI* PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD(WINAPI* PFN_XInputGetState)(DWORD, XINPUT_STATE*);
#define XInputGetCapabilities _grwl.win32.xinput.GetCapabilities
#define XInputGetState _grwl.win32.xinput.GetState

// dinput8.dll function pointer typedefs
typedef HRESULT(WINAPI* PFN_DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#define DirectInput8Create _grwl.win32.dinput8.Create

// user32.dll function pointer typedefs
typedef BOOL(WINAPI* PFN_SetProcessDPIAware)();
typedef BOOL(WINAPI* PFN_ChangeWindowMessageFilterEx)(HWND, UINT, DWORD, CHANGEFILTERSTRUCT*);
typedef BOOL(WINAPI* PFN_EnableNonClientDpiScaling)(HWND);
typedef BOOL(WINAPI* PFN_SetProcessDpiAwarenessContext)(HANDLE);
typedef UINT(WINAPI* PFN_GetDpiForWindow)(HWND);
typedef BOOL(WINAPI* PFN_AdjustWindowRectExForDpi)(LPRECT, DWORD, BOOL, DWORD, UINT);
typedef int(WINAPI* PFN_GetSystemMetricsForDpi)(int, UINT);
typedef LONG(WINAPI* PFN_GetDisplayConfigBufferSizes)(UINT32, UINT32*, UINT32*);
typedef LONG(WINAPI* PFN_QueryDisplayConfig)(UINT32, UINT32*, DISPLAYCONFIG_PATH_INFO*, UINT32*,
                                             DISPLAYCONFIG_MODE_INFO*, DISPLAYCONFIG_TOPOLOGY_ID*);
typedef LONG(WINAPI* PFN_DisplayConfigGetDeviceInfo)(DISPLAYCONFIG_DEVICE_INFO_HEADER*);

#define SetProcessDPIAware _grwl.win32.user32.SetProcessDPIAware_
#define ChangeWindowMessageFilterEx _grwl.win32.user32.ChangeWindowMessageFilterEx_
#define EnableNonClientDpiScaling _grwl.win32.user32.EnableNonClientDpiScaling_
#define SetProcessDpiAwarenessContext _grwl.win32.user32.SetProcessDpiAwarenessContext_
#define GetDpiForWindow _grwl.win32.user32.GetDpiForWindow_
#define AdjustWindowRectExForDpi _grwl.win32.user32.AdjustWindowRectExForDpi_
#define GetSystemMetricsForDpi _grwl.win32.user32.GetSystemMetricsForDpi_
#define GetDisplayConfigBufferSizes _grwl.win32.user32.GetDisplayConfigBufferSizes_
#define QueryDisplayConfig _grwl.win32.user32.QueryDisplayConfig_
#define DisplayConfigGetDeviceInfo _grwl.win32.user32.DisplayConfigGetDeviceInfo_

// dwmapi.dll function pointer typedefs
typedef HRESULT(WINAPI* PFN_DwmIsCompositionEnabled)(BOOL*);
typedef HRESULT(WINAPI* PFN_DwmFlush)(VOID);
typedef HRESULT(WINAPI* PFN_DwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);
typedef HRESULT(WINAPI* PFN_DwmGetColorizationColor)(DWORD*, BOOL*);
typedef HRESULT(WINAPI* PFN_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);

#define DwmIsCompositionEnabled _grwl.win32.dwmapi.IsCompositionEnabled
#define DwmFlush _grwl.win32.dwmapi.Flush
#define DwmEnableBlurBehindWindow _grwl.win32.dwmapi.EnableBlurBehindWindow
#define DwmGetColorizationColor _grwl.win32.dwmapi.GetColorizationColor
#define DwmSetWindowAttribute _grwl.win32.dwmapi.SetWindowAttribute

// shcore.dll function pointer typedefs
typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);
#define SetProcessDpiAwareness _grwl.win32.shcore.SetProcessDpiAwareness_
#define GetDpiForMonitor _grwl.win32.shcore.GetDpiForMonitor_

// ntdll.dll function pointer typedefs
typedef LONG(WINAPI* PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);
#define RtlVerifyVersionInfo _grwl.win32.ntdll.RtlVerifyVersionInfo_

// imm32 function pointer typedefs
typedef DWORD(WINAPI* PFN_ImmGetCandidateListW)(HIMC, DWORD, LPCANDIDATELIST, DWORD);
typedef LONG(WINAPI* PFN_ImmGetCompositionStringW)(HIMC, DWORD, LPVOID, DWORD);
typedef HIMC(WINAPI* PFN_ImmGetContext)(HWND);
typedef BOOL(WINAPI* PFN_ImmGetConversionStatus)(HIMC, LPDWORD, LPDWORD);
typedef UINT(WINAPI* PFN_ImmGetDescriptionW)(HKL, LPWSTR, UINT);
typedef BOOL(WINAPI* PFN_ImmGetOpenStatus)(HIMC);
typedef BOOL(WINAPI* PFN_ImmNotifyIME)(HIMC, DWORD, DWORD, DWORD);
typedef BOOL(WINAPI* PFN_ImmReleaseContext)(HWND, HIMC);
typedef BOOL(WINAPI* PFN_ImmSetCandidateWindow)(HIMC, LPCANDIDATEFORM);
typedef BOOL(WINAPI* PFN_ImmSetOpenStatus)(HIMC, BOOL);
#define ImmGetCandidateListW _grwl.win32.imm32.ImmGetCandidateListW_
#define ImmGetCompositionStringW _grwl.win32.imm32.ImmGetCompositionStringW_
#define ImmGetContext _grwl.win32.imm32.ImmGetContext_
#define ImmGetConversionStatus _grwl.win32.imm32.ImmGetConversionStatus_
#define ImmGetDescriptionW _grwl.win32.imm32.ImmGetDescriptionW_
#define ImmGetOpenStatus _grwl.win32.imm32.ImmGetOpenStatus_
#define ImmNotifyIME _grwl.win32.imm32.ImmNotifyIME_
#define ImmReleaseContext _grwl.win32.imm32.ImmReleaseContext_
#define ImmSetCandidateWindow _grwl.win32.imm32.ImmSetCandidateWindow_
#define ImmSetOpenStatus _grwl.win32.imm32.ImmSetOpenStatus_

// WGL extension pointer typedefs
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef BOOL(WINAPI* PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC, int, int, UINT, const int*, int*);
typedef const char*(WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)();
typedef const char*(WINAPI* PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
#define wglSwapIntervalEXT _grwl.wgl.SwapIntervalEXT
#define wglGetPixelFormatAttribivARB _grwl.wgl.GetPixelFormatAttribivARB
#define wglGetExtensionsStringEXT _grwl.wgl.GetExtensionsStringEXT
#define wglGetExtensionsStringARB _grwl.wgl.GetExtensionsStringARB
#define wglCreateContextAttribsARB _grwl.wgl.CreateContextAttribsARB

// opengl32.dll function pointer typedefs
typedef HGLRC(WINAPI* PFN_wglCreateContext)(HDC);
typedef BOOL(WINAPI* PFN_wglDeleteContext)(HGLRC);
typedef PROC(WINAPI* PFN_wglGetProcAddress)(LPCSTR);
typedef HDC(WINAPI* PFN_wglGetCurrentDC)();
typedef HGLRC(WINAPI* PFN_wglGetCurrentContext)();
typedef BOOL(WINAPI* PFN_wglMakeCurrent)(HDC, HGLRC);
typedef BOOL(WINAPI* PFN_wglShareLists)(HGLRC, HGLRC);
#define wglCreateContext _grwl.wgl.CreateContext
#define wglDeleteContext _grwl.wgl.DeleteContext
#define wglGetProcAddress _grwl.wgl.GetProcAddress
#define wglGetCurrentDC _grwl.wgl.GetCurrentDC
#define wglGetCurrentContext _grwl.wgl.GetCurrentContext
#define wglMakeCurrent _grwl.wgl.MakeCurrent
#define wglShareLists _grwl.wgl.ShareLists

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkWin32SurfaceCreateFlagsKHR flags;
    HINSTANCE hinstance;
    HWND hwnd;
} VkWin32SurfaceCreateInfoKHR;

typedef VkResult(APIENTRY* PFN_vkCreateWin32SurfaceKHR)(VkInstance, const VkWin32SurfaceCreateInfoKHR*,
                                                        const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef VkBool32(APIENTRY* PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice, uint32_t);

#define GRWL_WIN32_WINDOW_STATE _GRWLwindowWin32 win32;
#define GRWL_WIN32_LIBRARY_WINDOW_STATE _GRWLlibraryWin32 win32;
#define GRWL_WIN32_MONITOR_STATE _GRWLmonitorWin32 win32;
#define GRWL_WIN32_CURSOR_STATE _GRWLcursorWin32 win32;

#define GRWL_WGL_CONTEXT_STATE _GRWLcontextWGL wgl;
#define GRWL_WGL_LIBRARY_CONTEXT_STATE _GRWLlibraryWGL wgl;
#define GRWL_WGL_USER_CONTEXT_STATE _GRWLusercontextWGL wgl;

typedef BOOL(WINAPI* ShouldAppsUseDarkModePtr)();
typedef DWORD(WINAPI* GetImmersiveColorFromColorSetExPtr)(UINT, UINT, BOOL, UINT);
typedef int(WINAPI* GetImmersiveColorTypeFromNamePtr)(const WCHAR*);
typedef int(WINAPI* GetImmersiveUserColorSetPreferencePtr)(BOOL, BOOL);

typedef enum
{
    TBPF_NOPROGRESS = 0x0,
    TBPF_INDETERMINATE = 0x1,
    TBPF_NORMAL = 0x2,
    TBPF_ERROR = 0x4,
    TBPF_PAUSED = 0x8
} TBPFLAG;

static const IID IID_ITaskbarList3 = { 0xea1afb91, 0x9e28, 0x4b86, { 0x90, 0xe9, 0x9e, 0x9f, 0x8a, 0x5e, 0xef, 0xaf } };
static const IID CLSID_TaskbarList = { 0x56fdf344, 0xfd6d, 0x11d0, { 0x95, 0x8a, 0x00, 0x60, 0x97, 0xc9, 0xa0, 0x90 } };

typedef enum THUMBBUTTONMASK
{
    THB_BITMAP = 0x1,
    THB_ICON = 0x2,
    THB_TOOLTIP = 0x4,
    THB_FLAGS = 0x8
} THUMBBUTTONMASK;

typedef enum THUMBBUTTONFLAGS
{
    THBF_ENABLED = 0,
    THBF_DISABLED = 0x1,
    THBF_DISMISSONCLICK = 0x2,
    THBF_NOBACKGROUND = 0x4,
    THBF_HIDDEN = 0x8,
    THBF_NONINTERACTIVE = 0x10
} THUMBBUTTONFLAGS;

typedef struct THUMBBUTTON
{
    THUMBBUTTONMASK dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[260];
    THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON, *LPTHUMBBUTTON;

struct _IMAGELIST;
typedef struct _IMAGELIST* HIMAGELIST;

typedef struct ITaskbarList3 ITaskbarList3;

typedef struct ITaskbarList3Vtbl
{
    HRESULT(WINAPI* QueryInterface)(struct ITaskbarList3*, const IID* const, void**);
    ULONG(WINAPI* AddRef)(struct ITaskbarList3*);
    ULONG(WINAPI* Release)(struct ITaskbarList3*);
    HRESULT(WINAPI* HrInit)(struct ITaskbarList3*);
    HRESULT(WINAPI* AddTab)(struct ITaskbarList3*, HWND);
    HRESULT(WINAPI* DeleteTab)(struct ITaskbarList3*, HWND);
    HRESULT(WINAPI* ActivateTab)(struct ITaskbarList3*, HWND);
    HRESULT(WINAPI* SetActiveAlt)(struct ITaskbarList3*, HWND);
    HRESULT(WINAPI* MarkFullscreenWindow)(struct ITaskbarList3*, HWND, BOOL);
    HRESULT(WINAPI* SetProgressValue)(struct ITaskbarList3*, HWND, ULONGLONG, ULONGLONG);
    HRESULT(WINAPI* SetProgressState)(struct ITaskbarList3*, HWND, TBPFLAG);
    HRESULT(WINAPI* RegisterTab)(struct ITaskbarList3*, HWND, HWND);
    HRESULT(WINAPI* UnregisterTab)(struct ITaskbarList3*, HWND);
    HRESULT(WINAPI* SetTabOrder)(struct ITaskbarList3*, HWND, HWND);
    HRESULT(WINAPI* SetTabActive)(struct ITaskbarList3*, HWND, HWND, DWORD);
    HRESULT(WINAPI* ThumbBarAddButtons)(struct ITaskbarList3*, HWND, UINT, LPTHUMBBUTTON);
    HRESULT(WINAPI* ThumbBarUpdateButtons)(struct ITaskbarList3*, HWND, UINT, LPTHUMBBUTTON);
    HRESULT(WINAPI* ThumbBarSetImageList)(struct ITaskbarList3*, HWND, HIMAGELIST);
    HRESULT(WINAPI* SetOverlayIcon)(struct ITaskbarList3*, HWND, HICON, LPCWSTR);
    HRESULT(WINAPI* SetThumbnailTooltip)(struct ITaskbarList3*, HWND, LPCWSTR);
    HRESULT(WINAPI* SetThumbnailClip)(struct ITaskbarList3*, HWND, RECT*);
} ITaskbarList3Vtbl;

struct ITaskbarList3
{
    struct ITaskbarList3Vtbl* lpVtbl;
};

// WGL-specific per-context data
//
typedef struct _GRWLcontextWGL
{
    HDC dc;
    HGLRC handle;
    int interval;
} _GRWLcontextWGL;

// WGL-specific global data
//
typedef struct _GRWLlibraryWGL
{
    HINSTANCE instance;
    PFN_wglCreateContext CreateContext;
    PFN_wglDeleteContext DeleteContext;
    PFN_wglGetProcAddress GetProcAddress;
    PFN_wglGetCurrentDC GetCurrentDC;
    PFN_wglGetCurrentContext GetCurrentContext;
    PFN_wglMakeCurrent MakeCurrent;
    PFN_wglShareLists ShareLists;

    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC GetExtensionsStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;
    bool EXT_swap_control;
    bool EXT_colorspace;
    bool ARB_multisample;
    bool ARB_framebuffer_sRGB;
    bool EXT_framebuffer_sRGB;
    bool ARB_pixel_format;
    bool ARB_create_context;
    bool ARB_create_context_profile;
    bool EXT_create_context_es2_profile;
    bool ARB_create_context_robustness;
    bool ARB_create_context_no_error;
    bool ARB_context_flush_control;
} _GRWLlibraryWGL;

// WGL-specific per-usercontext data
//
typedef struct _GRWLusercontextWGL
{
    HDC dc;
    HGLRC handle;
} _GRWLusercontextWGL;

// Win32-specific per-window data
//
typedef struct _GRWLwindowWin32
{
    HWND handle;
    HICON bigIcon;
    HICON smallIcon;

    bool cursorTracked;
    bool frameAction;
    bool iconified;
    bool maximized;
    // Whether to enable framebuffer transparency on DWM
    bool transparent;
    bool scaleToMonitor;
    bool keymenu;
    bool genericBadge;

    // Cached size used to filter out duplicate events
    int width, height;

    // The last received cursor position, regardless of source
    int lastCursorPosX, lastCursorPosY;
    // The last received high surrogate when decoding pairs of UTF-16 messages
    WCHAR highSurrogate;

    ITaskbarList3* taskbarList;
    UINT taskbarListMsgID;
} _GRWLwindowWin32;

// Win32-specific global data
//
typedef struct _GRWLlibraryWin32
{
    HINSTANCE instance;
    HWND helperWindowHandle;
    ATOM helperWindowClass;
    ATOM mainWindowClass;
    HDEVNOTIFY deviceNotificationHandle;
    int acquiredMonitorCount;
    char* clipboardString;
    char* keyboardLayoutName;
    short int keycodes[512];
    short int scancodes[GRWL_KEY_LAST + 1];
    char keynames[GRWL_KEY_LAST + 1][5];
    // Where to place the cursor when re-enabled
    double restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _GRWLwindow* disabledCursorWindow;
    // The window the cursor is captured in
    _GRWLwindow* capturedCursorWindow;
    RAWINPUT* rawInput;
    int rawInputSize;
    UINT mouseTrailSize;

    struct
    {
        HINSTANCE instance;
        PFN_DirectInput8Create Create;
        IDirectInput8W* api;
    } dinput8;

    struct
    {
        HINSTANCE instance;
        PFN_XInputGetCapabilities GetCapabilities;
        PFN_XInputGetState GetState;
    } xinput;

    struct
    {
        HINSTANCE instance;
        PFN_SetProcessDPIAware SetProcessDPIAware_;
        PFN_ChangeWindowMessageFilterEx ChangeWindowMessageFilterEx_;
        PFN_EnableNonClientDpiScaling EnableNonClientDpiScaling_;
        PFN_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_;
        PFN_GetDpiForWindow GetDpiForWindow_;
        PFN_AdjustWindowRectExForDpi AdjustWindowRectExForDpi_;
        PFN_GetSystemMetricsForDpi GetSystemMetricsForDpi_;
        PFN_GetDisplayConfigBufferSizes GetDisplayConfigBufferSizes_;
        PFN_QueryDisplayConfig QueryDisplayConfig_;
        PFN_DisplayConfigGetDeviceInfo DisplayConfigGetDeviceInfo_;
    } user32;

    struct
    {
        HINSTANCE instance;
        PFN_DwmIsCompositionEnabled IsCompositionEnabled;
        PFN_DwmFlush Flush;
        PFN_DwmEnableBlurBehindWindow EnableBlurBehindWindow;
        PFN_DwmGetColorizationColor GetColorizationColor;
        PFN_DwmSetWindowAttribute SetWindowAttribute;
    } dwmapi;

    struct
    {
        HINSTANCE instance;
        PFN_SetProcessDpiAwareness SetProcessDpiAwareness_;
        PFN_GetDpiForMonitor GetDpiForMonitor_;
    } shcore;

    struct
    {
        HINSTANCE instance;
        PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo_;
    } ntdll;

    struct
    {
        HINSTANCE instance;
        PFN_ImmGetCandidateListW ImmGetCandidateListW_;
        PFN_ImmGetCompositionStringW ImmGetCompositionStringW_;
        PFN_ImmGetContext ImmGetContext_;
        PFN_ImmGetConversionStatus ImmGetConversionStatus_;
        PFN_ImmGetDescriptionW ImmGetDescriptionW_;
        PFN_ImmGetOpenStatus ImmGetOpenStatus_;
        PFN_ImmNotifyIME ImmNotifyIME_;
        PFN_ImmReleaseContext ImmReleaseContext_;
        PFN_ImmSetCandidateWindow ImmSetCandidateWindow_;
        PFN_ImmSetOpenStatus ImmSetOpenStatus_;
    } imm32;

    struct
    {
        HINSTANCE instance;
        bool uxThemeAvailable;
        bool darkTitleAvailable;
        ShouldAppsUseDarkModePtr ShouldAppsUseDarkMode;
        GetImmersiveColorFromColorSetExPtr GetImmersiveColorFromColorSetEx;
        GetImmersiveColorTypeFromNamePtr GetImmersiveColorTypeFromName;
        GetImmersiveUserColorSetPreferencePtr GetImmersiveUserColorSetPreference;
    } uxtheme;
} _GRWLlibraryWin32;

// Win32-specific per-monitor data
//
typedef struct _GRWLmonitorWin32
{
    HMONITOR handle;
    // This size matches the static size of DISPLAY_DEVICE.DeviceName
    WCHAR adapterName[32];
    WCHAR displayName[32];
    char publicAdapterName[32];
    char publicDisplayName[32];
    bool modesPruned;
    bool modeChanged;
} _GRWLmonitorWin32;

// Win32-specific per-cursor data
//
typedef struct _GRWLcursorWin32
{
    HCURSOR handle;
} _GRWLcursorWin32;

bool _grwlConnectWin32(int platformID, _GRWLplatform* platform);
bool _grwlInitWin32();
void _grwlTerminateWin32();

WCHAR* _grwlCreateWideStringFromUTF8Win32(const char* source);
char* _grwlCreateUTF8FromWideStringWin32(const WCHAR* source);
BOOL _grwlIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
BOOL _grwlIsWindows10BuildOrGreaterWin32(WORD build);
void _grwlInputErrorWin32(int error, const char* description);
void _grwlUpdateKeyNamesWin32();

void _grwlPollMonitorsWin32();
void _grwlSetVideoModeWin32(_GRWLmonitor* monitor, const GRWLvidmode* desired);
void _grwlRestoreVideoModeWin32(_GRWLmonitor* monitor);
void _grwlGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale);

bool _grwlCreateWindowWin32(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                            const _GRWLfbconfig* fbconfig);
void _grwlDestroyWindowWin32(_GRWLwindow* window);
void _grwlSetWindowTitleWin32(_GRWLwindow* window, const char* title);
void _grwlSetWindowIconWin32(_GRWLwindow* window, int count, const GRWLimage* images);
void _grwlSetWindowProgressIndicatorWin32(_GRWLwindow* window, int progressState, double value);
void _grwlSetWindowBadgeWin32(_GRWLwindow* window, int count);
void _grwlSetWindowBadgeStringWin32(_GRWLwindow* window, const char* string);
void _grwlGetWindowPosWin32(_GRWLwindow* window, int* xpos, int* ypos);
void _grwlSetWindowPosWin32(_GRWLwindow* window, int xpos, int ypos);
void _grwlGetWindowSizeWin32(_GRWLwindow* window, int* width, int* height);
void _grwlSetWindowSizeWin32(_GRWLwindow* window, int width, int height);
void _grwlSetWindowSizeLimitsWin32(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _grwlSetWindowAspectRatioWin32(_GRWLwindow* window, int numer, int denom);
void _grwlGetFramebufferSizeWin32(_GRWLwindow* window, int* width, int* height);
void _grwlGetWindowFrameSizeWin32(_GRWLwindow* window, int* left, int* top, int* right, int* bottom);
void _grwlGetWindowContentScaleWin32(_GRWLwindow* window, float* xscale, float* yscale);
void _grwlIconifyWindowWin32(_GRWLwindow* window);
void _grwlRestoreWindowWin32(_GRWLwindow* window);
void _grwlMaximizeWindowWin32(_GRWLwindow* window);
void _grwlShowWindowWin32(_GRWLwindow* window);
void _grwlHideWindowWin32(_GRWLwindow* window);
void _grwlRequestWindowAttentionWin32(_GRWLwindow* window);
void _grwlFocusWindowWin32(_GRWLwindow* window);
void _grwlSetWindowMonitorWin32(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                                int refreshRate);
bool _grwlWindowFocusedWin32(_GRWLwindow* window);
bool _grwlWindowIconifiedWin32(_GRWLwindow* window);
bool _grwlWindowVisibleWin32(_GRWLwindow* window);
bool _grwlWindowMaximizedWin32(_GRWLwindow* window);
bool _grwlWindowHoveredWin32(_GRWLwindow* window);
bool _grwlFramebufferTransparentWin32(_GRWLwindow* window);
void _grwlSetWindowResizableWin32(_GRWLwindow* window, bool enabled);
void _grwlSetWindowDecoratedWin32(_GRWLwindow* window, bool enabled);
void _grwlSetWindowFloatingWin32(_GRWLwindow* window, bool enabled);
void _grwlSetWindowMousePassthroughWin32(_GRWLwindow* window, bool enabled);
float _grwlGetWindowOpacityWin32(_GRWLwindow* window);
void _grwlSetWindowOpacityWin32(_GRWLwindow* window, float opacity);

void _grwlSetRawMouseMotionWin32(_GRWLwindow* window, bool enabled);
bool _grwlRawMouseMotionSupportedWin32();

void _grwlPollEventsWin32();
void _grwlWaitEventsWin32();
void _grwlWaitEventsTimeoutWin32(double timeout);
void _grwlPostEmptyEventWin32();

void _grwlGetCursorPosWin32(_GRWLwindow* window, double* xpos, double* ypos);
void _grwlSetCursorPosWin32(_GRWLwindow* window, double xpos, double ypos);
void _grwlSetCursorModeWin32(_GRWLwindow* window, int mode);
const char* _grwlGetScancodeNameWin32(int scancode);
int _grwlGetKeyScancodeWin32(int key);
const char* _grwlGetKeyboardLayoutNameWin32();
bool _grwlCreateCursorWin32(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot);
bool _grwlCreateStandardCursorWin32(_GRWLcursor* cursor, int shape);
void _grwlDestroyCursorWin32(_GRWLcursor* cursor);
void _grwlSetCursorWin32(_GRWLwindow* window, _GRWLcursor* cursor);
void _grwlSetClipboardStringWin32(const char* string);
const char* _grwlGetClipboardStringWin32();

void _grwlUpdatePreeditCursorRectangleWin32(_GRWLwindow* window);
void _grwlResetPreeditTextWin32(_GRWLwindow* window);
void _grwlSetIMEStatusWin32(_GRWLwindow* window, int active);
int _grwlGetIMEStatusWin32(_GRWLwindow* window);

EGLenum _grwlGetEGLPlatformWin32(EGLint** attribs);
EGLNativeDisplayType _grwlGetEGLNativeDisplayWin32();
EGLNativeWindowType _grwlGetEGLNativeWindowWin32(_GRWLwindow* window);

void _grwlGetRequiredInstanceExtensionsWin32(char** extensions);
bool _grwlGetPhysicalDevicePresentationSupportWin32(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _grwlCreateWindowSurfaceWin32(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR* surface);

void _grwlFreeMonitorWin32(_GRWLmonitor* monitor);
void _grwlGetMonitorPosWin32(_GRWLmonitor* monitor, int* xpos, int* ypos);
void _grwlGetMonitorContentScaleWin32(_GRWLmonitor* monitor, float* xscale, float* yscale);
void _grwlGetMonitorWorkareaWin32(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GRWLvidmode* _grwlGetVideoModesWin32(_GRWLmonitor* monitor, int* count);
void _grwlGetVideoModeWin32(_GRWLmonitor* monitor, GRWLvidmode* mode);
bool _grwlGetGammaRampWin32(_GRWLmonitor* monitor, GRWLgammaramp* ramp);
void _grwlSetGammaRampWin32(_GRWLmonitor* monitor, const GRWLgammaramp* ramp);

bool _grwlInitJoysticksWin32();
void _grwlTerminateJoysticksWin32();
bool _grwlPollJoystickWin32(_GRWLjoystick* js, int mode);
const char* _grwlGetMappingNameWin32();
void _grwlUpdateGamepadGUIDWin32(char* guid);

bool _grwlInitWGL();
void _grwlTerminateWGL();
bool _grwlCreateContextWGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig);

_GRWLusercontext* _grwlCreateUserContextWin32(_GRWLwindow* window);
_GRWLusercontext* _grwlCreateUserContextWGL(_GRWLwindow* window);
