//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(_GRWL_WIN32)

    #include <limits.h>
    #include <stdlib.h>
    #include <string.h>
    #include <windowsx.h>
    #include <shellapi.h>
    #include <wchar.h>
    #include <stdio.h>

// Converts utf16 units to Unicode code points (UTF32).
// Returns GRWL_TRUE when the converting completes and the result is assigned to
// the argument `codepoint`.
// Returns GRWL_FALSE when the converting is not yet completed (for
// Surrogate-pair processing) and the unit is assigned to the argument
// `highsurrogate`. It will be used in the next unit's processing.
//
static GRWLbool convertToUTF32FromUTF16(WCHAR utf16_unit, WCHAR* highsurrogate, uint32_t* codepoint)
{
    *codepoint = 0;

    if (utf16_unit >= 0xd800 && utf16_unit <= 0xdbff)
    {
        *highsurrogate = (WCHAR)utf16_unit;
        return GRWL_FALSE;
    }

    if (utf16_unit >= 0xdc00 && utf16_unit <= 0xdfff)
    {
        if (*highsurrogate)
        {
            *codepoint += (*highsurrogate - 0xd800) << 10;
            *codepoint += (WCHAR)utf16_unit - 0xdc00;
            *codepoint += 0x10000;
        }
    }
    else
    {
        *codepoint = (WCHAR)utf16_unit;
    }

    *highsurrogate = 0;
    return GRWL_TRUE;
}

    // Ref: https://docs.microsoft.com/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute
    #ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
        #define DWMWA_USE_IMMERSIVE_DARK_MODE 20
    #endif

// Update window theme (light/dark)
//
static void updateTheme(HWND hWnd)
{
    if (_grwl.win32.uxtheme.uxThemeAvailable && _grwl.win32.uxtheme.darkTitleAvailable)
    {
        BOOL value = _grwl.win32.uxtheme.ShouldAppsUseDarkMode() & 0x1;
        DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
    }
}

// Returns the window style for the specified window
//
static DWORD getWindowStyle(const _GRWLwindow* window)
{
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (window->monitor)
    {
        style |= WS_POPUP;
    }
    else
    {
        style |= WS_SYSMENU | WS_MINIMIZEBOX;

        if (window->decorated)
        {
            style |= WS_CAPTION;

            if (window->resizable)
            {
                style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
            }
        }
        else
        {
            style |= WS_POPUP;
        }
    }

    return style;
}

// Returns the extended window style for the specified window
//
static DWORD getWindowExStyle(const _GRWLwindow* window)
{
    DWORD style = WS_EX_APPWINDOW;

    if (window->monitor || window->floating)
    {
        style |= WS_EX_TOPMOST;
    }

    return style;
}

// Returns the image whose area most closely matches the desired one
//
static const GRWLimage* chooseImage(int count, const GRWLimage* images, int width, int height)
{
    int i, leastDiff = INT_MAX;
    const GRWLimage* closest = NULL;

    for (i = 0; i < count; i++)
    {
        const int currDiff = abs(images[i].width * images[i].height - width * height);
        if (currDiff < leastDiff)
        {
            closest = images + i;
            leastDiff = currDiff;
        }
    }

    return closest;
}

// Creates an RGBA icon or cursor
//
static HICON createIcon(const GRWLimage* image, int xhot, int yhot, GRWLbool icon)
{
    int i;
    HDC dc;
    HICON handle;
    HBITMAP color, mask;
    BITMAPV5HEADER bi;
    ICONINFO ii;
    unsigned char* target = NULL;
    unsigned char* source = image->pixels;

    ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = image->width;
    bi.bV5Height = -image->height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00ff0000;
    bi.bV5GreenMask = 0x0000ff00;
    bi.bV5BlueMask = 0x000000ff;
    bi.bV5AlphaMask = 0xff000000;

    dc = GetDC(NULL);
    color = CreateDIBSection(dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&target, NULL, (DWORD)0);
    ReleaseDC(NULL, dc);

    if (!color)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create RGBA bitmap");
        return NULL;
    }

    mask = CreateBitmap(image->width, image->height, 1, 1, NULL);
    if (!mask)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create mask bitmap");
        DeleteObject(color);
        return NULL;
    }

    for (i = 0; i < image->width * image->height; i++)
    {
        target[0] = source[2];
        target[1] = source[1];
        target[2] = source[0];
        target[3] = source[3];
        target += 4;
        source += 4;
    }

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon = icon;
    ii.xHotspot = xhot;
    ii.yHotspot = yhot;
    ii.hbmMask = mask;
    ii.hbmColor = color;

    handle = CreateIconIndirect(&ii);

    DeleteObject(color);
    DeleteObject(mask);

    if (!handle)
    {
        if (icon)
        {
            _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create icon");
        }
        else
        {
            _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create cursor");
        }
    }

    return handle;
}

// Enforce the content area aspect ratio based on which edge is being dragged
//
static void applyAspectRatio(_GRWLwindow* window, int edge, RECT* area)
{
    RECT frame = { 0 };
    const float ratio = (float)window->numer / (float)window->denom;
    const DWORD style = getWindowStyle(window);
    const DWORD exStyle = getWindowExStyle(window);

    if (_grwlIsWindows10Version1607OrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&frame, style, FALSE, exStyle, GetDpiForWindow(window->win32.handle));
    }
    else
    {
        AdjustWindowRectEx(&frame, style, FALSE, exStyle);
    }

    if (edge == WMSZ_LEFT || edge == WMSZ_BOTTOMLEFT || edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT)
    {
        area->bottom = area->top + (frame.bottom - frame.top) +
                       (int)(((area->right - area->left) - (frame.right - frame.left)) / ratio);
    }
    else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
    {
        area->top = area->bottom - (frame.bottom - frame.top) -
                    (int)(((area->right - area->left) - (frame.right - frame.left)) / ratio);
    }
    else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
    {
        area->right = area->left + (frame.right - frame.left) +
                      (int)(((area->bottom - area->top) - (frame.bottom - frame.top)) * ratio);
    }
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_GRWLwindow* window)
{
    if (window->cursorMode == GRWL_CURSOR_NORMAL || window->cursorMode == GRWL_CURSOR_CAPTURED)
    {
        if (window->cursor)
        {
            SetCursor(window->cursor->win32.handle);
        }
        else
        {
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
        }
    }
    else
    {
        SetCursor(NULL);
    }
}

// Sets the cursor clip rect to the window content area
//
static void captureCursor(_GRWLwindow* window)
{
    RECT clipRect;
    GetClientRect(window->win32.handle, &clipRect);
    ClientToScreen(window->win32.handle, (POINT*)&clipRect.left);
    ClientToScreen(window->win32.handle, (POINT*)&clipRect.right);
    ClipCursor(&clipRect);
    _grwl.win32.capturedCursorWindow = window;
}

// Disabled clip cursor
//
static void releaseCursor(void)
{
    ClipCursor(NULL);
    _grwl.win32.capturedCursorWindow = NULL;
}

// Enables WM_INPUT messages for the mouse for the specified window
//
static void enableRawMouseMotion(_GRWLwindow* window)
{
    const RAWINPUTDEVICE rid = { 0x01, 0x02, 0, window->win32.handle };

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to register raw input device");
    }
}

// Disables WM_INPUT messages for the mouse
//
static void disableRawMouseMotion(_GRWLwindow* window)
{
    const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, NULL };

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to remove raw input device");
    }
}

// Apply disabled cursor mode to a focused window
//
static void disableCursor(_GRWLwindow* window)
{
    _grwl.win32.disabledCursorWindow = window;
    _grwlGetCursorPosWin32(window, &_grwl.win32.restoreCursorPosX, &_grwl.win32.restoreCursorPosY);
    updateCursorImage(window);
    _grwlCenterCursorInContentArea(window);
    captureCursor(window);

    if (window->rawMouseMotion)
    {
        enableRawMouseMotion(window);
    }
}

// Exit disabled cursor mode for the specified window
//
static void enableCursor(_GRWLwindow* window)
{
    if (window->rawMouseMotion)
    {
        disableRawMouseMotion(window);
    }

    _grwl.win32.disabledCursorWindow = NULL;
    releaseCursor();
    _grwlSetCursorPosWin32(window, _grwl.win32.restoreCursorPosX, _grwl.win32.restoreCursorPosY);
    updateCursorImage(window);
}

// Returns whether the cursor is in the content area of the specified window
//
static GRWLbool cursorInContentArea(_GRWLwindow* window)
{
    RECT area;
    POINT pos;

    if (!GetCursorPos(&pos))
    {
        return GRWL_FALSE;
    }

    if (WindowFromPoint(pos) != window->win32.handle)
    {
        return GRWL_FALSE;
    }

    GetClientRect(window->win32.handle, &area);
    ClientToScreen(window->win32.handle, (POINT*)&area.left);
    ClientToScreen(window->win32.handle, (POINT*)&area.right);

    return PtInRect(&area, pos);
}

// Update native window styles to match attributes
//
static void updateWindowStyles(const _GRWLwindow* window)
{
    RECT rect;
    DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
    style |= getWindowStyle(window);

    GetClientRect(window->win32.handle, &rect);

    if (_grwlIsWindows10Version1607OrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&rect, style, FALSE, getWindowExStyle(window), GetDpiForWindow(window->win32.handle));
    }
    else
    {
        AdjustWindowRectEx(&rect, style, FALSE, getWindowExStyle(window));
    }

    ClientToScreen(window->win32.handle, (POINT*)&rect.left);
    ClientToScreen(window->win32.handle, (POINT*)&rect.right);
    SetWindowLongW(window->win32.handle, GWL_STYLE, style);
    SetWindowPos(window->win32.handle, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER);
}

// Update window framebuffer transparency
//
static void updateFramebufferTransparency(const _GRWLwindow* window)
{
    BOOL composition, opaque;
    DWORD color;

    if (!IsWindowsVistaOrGreater())
    {
        return;
    }

    if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
    {
        return;
    }

    if (IsWindows8OrGreater() || (SUCCEEDED(DwmGetColorizationColor(&color, &opaque)) && !opaque))
    {
        HRGN region = CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;

        DwmEnableBlurBehindWindow(window->win32.handle, &bb);
        DeleteObject(region);
    }
    else
    {
        // HACK: Disable framebuffer transparency on Windows 7 when the
        //       colorization color is opaque, because otherwise the window
        //       contents is blended additively with the previous frame instead
        //       of replacing it
        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE;
        DwmEnableBlurBehindWindow(window->win32.handle, &bb);
    }
}

// Retrieves and translates modifier keys
//
static int getKeyMods(void)
{
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
        mods |= GRWL_MOD_SHIFT;
    }
    if (GetKeyState(VK_CONTROL) & 0x8000)
    {
        mods |= GRWL_MOD_CONTROL;
    }
    if (GetKeyState(VK_MENU) & 0x8000)
    {
        mods |= GRWL_MOD_ALT;
    }
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
    {
        mods |= GRWL_MOD_SUPER;
    }
    if (GetKeyState(VK_CAPITAL) & 1)
    {
        mods |= GRWL_MOD_CAPS_LOCK;
    }
    if (GetKeyState(VK_NUMLOCK) & 1)
    {
        mods |= GRWL_MOD_NUM_LOCK;
    }

    return mods;
}

