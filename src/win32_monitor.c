//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(_GRWL_WIN32)

    #include <assert.h>
    #include <stdlib.h>
    #include <string.h>
    #include <limits.h>
    #include <wchar.h>

    #if WINVER < 0x0601 // Windows 7
const UINT32 QDC_ONLY_ACTIVE_PATHS = 0x00000002;
    #endif

DISPLAYCONFIG_PATH_INFO* getDisplayPaths(UINT32* out_pathsCount)
{
    DISPLAYCONFIG_PATH_INFO* paths;
    DISPLAYCONFIG_MODE_INFO* modes;
    UINT32 modeCount;
    UINT32 pathsCount;
    LONG rc;

    paths = NULL;
    modes = NULL;
    modeCount = 0;
    pathsCount = 0;
    rc = 0;

    do
    {
        rc = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathsCount, &modeCount);
        if (rc != ERROR_SUCCESS)
        {
            break;
        }

        assert(paths == NULL);
        assert(modes == NULL);
        paths = (DISPLAYCONFIG_PATH_INFO*)malloc(sizeof(DISPLAYCONFIG_PATH_INFO) * pathsCount);
        modes = (DISPLAYCONFIG_MODE_INFO*)malloc(sizeof(DISPLAYCONFIG_MODE_INFO) * modeCount);

        if (paths == NULL)
        {
            free(modes);
            modes = NULL;
            break;
        }

        if (modes == NULL)
        {
            free(paths);
            paths = NULL;
            break;
        }

        rc = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathsCount, paths, &modeCount, modes, 0);
        if (rc == ERROR_SUCCESS)
        {
            free(modes); // We won't use it.
            modes = NULL;
        }
        else
        {
            free(paths);
            paths = NULL;
            free(modes);
            modes = NULL;
        }

    } while (rc == ERROR_INSUFFICIENT_BUFFER);

    assert(modes == NULL);

    *out_pathsCount = pathsCount;
    return paths;
}

char* getMonitorNameFromPath(const WCHAR* deviceName, const DISPLAYCONFIG_PATH_INFO* paths, const UINT32 pathCount)
{
    UINT32 i;
    LONG rc;
    char* monitorName;

    i = 0;
    rc = 0;
    monitorName = NULL;

    for (i = 0; i < pathCount; i++)
    {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName;
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName;

        ZeroMemory(&sourceName, sizeof(sourceName));
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        sourceName.header.adapterId = paths[i].sourceInfo.adapterId;
        sourceName.header.id = paths[i].sourceInfo.id;

        rc = DisplayConfigGetDeviceInfo(&sourceName.header);
        if (rc != ERROR_SUCCESS)
        {
            break;
        }

        if (wcscmp(deviceName, sourceName.viewGdiDeviceName) != 0)
        {
            continue;
        }

        ZeroMemory(&targetName, sizeof(targetName));
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;

        rc = DisplayConfigGetDeviceInfo(&targetName.header);
        if (rc != ERROR_SUCCESS)
        {
            break;
        }

        monitorName = _grwlCreateUTF8FromWideStringWin32(targetName.monitorFriendlyDeviceName);
        if (monitorName && (*monitorName == '\0')) // If we got an empty string, treat it as failure so we'll fallback
                                                   // to getting the generic name.
        {
            free(monitorName);
            monitorName = NULL;
        }
        break;
    }

    return monitorName;
}

// Returns NULL if an error happens or the OS doesn't provide the needed feature (anything below Win7)
// If the pointer is valid, the caller takes ownership of the underlying memory and has to free it when done.
//
char* tryGetAccurateMonitorName(const WCHAR* deviceName)
{
    char* monitorName;
    DISPLAYCONFIG_PATH_INFO* paths;
    UINT32 pathCount;

    monitorName = NULL;
    paths = NULL;
    pathCount = 0;

    if (QueryDisplayConfig == NULL)
    {
        return NULL;
    }

    // If QueryDisplayConfig is present then GetDisplayConfigBufferSizes and DisplayConfigGetDeviceInfo also should be
    // present.
    assert(GetDisplayConfigBufferSizes != NULL);
    assert(DisplayConfigGetDeviceInfo != NULL);

    paths = getDisplayPaths(&pathCount);
    if (paths == NULL)
    {
        return NULL;
    }

    monitorName = getMonitorNameFromPath(deviceName, paths, pathCount);

    free(paths);
    return monitorName;
}

