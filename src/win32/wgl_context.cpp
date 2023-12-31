//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_WIN32)

    #include <cstdlib>
    #include <cassert>

// Return the value corresponding to the specified attribute
//
static int findPixelFormatAttribValueWGL(const int* attribs, int attribCount, const int* values, int attrib)
{
    for (int i = 0; i < attribCount; i++)
    {
        if (attribs[i] == attrib)
        {
            return values[i];
        }
    }

    _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Unknown pixel format attribute requested");
    return 0;
}

    #define ADD_ATTRIB(a)                                                       \
        {                                                                       \
            assert((size_t)attribCount < sizeof(attribs) / sizeof(attribs[0])); \
            attribs[attribCount++] = a;                                         \
        }
    #define FIND_ATTRIB_VALUE(a) findPixelFormatAttribValueWGL(attribs, attribCount, values, a)

// Return a list of available and usable framebuffer configs
//
static int choosePixelFormatWGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig)
{
    _GRWLfbconfig* usableConfigs;
    const _GRWLfbconfig* closest;
    int pixelFormat, nativeCount, usableCount = 0, attribCount = 0;
    int attribs[40];
    int values[sizeof(attribs) / sizeof(attribs[0])];

    if (_grwl.wgl.ARB_pixel_format)
    {
        int attrib[] = { WGL_NUMBER_PIXEL_FORMATS_ARB };

        wglGetPixelFormatAttribivARB(window->context.wgl.dc, 1, 0, 1, attrib, &nativeCount);
    }
    else
    {
        nativeCount = DescribePixelFormat(window->context.wgl.dc, 1, sizeof(PIXELFORMATDESCRIPTOR), nullptr);
    }

    if (_grwl.wgl.ARB_pixel_format)
    {
        ADD_ATTRIB(WGL_SUPPORT_OPENGL_ARB);
        ADD_ATTRIB(WGL_DRAW_TO_WINDOW_ARB);
        ADD_ATTRIB(WGL_PIXEL_TYPE_ARB);
        ADD_ATTRIB(WGL_ACCELERATION_ARB);
        ADD_ATTRIB(WGL_RED_BITS_ARB);
        ADD_ATTRIB(WGL_RED_SHIFT_ARB);
        ADD_ATTRIB(WGL_GREEN_BITS_ARB);
        ADD_ATTRIB(WGL_GREEN_SHIFT_ARB);
        ADD_ATTRIB(WGL_BLUE_BITS_ARB);
        ADD_ATTRIB(WGL_BLUE_SHIFT_ARB);
        ADD_ATTRIB(WGL_ALPHA_BITS_ARB);
        ADD_ATTRIB(WGL_ALPHA_SHIFT_ARB);
        ADD_ATTRIB(WGL_DEPTH_BITS_ARB);
        ADD_ATTRIB(WGL_STENCIL_BITS_ARB);
        ADD_ATTRIB(WGL_ACCUM_BITS_ARB);
        ADD_ATTRIB(WGL_ACCUM_RED_BITS_ARB);
        ADD_ATTRIB(WGL_ACCUM_GREEN_BITS_ARB);
        ADD_ATTRIB(WGL_ACCUM_BLUE_BITS_ARB);
        ADD_ATTRIB(WGL_ACCUM_ALPHA_BITS_ARB);
        ADD_ATTRIB(WGL_AUX_BUFFERS_ARB);
        ADD_ATTRIB(WGL_STEREO_ARB);
        ADD_ATTRIB(WGL_DOUBLE_BUFFER_ARB);

        if (_grwl.wgl.ARB_multisample)
        {
            ADD_ATTRIB(WGL_SAMPLES_ARB);
        }

        if (ctxconfig->client == GRWL_OPENGL_API)
        {
            if (_grwl.wgl.ARB_framebuffer_sRGB || _grwl.wgl.EXT_framebuffer_sRGB)
            {
                ADD_ATTRIB(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
            }
        }
        else
        {
            if (_grwl.wgl.EXT_colorspace)
            {
                ADD_ATTRIB(WGL_COLORSPACE_EXT);
            }
        }
    }

    usableConfigs = (_GRWLfbconfig*)_grwl_calloc(nativeCount, sizeof(_GRWLfbconfig));

    for (int i = 0; i < nativeCount; i++)
    {
        _GRWLfbconfig* u = usableConfigs + usableCount;
        pixelFormat = i + 1;

        if (_grwl.wgl.ARB_pixel_format)
        {
            // Get pixel format attributes through "modern" extension

            if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc, pixelFormat, 0, attribCount, attribs, values))
            {
                _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to retrieve pixel format attributes");

                _grwl_free(usableConfigs);
                return 0;
            }

            if (!FIND_ATTRIB_VALUE(WGL_SUPPORT_OPENGL_ARB) || !FIND_ATTRIB_VALUE(WGL_DRAW_TO_WINDOW_ARB))
            {
                continue;
            }

            if (FIND_ATTRIB_VALUE(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB)
            {
                continue;
            }

            if (FIND_ATTRIB_VALUE(WGL_ACCELERATION_ARB) == WGL_NO_ACCELERATION_ARB)
            {
                continue;
            }

            if (FIND_ATTRIB_VALUE(WGL_DOUBLE_BUFFER_ARB) != (int)fbconfig->doublebuffer)
            {
                continue;
            }

            u->redBits = FIND_ATTRIB_VALUE(WGL_RED_BITS_ARB);
            u->greenBits = FIND_ATTRIB_VALUE(WGL_GREEN_BITS_ARB);
            u->blueBits = FIND_ATTRIB_VALUE(WGL_BLUE_BITS_ARB);
            u->alphaBits = FIND_ATTRIB_VALUE(WGL_ALPHA_BITS_ARB);

            u->depthBits = FIND_ATTRIB_VALUE(WGL_DEPTH_BITS_ARB);
            u->stencilBits = FIND_ATTRIB_VALUE(WGL_STENCIL_BITS_ARB);

            u->accumRedBits = FIND_ATTRIB_VALUE(WGL_ACCUM_RED_BITS_ARB);
            u->accumGreenBits = FIND_ATTRIB_VALUE(WGL_ACCUM_GREEN_BITS_ARB);
            u->accumBlueBits = FIND_ATTRIB_VALUE(WGL_ACCUM_BLUE_BITS_ARB);
            u->accumAlphaBits = FIND_ATTRIB_VALUE(WGL_ACCUM_ALPHA_BITS_ARB);

            u->auxBuffers = FIND_ATTRIB_VALUE(WGL_AUX_BUFFERS_ARB);

            if (FIND_ATTRIB_VALUE(WGL_STEREO_ARB))
            {
                u->stereo = true;
            }

            if (_grwl.wgl.ARB_multisample)
            {
                u->samples = FIND_ATTRIB_VALUE(WGL_SAMPLES_ARB);
            }

            if (ctxconfig->client == GRWL_OPENGL_API)
            {
                if (_grwl.wgl.ARB_framebuffer_sRGB || _grwl.wgl.EXT_framebuffer_sRGB)
                {
                    if (FIND_ATTRIB_VALUE(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB))
                    {
                        u->sRGB = true;
                    }
                }
            }
            else
            {
                if (_grwl.wgl.EXT_colorspace)
                {
                    if (FIND_ATTRIB_VALUE(WGL_COLORSPACE_EXT) == WGL_COLORSPACE_SRGB_EXT)
                    {
                        u->sRGB = true;
                    }
                }
            }
        }
        else
        {
            // Get pixel format attributes through legacy PFDs

            PIXELFORMATDESCRIPTOR pfd;

            if (!DescribePixelFormat(window->context.wgl.dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
            {
                _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to describe pixel format");

                _grwl_free(usableConfigs);
                return 0;
            }

            if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) || !(pfd.dwFlags & PFD_SUPPORT_OPENGL))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
            {
                continue;
            }

            if (pfd.iPixelType != PFD_TYPE_RGBA)
            {
                continue;
            }

            if (!!(pfd.dwFlags & PFD_DOUBLEBUFFER) != fbconfig->doublebuffer)
            {
                continue;
            }

            u->redBits = pfd.cRedBits;
            u->greenBits = pfd.cGreenBits;
            u->blueBits = pfd.cBlueBits;
            u->alphaBits = pfd.cAlphaBits;

            u->depthBits = pfd.cDepthBits;
            u->stencilBits = pfd.cStencilBits;

            u->accumRedBits = pfd.cAccumRedBits;
            u->accumGreenBits = pfd.cAccumGreenBits;
            u->accumBlueBits = pfd.cAccumBlueBits;
            u->accumAlphaBits = pfd.cAccumAlphaBits;

            u->auxBuffers = pfd.cAuxBuffers;

            if (pfd.dwFlags & PFD_STEREO)
            {
                u->stereo = true;
            }
        }

        u->handle = pixelFormat;
        usableCount++;
    }

    if (!usableCount)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "WGL: The driver does not appear to support OpenGL");

        _grwl_free(usableConfigs);
        return 0;
    }

    closest = _grwlChooseFBConfig(fbconfig, usableConfigs, usableCount);
    if (!closest)
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "WGL: Failed to find a suitable pixel format");

        _grwl_free(usableConfigs);
        return 0;
    }

    pixelFormat = (int)closest->handle;
    _grwl_free(usableConfigs);

    return pixelFormat;
}

    #undef ADD_ATTRIB
    #undef FIND_ATTRIB_VALUE