static void fitToMonitor(_GRWLwindow* window)
{
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoW(window->monitor->win32.handle, &mi);
    // On Win32, the window behaves in a special way when the window covers the screen exactly.
    // In this case, the window is a strong exclusive exceeding HWND_TOPMOST and it doen't
    // display OS notifications such as the sound volumn changed.
    // This behavior can be deactivated by shifting the size slightly.
    // This is especially necessary for the IME(Input Method Editor/Engine) to display properly.
    // This can also effect transparency to fullscreen.
    SetWindowPos(window->win32.handle, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
                 mi.rcMonitor.right - mi.rcMonitor.left,
                 _grwl.hints.window.softFullscreen ? mi.rcMonitor.bottom - mi.rcMonitor.top + 1
                                                   : mi.rcMonitor.bottom - mi.rcMonitor.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_GRWLwindow* window)
{
    if (!_grwl.win32.acquiredMonitorCount)
    {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);

        // HACK: When mouse trails are enabled the cursor becomes invisible when
        //       the OpenGL ICD switches to page flipping
        SystemParametersInfoW(SPI_GETMOUSETRAILS, 0, &_grwl.win32.mouseTrailSize, 0);
        SystemParametersInfoW(SPI_SETMOUSETRAILS, 0, 0, 0);
    }

    if (!window->monitor->window)
    {
        _grwl.win32.acquiredMonitorCount++;
    }

    _grwlSetVideoModeWin32(window->monitor, &window->videoMode);
    _grwlInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_GRWLwindow* window)
{
    if (window->monitor->window != window)
    {
        return;
    }

    _grwl.win32.acquiredMonitorCount--;
    if (!_grwl.win32.acquiredMonitorCount)
    {
        SetThreadExecutionState(ES_CONTINUOUS);

        // HACK: Restore mouse trail length saved in acquireMonitor
        SystemParametersInfoW(SPI_SETMOUSETRAILS, _grwl.win32.mouseTrailSize, 0, 0);
    }

    _grwlInputMonitorWindow(window->monitor, NULL);
    _grwlRestoreVideoModeWin32(window->monitor);
}

// Manually maximize the window, for when SW_MAXIMIZE cannot be used
//
static void maximizeWindowManually(_GRWLwindow* window)
{
    RECT rect;
    DWORD style;
    MONITORINFO mi = { sizeof(mi) };

    GetMonitorInfoW(MonitorFromWindow(window->win32.handle, MONITOR_DEFAULTTONEAREST), &mi);

    rect = mi.rcWork;

    if (window->maxwidth != GRWL_DONT_CARE && window->maxheight != GRWL_DONT_CARE)
    {
        rect.right = _grwl_min(rect.right, rect.left + window->maxwidth);
        rect.bottom = _grwl_min(rect.bottom, rect.top + window->maxheight);
    }

    style = GetWindowLongW(window->win32.handle, GWL_STYLE);
    style |= WS_MAXIMIZE;
    SetWindowLongW(window->win32.handle, GWL_STYLE, style);

    if (window->decorated)
    {
        const DWORD exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);

        if (_grwlIsWindows10Version1607OrGreaterWin32())
        {
            const UINT dpi = GetDpiForWindow(window->win32.handle);
            AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
            OffsetRect(&rect, 0, GetSystemMetricsForDpi(SM_CYCAPTION, dpi));
        }
        else
        {
            AdjustWindowRectEx(&rect, style, FALSE, exStyle);
            OffsetRect(&rect, 0, GetSystemMetrics(SM_CYCAPTION));
        }

        rect.bottom = _grwl_min(rect.bottom, mi.rcWork.bottom);
    }

    SetWindowPos(window->win32.handle, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

// Store candidate text from the buffer data
//
static void setCandidate(_GRWLpreeditcandidate* candidate, LPWSTR buffer)
{
    size_t bufferCount = wcslen(buffer);
    int textBufferCount = candidate->textBufferCount;
    uint32_t codepoint;
    WCHAR highSurrogate = 0;
    int convertedLength = 0;
    int i;

    while ((size_t)textBufferCount < bufferCount + 1)
    {
        textBufferCount = (textBufferCount == 0) ? 1 : textBufferCount * 2;
    }
    if (textBufferCount != candidate->textBufferCount)
    {
        unsigned int* text = _grwl_realloc(candidate->text, sizeof(unsigned int) * textBufferCount);
        if (text == NULL)
        {
            return;
        }
        candidate->text = text;
        candidate->textBufferCount = textBufferCount;
    }

    for (i = 0; (size_t)i < bufferCount; ++i)
    {
        if (convertToUTF32FromUTF16(buffer[i], &highSurrogate, &codepoint))
        {
            candidate->text[convertedLength++] = codepoint;
        }
    }

    candidate->textCount = convertedLength;
}

// Get preedit candidates of Imm32 and pass them to candidate-callback
//
static void getImmCandidates(_GRWLwindow* window)
{
    _GRWLpreedit* preedit = &window->preedit;
    HIMC hIMC = ImmGetContext(window->win32.handle);
    DWORD candidateListBytes = ImmGetCandidateListW(hIMC, 0, NULL, 0);

    if (candidateListBytes == 0)
    {
        ImmReleaseContext(window->win32.handle, hIMC);
        return;
    }

    {
        int i;
        int bufferCount = preedit->candidateBufferCount;
        LPCANDIDATELIST candidateList = _grwl_calloc(candidateListBytes, 1);
        if (candidateList == NULL)
        {
            ImmReleaseContext(window->win32.handle, hIMC);
            return;
        }
        ImmGetCandidateListW(hIMC, 0, candidateList, candidateListBytes);
        ImmReleaseContext(window->win32.handle, hIMC);

        while ((DWORD)bufferCount < candidateList->dwCount + 1)
        {
            bufferCount = (bufferCount == 0) ? 1 : bufferCount * 2;
        }
        if (bufferCount != preedit->candidateBufferCount)
        {
            _GRWLpreeditcandidate* candidates =
                _grwl_realloc(preedit->candidates, sizeof(_GRWLpreeditcandidate) * bufferCount);
            if (candidates == NULL)
            {
                _grwl_free(candidateList);
                return;
            }
            // `realloc` does not initialize the increased area with 0.
            // This logic should be moved to a more appropriate place to share
            // when other platforms support this feature.
            for (i = preedit->candidateBufferCount; i < bufferCount; ++i)
            {
                candidates[i].text = NULL;
                candidates[i].textCount = 0;
                candidates[i].textBufferCount = 0;
            }
            preedit->candidates = candidates;
            preedit->candidateBufferCount = bufferCount;
        }

        for (i = 0; (DWORD)i < candidateList->dwCount; ++i)
        {
            setCandidate(&preedit->candidates[i], (LPWSTR)((char*)candidateList + candidateList->dwOffset[i]));
        }

        preedit->candidateCount = candidateList->dwCount;
        preedit->candidateSelection = candidateList->dwSelection;
        preedit->candidatePageStart = candidateList->dwPageStart;
        preedit->candidatePageSize = candidateList->dwPageSize;

        _grwl_free(candidateList);
    }

    _grwlInputPreeditCandidate(window);
}

// Clear preedit candidates
static void clearImmCandidate(_GRWLwindow* window)
{
    window->preedit.candidateCount = 0;
    window->preedit.candidateSelection = 0;
    window->preedit.candidatePageStart = 0;
    window->preedit.candidatePageSize = 0;
    _grwlInputPreeditCandidate(window);
}

// Get preedit texts of Imm32 and pass them to preedit-callback
//
static GRWLbool getImmPreedit(_GRWLwindow* window)
{
    _GRWLpreedit* preedit = &window->preedit;
    HIMC hIMC = ImmGetContext(window->win32.handle);
    // get preedit data sizes
    LONG preeditBytes = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0);
    LONG attrBytes = ImmGetCompositionStringW(hIMC, GCS_COMPATTR, NULL, 0);
    LONG clauseBytes = ImmGetCompositionStringW(hIMC, GCS_COMPCLAUSE, NULL, 0);
    LONG cursorPos = ImmGetCompositionStringW(hIMC, GCS_CURSORPOS, NULL, 0);

    if (preeditBytes > 0)
    {
        int textBufferCount = preedit->textBufferCount;
        int blockBufferCount = preedit->blockSizesBufferCount;
        int textLen = preeditBytes / sizeof(WCHAR);
        LPWSTR buffer = _grwl_calloc(preeditBytes, 1);
        LPSTR attributes = _grwl_calloc(attrBytes, 1);
        DWORD* clauses = _grwl_calloc(clauseBytes, 1);

        if (!buffer || (attrBytes > 0 && !attributes) || (clauseBytes > 0 && !clauses))
        {
            _grwl_free(buffer);
            _grwl_free(attributes);
            _grwl_free(clauses);
            ImmReleaseContext(window->win32.handle, hIMC);
            return GRWL_FALSE;
        }

        // get preedit data
        ImmGetCompositionStringW(hIMC, GCS_COMPSTR, buffer, preeditBytes);
        if (attributes)
        {
            ImmGetCompositionStringW(hIMC, GCS_COMPATTR, attributes, attrBytes);
        }
        if (clauses)
        {
            ImmGetCompositionStringW(hIMC, GCS_COMPCLAUSE, clauses, clauseBytes);
        }

        // realloc preedit text
        while (textBufferCount < textLen + 1)
        {
            textBufferCount = (textBufferCount == 0) ? 1 : textBufferCount * 2;
        }
        if (textBufferCount != preedit->textBufferCount)
        {
            size_t bufsize = sizeof(unsigned int) * textBufferCount;
            unsigned int* preeditText = _grwl_realloc(preedit->text, bufsize);

            if (preeditText == NULL)
            {
                _grwl_free(buffer);
                _grwl_free(attributes);
                _grwl_free(clauses);
                ImmReleaseContext(window->win32.handle, hIMC);
                return GRWL_FALSE;
            }
            preedit->text = preeditText;
            preedit->textBufferCount = textBufferCount;
        }

        // realloc blocks
        preedit->blockSizesCount = clauses ? clauseBytes / sizeof(DWORD) - 1 : 1;
        while (blockBufferCount < preedit->blockSizesCount)
        {
            blockBufferCount = (blockBufferCount == 0) ? 1 : blockBufferCount * 2;
        }
        if (blockBufferCount != preedit->blockSizesBufferCount)
        {
            size_t bufsize = sizeof(int) * blockBufferCount;
            int* blocks = _grwl_realloc(preedit->blockSizes, bufsize);

            if (blocks == NULL)
            {
                _grwl_free(buffer);
                _grwl_free(attributes);
                _grwl_free(clauses);
                ImmReleaseContext(window->win32.handle, hIMC);
                return GRWL_FALSE;
            }
            preedit->blockSizes = blocks;
            preedit->blockSizesBufferCount = blockBufferCount;
        }

        // store preedit text & block sizes
        {
            // Win32 API handles text data in UTF16, so we have to convert them
            // to UTF32. Not only the encoding, but also the number of characters,
            // the position of each block and the cursor.
            int i;
            uint32_t codepoint;
            WCHAR highSurrogate = 0;
            int convertedLength = 0;
            int blockIndex = 0;
            int currentBlockLength = 0;

            // The last element of clauses is a block count, but
            // text length is convenient.
            if (clauses)
            {
                clauses[preedit->blockSizesCount] = textLen;
            }

            for (i = 0; i < textLen; i++)
            {
                if (clauses && clauses[blockIndex + 1] <= (DWORD)i)
                {
                    preedit->blockSizes[blockIndex++] = currentBlockLength;
                    currentBlockLength = 0;
                }

                if (convertToUTF32FromUTF16(buffer[i], &highSurrogate, &codepoint))
                {
                    preedit->text[convertedLength++] = codepoint;
                    currentBlockLength++;
                }
                else if ((LONG)i < cursorPos)
                {
                    // A high surrogate appears before cursorPos, so needs to
                    // fix cursorPos on UTF16 for UTF32
                    cursorPos--;
                }
            }
            preedit->blockSizes[blockIndex] = currentBlockLength;
            preedit->textCount = convertedLength;
            preedit->text[convertedLength] = 0;
            preedit->caretIndex = cursorPos;

            preedit->focusedBlockIndex = 0;
            if (attributes && clauses)
            {
                for (i = 0; i < preedit->blockSizesCount; i++)
                {
                    if (attributes[clauses[i]] == ATTR_TARGET_CONVERTED ||
                        attributes[clauses[i]] == ATTR_TARGET_NOTCONVERTED)
                    {
                        preedit->focusedBlockIndex = i;
                        break;
                    }
                }
            }
        }

        _grwl_free(buffer);
        _grwl_free(attributes);
        _grwl_free(clauses);

        _grwlInputPreedit(window);
    }

    ImmReleaseContext(window->win32.handle, hIMC);
    return GRWL_TRUE;
}