// Callback for EnumDisplayMonitors in createMonitor
//
static BOOL CALLBACK monitorCallback(HMONITOR handle, HDC dc, RECT* rect, LPARAM data)
{
    MONITORINFOEXW mi;
    char* accurateMonitorName = NULL;
    _GRWLmonitor* monitor = (_GRWLmonitor*)data;

    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);

    if (GetMonitorInfoW(handle, (MONITORINFO*)&mi) == 0)
    {
        return TRUE;
    }

    if (wcscmp(mi.szDevice, monitor->win32.adapterName) != 0)
    {
        return TRUE;
    }

    monitor->win32.handle = handle;

    // If the monitor driver is installed, we will already have an accurate name for the monitor.
    if (strcmp(monitor->name, "Generic PnP Monitor") != 0)
    {
        return TRUE;
    }

    accurateMonitorName = tryGetAccurateMonitorName(mi.szDevice);
    if (accurateMonitorName != NULL)
    {
        strncpy(monitor->name, accurateMonitorName, sizeof(monitor->name) - 1);
        free(accurateMonitorName);
        accurateMonitorName = NULL;
    }

    return TRUE;
}

// Create monitor from an adapter and (optionally) a display
//
static _GRWLmonitor* createMonitor(DISPLAY_DEVICEW* adapter, DISPLAY_DEVICEW* display)
{
    _GRWLmonitor* monitor;
    int widthMM, heightMM;
    char* name;
    HDC dc;
    DEVMODEW dm;
    RECT rect;

    if (display)
    {
        name = _grwlCreateUTF8FromWideStringWin32(display->DeviceString);
    }
    else
    {
        name = _grwlCreateUTF8FromWideStringWin32(adapter->DeviceString);
    }
    if (!name)
    {
        return NULL;
    }

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsW(adapter->DeviceName, ENUM_CURRENT_SETTINGS, &dm);

    dc = CreateDCW(L"DISPLAY", adapter->DeviceName, NULL, NULL);

    if (IsWindows8Point1OrGreater())
    {
        widthMM = GetDeviceCaps(dc, HORZSIZE);
        heightMM = GetDeviceCaps(dc, VERTSIZE);
    }
    else
    {
        widthMM = (int)(dm.dmPelsWidth * 25.4f / GetDeviceCaps(dc, LOGPIXELSX));
        heightMM = (int)(dm.dmPelsHeight * 25.4f / GetDeviceCaps(dc, LOGPIXELSY));
    }

    DeleteDC(dc);

    monitor = _grwlAllocMonitor(name, widthMM, heightMM);
    _grwl_free(name);

    if (adapter->StateFlags & DISPLAY_DEVICE_MODESPRUNED)
    {
        monitor->win32.modesPruned = GRWL_TRUE;
    }

    wcscpy(monitor->win32.adapterName, adapter->DeviceName);
    WideCharToMultiByte(CP_UTF8, 0, adapter->DeviceName, -1, monitor->win32.publicAdapterName,
                        sizeof(monitor->win32.publicAdapterName), NULL, NULL);

    if (display)
    {
        wcscpy(monitor->win32.displayName, display->DeviceName);
        WideCharToMultiByte(CP_UTF8, 0, display->DeviceName, -1, monitor->win32.publicDisplayName,
                            sizeof(monitor->win32.publicDisplayName), NULL, NULL);
    }

    rect.left = dm.dmPosition.x;
    rect.top = dm.dmPosition.y;
    rect.right = dm.dmPosition.x + dm.dmPelsWidth;
    rect.bottom = dm.dmPosition.y + dm.dmPelsHeight;

    EnumDisplayMonitors(NULL, &rect, monitorCallback, (LPARAM)monitor);
    return monitor;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _grwlPollMonitorsWin32(void)
{
    int i, disconnectedCount;
    _GRWLmonitor** disconnected = NULL;
    DWORD adapterIndex, displayIndex;
    DISPLAY_DEVICEW adapter, display;
    _GRWLmonitor* monitor;

    disconnectedCount = _grwl.monitorCount;
    if (disconnectedCount)
    {
        disconnected = _grwl_calloc(_grwl.monitorCount, sizeof(_GRWLmonitor*));
        memcpy(disconnected, _grwl.monitors, _grwl.monitorCount * sizeof(_GRWLmonitor*));
    }

    for (adapterIndex = 0;; adapterIndex++)
    {
        int type = _GRWL_INSERT_LAST;

        ZeroMemory(&adapter, sizeof(adapter));
        adapter.cb = sizeof(adapter);

        if (!EnumDisplayDevicesW(NULL, adapterIndex, &adapter, 0))
        {
            break;
        }

        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
        {
            continue;
        }

        if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            type = _GRWL_INSERT_FIRST;
        }

        for (displayIndex = 0;; displayIndex++)
        {
            ZeroMemory(&display, sizeof(display));
            display.cb = sizeof(display);

            if (!EnumDisplayDevicesW(adapter.DeviceName, displayIndex, &display, 0))
            {
                break;
            }

            if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
            {
                continue;
            }

            for (i = 0; i < disconnectedCount; i++)
            {
                if (disconnected[i] && wcscmp(disconnected[i]->win32.displayName, display.DeviceName) == 0)
                {
                    disconnected[i] = NULL;
                    // handle may have changed, update
                    EnumDisplayMonitors(NULL, NULL, monitorCallback, (LPARAM)_grwl.monitors[i]);
                    break;
                }
            }

            if (i < disconnectedCount)
            {
                continue;
            }

            monitor = createMonitor(&adapter, &display);
            if (!monitor)
            {
                _grwl_free(disconnected);
                return;
            }

            _grwlInputMonitor(monitor, GRWL_CONNECTED, type);

            type = _GRWL_INSERT_LAST;
        }

        // HACK: If an active adapter does not have any display devices
        //       (as sometimes happens), add it directly as a monitor
        if (displayIndex == 0)
        {
            for (i = 0; i < disconnectedCount; i++)
            {
                if (disconnected[i] && wcscmp(disconnected[i]->win32.adapterName, adapter.DeviceName) == 0)
                {
                    disconnected[i] = NULL;
                    break;
                }
            }

            if (i < disconnectedCount)
            {
                continue;
            }

            monitor = createMonitor(&adapter, NULL);
            if (!monitor)
            {
                _grwl_free(disconnected);
                return;
            }

            _grwlInputMonitor(monitor, GRWL_CONNECTED, type);
        }
    }

    for (i = 0; i < disconnectedCount; i++)
    {
        if (disconnected[i])
        {
            _grwlInputMonitor(disconnected[i], GRWL_DISCONNECTED, 0);
        }
    }

    _grwl_free(disconnected);
}