static void makeContextCurrentWGL(_GRWLwindow* window)
{
    if (window)
    {
        if (wglMakeCurrent(window->context.wgl.dc, window->context.wgl.handle))
        {
            _grwlPlatformSetTls(&_grwl.contextSlot, window);
        }
        else
        {
            _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to make context current");
            _grwlPlatformSetTls(&_grwl.contextSlot, nullptr);
        }
    }
    else
    {
        if (!wglMakeCurrent(nullptr, nullptr))
        {
            _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to clear current context");
        }

        _grwlPlatformSetTls(&_grwl.contextSlot, nullptr);
    }
}

static void swapBuffersWGL(_GRWLwindow* window)
{
    if (!window->monitor)
    {
        // HACK: Use DwmFlush when desktop composition is enabled on Windows Vista and 7
        if (!IsWindows8OrGreater() && IsWindowsVistaOrGreater())
        {
            BOOL enabled = FALSE;

            if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
            {
                int count = abs(window->context.wgl.interval);
                while (count--)
                {
                    DwmFlush();
                }
            }
        }
    }

    SwapBuffers(window->context.wgl.dc);
}

static void swapIntervalWGL(int interval)
{
    _GRWLwindow* window = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);

    window->context.wgl.interval = interval;

    if (!window->monitor)
    {
        // HACK: Disable WGL swap interval when desktop composition is enabled on Windows
        //       Vista and 7 to avoid interfering with DWM vsync
        if (!IsWindows8OrGreater() && IsWindowsVistaOrGreater())
        {
            BOOL enabled = FALSE;

            if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
            {
                interval = 0;
            }
        }
    }

    if (_grwl.wgl.EXT_swap_control)
    {
        wglSwapIntervalEXT(interval);
    }
}