// Clear peedit data
//
static void clearImmPreedit(_GRWLwindow* window)
{
    window->preedit.blockSizesCount = 0;
    window->preedit.textCount = 0;
    window->preedit.focusedBlockIndex = 0;
    window->preedit.caretIndex = 0;
    _grwlInputPreedit(window);
}

// Commit the result texts of Imm32 to character-callback
//
static GRWLbool commitImmResultStr(_GRWLwindow* window)
{
    HIMC hIMC;
    LONG bytes;
    uint32_t codepoint;
    WCHAR highSurrogate = 0;

    if (!window->callbacks.character)
    {
        return GRWL_FALSE;
    }

    hIMC = ImmGetContext(window->win32.handle);
    // get preedit data sizes
    bytes = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);

    if (bytes > 0)
    {
        int i;
        int length = bytes / sizeof(WCHAR);
        LPWSTR buffer = _grwl_calloc(bytes, 1);

        // get preedit data
        ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, buffer, bytes);

        for (i = 0; i < length; i++)
        {
            if (convertToUTF32FromUTF16(buffer[i], &highSurrogate, &codepoint))
            {
                window->callbacks.character((GRWLwindow*)window, codepoint);
            }
        }

        _grwl_free(buffer);
    }

    ImmReleaseContext(window->win32.handle, hIMC);
    return GRWL_TRUE;
}