// Change the current video mode
//
void _grwlSetVideoModeWin32(_GRWLmonitor* monitor, const GRWLvidmode* desired)
{
    GRWLvidmode current;
    const GRWLvidmode* best;
    DEVMODEW dm;
    LONG result;

    best = _grwlChooseVideoMode(monitor, desired);
    _grwlGetVideoModeWin32(monitor, &current);
    if (_grwlCompareVideoModes(&current, best) == 0)
    {
        return;
    }

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
    dm.dmPelsWidth = best->width;
    dm.dmPelsHeight = best->height;
    dm.dmBitsPerPel = best->redBits + best->greenBits + best->blueBits;
    dm.dmDisplayFrequency = best->refreshRate;

    if (dm.dmBitsPerPel < 15 || dm.dmBitsPerPel >= 24)
    {
        dm.dmBitsPerPel = 32;
    }

    result = ChangeDisplaySettingsExW(monitor->win32.adapterName, &dm, NULL, CDS_FULLSCREEN, NULL);
    if (result == DISP_CHANGE_SUCCESSFUL)
    {
        monitor->win32.modeChanged = GRWL_TRUE;
    }
    else
    {
        const char* description = "Unknown error";

        if (result == DISP_CHANGE_BADDUALVIEW)
        {
            description = "The system uses DualView";
        }
        else if (result == DISP_CHANGE_BADFLAGS)
        {
            description = "Invalid flags";
        }
        else if (result == DISP_CHANGE_BADMODE)
        {
            description = "Graphics mode not supported";
        }
        else if (result == DISP_CHANGE_BADPARAM)
        {
            description = "Invalid parameter";
        }
        else if (result == DISP_CHANGE_FAILED)
        {
            description = "Graphics mode failed";
        }
        else if (result == DISP_CHANGE_NOTUPDATED)
        {
            description = "Failed to write to registry";
        }
        else if (result == DISP_CHANGE_RESTART)
        {
            description = "Computer restart required";
        }

        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to set video mode: %s", description);
    }
}