static int extensionSupportedWGL(const char* extension)
{
    const char* extensions = nullptr;

    if (_grwl.wgl.GetExtensionsStringARB)
    {
        extensions = wglGetExtensionsStringARB(wglGetCurrentDC());
    }
    else if (_grwl.wgl.GetExtensionsStringEXT)
    {
        extensions = wglGetExtensionsStringEXT();
    }

    if (!extensions)
    {
        return false;
    }

    return _grwlStringInExtensionString(extension, extensions);
}

static GRWLglproc getProcAddressWGL(const char* procname)
{
    const GRWLglproc proc = (GRWLglproc)wglGetProcAddress(procname);
    if (proc)
    {
        return proc;
    }

    return (GRWLglproc)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, procname);
}

static void destroyContextWGL(_GRWLwindow* window)
{
    if (window->context.wgl.handle)
    {
        wglDeleteContext(window->context.wgl.handle);
        window->context.wgl.handle = nullptr;
    }
}

// Initialize WGL
//
bool _grwlInitWGL()
{
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC prc, rc;
    HDC pdc, dc;

    if (_grwl.wgl.instance)
    {
        return true;
    }

    _grwl.wgl.instance = (HINSTANCE)_grwlPlatformLoadModule("opengl32.dll");
    if (!_grwl.wgl.instance)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to load opengl32.dll");
        return false;
    }

    _grwl.wgl.CreateContext =
        (PFN_wglCreateContext)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglCreateContext");
    _grwl.wgl.DeleteContext =
        (PFN_wglDeleteContext)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglDeleteContext");
    _grwl.wgl.GetProcAddress =
        (PFN_wglGetProcAddress)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglGetProcAddress");
    _grwl.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglGetCurrentDC");
    _grwl.wgl.GetCurrentContext =
        (PFN_wglGetCurrentContext)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglGetCurrentContext");
    _grwl.wgl.MakeCurrent = (PFN_wglMakeCurrent)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglMakeCurrent");
    _grwl.wgl.ShareLists = (PFN_wglShareLists)_grwlPlatformGetModuleSymbol(_grwl.wgl.instance, "wglShareLists");

    // NOTE: A dummy context has to be created for opengl32.dll to load the
    //       OpenGL ICD, from which we can then query WGL extensions
    // NOTE: This code will accept the Microsoft GDI ICD; accelerated context
    //       creation failure occurs during manual pixel format enumeration

    dc = GetDC(_grwl.win32.helperWindowHandle);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    if (!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to set pixel format for dummy context");
        return false;
    }

    rc = wglCreateContext(dc);
    if (!rc)
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to create dummy context");
        return false;
    }

    pdc = wglGetCurrentDC();
    prc = wglGetCurrentContext();

    if (!wglMakeCurrent(dc, rc))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to make dummy context current");
        wglMakeCurrent(pdc, prc);
        wglDeleteContext(rc);
        return false;
    }

    // NOTE: Functions must be loaded first as they're needed to retrieve the
    //       extension string that tells us whether the functions are supported
    _grwl.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    _grwl.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    _grwl.wgl.CreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    _grwl.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    _grwl.wgl.GetPixelFormatAttribivARB =
        (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");

    // NOTE: WGL_ARB_extensions_string and WGL_EXT_extensions_string are not
    //       checked below as we are already using them
    _grwl.wgl.ARB_multisample = extensionSupportedWGL("WGL_ARB_multisample");
    _grwl.wgl.ARB_framebuffer_sRGB = extensionSupportedWGL("WGL_ARB_framebuffer_sRGB");
    _grwl.wgl.EXT_framebuffer_sRGB = extensionSupportedWGL("WGL_EXT_framebuffer_sRGB");
    _grwl.wgl.ARB_create_context = extensionSupportedWGL("WGL_ARB_create_context");
    _grwl.wgl.ARB_create_context_profile = extensionSupportedWGL("WGL_ARB_create_context_profile");
    _grwl.wgl.EXT_create_context_es2_profile = extensionSupportedWGL("WGL_EXT_create_context_es2_profile");
    _grwl.wgl.ARB_create_context_robustness = extensionSupportedWGL("WGL_ARB_create_context_robustness");
    _grwl.wgl.ARB_create_context_no_error = extensionSupportedWGL("WGL_ARB_create_context_no_error");
    _grwl.wgl.EXT_swap_control = extensionSupportedWGL("WGL_EXT_swap_control");
    _grwl.wgl.EXT_colorspace = extensionSupportedWGL("WGL_EXT_colorspace");
    _grwl.wgl.ARB_pixel_format = extensionSupportedWGL("WGL_ARB_pixel_format");
    _grwl.wgl.ARB_context_flush_control = extensionSupportedWGL("WGL_ARB_context_flush_control");

    wglMakeCurrent(pdc, prc);
    wglDeleteContext(rc);
    return true;
}