// Window procedure for user-created windows
//
static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _GRWLwindow* window = GetPropW(hWnd, L"GRWL");
    if (!window)
    {
        if (uMsg == WM_NCCREATE)
        {
            if (_grwlIsWindows10Version1607OrGreaterWin32())
            {
                const CREATESTRUCTW* cs = (const CREATESTRUCTW*)lParam;
                const _GRWLwndconfig* wndconfig = cs->lpCreateParams;

                // On per-monitor DPI aware V1 systems, only enable
                // non-client scaling for windows that scale the client area
                // We need WM_GETDPISCALEDSIZE from V2 to keep the client
                // area static when the non-client area is scaled
                if (wndconfig && wndconfig->scaleToMonitor)
                {
                    EnableNonClientDpiScaling(hWnd);
                }
            }
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
        case WM_IME_SETCONTEXT:
        {
            // To draw preedit text by an application side
            if (lParam & ISC_SHOWUICOMPOSITIONWINDOW)
            {
                lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
            }

            if (_grwl.hints.init.managePreeditCandidate && (lParam & ISC_SHOWUICANDIDATEWINDOW))
            {
                lParam &= ~ISC_SHOWUICANDIDATEWINDOW;
            }
            break;
        }

        case WM_MOUSEACTIVATE:
        {
            // HACK: Postpone cursor disabling when the window was activated by
            //       clicking a caption button
            if (HIWORD(lParam) == WM_LBUTTONDOWN)
            {
                if (LOWORD(lParam) != HTCLIENT)
                {
                    window->win32.frameAction = GRWL_TRUE;
                }
            }

            break;
        }

        case WM_CAPTURECHANGED:
        {
            // HACK: Disable the cursor once the caption button action has been
            //       completed or cancelled
            if (lParam == 0 && window->win32.frameAction)
            {
                if (window->cursorMode == GRWL_CURSOR_DISABLED)
                {
                    disableCursor(window);
                }
                else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
                {
                    captureCursor(window);
                }

                window->win32.frameAction = GRWL_FALSE;
            }

            break;
        }

        case WM_SETFOCUS:
        {
            _grwlInputWindowFocus(window, GRWL_TRUE);

            // HACK: Do not disable cursor while the user is interacting with
            //       a caption button
            if (window->win32.frameAction)
            {
                break;
            }

            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                disableCursor(window);
            }
            else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
            {
                captureCursor(window);
            }

            return 0;
        }

        case WM_KILLFOCUS:
        {
            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                enableCursor(window);
            }
            else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
            {
                releaseCursor();
            }

            if (window->monitor && window->autoIconify)
            {
                _grwlIconifyWindowWin32(window);
            }

            _grwlInputWindowFocus(window, GRWL_FALSE);
            return 0;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam & 0xfff0)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                {
                    if (window->monitor)
                    {
                        // We are running in full screen mode, so disallow
                        // screen saver and screen blanking
                        return 0;
                    }
                    else
                    {
                        break;
                    }
                }

                // User trying to access application menu using ALT?
                case SC_KEYMENU:
                {
                    if (!window->win32.keymenu)
                    {
                        return 0;
                    }

                    break;
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            _grwlInputWindowCloseRequest(window);
            return 0;
        }

        case WM_INPUTLANGCHANGE:
        {
            _grwlUpdateKeyNamesWin32();
            _grwlInputKeyboardLayout();
            break;
        }

        case WM_CHAR:
        case WM_SYSCHAR:
        {
            uint32_t codepoint;
            if (convertToUTF32FromUTF16((WCHAR)wParam, &window->win32.highSurrogate, &codepoint))
            {
                _grwlInputChar(window, codepoint, getKeyMods(), uMsg != WM_SYSCHAR);
            }

            if (uMsg == WM_SYSCHAR && window->win32.keymenu)
            {
                break;
            }

            return 0;
        }

        case WM_UNICHAR:
        {
            if (wParam == UNICODE_NOCHAR)
            {
                // WM_UNICHAR is not sent by Windows, but is sent by some
                // third-party input method engine
                // Returning TRUE here announces support for this message
                return TRUE;
            }

            _grwlInputChar(window, (uint32_t)wParam, getKeyMods(), GRWL_TRUE);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            int key, scancode;
            const int action = (HIWORD(lParam) & KF_UP) ? GRWL_RELEASE : GRWL_PRESS;
            const int mods = getKeyMods();

            scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
            if (!scancode)
            {
                // NOTE: Some synthetic key messages have a scancode of zero
                // HACK: Map the virtual key back to a usable scancode
                scancode = MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);
            }

            // HACK: Alt+PrtSc has a different scancode than just PrtSc
            if (scancode == 0x54)
            {
                scancode = 0x137;
            }

            // HACK: Ctrl+Pause has a different scancode than just Pause
            if (scancode == 0x146)
            {
                scancode = 0x45;
            }

            // HACK: CJK IME sets the extended bit for right Shift
            if (scancode == 0x136)
            {
                scancode = 0x36;
            }

            key = _grwl.win32.keycodes[scancode];

            // The Ctrl keys require special handling
            if (wParam == VK_CONTROL)
            {
                if (HIWORD(lParam) & KF_EXTENDED)
                {
                    // Right side keys have the extended key bit set
                    key = GRWL_KEY_RIGHT_CONTROL;
                }
                else
                {
                    // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                    // HACK: We only want one event for Alt Gr, so if we detect
                    //       this sequence we discard this Left Ctrl message now
                    //       and later report Right Alt normally
                    MSG next;
                    const DWORD time = GetMessageTime();

                    if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                    {
                        if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN || next.message == WM_KEYUP ||
                            next.message == WM_SYSKEYUP)
                        {
                            if (next.wParam == VK_MENU && (HIWORD(next.lParam) & KF_EXTENDED) && next.time == time)
                            {
                                // Next message is Right Alt down so discard this
                                break;
                            }
                        }
                    }

                    // This is a regular Left Ctrl message
                    key = GRWL_KEY_LEFT_CONTROL;
                }
            }
            else if (wParam == VK_PROCESSKEY)
            {
                // IME notifies that keys have been filtered by setting the
                // virtual key-code to VK_PROCESSKEY
                break;
            }

            if (action == GRWL_RELEASE && wParam == VK_SHIFT)
            {
                // HACK: Release both Shift keys on Shift up event, as when both
                //       are pressed the first release does not emit any event
                // NOTE: The other half of this is in _grwlPollEventsWin32
                _grwlInputKey(window, GRWL_KEY_LEFT_SHIFT, scancode, action, mods);
                _grwlInputKey(window, GRWL_KEY_RIGHT_SHIFT, scancode, action, mods);
            }
            else if (wParam == VK_SNAPSHOT)
            {
                // HACK: Key down is not reported for the Print Screen key
                _grwlInputKey(window, key, scancode, GRWL_PRESS, mods);
                _grwlInputKey(window, key, scancode, GRWL_RELEASE, mods);
            }
            else
            {
                _grwlInputKey(window, key, scancode, action, mods);
            }

            break;
        }

        case WM_IME_COMPOSITION:
        {
            if (lParam & (GCS_RESULTSTR | GCS_COMPSTR))
            {
                if (lParam & GCS_RESULTSTR)
                {
                    commitImmResultStr(window);
                }
                if (lParam & GCS_COMPSTR)
                {
                    getImmPreedit(window);
                }
                return TRUE;
            }
            break;
        }

        case WM_IME_ENDCOMPOSITION:
        {
            clearImmPreedit(window);
            // Usually clearing candidates in IMN_CLOSECANDIDATE is sufficient.
            // However, some IME need it here, e.g. Google Japanese Input.
            clearImmCandidate(window);
            return TRUE;
        }

        case WM_IME_NOTIFY:
        {
            switch (wParam)
            {
                case IMN_SETOPENSTATUS:
                {
                    _grwlInputIMEStatus(window);
                    return TRUE;
                }

                case IMN_OPENCANDIDATE:
                case IMN_CHANGECANDIDATE:
                {
                    getImmCandidates(window);
                    return TRUE;
                }

                case IMN_CLOSECANDIDATE:
                {
                    clearImmCandidate(window);
                    return TRUE;
                }
            }
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int i, button, action;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
            {
                button = GRWL_MOUSE_BUTTON_LEFT;
            }
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
            {
                button = GRWL_MOUSE_BUTTON_RIGHT;
            }
            else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
            {
                button = GRWL_MOUSE_BUTTON_MIDDLE;
            }
            else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
            {
                button = GRWL_MOUSE_BUTTON_4;
            }
            else
            {
                button = GRWL_MOUSE_BUTTON_5;
            }

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)
            {
                action = GRWL_PRESS;
            }
            else
            {
                action = GRWL_RELEASE;
            }

            for (i = 0; i <= GRWL_MOUSE_BUTTON_LAST; i++)
            {
                if (window->mouseButtons[i] == GRWL_PRESS)
                {
                    break;
                }
            }

            if (i > GRWL_MOUSE_BUTTON_LAST)
            {
                SetCapture(hWnd);
            }

            _grwlInputMouseClick(window, button, action, getKeyMods());

            for (i = 0; i <= GRWL_MOUSE_BUTTON_LAST; i++)
            {
                if (window->mouseButtons[i] == GRWL_PRESS)
                {
                    break;
                }
            }

            if (i > GRWL_MOUSE_BUTTON_LAST)
            {
                ReleaseCapture();
            }

            if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP)
            {
                return TRUE;
            }

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);

            if (!window->win32.cursorTracked)
            {
                TRACKMOUSEEVENT tme;
                ZeroMemory(&tme, sizeof(tme));
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = window->win32.handle;
                TrackMouseEvent(&tme);

                window->win32.cursorTracked = GRWL_TRUE;
                _grwlInputCursorEnter(window, GRWL_TRUE);
            }

            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                const int dx = x - window->win32.lastCursorPosX;
                const int dy = y - window->win32.lastCursorPosY;

                if (_grwl.win32.disabledCursorWindow != window)
                {
                    break;
                }
                if (window->rawMouseMotion)
                {
                    break;
                }

                _grwlInputCursorPos(window, window->virtualCursorPosX + dx, window->virtualCursorPosY + dy);
            }
            else
            {
                _grwlInputCursorPos(window, x, y);
            }

            window->win32.lastCursorPosX = x;
            window->win32.lastCursorPosY = y;

            return 0;
        }

        case WM_INPUT:
        {
            UINT size = 0;
            HRAWINPUT ri = (HRAWINPUT)lParam;
            RAWINPUT* data = NULL;
            int dx, dy;

            if (_grwl.win32.disabledCursorWindow != window)
            {
                break;
            }
            if (!window->rawMouseMotion)
            {
                break;
            }

            GetRawInputData(ri, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
            if (size > (UINT)_grwl.win32.rawInputSize)
            {
                _grwl_free(_grwl.win32.rawInput);
                _grwl.win32.rawInput = _grwl_calloc(size, 1);
                _grwl.win32.rawInputSize = size;
            }

            size = _grwl.win32.rawInputSize;
            if (GetRawInputData(ri, RID_INPUT, _grwl.win32.rawInput, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
            {
                _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to retrieve raw input data");
                break;
            }

            data = _grwl.win32.rawInput;
            if (data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
            {
                dx = data->data.mouse.lLastX - window->win32.lastCursorPosX;
                dy = data->data.mouse.lLastY - window->win32.lastCursorPosY;
            }
            else
            {
                dx = data->data.mouse.lLastX;
                dy = data->data.mouse.lLastY;
            }

            _grwlInputCursorPos(window, window->virtualCursorPosX + dx, window->virtualCursorPosY + dy);

            window->win32.lastCursorPosX += dx;
            window->win32.lastCursorPosY += dy;
            break;
        }

        case WM_MOUSELEAVE:
        {
            window->win32.cursorTracked = GRWL_FALSE;
            _grwlInputCursorEnter(window, GRWL_FALSE);
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            _grwlInputScroll(window, 0.0, (SHORT)HIWORD(wParam) / (double)WHEEL_DELTA);
            return 0;
        }

        case WM_MOUSEHWHEEL:
        {
            // This message is only sent on Windows Vista and later
            // NOTE: The X-axis is inverted for consistency with macOS and X11
            _grwlInputScroll(window, -((SHORT)HIWORD(wParam) / (double)WHEEL_DELTA), 0.0);
            return 0;
        }

        case WM_ENTERSIZEMOVE:
        case WM_ENTERMENULOOP:
        {
            if (window->win32.frameAction)
            {
                break;
            }

            // HACK: Enable the cursor while the user is moving or
            //       resizing the window or using the window menu
            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                enableCursor(window);
            }
            else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
            {
                releaseCursor();
            }

            break;
        }

        case WM_EXITSIZEMOVE:
        case WM_EXITMENULOOP:
        {
            if (window->win32.frameAction)
            {
                break;
            }

            // HACK: Disable the cursor once the user is done moving or
            //       resizing the window or using the menu
            if (window->cursorMode == GRWL_CURSOR_DISABLED)
            {
                disableCursor(window);
            }
            else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
            {
                captureCursor(window);
            }

            break;
        }

        case WM_SIZE:
        {
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            const GRWLbool iconified = wParam == SIZE_MINIMIZED;
            const GRWLbool maximized = wParam == SIZE_MAXIMIZED || (window->win32.maximized && wParam != SIZE_RESTORED);

            if (_grwl.win32.capturedCursorWindow == window)
            {
                captureCursor(window);
            }

            if (window->win32.iconified != iconified)
            {
                _grwlInputWindowIconify(window, iconified);
            }

            if (window->win32.maximized != maximized)
            {
                _grwlInputWindowMaximize(window, maximized);
            }

            if (width != window->win32.width || height != window->win32.height)
            {
                window->win32.width = width;
                window->win32.height = height;

                _grwlInputFramebufferSize(window, width, height);
                _grwlInputWindowSize(window, width, height);
            }

            if (window->monitor && window->win32.iconified != iconified)
            {
                if (iconified)
                {
                    releaseMonitor(window);
                }
                else
                {
                    acquireMonitor(window);
                    fitToMonitor(window);
                }
            }

            window->win32.iconified = iconified;
            window->win32.maximized = maximized;
            return 0;
        }

        case WM_MOVE:
        {
            if (_grwl.win32.capturedCursorWindow == window)
            {
                captureCursor(window);
            }

            // NOTE: This cannot use LOWORD/HIWORD recommended by MSDN, as
            // those macros do not handle negative window positions correctly
            _grwlInputWindowPos(window, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_SIZING:
        {
            if (window->numer == GRWL_DONT_CARE || window->denom == GRWL_DONT_CARE)
            {
                break;
            }

            applyAspectRatio(window, (int)wParam, (RECT*)lParam);
            return TRUE;
        }

        case WM_GETMINMAXINFO:
        {
            RECT frame = { 0 };
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            const DWORD style = getWindowStyle(window);
            const DWORD exStyle = getWindowExStyle(window);

            if (window->monitor)
            {
                break;
            }

            if (_grwlIsWindows10Version1607OrGreaterWin32())
            {
                AdjustWindowRectExForDpi(&frame, style, FALSE, exStyle, GetDpiForWindow(window->win32.handle));
            }
            else
            {
                AdjustWindowRectEx(&frame, style, FALSE, exStyle);
            }

            if (window->minwidth != GRWL_DONT_CARE && window->minheight != GRWL_DONT_CARE)
            {
                mmi->ptMinTrackSize.x = window->minwidth + frame.right - frame.left;
                mmi->ptMinTrackSize.y = window->minheight + frame.bottom - frame.top;
            }

            if (window->maxwidth != GRWL_DONT_CARE && window->maxheight != GRWL_DONT_CARE)
            {
                mmi->ptMaxTrackSize.x = window->maxwidth + frame.right - frame.left;
                mmi->ptMaxTrackSize.y = window->maxheight + frame.bottom - frame.top;
            }

            if (!window->decorated)
            {
                MONITORINFO mi;
                const HMONITOR mh = MonitorFromWindow(window->win32.handle, MONITOR_DEFAULTTONEAREST);

                ZeroMemory(&mi, sizeof(mi));
                mi.cbSize = sizeof(mi);
                GetMonitorInfoW(mh, &mi);

                mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
                mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
                mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
                mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
            }

            return 0;
        }

        case WM_PAINT:
        {
            _grwlInputWindowDamage(window);
            break;
        }

        case WM_ERASEBKGND:
        {
            return TRUE;
        }

        case WM_NCACTIVATE:
        case WM_NCPAINT:
        {
            // Prevent title bar from being drawn after restoring a minimized
            // undecorated window
            if (!window->decorated)
            {
                return TRUE;
            }

            break;
        }

        case WM_DWMCOMPOSITIONCHANGED:
        case WM_DWMCOLORIZATIONCOLORCHANGED:
        {
            if (window->win32.transparent)
            {
                updateFramebufferTransparency(window);
            }
            return 0;
        }

        case WM_THEMECHANGED:
        case WM_SETTINGCHANGE:
        {
            updateTheme(window->win32.handle);
            break;
        }

        case WM_GETDPISCALEDSIZE:
        {
            if (window->win32.scaleToMonitor)
            {
                break;
            }

            // Adjust the window size to keep the content area size constant
            if (_grwlIsWindows10Version1703OrGreaterWin32())
            {
                RECT source = { 0 }, target = { 0 };
                SIZE* size = (SIZE*)lParam;

                AdjustWindowRectExForDpi(&source, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                         GetDpiForWindow(window->win32.handle));
                AdjustWindowRectExForDpi(&target, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                         LOWORD(wParam));

                size->cx += (target.right - target.left) - (source.right - source.left);
                size->cy += (target.bottom - target.top) - (source.bottom - source.top);
                return TRUE;
            }

            break;
        }

        case WM_DPICHANGED:
        {
            const float xscale = HIWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;
            const float yscale = LOWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;

            // Resize windowed mode windows that either permit rescaling or that
            // need it to compensate for non-client area scaling
            if (!window->monitor && (window->win32.scaleToMonitor || _grwlIsWindows10Version1703OrGreaterWin32()))
            {
                RECT* suggested = (RECT*)lParam;
                SetWindowPos(window->win32.handle, HWND_TOP, suggested->left, suggested->top,
                             suggested->right - suggested->left, suggested->bottom - suggested->top,
                             SWP_NOACTIVATE | SWP_NOZORDER);
            }

            _grwlInputWindowContentScale(window, xscale, yscale);
            break;
        }

        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                updateCursorImage(window);
                return TRUE;
            }

            break;
        }

        case WM_DROPFILES:
        {
            HDROP drop = (HDROP)wParam;
            POINT pt;
            int i;

            const int count = DragQueryFileW(drop, 0xffffffff, NULL, 0);
            char** paths = _grwl_calloc(count, sizeof(char*));

            // Move the mouse to the position of the drop
            DragQueryPoint(drop, &pt);
            _grwlInputCursorPos(window, pt.x, pt.y);

            for (i = 0; i < count; i++)
            {
                const UINT length = DragQueryFileW(drop, i, NULL, 0);
                WCHAR* buffer = _grwl_calloc((size_t)length + 1, sizeof(WCHAR));

                DragQueryFileW(drop, i, buffer, length + 1);
                paths[i] = _grwlCreateUTF8FromWideStringWin32(buffer);

                _grwl_free(buffer);
            }

            _grwlInputDrop(window, count, (const char**)paths);

            for (i = 0; i < count; i++)
            {
                _grwl_free(paths[i]);
            }
            _grwl_free(paths);

            DragFinish(drop);
            return 0;
        }
    }

    if (uMsg == window->win32.taskbarListMsgID)
    {
        HRESULT res = CoCreateInstance(&CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, &IID_ITaskbarList3,
                                       (LPVOID*)&window->win32.taskbarList);
        if (res != S_OK && window->win32.taskbarList)
        {
            window->win32.taskbarList->lpVtbl->Release(window->win32.taskbarList);
        }
        else
        {
            window->win32.taskbarList->lpVtbl->AddRef(window->win32.taskbarList);
            window->win32.taskbarList->lpVtbl->HrInit(window->win32.taskbarList);
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// Creates the GRWL window
//
static int createNativeWindow(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLfbconfig* fbconfig)
{
    int frameX, frameY, frameWidth, frameHeight;
    WCHAR* wideTitle;
    DWORD style = getWindowStyle(window);
    DWORD exStyle = getWindowExStyle(window);

    if (!_grwl.win32.mainWindowClass)
    {
        WNDCLASSEXW wc = { sizeof(wc) };
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = windowProc;
        wc.hInstance = _grwl.win32.instance;
        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    #if defined(_GRWL_WNDCLASSNAME)
        wc.lpszClassName = _GRWL_WNDCLASSNAME;
    #else
        wc.lpszClassName = L"GRWL0";
    #endif
        // Load user-provided icon if available
        wc.hIcon = LoadImageW(GetModuleHandleW(NULL), L"GRWL_ICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if (!wc.hIcon)
        {
            // No user-provided icon found, load default icon
            wc.hIcon = LoadImageW(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        }

        _grwl.win32.mainWindowClass = RegisterClassExW(&wc);
        if (!_grwl.win32.mainWindowClass)
        {
            _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to register window class");
            return GRWL_FALSE;
        }
    }

    if (window->monitor)
    {
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfoW(window->monitor->win32.handle, &mi);

        // NOTE: This window placement is temporary and approximate, as the
        //       correct position and size cannot be known until the monitor
        //       video mode has been picked in _grwlSetVideoModeWin32
        frameX = mi.rcMonitor.left;
        frameY = mi.rcMonitor.top;
        frameWidth = mi.rcMonitor.right - mi.rcMonitor.left;
        frameHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }
    else
    {
        RECT rect = { 0, 0, wndconfig->width, wndconfig->height };

        window->win32.maximized = wndconfig->maximized;
        if (wndconfig->maximized)
        {
            style |= WS_MAXIMIZE;
        }

        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        if (wndconfig->xpos == GRWL_ANY_POSITION && wndconfig->ypos == GRWL_ANY_POSITION)
        {
            frameX = CW_USEDEFAULT;
            frameY = CW_USEDEFAULT;
        }
        else
        {
            frameX = wndconfig->xpos + rect.left;
            frameY = wndconfig->ypos + rect.top;
        }

        frameWidth = rect.right - rect.left;
        frameHeight = rect.bottom - rect.top;
    }

    wideTitle = _grwlCreateWideStringFromUTF8Win32(wndconfig->title);
    if (!wideTitle)
    {
        return GRWL_FALSE;
    }

    window->win32.handle = CreateWindowExW(exStyle, MAKEINTATOM(_grwl.win32.mainWindowClass), wideTitle, style, frameX,
                                           frameY, frameWidth, frameHeight,
                                           NULL, // No parent window
                                           NULL, // No window menu
                                           _grwl.win32.instance, (LPVOID)wndconfig);

    _grwl_free(wideTitle);

    if (!window->win32.handle)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create window");
        return GRWL_FALSE;
    }

    SetPropW(window->win32.handle, L"GRWL", window);

    if (IsWindows7OrGreater())
    {
        ChangeWindowMessageFilterEx(window->win32.handle, WM_DROPFILES, MSGFLT_ALLOW, NULL);
        ChangeWindowMessageFilterEx(window->win32.handle, WM_COPYDATA, MSGFLT_ALLOW, NULL);
        ChangeWindowMessageFilterEx(window->win32.handle, WM_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);

        window->win32.taskbarListMsgID = RegisterWindowMessageW(L"TaskbarButtonCreated");
        if (window->win32.taskbarListMsgID)
        {
            ChangeWindowMessageFilterEx(window->win32.handle, window->win32.taskbarListMsgID, MSGFLT_ALLOW, NULL);
        }
    }

    window->win32.scaleToMonitor = wndconfig->scaleToMonitor;
    window->win32.keymenu = wndconfig->win32.keymenu;
    window->win32.genericBadge = wndconfig->win32.genericBadge;

    if (!window->monitor)
    {
        RECT rect = { 0, 0, wndconfig->width, wndconfig->height };
        WINDOWPLACEMENT wp = { sizeof(wp) };
        const HMONITOR mh = MonitorFromWindow(window->win32.handle, MONITOR_DEFAULTTONEAREST);

        // Adjust window rect to account for DPI scaling of the window frame and
        // (if enabled) DPI scaling of the content area
        // This cannot be done until we know what monitor the window was placed on
        // Only update the restored window rect as the window may be maximized

        if (wndconfig->scaleToMonitor)
        {
            float xscale, yscale;
            _grwlGetHMONITORContentScaleWin32(mh, &xscale, &yscale);

            if (xscale > 0.f && yscale > 0.f)
            {
                rect.right = (int)(rect.right * xscale);
                rect.bottom = (int)(rect.bottom * yscale);
            }
        }

        if (_grwlIsWindows10Version1607OrGreaterWin32())
        {
            AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, GetDpiForWindow(window->win32.handle));
        }
        else
        {
            AdjustWindowRectEx(&rect, style, FALSE, exStyle);
        }

        GetWindowPlacement(window->win32.handle, &wp);
        OffsetRect(&rect, wp.rcNormalPosition.left - rect.left, wp.rcNormalPosition.top - rect.top);

        wp.rcNormalPosition = rect;
        wp.showCmd = SW_HIDE;
        SetWindowPlacement(window->win32.handle, &wp);

        // Adjust rect of maximized undecorated window, because by default Windows will
        // make such a window cover the whole monitor instead of its workarea

        if (wndconfig->maximized && !wndconfig->decorated)
        {
            MONITORINFO mi = { sizeof(mi) };
            GetMonitorInfoW(mh, &mi);

            SetWindowPos(window->win32.handle, HWND_TOP, mi.rcWork.left, mi.rcWork.top,
                         mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top,
                         SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }

    DragAcceptFiles(window->win32.handle, TRUE);

    if (fbconfig->transparent)
    {
        updateFramebufferTransparency(window);
        window->win32.transparent = GRWL_TRUE;
    }

    _grwlGetWindowSizeWin32(window, &window->win32.width, &window->win32.height);

    updateTheme(window->win32.handle);

    return GRWL_TRUE;
}

GRWLbool _grwlCreateWindowWin32(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                                const _GRWLfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig, fbconfig))
    {
        return GRWL_FALSE;
    }

    if (ctxconfig->client != GRWL_NO_API)
    {
        if (ctxconfig->source == GRWL_NATIVE_CONTEXT_API)
        {
            if (!_grwlInitWGL())
            {
                return GRWL_FALSE;
            }
            if (!_grwlCreateContextWGL(window, ctxconfig, fbconfig))
            {
                return GRWL_FALSE;
            }
        }
        else if (ctxconfig->source == GRWL_EGL_CONTEXT_API)
        {
            if (!_grwlInitEGL())
            {
                return GRWL_FALSE;
            }
            if (!_grwlCreateContextEGL(window, ctxconfig, fbconfig))
            {
                return GRWL_FALSE;
            }
        }
        else if (ctxconfig->source == GRWL_OSMESA_CONTEXT_API)
        {
            if (!_grwlInitOSMesa())
            {
                return GRWL_FALSE;
            }
            if (!_grwlCreateContextOSMesa(window, ctxconfig, fbconfig))
            {
                return GRWL_FALSE;
            }
        }

        if (!_grwlRefreshContextAttribs(window, ctxconfig))
        {
            return GRWL_FALSE;
        }
    }

    if (wndconfig->mousePassthrough)
    {
        _grwlSetWindowMousePassthroughWin32(window, GRWL_TRUE);
    }

    if (window->monitor)
    {
        _grwlShowWindowWin32(window);
        _grwlFocusWindowWin32(window);
        acquireMonitor(window);
        fitToMonitor(window);

        if (wndconfig->centerCursor)
        {
            _grwlCenterCursorInContentArea(window);
        }
    }
    else
    {
        if (wndconfig->visible)
        {
            _grwlShowWindowWin32(window);
            if (wndconfig->focused)
            {
                _grwlFocusWindowWin32(window);
            }
        }
    }

    return GRWL_TRUE;
}

void _grwlDestroyWindowWin32(_GRWLwindow* window)
{
    if (window->monitor)
    {
        releaseMonitor(window);
    }

    if (window->context.destroy)
    {
        window->context.destroy(window);
    }

    if (_grwl.win32.disabledCursorWindow == window)
    {
        enableCursor(window);
    }

    if (_grwl.win32.capturedCursorWindow == window)
    {
        releaseCursor();
    }

    if (window->win32.taskbarList)
    {
        window->win32.taskbarList->lpVtbl->Release(window->win32.taskbarList);
    }

    if (window->win32.handle)
    {
        RemovePropW(window->win32.handle, L"GRWL");
        DestroyWindow(window->win32.handle);
        window->win32.handle = NULL;
    }

    if (window->win32.bigIcon)
    {
        DestroyIcon(window->win32.bigIcon);
    }

    if (window->win32.smallIcon)
    {
        DestroyIcon(window->win32.smallIcon);
    }
}

void _grwlSetWindowTitleWin32(_GRWLwindow* window, const char* title)
{
    WCHAR* wideTitle = _grwlCreateWideStringFromUTF8Win32(title);
    if (!wideTitle)
    {
        return;
    }

    SetWindowTextW(window->win32.handle, wideTitle);
    _grwl_free(wideTitle);
}

void _grwlSetWindowIconWin32(_GRWLwindow* window, int count, const GRWLimage* images)
{
    HICON bigIcon = NULL, smallIcon = NULL;

    if (count)
    {
        const GRWLimage* bigImage =
            chooseImage(count, images, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
        const GRWLimage* smallImage =
            chooseImage(count, images, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));

        bigIcon = createIcon(bigImage, 0, 0, GRWL_TRUE);
        smallIcon = createIcon(smallImage, 0, 0, GRWL_TRUE);
    }
    else
    {
        bigIcon = (HICON)GetClassLongPtrW(window->win32.handle, GCLP_HICON);
        smallIcon = (HICON)GetClassLongPtrW(window->win32.handle, GCLP_HICONSM);
    }

    SendMessageW(window->win32.handle, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
    SendMessageW(window->win32.handle, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);

    if (window->win32.bigIcon)
    {
        DestroyIcon(window->win32.bigIcon);
    }

    if (window->win32.smallIcon)
    {
        DestroyIcon(window->win32.smallIcon);
    }

    if (count)
    {
        window->win32.bigIcon = bigIcon;
        window->win32.smallIcon = smallIcon;
    }
}

void _grwlSetWindowProgressIndicatorWin32(_GRWLwindow* window, int progressState, double value)
{
    HRESULT res = S_OK;
    int winProgressState = 0;
    int progressValue = (int)(value * 100.0);

    if (!IsWindows7OrGreater())
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Win32: Taskbar progress is only supported on Windows 7 and newer");
        return;
    }

    if (!window->win32.taskbarList)
    {
        return;
    }

    res = window->win32.taskbarList->lpVtbl->SetProgressValue(window->win32.taskbarList, window->win32.handle,
                                                              progressValue, 100);
    if (res != S_OK)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to set taskbar progress value");
        return;
    }

    switch (progressState)
    {
        case GRWL_PROGRESS_INDICATOR_INDETERMINATE:
            winProgressState = TBPF_INDETERMINATE;
            break;
        case GRWL_PROGRESS_INDICATOR_NORMAL:
            winProgressState = TBPF_NORMAL;
            break;
        case GRWL_PROGRESS_INDICATOR_ERROR:
            winProgressState = TBPF_ERROR;
            break;
        case GRWL_PROGRESS_INDICATOR_PAUSED:
            winProgressState = TBPF_PAUSED;
            break;

        default:
            winProgressState = TBPF_NOPROGRESS;
            break;
    }

    res = window->win32.taskbarList->lpVtbl->SetProgressState(window->win32.taskbarList, window->win32.handle,
                                                              winProgressState);
    if (res != S_OK)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to set taskbar progress state");
    }
}

typedef struct
{
    HDC hdc;
    HDC hdcMem;
    HDC hdcMemMask;

    HBITMAP hBitmap;
    HBITMAP hBitmapMask;
    HBITMAP hOldBitmap;
    HBITMAP hOldBitmapMask;

    HBRUSH hForegroundBrush;
    HBRUSH hBackgroundBrush;
    HBRUSH hOldBrush;

    HFONT hFont;
    HFONT hOldFont;

    COLORREF hOldColor;

    int oldBkMode;

    UINT oldTextAlign;
} BadgeData;

static void CleanupBadgeData(HWND hWnd, BadgeData* data)
{
    if (!data)
    {
        return;
    }

    if (data->oldTextAlign != GDI_ERROR)
    {
        SetTextAlign(data->hdcMem, data->oldTextAlign);
    }

    if (data->oldBkMode)
    {
        SetBkMode(data->hdcMem, data->oldBkMode);
    }

    if (data->hOldColor != CLR_INVALID)
    {
        SetTextColor(data->hdcMem, data->hOldColor);
    }

    if (data->hFont)
    {
        DeleteObject(data->hFont);
    }
    if (data->hOldFont)
    {
        SelectObject(data->hdcMem, data->hOldFont);
    }

    if (data->hForegroundBrush)
    {
        DeleteObject(data->hForegroundBrush);
    }
    if (data->hBackgroundBrush)
    {
        DeleteObject(data->hBackgroundBrush);
    }

    if (data->hOldBrush)
    {
        SelectObject(data->hdcMem, data->hOldBrush);
    }

    if (data->hOldBitmap)
    {
        SelectObject(data->hdcMem, data->hOldBitmap);
    }
    if (data->hOldBitmapMask)
    {
        SelectObject(data->hdcMem, data->hOldBitmapMask);
    }

    if (data->hBitmap)
    {
        DeleteObject(data->hBitmap);
    }
    if (data->hBitmapMask)
    {
        DeleteObject(data->hBitmapMask);
    }

    if (data->hdcMem)
    {
        DeleteDC(data->hdcMem);
    }
    if (data->hdcMemMask)
    {
        DeleteDC(data->hdcMemMask);
    }

    if (data->hdc)
    {
        ReleaseDC(hWnd, data->hdc);
    }
}

static HICON GenerateTextBadgeIcon(HWND hWnd, WCHAR* text)
{
    BadgeData badgeData;

    void* bits = NULL;
    DWORD* pixels = NULL;
    ICONINFO iconInfo;
    int width = 16, height = 16;
    int fontSize = 16, weight = FW_REGULAR;
    RECT contentRect = { 0, 0, width, height };
    HICON hIcon = NULL;
    BITMAPINFO bmi = { 0 };
    int x = 0, y = 0;

    memset(&badgeData, 0, sizeof(BadgeData));

    if (!text)
    {
        return NULL;
    }

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    badgeData.hdc = GetDC(hWnd);
    if (!badgeData.hdc)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hdcMem = CreateCompatibleDC(badgeData.hdc);
    if (!badgeData.hdcMem)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hdcMemMask = CreateCompatibleDC(badgeData.hdc);
    if (!badgeData.hdcMemMask)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hBitmap = CreateDIBSection(badgeData.hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!badgeData.hBitmap)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }
    pixels = (DWORD*)bits;

    badgeData.hBitmapMask = CreateCompatibleBitmap(badgeData.hdc, width, height);
    if (!badgeData.hBitmapMask)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hOldBitmap = (HBITMAP)SelectObject(badgeData.hdcMem, badgeData.hBitmap);
    badgeData.hOldBitmapMask = (HBITMAP)SelectObject(badgeData.hdcMemMask, badgeData.hBitmapMask);

    if (BitBlt(badgeData.hdcMemMask, 0, 0, width, height, badgeData.hdcMem, 0, 0, SRCCOPY) == FALSE)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hBackgroundBrush = CreateSolidBrush(RGB(0x26, 0x25, 0x2D));
    if (!badgeData.hBackgroundBrush)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hOldBrush = (HBRUSH)SelectObject(badgeData.hdcMem, badgeData.hBackgroundBrush);

    if (Ellipse(badgeData.hdcMem, 0, 0, width + 1, height + 1) == FALSE) // 17x17 gives a more fancy ellipse
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    // Adjust font size depending on digits to display
    if (lstrlen(text) > 2)
    {
        fontSize = 10;
        weight = FW_LIGHT;
    }
    else if (lstrlen(text) > 1)
    {
        fontSize = 14;
    }

    // Create and set font
    badgeData.hFont = CreateFont(fontSize, 0, 0, 0, weight, FALSE, FALSE, FALSE, 0, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                                 TEXT("Segeo UI"));
    if (!badgeData.hFont)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hOldFont = (HFONT)SelectObject(badgeData.hdcMem, badgeData.hFont);

    // Draw text (center aligned)
    badgeData.hOldColor = SetTextColor(badgeData.hdcMem, RGB(255, 255, 255)); // Use white text color
    if (badgeData.hOldColor == CLR_INVALID)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.oldBkMode = SetBkMode(badgeData.hdcMem, TRANSPARENT); // Make font background transparent
    if (!badgeData.oldBkMode)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.oldTextAlign = SetTextAlign(badgeData.hdcMem, TA_LEFT | TA_TOP | TA_NOUPDATECP);
    if (badgeData.oldTextAlign == GDI_ERROR)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    if (!DrawText(badgeData.hdcMem, text, lstrlen(text), &contentRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE))
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    // Transparency
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            const DWORD pixel = pixels[y * width + x];
            if (pixel == 0x0026252Du || pixel == 0x00FFFFFFu) // Pixel is text or ellipsis
            {
                pixels[y * width + x] |= 0xFF000000u; // Set opaque
            }
            else
            {
                pixels[y * width + x] &= 0xFF000000u; // Set fully transparent
            }
        }
    }

    SelectObject(badgeData.hdcMem, badgeData.hOldBitmap);
    badgeData.hOldBitmap = NULL;
    SelectObject(badgeData.hdcMemMask, badgeData.hOldBitmapMask);
    badgeData.hOldBitmapMask = NULL;

    // Generate icon from bitmap
    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;
    iconInfo.yHotspot = 0;
    iconInfo.hbmMask = badgeData.hBitmapMask;
    iconInfo.hbmColor = badgeData.hBitmap;
    hIcon = CreateIconIndirect(&iconInfo);

    CleanupBadgeData(hWnd, &badgeData);

    return hIcon;
}