// Restore the previously saved (original) video mode
//
void _grwlRestoreVideoModeWin32(_GRWLmonitor* monitor)
{
    if (monitor->win32.modeChanged)
    {
        ChangeDisplaySettingsExW(monitor->win32.adapterName, NULL, NULL, CDS_FULLSCREEN, NULL);
        monitor->win32.modeChanged = GRWL_FALSE;
    }
}

void _grwlGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale)
{
    UINT xdpi, ydpi;

    if (xscale)
    {
        *xscale = 0.f;
    }
    if (yscale)
    {
        *yscale = 0.f;
    }

    if (IsWindows8Point1OrGreater())
    {
        if (GetDpiForMonitor(handle, MDT_EFFECTIVE_DPI, &xdpi, &ydpi) != S_OK)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to query monitor DPI");
            return;
        }
    }
    else
    {
        const HDC dc = GetDC(NULL);
        xdpi = GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(NULL, dc);
    }

    if (xscale)
    {
        *xscale = xdpi / (float)USER_DEFAULT_SCREEN_DPI;
    }
    if (yscale)
    {
        *yscale = ydpi / (float)USER_DEFAULT_SCREEN_DPI;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlFreeMonitorWin32(_GRWLmonitor* monitor)
{
}

void _grwlGetMonitorPosWin32(_GRWLmonitor* monitor, int* xpos, int* ypos)
{
    DEVMODEW dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

    EnumDisplaySettingsExW(monitor->win32.adapterName, ENUM_CURRENT_SETTINGS, &dm, EDS_ROTATEDMODE);

    if (xpos)
    {
        *xpos = dm.dmPosition.x;
    }
    if (ypos)
    {
        *ypos = dm.dmPosition.y;
    }
}

void _grwlGetMonitorContentScaleWin32(_GRWLmonitor* monitor, float* xscale, float* yscale)
{
    _grwlGetHMONITORContentScaleWin32(monitor->win32.handle, xscale, yscale);
}

void _grwlGetMonitorWorkareaWin32(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height)
{
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoW(monitor->win32.handle, &mi);

    if (xpos)
    {
        *xpos = mi.rcWork.left;
    }
    if (ypos)
    {
        *ypos = mi.rcWork.top;
    }
    if (width)
    {
        *width = mi.rcWork.right - mi.rcWork.left;
    }
    if (height)
    {
        *height = mi.rcWork.bottom - mi.rcWork.top;
    }
}

GRWLvidmode* _grwlGetVideoModesWin32(_GRWLmonitor* monitor, int* count)
{
    int modeIndex = 0, size = 1;
    GRWLvidmode* result = NULL;

    *count = 1;
    // HACK: Always return the current video mode
    result = _grwl_calloc(1, sizeof(GRWLvidmode));
    _grwlGetVideoModeWin32(monitor, result);

    for (;;)
    {
        int i;
        GRWLvidmode mode;
        DEVMODEW dm;

        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);

        if (!EnumDisplaySettingsW(monitor->win32.adapterName, modeIndex, &dm))
        {
            break;
        }

        modeIndex++;

        // Skip modes with less than 15 BPP
        if (dm.dmBitsPerPel < 15)
        {
            continue;
        }

        mode.width = dm.dmPelsWidth;
        mode.height = dm.dmPelsHeight;
        mode.refreshRate = dm.dmDisplayFrequency;
        _grwlSplitBPP(dm.dmBitsPerPel, &mode.redBits, &mode.greenBits, &mode.blueBits);

        for (i = 0; i < *count; i++)
        {
            if (_grwlCompareVideoModes(result + i, &mode) == 0)
            {
                break;
            }
        }

        // Skip duplicate modes
        if (i < *count)
        {
            continue;
        }

        if (monitor->win32.modesPruned)
        {
            // Skip modes not supported by the connected displays
            if (ChangeDisplaySettingsExW(monitor->win32.adapterName, &dm, NULL, CDS_TEST, NULL) !=
                DISP_CHANGE_SUCCESSFUL)
            {
                continue;
            }
        }

        if (*count == size)
        {
            size += 128;
            result = (GRWLvidmode*)_grwl_realloc(result, size * sizeof(GRWLvidmode));
        }

        (*count)++;
        result[*count - 1] = mode;
    }

    return result;
}