// Terminate WGL
//
void _grwlTerminateWGL()
{
    if (_grwl.wgl.instance)
    {
        _grwlPlatformFreeModule(_grwl.wgl.instance);
    }
}

    #define SET_ATTRIB(a, v)                                                    \
        {                                                                       \
            assert(((size_t)index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
            attribs[index++] = a;                                               \
            attribs[index++] = v;                                               \
        }

// Create the OpenGL or OpenGL ES context for the given HDC
//
bool _grwlCreateContextForDCWGL(HDC dc, const _GRWLctxconfig* ctxconfig, HGLRC* context)
{
    int attribs[40];
    HGLRC share = nullptr;

    *context = nullptr;
    if (ctxconfig->share)
    {
        share = ctxconfig->share->context.wgl.handle;
    }

    if (ctxconfig->client == GRWL_OPENGL_API)
    {
        if (ctxconfig->forward)
        {
            if (!_grwl.wgl.ARB_create_context)
            {
                _grwlInputError(
                    GRWL_VERSION_UNAVAILABLE,
                    "WGL: A forward compatible OpenGL context requested but WGL_ARB_create_context is unavailable");
                return false;
            }
        }

        if (ctxconfig->profile)
        {
            if (!_grwl.wgl.ARB_create_context_profile)
            {
                _grwlInputError(GRWL_VERSION_UNAVAILABLE,
                                "WGL: OpenGL profile requested but WGL_ARB_create_context_profile is unavailable");
                return false;
            }
        }
    }
    else
    {
        if (!_grwl.wgl.ARB_create_context || !_grwl.wgl.ARB_create_context_profile ||
            !_grwl.wgl.EXT_create_context_es2_profile)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE,
                            "WGL: OpenGL ES requested but WGL_ARB_create_context_es2_profile is unavailable");
            return false;
        }
    }

    if (_grwl.wgl.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == GRWL_OPENGL_API)
        {
            if (ctxconfig->forward)
            {
                flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            }

            if (ctxconfig->profile == GRWL_OPENGL_CORE_PROFILE)
            {
                mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            }
            else if (ctxconfig->profile == GRWL_OPENGL_COMPAT_PROFILE)
            {
                mask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            }
        }
        else
        {
            mask |= WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
        }

        if (ctxconfig->debug)
        {
            flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
        }

        if (ctxconfig->robustness)
        {
            if (_grwl.wgl.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == GRWL_NO_RESET_NOTIFICATION)
                {
                    SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, WGL_NO_RESET_NOTIFICATION_ARB);
                }
                else if (ctxconfig->robustness == GRWL_LOSE_CONTEXT_ON_RESET)
                {
                    SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, WGL_LOSE_CONTEXT_ON_RESET_ARB);
                }

                flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_grwl.wgl.ARB_context_flush_control)
            {
                if (ctxconfig->release == GRWL_RELEASE_BEHAVIOR_NONE)
                {
                    SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB, WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                }
                else if (ctxconfig->release == GRWL_RELEASE_BEHAVIOR_FLUSH)
                {
                    SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB, WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror)
        {
            if (_grwl.wgl.ARB_create_context_no_error)
            {
                SET_ATTRIB(WGL_CONTEXT_OPENGL_NO_ERROR_ARB, true);
            }
        }

        // NOTE: Only request an explicitly versioned context when necessary, as
        //       explicitly requesting version 1.0 does not always return the
        //       highest version supported by the driver
        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            SET_ATTRIB(WGL_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
            SET_ATTRIB(WGL_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
        }

        if (flags)
        {
            SET_ATTRIB(WGL_CONTEXT_FLAGS_ARB, flags);
        }

        if (mask)
        {
            SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, mask);
        }

        SET_ATTRIB(0, 0);

        *context = wglCreateContextAttribsARB(dc, share, attribs);
        if (!(*context))
        {
            const DWORD error = GetLastError();

            if (error == (0xc0070000 | ERROR_INVALID_VERSION_ARB))
            {
                if (ctxconfig->client == GRWL_OPENGL_API)
                {
                    _grwlInputError(GRWL_VERSION_UNAVAILABLE, "WGL: Driver does not support OpenGL version %i.%i",
                                    ctxconfig->major, ctxconfig->minor);
                }
                else
                {
                    _grwlInputError(GRWL_VERSION_UNAVAILABLE, "WGL: Driver does not support OpenGL ES version %i.%i",
                                    ctxconfig->major, ctxconfig->minor);
                }
            }
            else if (error == (0xc0070000 | ERROR_INVALID_PROFILE_ARB))
            {
                _grwlInputError(GRWL_VERSION_UNAVAILABLE, "WGL: Driver does not support the requested OpenGL profile");
            }
            else if (error == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB))
            {
                _grwlInputError(GRWL_INVALID_VALUE,
                                "WGL: The share context is not compatible with the requested context");
            }
            else
            {
                if (ctxconfig->client == GRWL_OPENGL_API)
                {
                    _grwlInputError(GRWL_VERSION_UNAVAILABLE, "WGL: Failed to create OpenGL context");
                }
                else
                {
                    _grwlInputError(GRWL_VERSION_UNAVAILABLE, "WGL: Failed to create OpenGL ES context");
                }
            }

            return false;
        }
    }
    else
    {
        *context = wglCreateContext(dc);
        if (!(*context))
        {
            _grwlInputErrorWin32(GRWL_VERSION_UNAVAILABLE, "WGL: Failed to create OpenGL context");
            return false;
        }

        if (share)
        {
            if (!wglShareLists(share, *context))
            {
                _grwlInputErrorWin32(GRWL_PLATFORM_ERROR,
                                     "WGL: Failed to enable sharing with specified OpenGL context");
                return false;
            }
        }
    }

    return true;
}