static HICON GenerateGenericBadgeIcon(HWND hWnd)
{
    BadgeData badgeData;

    void* bits = NULL;
    DWORD* pixels = NULL;
    ICONINFO iconInfo;
    int width = 32, height = 32;
    HICON hIcon = NULL;
    BITMAPINFO bmi;
    int x = 0, y = 0;

    memset(&badgeData, 0, sizeof(BadgeData));

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    badgeData.hdc = GetDC(hWnd);
    if (!badgeData.hdc)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hdcMem = CreateCompatibleDC(badgeData.hdc);
    if (!badgeData.hdcMem)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hdcMemMask = CreateCompatibleDC(badgeData.hdc);
    if (!badgeData.hdcMemMask)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hBitmap = CreateDIBSection(badgeData.hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!badgeData.hBitmap)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }
    pixels = (DWORD*)bits;

    badgeData.hBitmapMask = CreateCompatibleBitmap(badgeData.hdc, width, height);
    if (!badgeData.hBitmapMask)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hOldBitmap = (HBITMAP)SelectObject(badgeData.hdcMem, badgeData.hBitmap);
    badgeData.hOldBitmapMask = (HBITMAP)SelectObject(badgeData.hdcMemMask, badgeData.hBitmapMask);

    if (BitBlt(badgeData.hdcMemMask, 0, 0, width, height, badgeData.hdcMem, 0, 0, SRCCOPY) == FALSE)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hBackgroundBrush = CreateSolidBrush(RGB(0xEB, 0x5A, 0x5E));
    if (!badgeData.hBackgroundBrush)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hOldBrush = (HBRUSH)SelectObject(badgeData.hdcMem, badgeData.hBackgroundBrush);

    if (Ellipse(badgeData.hdcMem, 0, 0, width + 1, height + 1) == FALSE) // 17x17 gives a more fancy ellipse
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    badgeData.hForegroundBrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
    if (!badgeData.hForegroundBrush)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    SelectObject(badgeData.hdcMem, badgeData.hForegroundBrush);

    if (Ellipse(badgeData.hdcMem, 9, 9, (width - 8), (height - 8)) == FALSE)
    {
        CleanupBadgeData(hWnd, &badgeData);
        return NULL;
    }

    SelectObject(badgeData.hdcMem, badgeData.hOldBitmap);
    badgeData.hOldBitmap = NULL;
    SelectObject(badgeData.hdcMemMask, badgeData.hOldBitmapMask);
    badgeData.hOldBitmapMask = NULL;

    // Transparency
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            const DWORD pixel = pixels[y * width + x];
            if (pixel >= 0x00010101u) // Pixel is ellipsis
            {
                pixels[y * width + x] |= 0xFF000000u; // Set opaque
            }
            else
            {
                pixels[y * width + x] &= 0xFF000000u; // Set fully transparent
            }
        }
    }

    // Generate icon from bitmap
    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;
    iconInfo.yHotspot = 0;
    iconInfo.hbmMask = badgeData.hBitmapMask;
    iconInfo.hbmColor = badgeData.hBitmap;
    hIcon = CreateIconIndirect(&iconInfo);

    CleanupBadgeData(hWnd, &badgeData);

    return hIcon;
}