void _grwlGetVideoModeWin32(_GRWLmonitor* monitor, GRWLvidmode* mode)
{
    DEVMODEW dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

    EnumDisplaySettingsW(monitor->win32.adapterName, ENUM_CURRENT_SETTINGS, &dm);

    mode->width = dm.dmPelsWidth;
    mode->height = dm.dmPelsHeight;
    mode->refreshRate = dm.dmDisplayFrequency;
    _grwlSplitBPP(dm.dmBitsPerPel, &mode->redBits, &mode->greenBits, &mode->blueBits);
}

GRWLbool _grwlGetGammaRampWin32(_GRWLmonitor* monitor, GRWLgammaramp* ramp)
{
    HDC dc;
    WORD values[3][256];

    dc = CreateDCW(L"DISPLAY", monitor->win32.adapterName, NULL, NULL);
    GetDeviceGammaRamp(dc, values);
    DeleteDC(dc);

    _grwlAllocGammaArrays(ramp, 256);

    memcpy(ramp->red, values[0], sizeof(values[0]));
    memcpy(ramp->green, values[1], sizeof(values[1]));
    memcpy(ramp->blue, values[2], sizeof(values[2]));

    return GRWL_TRUE;
}

void _grwlSetGammaRampWin32(_GRWLmonitor* monitor, const GRWLgammaramp* ramp)
{
    HDC dc;
    WORD values[3][256];

    if (ramp->size != 256)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Gamma ramp size must be 256");
        return;
    }

    memcpy(values[0], ramp->red, sizeof(values[0]));
    memcpy(values[1], ramp->green, sizeof(values[1]));
    memcpy(values[2], ramp->blue, sizeof(values[2]));

    dc = CreateDCW(L"DISPLAY", monitor->win32.adapterName, NULL, NULL);
    SetDeviceGammaRamp(dc, values);
    DeleteDC(dc);
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI const char* grwlGetWin32Adapter(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->win32.publicAdapterName;
}

GRWLAPI const char* grwlGetWin32Monitor(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->win32.publicDisplayName;
}

#endif // _GRWL_WIN32