// Create the OpenGL or OpenGL ES context
//
bool _grwlCreateContextWGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig)
{
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;

    window->context.wgl.dc = GetDC(window->win32.handle);
    if (!window->context.wgl.dc)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "WGL: Failed to retrieve DC for window");
        return false;
    }

    pixelFormat = choosePixelFormatWGL(window, ctxconfig, fbconfig);
    if (!pixelFormat)
    {
        return false;
    }

    if (!DescribePixelFormat(window->context.wgl.dc, pixelFormat, sizeof(pfd), &pfd))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to retrieve PFD for selected pixel format");
        return false;
    }

    if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to set selected pixel format");
        return false;
    }

    if (!_grwlCreateContextForDCWGL(window->context.wgl.dc, ctxconfig, &window->context.wgl.handle))
    {
        return false;
    }

    window->context.makeCurrent = makeContextCurrentWGL;
    window->context.swapBuffers = swapBuffersWGL;
    window->context.swapInterval = swapIntervalWGL;
    window->context.extensionSupported = extensionSupportedWGL;
    window->context.getProcAddress = getProcAddressWGL;
    window->context.destroy = destroyContextWGL;

    return true;
}

static void _grwlMakeUserContextCurrentWGL(_GRWLusercontext* context)
{
    if (!wglMakeCurrent(context->window->context.wgl.dc, context->wgl.handle))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to make user context current");
        _grwlPlatformSetTls(&_grwl.usercontextSlot, nullptr);
        return;
    }
    _grwlPlatformSetTls(&_grwl.usercontextSlot, context);
}