void _grwlSetWindowBadgeWin32(_GRWLwindow* window, int count)
{
    HRESULT res = S_OK;
    HICON icon = NULL;
    char countStr[4];
    WCHAR* countWStr = NULL;

    if (window == NULL)
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Win32: Taskbar badge requires a valid window handle");
        return;
    }

    if (!IsWindows7OrGreater())
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Win32: Taskbar badge is only supported on Windows 7 and newer");
        return;
    }

    if (!window->win32.taskbarList)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to set taskbar badge count");
        return;
    }

    count = _grwl_min(count, 999);
    if (count > 0)
    {
        if (window->win32.genericBadge)
        {
            icon = GenerateGenericBadgeIcon(window->win32.handle);
        }
        else
        {
            // Convert count to string (its guaranteed to be at max 3 digits)
            memset(countStr, 0, 4 * sizeof(char));
            sprintf(countStr, "%d", count);
            countWStr = _grwlCreateWideStringFromUTF8Win32(countStr);
            if (!countWStr)
            {
                _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to set taskbar badge count");
                return;
            }

            icon = GenerateTextBadgeIcon(window->win32.handle, countWStr);
        }

        if (!icon)
        {
            _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to set taskbar badge count");
            _grwl_free(countWStr);
            return;
        }
    }

    res = window->win32.taskbarList->lpVtbl->SetOverlayIcon(window->win32.taskbarList, window->win32.handle, icon,
                                                            countWStr ? countWStr : TEXT(""));

    if (countWStr)
    {
        _grwl_free(countWStr);
    }

    if (icon)
    {
        DestroyIcon(icon);
    }

    if (res != S_OK)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to set taskbar badge count");
        return;
    }
}

void _grwlSetWindowBadgeStringWin32(_GRWLwindow* window, const char* string)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Win32: Unable to set a string badge. Only integer badges are supported");
    // In reality you can display a string but it could only be 3 maybe 4 characters long till it becomes an unreadable
    // mess.
}

void _grwlGetWindowPosWin32(_GRWLwindow* window, int* xpos, int* ypos)
{
    POINT pos = { 0, 0 };
    ClientToScreen(window->win32.handle, &pos);

    if (xpos)
    {
        *xpos = pos.x;
    }
    if (ypos)
    {
        *ypos = pos.y;
    }
}

void _grwlSetWindowPosWin32(_GRWLwindow* window, int xpos, int ypos)
{
    RECT rect = { xpos, ypos, xpos, ypos };

    if (_grwlIsWindows10Version1607OrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                 GetDpiForWindow(window->win32.handle));
    }
    else
    {
        AdjustWindowRectEx(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window));
    }

    SetWindowPos(window->win32.handle, NULL, rect.left, rect.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void _grwlGetWindowSizeWin32(_GRWLwindow* window, int* width, int* height)
{
    RECT area;
    GetClientRect(window->win32.handle, &area);

    if (width)
    {
        *width = area.right;
    }
    if (height)
    {
        *height = area.bottom;
    }
}

void _grwlSetWindowSizeWin32(_GRWLwindow* window, int width, int height)
{
    if (window->monitor)
    {
        if (window->monitor->window == window)
        {
            acquireMonitor(window);
            fitToMonitor(window);
        }
    }
    else
    {
        RECT rect = { 0, 0, width, height };

        if (_grwlIsWindows10Version1607OrGreaterWin32())
        {
            AdjustWindowRectExForDpi(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                     GetDpiForWindow(window->win32.handle));
        }
        else
        {
            AdjustWindowRectEx(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window));
        }

        SetWindowPos(window->win32.handle, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }
}

void _grwlSetWindowSizeLimitsWin32(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight)
{
    RECT area;

    if ((minwidth == GRWL_DONT_CARE || minheight == GRWL_DONT_CARE) &&
        (maxwidth == GRWL_DONT_CARE || maxheight == GRWL_DONT_CARE))
    {
        return;
    }

    GetWindowRect(window->win32.handle, &area);
    MoveWindow(window->win32.handle, area.left, area.top, area.right - area.left, area.bottom - area.top, TRUE);
}

void _grwlSetWindowAspectRatioWin32(_GRWLwindow* window, int numer, int denom)
{
    RECT area;

    if (numer == GRWL_DONT_CARE || denom == GRWL_DONT_CARE)
    {
        return;
    }

    GetWindowRect(window->win32.handle, &area);
    applyAspectRatio(window, WMSZ_BOTTOMRIGHT, &area);
    MoveWindow(window->win32.handle, area.left, area.top, area.right - area.left, area.bottom - area.top, TRUE);
}

void _grwlGetFramebufferSizeWin32(_GRWLwindow* window, int* width, int* height)
{
    _grwlGetWindowSizeWin32(window, width, height);
}

void _grwlGetWindowFrameSizeWin32(_GRWLwindow* window, int* left, int* top, int* right, int* bottom)
{
    RECT rect;
    int width, height;

    _grwlGetWindowSizeWin32(window, &width, &height);
    SetRect(&rect, 0, 0, width, height);

    if (_grwlIsWindows10Version1607OrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                 GetDpiForWindow(window->win32.handle));
    }
    else
    {
        AdjustWindowRectEx(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window));
    }

    if (left)
    {
        *left = -rect.left;
    }
    if (top)
    {
        *top = -rect.top;
    }
    if (right)
    {
        *right = rect.right - width;
    }
    if (bottom)
    {
        *bottom = rect.bottom - height;
    }
}