static void _grwlDestroyUserContextWGL(_GRWLusercontext* context)
{
    wglDeleteContext(context->wgl.handle);
    free(context);
}

_GRWLusercontext* _grwlCreateUserContextWGL(_GRWLwindow* window)
{
    _GRWLusercontext* context;
    _GRWLctxconfig ctxconfig;

    context = (_GRWLusercontext*)calloc(1, sizeof(_GRWLusercontext));
    context->window = window;

    ctxconfig = _grwl.hints.context;
    ctxconfig.share = window;

    if (!_grwlCreateContextForDCWGL(window->context.wgl.dc, &ctxconfig, &context->wgl.handle))
    {
        _grwlInputErrorWin32(GRWL_PLATFORM_ERROR, "WGL: Failed to create user OpenGL context");
        free(context);
        return nullptr;
    }

    context->makeCurrent = _grwlMakeUserContextCurrentWGL;
    context->destroy = _grwlDestroyUserContextWGL;

    return context;
}

    //////////////////////////////////////////////////////////////////////////
    //////                        GRWL native API                       //////
    //////////////////////////////////////////////////////////////////////////
    #undef SET_ATTRIB

GRWLAPI HGLRC grwlGetWGLContext(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    if (_grwl.platform.platformID != GRWL_PLATFORM_WIN32)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "WGL: Platform not initialized");
        return nullptr;
    }

    if (window->context.source != GRWL_NATIVE_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, nullptr);
        return nullptr;
    }

    return window->context.wgl.handle;
}

#endif // _GRWL_WIN32