void _grwlGetWindowContentScaleWin32(_GRWLwindow* window, float* xscale, float* yscale)
{
    const HANDLE handle = MonitorFromWindow(window->win32.handle, MONITOR_DEFAULTTONEAREST);
    _grwlGetHMONITORContentScaleWin32(handle, xscale, yscale);
}

void _grwlIconifyWindowWin32(_GRWLwindow* window)
{
    ShowWindow(window->win32.handle, SW_MINIMIZE);
}

void _grwlRestoreWindowWin32(_GRWLwindow* window)
{
    ShowWindow(window->win32.handle, SW_RESTORE);
}

void _grwlMaximizeWindowWin32(_GRWLwindow* window)
{
    if (IsWindowVisible(window->win32.handle))
    {
        ShowWindow(window->win32.handle, SW_MAXIMIZE);
    }
    else
    {
        maximizeWindowManually(window);
    }
}

void _grwlShowWindowWin32(_GRWLwindow* window)
{
    ShowWindow(window->win32.handle, SW_SHOWNA);
}

void _grwlHideWindowWin32(_GRWLwindow* window)
{
    ShowWindow(window->win32.handle, SW_HIDE);
}

void _grwlRequestWindowAttentionWin32(_GRWLwindow* window)
{
    FlashWindow(window->win32.handle, TRUE);
}

void _grwlFocusWindowWin32(_GRWLwindow* window)
{
    BringWindowToTop(window->win32.handle);
    SetForegroundWindow(window->win32.handle);
    SetFocus(window->win32.handle);
}

void _grwlSetWindowMonitorWin32(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                                int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
            {
                acquireMonitor(window);
                fitToMonitor(window);
            }
        }
        else
        {
            RECT rect = { xpos, ypos, xpos + width, ypos + height };

            if (_grwlIsWindows10Version1607OrGreaterWin32())
            {
                AdjustWindowRectExForDpi(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                         GetDpiForWindow(window->win32.handle));
            }
            else
            {
                AdjustWindowRectEx(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window));
            }

            SetWindowPos(window->win32.handle, HWND_TOP, rect.left, rect.top, rect.right - rect.left,
                         rect.bottom - rect.top, SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER);
        }

        return;
    }

    if (window->monitor)
    {
        releaseMonitor(window);
    }

    _grwlInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        MONITORINFO mi = { sizeof(mi) };
        UINT flags = SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS;

        if (window->decorated)
        {
            DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
            style &= ~WS_OVERLAPPEDWINDOW;
            style |= getWindowStyle(window);
            SetWindowLongW(window->win32.handle, GWL_STYLE, style);
            flags |= SWP_FRAMECHANGED;
        }

        acquireMonitor(window);

        GetMonitorInfoW(window->monitor->win32.handle, &mi);
        // On Win32, the window behaves in a special way when the window covers the screen exactly.
        // In this case, the window is a strong exclusive exceeding HWND_TOPMOST and it doen't
        // display OS notifications such as the sound volumn changed.
        // This behavior can be deactivated by shifting the size slightly.
        // This is especially necessary for the IME(Input Method Editor/Engine) to display properly.
        // This can also effect transparency to fullscreen.
        SetWindowPos(window->win32.handle, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     _grwl.hints.window.softFullscreen ? mi.rcMonitor.bottom - mi.rcMonitor.top + 1
                                                       : mi.rcMonitor.bottom - mi.rcMonitor.top,
                     flags);
    }
    else
    {
        HWND after;
        RECT rect = { xpos, ypos, xpos + width, ypos + height };
        DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
        UINT flags = SWP_NOACTIVATE | SWP_NOCOPYBITS;

        if (window->decorated)
        {
            style &= ~WS_POPUP;
            style |= getWindowStyle(window);
            SetWindowLongW(window->win32.handle, GWL_STYLE, style);

            flags |= SWP_FRAMECHANGED;
        }

        if (window->floating)
        {
            after = HWND_TOPMOST;
        }
        else
        {
            after = HWND_NOTOPMOST;
        }

        if (_grwlIsWindows10Version1607OrGreaterWin32())
        {
            AdjustWindowRectExForDpi(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window),
                                     GetDpiForWindow(window->win32.handle));
        }
        else
        {
            AdjustWindowRectEx(&rect, getWindowStyle(window), FALSE, getWindowExStyle(window));
        }

        SetWindowPos(window->win32.handle, after, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                     flags);
    }
}

GRWLbool _grwlWindowFocusedWin32(_GRWLwindow* window)
{
    return window->win32.handle == GetForegroundWindow();
}

GRWLbool _grwlWindowIconifiedWin32(_GRWLwindow* window)
{
    return IsIconic(window->win32.handle);
}

GRWLbool _grwlWindowVisibleWin32(_GRWLwindow* window)
{
    return IsWindowVisible(window->win32.handle);
}

GRWLbool _grwlWindowMaximizedWin32(_GRWLwindow* window)
{
    return IsZoomed(window->win32.handle);
}

GRWLbool _grwlWindowHoveredWin32(_GRWLwindow* window)
{
    return cursorInContentArea(window);
}

GRWLbool _grwlFramebufferTransparentWin32(_GRWLwindow* window)
{
    BOOL composition, opaque;
    DWORD color;

    if (!window->win32.transparent)
    {
        return GRWL_FALSE;
    }

    if (!IsWindowsVistaOrGreater())
    {
        return GRWL_FALSE;
    }

    if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
    {
        return GRWL_FALSE;
    }

    if (!IsWindows8OrGreater())
    {
        // HACK: Disable framebuffer transparency on Windows 7 when the
        //       colorization color is opaque, because otherwise the window
        //       contents is blended additively with the previous frame instead
        //       of replacing it
        if (FAILED(DwmGetColorizationColor(&color, &opaque)) || opaque)
        {
            return GRWL_FALSE;
        }
    }

    return GRWL_TRUE;
}

void _grwlSetWindowResizableWin32(_GRWLwindow* window, GRWLbool enabled)
{
    updateWindowStyles(window);
}

void _grwlSetWindowDecoratedWin32(_GRWLwindow* window, GRWLbool enabled)
{
    updateWindowStyles(window);
}

void _grwlSetWindowFloatingWin32(_GRWLwindow* window, GRWLbool enabled)
{
    const HWND after = enabled ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(window->win32.handle, after, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

void _grwlSetWindowMousePassthroughWin32(_GRWLwindow* window, GRWLbool enabled)
{
    COLORREF key = 0;
    BYTE alpha = 0;
    DWORD flags = 0;
    DWORD exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);

    if (exStyle & WS_EX_LAYERED)
    {
        GetLayeredWindowAttributes(window->win32.handle, &key, &alpha, &flags);
    }

    if (enabled)
    {
        exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
    }
    else
    {
        exStyle &= ~WS_EX_TRANSPARENT;
        // NOTE: Window opacity also needs the layered window style so do not
        //       remove it if the window is alpha blended
        if (exStyle & WS_EX_LAYERED)
        {
            if (!(flags & LWA_ALPHA))
            {
                exStyle &= ~WS_EX_LAYERED;
            }
        }
    }

    SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);

    if (enabled)
    {
        SetLayeredWindowAttributes(window->win32.handle, key, alpha, flags);
    }
}

float _grwlGetWindowOpacityWin32(_GRWLwindow* window)
{
    BYTE alpha;
    DWORD flags;

    if ((GetWindowLongW(window->win32.handle, GWL_EXSTYLE) & WS_EX_LAYERED) &&
        GetLayeredWindowAttributes(window->win32.handle, NULL, &alpha, &flags))
    {
        if (flags & LWA_ALPHA)
        {
            return alpha / 255.f;
        }
    }

    return 1.f;
}

void _grwlSetWindowOpacityWin32(_GRWLwindow* window, float opacity)
{
    LONG exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);
    if (opacity < 1.f || (exStyle & WS_EX_TRANSPARENT))
    {
        const BYTE alpha = (BYTE)(255 * opacity);
        exStyle |= WS_EX_LAYERED;
        SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);
        SetLayeredWindowAttributes(window->win32.handle, 0, alpha, LWA_ALPHA);
    }
    else if (exStyle & WS_EX_TRANSPARENT)
    {
        SetLayeredWindowAttributes(window->win32.handle, 0, 0, 0);
    }
    else
    {
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);
    }
}

void _grwlSetRawMouseMotionWin32(_GRWLwindow* window, GRWLbool enabled)
{
    if (_grwl.win32.disabledCursorWindow != window)
    {
        return;
    }

    if (enabled)
    {
        enableRawMouseMotion(window);
    }
    else
    {
        disableRawMouseMotion(window);
    }
}

GRWLbool _grwlRawMouseMotionSupportedWin32(void)
{
    return GRWL_TRUE;
}

void _grwlPollEventsWin32(void)
{
    MSG msg;
    HWND handle;
    _GRWLwindow* window;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // NOTE: While GRWL does not itself post WM_QUIT, other processes
            //       may post it to this one, for example Task Manager
            // HACK: Treat WM_QUIT as a close on all windows

            window = _grwl.windowListHead;
            while (window)
            {
                _grwlInputWindowCloseRequest(window);
                window = window->next;
            }
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // HACK: Release modifier keys that the system did not emit KEYUP for
    // NOTE: Shift keys on Windows tend to "stick" when both are pressed as
    //       no key up message is generated by the first key release
    // NOTE: Windows key is not reported as released by the Win+V hotkey
    //       Other Win hotkeys are handled implicitly by _grwlInputWindowFocus
    //       because they change the input focus
    // NOTE: The other half of this is in the WM_*KEY* handler in windowProc
    handle = GetActiveWindow();
    if (handle)
    {
        window = GetPropW(handle, L"GRWL");
        if (window)
        {
            int i;
            const int keys[4][2] = { { VK_LSHIFT, GRWL_KEY_LEFT_SHIFT },
                                     { VK_RSHIFT, GRWL_KEY_RIGHT_SHIFT },
                                     { VK_LWIN, GRWL_KEY_LEFT_SUPER },
                                     { VK_RWIN, GRWL_KEY_RIGHT_SUPER } };

            for (i = 0; i < 4; i++)
            {
                const int vk = keys[i][0];
                const int key = keys[i][1];
                const int scancode = _grwl.win32.scancodes[key];

                if ((GetKeyState(vk) & 0x8000))
                {
                    continue;
                }
                if (window->keys[key] != GRWL_PRESS)
                {
                    continue;
                }

                _grwlInputKey(window, key, scancode, GRWL_RELEASE, getKeyMods());
            }
        }
    }

    window = _grwl.win32.disabledCursorWindow;
    if (window)
    {
        int width, height;
        _grwlGetWindowSizeWin32(window, &width, &height);

        // NOTE: Re-center the cursor only if it has moved since the last call,
        //       to avoid breaking grwlWaitEvents with WM_MOUSEMOVE
        if (window->win32.lastCursorPosX != width / 2 || window->win32.lastCursorPosY != height / 2)
        {
            _grwlSetCursorPosWin32(window, width / 2, height / 2);
        }
    }
}

void _grwlWaitEventsWin32(void)
{
    WaitMessage();

    _grwlPollEventsWin32();
}

void _grwlWaitEventsTimeoutWin32(double timeout)
{
    MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)(timeout * 1e3), QS_ALLEVENTS);

    _grwlPollEventsWin32();
}

void _grwlPostEmptyEventWin32(void)
{
    PostMessageW(_grwl.win32.helperWindowHandle, WM_NULL, 0, 0);
}

void _grwlGetCursorPosWin32(_GRWLwindow* window, double* xpos, double* ypos)
{
    POINT pos;

    if (GetCursorPos(&pos))
    {
        ScreenToClient(window->win32.handle, &pos);

        if (xpos)
        {
            *xpos = pos.x;
        }
        if (ypos)
        {
            *ypos = pos.y;
        }
    }
}

void _grwlSetCursorPosWin32(_GRWLwindow* window, double xpos, double ypos)
{
    POINT pos = { (int)xpos, (int)ypos };

    // Store the new position so it can be recognized later
    window->win32.lastCursorPosX = pos.x;
    window->win32.lastCursorPosY = pos.y;

    ClientToScreen(window->win32.handle, &pos);
    SetCursorPos(pos.x, pos.y);
}

void _grwlSetCursorModeWin32(_GRWLwindow* window, int mode)
{
    if (_grwlWindowFocusedWin32(window))
    {
        if (mode == GRWL_CURSOR_DISABLED)
        {
            _grwlGetCursorPosWin32(window, &_grwl.win32.restoreCursorPosX, &_grwl.win32.restoreCursorPosY);
            _grwlCenterCursorInContentArea(window);
            if (window->rawMouseMotion)
            {
                enableRawMouseMotion(window);
            }
        }
        else if (_grwl.win32.disabledCursorWindow == window)
        {
            if (window->rawMouseMotion)
            {
                disableRawMouseMotion(window);
            }
        }

        if (mode == GRWL_CURSOR_DISABLED || mode == GRWL_CURSOR_CAPTURED)
        {
            captureCursor(window);
        }
        else
        {
            releaseCursor();
        }

        if (mode == GRWL_CURSOR_DISABLED)
        {
            _grwl.win32.disabledCursorWindow = window;
        }
        else if (_grwl.win32.disabledCursorWindow == window)
        {
            _grwl.win32.disabledCursorWindow = NULL;
            _grwlSetCursorPosWin32(window, _grwl.win32.restoreCursorPosX, _grwl.win32.restoreCursorPosY);
        }
    }

    if (cursorInContentArea(window))
    {
        updateCursorImage(window);
    }
}

const char* _grwlGetScancodeNameWin32(int scancode)
{
    if (scancode < 0 || scancode > (KF_EXTENDED | 0xff) || _grwl.win32.keycodes[scancode] == GRWL_KEY_UNKNOWN)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    return _grwl.win32.keynames[_grwl.win32.keycodes[scancode]];
}

int _grwlGetKeyScancodeWin32(int key)
{
    return _grwl.win32.scancodes[key];
}

const char* _grwlGetKeyboardLayoutNameWin32(void)
{
    WCHAR klid[KL_NAMELENGTH];
    int size;
    LCID lcid;
    WCHAR* language;

    if (!GetKeyboardLayoutNameW(klid))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to retrieve keyboard layout name");
        return NULL;
    }

    // NOTE: We only care about the language part of the keyboard layout ID
    lcid = MAKELCID(LANGIDFROMLCID(wcstoul(klid, NULL, 16)), 0);

    size = GetLocaleInfoW(lcid, LOCALE_SLANGUAGE, NULL, 0);
    if (!size)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to retrieve keyboard layout name length");
        return NULL;
    }

    language = calloc(size, sizeof(WCHAR));

    if (!GetLocaleInfoW(lcid, LOCALE_SLANGUAGE, language, size))
    {
        free(language);
        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to translate keyboard layout name");
        return NULL;
    }

    free(_grwl.win32.keyboardLayoutName);
    _grwl.win32.keyboardLayoutName = _grwlCreateUTF8FromWideStringWin32(language);
    free(language);

    return _grwl.win32.keyboardLayoutName;
}

GRWLbool _grwlCreateCursorWin32(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot)
{
    cursor->win32.handle = (HCURSOR)createIcon(image, xhot, yhot, GRWL_FALSE);
    if (!cursor->win32.handle)
    {
        return GRWL_FALSE;
    }

    return GRWL_TRUE;
}

GRWLbool _grwlCreateStandardCursorWin32(_GRWLcursor* cursor, int shape)
{
    int id = 0;

    switch (shape)
    {
        case GRWL_ARROW_CURSOR:
            id = OCR_NORMAL;
            break;
        case GRWL_IBEAM_CURSOR:
            id = OCR_IBEAM;
            break;
        case GRWL_CROSSHAIR_CURSOR:
            id = OCR_CROSS;
            break;
        case GRWL_POINTING_HAND_CURSOR:
            id = OCR_HAND;
            break;
        case GRWL_RESIZE_EW_CURSOR:
            id = OCR_SIZEWE;
            break;
        case GRWL_RESIZE_NS_CURSOR:
            id = OCR_SIZENS;
            break;
        case GRWL_RESIZE_NWSE_CURSOR:
            id = OCR_SIZENWSE;
            break;
        case GRWL_RESIZE_NESW_CURSOR:
            id = OCR_SIZENESW;
            break;
        case GRWL_RESIZE_ALL_CURSOR:
            id = OCR_SIZEALL;
            break;
        case GRWL_NOT_ALLOWED_CURSOR:
            id = OCR_NO;
            break;
        default:
            _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Unknown standard cursor");
            return GRWL_FALSE;
    }

    cursor->win32.handle = LoadImageW(NULL, MAKEINTRESOURCEW(id), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    if (!cursor->win32.handle)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to create standard cursor");
        return GRWL_FALSE;
    }

    return GRWL_TRUE;
}

void _grwlDestroyCursorWin32(_GRWLcursor* cursor)
{
    if (cursor->win32.handle)
    {
        DestroyIcon((HICON)cursor->win32.handle);
    }
}

void _grwlSetCursorWin32(_GRWLwindow* window, _GRWLcursor* cursor)
{
    if (cursorInContentArea(window))
    {
        updateCursorImage(window);
    }
}

void _grwlSetClipboardStringWin32(const char* string)
{
    int characterCount;
    HANDLE object;
    WCHAR* buffer;

    characterCount = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
    if (!characterCount)
    {
        return;
    }

    object = GlobalAlloc(GMEM_MOVEABLE, characterCount * sizeof(WCHAR));
    if (!object)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to allocate global handle for clipboard");
        return;
    }

    buffer = GlobalLock(object);
    if (!buffer)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to lock global handle");
        GlobalFree(object);
        return;
    }

    MultiByteToWideChar(CP_UTF8, 0, string, -1, buffer, characterCount);
    GlobalUnlock(object);

    if (!OpenClipboard(_grwl.win32.helperWindowHandle))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to open clipboard");
        GlobalFree(object);
        return;
    }

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, object);
    CloseClipboard();
}

const char* _grwlGetClipboardStringWin32(void)
{
    HANDLE object;
    WCHAR* buffer;

    if (!OpenClipboard(_grwl.win32.helperWindowHandle))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to open clipboard");
        return NULL;
    }

    object = GetClipboardData(CF_UNICODETEXT);
    if (!object)
    {
        _grwlInputErrorWin32(GRWL_FORMAT_UNAVAILABLE, "Win32: Failed to convert clipboard to string");
        CloseClipboard();
        return NULL;
    }

    buffer = GlobalLock(object);
    if (!buffer)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "Win32: Failed to lock global handle");
        CloseClipboard();
        return NULL;
    }

    _grwl_free(_grwl.win32.clipboardString);
    _grwl.win32.clipboardString = _grwlCreateUTF8FromWideStringWin32(buffer);

    GlobalUnlock(object);
    CloseClipboard();

    return _grwl.win32.clipboardString;
}

void _grwlUpdatePreeditCursorRectangleWin32(_GRWLwindow* window)
{
    _GRWLpreedit* preedit = &window->preedit;
    HWND hWnd = window->win32.handle;
    HIMC hIMC = ImmGetContext(hWnd);

    int x = preedit->cursorPosX;
    int y = preedit->cursorPosY;
    int w = preedit->cursorWidth;
    int h = preedit->cursorHeight;
    CANDIDATEFORM excludeRect = { 0, CFS_EXCLUDE, { x, y }, { x, y, x + w, y + h } };

    ImmSetCandidateWindow(hIMC, &excludeRect);

    ImmReleaseContext(hWnd, hIMC);
}

void _grwlResetPreeditTextWin32(_GRWLwindow* window)
{
    HWND hWnd = window->win32.handle;
    HIMC hIMC = ImmGetContext(hWnd);
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    ImmReleaseContext(hWnd, hIMC);
}

void _grwlSetIMEStatusWin32(_GRWLwindow* window, int active)
{
    HWND hWnd = window->win32.handle;
    HIMC hIMC = ImmGetContext(hWnd);
    ImmSetOpenStatus(hIMC, active ? TRUE : FALSE);
    ImmReleaseContext(hWnd, hIMC);
}

int _grwlGetIMEStatusWin32(_GRWLwindow* window)
{
    HWND hWnd = window->win32.handle;
    HIMC hIMC = ImmGetContext(hWnd);
    BOOL result = ImmGetOpenStatus(hIMC);
    ImmReleaseContext(hWnd, hIMC);
    return result ? GRWL_TRUE : GRWL_FALSE;
}

EGLenum _grwlGetEGLPlatformWin32(EGLint** attribs)
{
    if (_grwl.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_grwl.egl.ANGLE_platform_angle_opengl)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_OPENGL)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
            }
            else if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_OPENGLES)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE;
            }
        }

        if (_grwl.egl.ANGLE_platform_angle_d3d)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_D3D9)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
            }
            else if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_D3D11)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
            }
        }

        if (_grwl.egl.ANGLE_platform_angle_vulkan)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_VULKAN)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
            }
        }

        if (type)
        {
            *attribs = _grwl_calloc(3, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    return 0;
}

EGLNativeDisplayType _grwlGetEGLNativeDisplayWin32(void)
{
    return GetDC(_grwl.win32.helperWindowHandle);
}

EGLNativeWindowType _grwlGetEGLNativeWindowWin32(_GRWLwindow* window)
{
    return window->win32.handle;
}

void _grwlGetRequiredInstanceExtensionsWin32(char** extensions)
{
    if (!_grwl.vk.KHR_surface || !_grwl.vk.KHR_win32_surface)
    {
        return;
    }

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_win32_surface";
}

GRWLbool _grwlGetPhysicalDevicePresentationSupportWin32(VkInstance instance, VkPhysicalDevice device,
                                                        uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(
            instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    if (!vkGetPhysicalDeviceWin32PresentationSupportKHR)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Win32: Vulkan instance missing VK_KHR_win32_surface extension");
        return GRWL_FALSE;
    }

    return vkGetPhysicalDeviceWin32PresentationSupportKHR(device, queuefamily);
}

VkResult _grwlCreateWindowSurfaceWin32(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR* surface)
{
    VkResult err;
    VkWin32SurfaceCreateInfoKHR sci;
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!vkCreateWin32SurfaceKHR)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Win32: Vulkan instance missing VK_KHR_win32_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = _grwl.win32.instance;
    sci.hwnd = window->win32.handle;

    err = vkCreateWin32SurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Win32: Failed to create Vulkan surface: %s",
                        _grwlGetVulkanResultString(err));
    }

    return err;
}

_GRWLusercontext* _grwlCreateUserContextWin32(_GRWLwindow* window)
{
    if (window->context.wgl.handle)
    {
        return _grwlCreateUserContextWGL(window);
    }
    else if (window->context.egl.handle)
    {
        return _grwlCreateUserContextEGL(window);
    }
    else if (window->context.osmesa.handle)
    {
        return _grwlCreateUserContextOSMesa(window);
    }

    return GRWL_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI HWND grwlGetWin32Window(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (_grwl.platform.platformID != GRWL_PLATFORM_WIN32)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "Win32: Platform not initialized");
        return NULL;
    }

    return window->win32.handle;
}

#endif // _GRWL_WIN32
