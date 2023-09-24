//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// Return a description of the specified EGL error
//
static const char* getEGLErrorString(EGLint error)
{
    switch (error)
    {
        case EGL_SUCCESS:
            return "Success";
        case EGL_NOT_INITIALIZED:
            return "EGL is not or could not be initialized";
        case EGL_BAD_ACCESS:
            return "EGL cannot access a requested resource";
        case EGL_BAD_ALLOC:
            return "EGL failed to allocate resources for the requested operation";
        case EGL_BAD_ATTRIBUTE:
            return "An unrecognized attribute or attribute value was passed in the attribute list";
        case EGL_BAD_CONTEXT:
            return "An EGLContext argument does not name a valid EGL rendering context";
        case EGL_BAD_CONFIG:
            return "An EGLConfig argument does not name a valid EGL frame buffer configuration";
        case EGL_BAD_CURRENT_SURFACE:
            return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid";
        case EGL_BAD_DISPLAY:
            return "An EGLDisplay argument does not name a valid EGL display connection";
        case EGL_BAD_SURFACE:
            return "An EGLSurface argument does not name a valid surface configured for GL rendering";
        case EGL_BAD_MATCH:
            return "Arguments are inconsistent";
        case EGL_BAD_PARAMETER:
            return "One or more argument values are invalid";
        case EGL_BAD_NATIVE_PIXMAP:
            return "A NativePixmapType argument does not refer to a valid native pixmap";
        case EGL_BAD_NATIVE_WINDOW:
            return "A NativeWindowType argument does not refer to a valid native window";
        case EGL_CONTEXT_LOST:
            return "The application must destroy all contexts and reinitialise";
        default:
            return "ERROR: UNKNOWN EGL ERROR";
    }
}

// Returns the specified attribute of the specified EGLConfig
//
static int getEGLConfigAttrib(EGLConfig config, int attrib)
{
    int value;
    eglGetConfigAttrib(_grwl.egl.display, config, attrib, &value);
    return value;
}

// Return the EGLConfig most closely matching the specified hints
//
static bool chooseEGLConfig(const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig, EGLConfig* result)
{
    EGLConfig* nativeConfigs;
    _GRWLfbconfig* usableConfigs;
    const _GRWLfbconfig* closest;
    int nativeCount, usableCount, apiBit;
    bool wrongApiAvailable = false;

    if (ctxconfig->client == GRWL_OPENGL_ES_API)
    {
        if (ctxconfig->major == 1)
        {
            apiBit = EGL_OPENGL_ES_BIT;
        }
        else
        {
            apiBit = EGL_OPENGL_ES2_BIT;
        }
    }
    else
    {
        apiBit = EGL_OPENGL_BIT;
    }

    if (fbconfig->stereo)
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "EGL: Stereo rendering not supported");
        return false;
    }

    eglGetConfigs(_grwl.egl.display, nullptr, 0, &nativeCount);
    if (!nativeCount)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: No EGLConfigs returned");
        return false;
    }

    nativeConfigs = (EGLConfig*)_grwl_calloc(nativeCount, sizeof(EGLConfig));
    eglGetConfigs(_grwl.egl.display, nativeConfigs, nativeCount, &nativeCount);

    usableConfigs = (_GRWLfbconfig*)_grwl_calloc(nativeCount, sizeof(_GRWLfbconfig));
    usableCount = 0;

    for (int i = 0; i < nativeCount; i++)
    {
        const EGLConfig n = nativeConfigs[i];
        _GRWLfbconfig* u = usableConfigs + usableCount;

        // Only consider RGB(A) EGLConfigs
        if (getEGLConfigAttrib(n, EGL_COLOR_BUFFER_TYPE) != EGL_RGB_BUFFER)
        {
            continue;
        }

        // Only consider window EGLConfigs
        if (!(getEGLConfigAttrib(n, EGL_SURFACE_TYPE) & EGL_WINDOW_BIT))
        {
            continue;
        }

#if defined(_GRWL_X11)
        if (_grwl.platform.platformID == GRWL_PLATFORM_X11)
        {
            XVisualInfo vi = { 0 };

            // Only consider EGLConfigs with associated Visuals
            vi.visualid = getEGLConfigAttrib(n, EGL_NATIVE_VISUAL_ID);
            if (!vi.visualid)
            {
                continue;
            }

            if (fbconfig->transparent)
            {
                int count;
                XVisualInfo* vis = XGetVisualInfo(_grwl.x11.display, VisualIDMask, &vi, &count);
                if (vis)
                {
                    u->transparent = _grwlIsVisualTransparentX11(vis[0].visual);
                    XFree(vis);
                }
            }
        }
#endif // _GRWL_X11

        if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & apiBit))
        {
            wrongApiAvailable = true;
            continue;
        }

        u->redBits = getEGLConfigAttrib(n, EGL_RED_SIZE);
        u->greenBits = getEGLConfigAttrib(n, EGL_GREEN_SIZE);
        u->blueBits = getEGLConfigAttrib(n, EGL_BLUE_SIZE);

        u->alphaBits = getEGLConfigAttrib(n, EGL_ALPHA_SIZE);
        u->depthBits = getEGLConfigAttrib(n, EGL_DEPTH_SIZE);
        u->stencilBits = getEGLConfigAttrib(n, EGL_STENCIL_SIZE);

#if defined(_GRWL_WAYLAND)
        if (_grwl.platform.platformID == GRWL_PLATFORM_WAYLAND)
        {
            // NOTE: The wl_surface opaque region is no guarantee that its buffer
            //       is presented as opaque, if it also has an alpha channel
            // HACK: If EGL_EXT_present_opaque is unavailable, ignore any config
            //       with an alpha channel to ensure the buffer is opaque
            if (!_grwl.egl.EXT_present_opaque)
            {
                if (!fbconfig->transparent && u->alphaBits > 0)
                {
                    continue;
                }
            }
        }
#endif // _GRWL_WAYLAND

        u->samples = getEGLConfigAttrib(n, EGL_SAMPLES);
        u->doublebuffer = fbconfig->doublebuffer;

        u->handle = (uintptr_t)n;
        usableCount++;
    }

    closest = _grwlChooseFBConfig(fbconfig, usableConfigs, usableCount);
    if (closest)
    {
        *result = (EGLConfig)closest->handle;
    }
    else
    {
        if (wrongApiAvailable)
        {
            if (ctxconfig->client == GRWL_OPENGL_ES_API)
            {
                if (ctxconfig->major == 1)
                {
                    _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to find support for OpenGL ES 1.x");
                }
                else
                {
                    _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to find support for OpenGL ES 2 or later");
                }
            }
            else
            {
                _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to find support for OpenGL");
            }
        }
        else
        {
            _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "EGL: Failed to find a suitable EGLConfig");
        }
    }

    _grwl_free(nativeConfigs);
    _grwl_free(usableConfigs);

    return closest != nullptr;
}

static void makeContextCurrentEGL(_GRWLwindow* window)
{
    if (window)
    {
        if (!eglMakeCurrent(_grwl.egl.display, window->context.egl.surface, window->context.egl.surface,
                            window->context.egl.handle))
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to make context current: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }
    else
    {
        if (!eglMakeCurrent(_grwl.egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to clear current context: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }

    _grwlPlatformSetTls(&_grwl.contextSlot, window);
}

static void swapBuffersEGL(_GRWLwindow* window)
{
    if (window != _grwlPlatformGetTls(&_grwl.contextSlot))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR,
                        "EGL: The context must be current on the calling thread when swapping buffers");
        return;
    }

#if defined(_GRWL_WAYLAND)
    if (_grwl.platform.platformID == GRWL_PLATFORM_WAYLAND)
    {
        // NOTE: Swapping buffers on a hidden window on Wayland makes it visible
        if (!window->wl.visible)
        {
            return;
        }
    }
#endif

    eglSwapBuffers(_grwl.egl.display, window->context.egl.surface);
}

static void swapIntervalEGL(int interval)
{
    eglSwapInterval(_grwl.egl.display, interval);
}

static int extensionSupportedEGL(const char* extension)
{
    const char* extensions = eglQueryString(_grwl.egl.display, EGL_EXTENSIONS);
    if (extensions)
    {
        if (_grwlStringInExtensionString(extension, extensions))
        {
            return true;
        }
    }

    return false;
}

static GRWLglproc getProcAddressEGL(const char* procname)
{
    _GRWLwindow* window = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);

    if (window && window->context.egl.client)
    {
        GRWLglproc proc = (GRWLglproc)_grwlPlatformGetModuleSymbol(window->context.egl.client, procname);
        if (proc)
        {
            return proc;
        }
    }

    return eglGetProcAddress(procname);
}

static void destroyContextEGL(_GRWLwindow* window)
{
    // NOTE: Do not unload libGL.so.1 while the X11 display is still open,
    //       as it will make XCloseDisplay segfault
    if (_grwl.platform.platformID != GRWL_PLATFORM_X11 || window->context.client != GRWL_OPENGL_API)
    {
        if (window->context.egl.client)
        {
            _grwlPlatformFreeModule(window->context.egl.client);
            window->context.egl.client = nullptr;
        }
    }

    if (window->context.egl.surface)
    {
        eglDestroySurface(_grwl.egl.display, window->context.egl.surface);
        window->context.egl.surface = EGL_NO_SURFACE;
    }

    if (window->context.egl.handle)
    {
        eglDestroyContext(_grwl.egl.display, window->context.egl.handle);
        window->context.egl.handle = EGL_NO_CONTEXT;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize EGL
//
bool _grwlInitEGL()
{
    EGLint* attribs = nullptr;
    const char* extensions;
    const char* sonames[] = {
#if defined(_GRWL_EGL_LIBRARY)
        _GRWL_EGL_LIBRARY,
#elif defined(_GRWL_WIN32)
        "libEGL.dll",
        "EGL.dll",
#elif defined(_GRWL_COCOA)
        "libEGL.dylib",
#elif defined(__CYGWIN__)
        "libEGL-1.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libEGL.so",
#else
        "libEGL.so.1",
#endif
        nullptr
    };

    if (_grwl.egl.handle)
    {
        return true;
    }
    // todo - Yikes this should probably be handled nicer
    int i;
    for (i = 0; sonames[i]; i++)
    {
        _grwl.egl.handle = _grwlPlatformLoadModule(sonames[i]);
        if (_grwl.egl.handle)
        {
            break;
        }
    }

    if (!_grwl.egl.handle)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Library not found");
        return false;
    }

    _grwl.egl.prefix = (strncmp(sonames[i], "lib", 3) == 0);

    _grwl.egl.GetConfigAttrib =
        (PFN_eglGetConfigAttrib)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglGetConfigAttrib");
    _grwl.egl.GetConfigs = (PFN_eglGetConfigs)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglGetConfigs");
    _grwl.egl.GetDisplay = (PFN_eglGetDisplay)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglGetDisplay");
    _grwl.egl.GetError = (PFN_eglGetError)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglGetError");
    _grwl.egl.Initialize = (PFN_eglInitialize)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglInitialize");
    _grwl.egl.Terminate = (PFN_eglTerminate)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglTerminate");
    _grwl.egl.BindAPI = (PFN_eglBindAPI)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglBindAPI");
    _grwl.egl.CreateContext = (PFN_eglCreateContext)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglCreateContext");
    _grwl.egl.DestroySurface =
        (PFN_eglDestroySurface)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglDestroySurface");
    _grwl.egl.DestroyContext =
        (PFN_eglDestroyContext)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglDestroyContext");
    _grwl.egl.CreateWindowSurface =
        (PFN_eglCreateWindowSurface)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglCreateWindowSurface");
    _grwl.egl.MakeCurrent = (PFN_eglMakeCurrent)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglMakeCurrent");
    _grwl.egl.SwapBuffers = (PFN_eglSwapBuffers)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglSwapBuffers");
    _grwl.egl.SwapInterval = (PFN_eglSwapInterval)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglSwapInterval");
    _grwl.egl.QueryString = (PFN_eglQueryString)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglQueryString");
    _grwl.egl.GetProcAddress =
        (PFN_eglGetProcAddress)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglGetProcAddress");
    _grwl.egl.CreatePbufferSurface =
        (PFN_eglCreatePbufferSurface)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglCreatePbufferSurface");
    _grwl.egl.ChooseConfig = (PFN_eglChooseConfig)_grwlPlatformGetModuleSymbol(_grwl.egl.handle, "eglChooseConfig");

    if (!_grwl.egl.GetConfigAttrib || !_grwl.egl.GetConfigs || !_grwl.egl.GetDisplay || !_grwl.egl.GetError ||
        !_grwl.egl.Initialize || !_grwl.egl.Terminate || !_grwl.egl.BindAPI || !_grwl.egl.CreateContext ||
        !_grwl.egl.DestroySurface || !_grwl.egl.DestroyContext || !_grwl.egl.CreateWindowSurface ||
        !_grwl.egl.MakeCurrent || !_grwl.egl.SwapBuffers || !_grwl.egl.SwapInterval || !_grwl.egl.QueryString ||
        !_grwl.egl.GetProcAddress || !_grwl.egl.CreatePbufferSurface || !_grwl.egl.ChooseConfig)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to load required entry points");

        _grwlTerminateEGL();
        return false;
    }

    extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (extensions && eglGetError() == EGL_SUCCESS)
    {
        _grwl.egl.EXT_client_extensions = true;
    }

    if (_grwl.egl.EXT_client_extensions)
    {
        _grwl.egl.EXT_platform_base = _grwlStringInExtensionString("EGL_EXT_platform_base", extensions);
        _grwl.egl.EXT_platform_x11 = _grwlStringInExtensionString("EGL_EXT_platform_x11", extensions);
        _grwl.egl.EXT_platform_wayland = _grwlStringInExtensionString("EGL_EXT_platform_wayland", extensions);
        _grwl.egl.ANGLE_platform_angle = _grwlStringInExtensionString("EGL_ANGLE_platform_angle", extensions);
        _grwl.egl.ANGLE_platform_angle_opengl =
            _grwlStringInExtensionString("EGL_ANGLE_platform_angle_opengl", extensions);
        _grwl.egl.ANGLE_platform_angle_d3d = _grwlStringInExtensionString("EGL_ANGLE_platform_angle_d3d", extensions);
        _grwl.egl.ANGLE_platform_angle_vulkan =
            _grwlStringInExtensionString("EGL_ANGLE_platform_angle_vulkan", extensions);
        _grwl.egl.ANGLE_platform_angle_metal =
            _grwlStringInExtensionString("EGL_ANGLE_platform_angle_metal", extensions);
    }

    if (_grwl.egl.EXT_platform_base)
    {
        _grwl.egl.GetPlatformDisplayEXT =
            (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
        _grwl.egl.CreatePlatformWindowSurfaceEXT =
            (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
    }

    _grwl.egl.platform = _grwl.platform.getEGLPlatform(&attribs);
    if (_grwl.egl.platform)
    {
        _grwl.egl.display = eglGetPlatformDisplayEXT(_grwl.egl.platform, _grwl.platform.getEGLNativeDisplay(), attribs);
    }
    else
    {
        _grwl.egl.display = eglGetDisplay(_grwl.platform.getEGLNativeDisplay());
    }

    _grwl_free(attribs);

    if (_grwl.egl.display == EGL_NO_DISPLAY)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to get EGL display: %s", getEGLErrorString(eglGetError()));

        _grwlTerminateEGL();
        return false;
    }

    if (!eglInitialize(_grwl.egl.display, &_grwl.egl.major, &_grwl.egl.minor))
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to initialize EGL: %s", getEGLErrorString(eglGetError()));

        _grwlTerminateEGL();
        return false;
    }

    _grwl.egl.KHR_create_context = extensionSupportedEGL("EGL_KHR_create_context");
    _grwl.egl.KHR_create_context_no_error = extensionSupportedEGL("EGL_KHR_create_context_no_error");
    _grwl.egl.KHR_gl_colorspace = extensionSupportedEGL("EGL_KHR_gl_colorspace");
    _grwl.egl.KHR_get_all_proc_addresses = extensionSupportedEGL("EGL_KHR_get_all_proc_addresses");
    _grwl.egl.KHR_context_flush_control = extensionSupportedEGL("EGL_KHR_context_flush_control");
    _grwl.egl.EXT_present_opaque = extensionSupportedEGL("EGL_EXT_present_opaque");

    return true;
}

// Terminate EGL
//
void _grwlTerminateEGL()
{
    if (_grwl.egl.display)
    {
        eglTerminate(_grwl.egl.display);
        _grwl.egl.display = EGL_NO_DISPLAY;
    }

    if (_grwl.egl.handle)
    {
        _grwlPlatformFreeModule(_grwl.egl.handle);
        _grwl.egl.handle = nullptr;
    }
}

#define SET_ATTRIB(a, v)                                                    \
    {                                                                       \
        assert(((size_t)index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
        attribs[index++] = a;                                               \
        attribs[index++] = v;                                               \
    }

// Create the OpenGL or OpenGL ES context for the window eglConfig
//
bool _grwlCreateContextForConfigEGL(EGLConfig eglConfig, const _GRWLctxconfig* ctxconfig, EGLContext* context)
{
    EGLint attribs[40];
    int index = 0;
    EGLContext share = nullptr;

    if (!_grwl.egl.display)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: API not available");
        return false;
    }

    if (ctxconfig->share)
    {
        share = ctxconfig->share->context.egl.handle;
    }

    if (ctxconfig->client == GRWL_OPENGL_ES_API)
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to bind OpenGL ES: %s",
                            getEGLErrorString(eglGetError()));
            return false;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to bind OpenGL: %s", getEGLErrorString(eglGetError()));
            return false;
        }
    }

    if (_grwl.egl.KHR_create_context)
    {
        int mask = 0, flags = 0;

        if (ctxconfig->client == GRWL_OPENGL_API)
        {
            if (ctxconfig->forward)
            {
                flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;
            }

            if (ctxconfig->profile == GRWL_OPENGL_CORE_PROFILE)
            {
                mask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
            }
            else if (ctxconfig->profile == GRWL_OPENGL_COMPAT_PROFILE)
            {
                mask |= EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;
            }
        }

        if (ctxconfig->debug)
        {
            flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
        }

        if (ctxconfig->robustness)
        {
            if (ctxconfig->robustness == GRWL_NO_RESET_NOTIFICATION)
            {
                SET_ATTRIB(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR, EGL_NO_RESET_NOTIFICATION_KHR);
            }
            else if (ctxconfig->robustness == GRWL_LOSE_CONTEXT_ON_RESET)
            {
                SET_ATTRIB(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR, EGL_LOSE_CONTEXT_ON_RESET_KHR);
            }

            flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            SET_ATTRIB(EGL_CONTEXT_MAJOR_VERSION_KHR, ctxconfig->major);
            SET_ATTRIB(EGL_CONTEXT_MINOR_VERSION_KHR, ctxconfig->minor);
        }

        if (ctxconfig->noerror)
        {
            if (_grwl.egl.KHR_create_context_no_error)
            {
                SET_ATTRIB(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, true);
            }
        }

        if (mask)
        {
            SET_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, mask);
        }

        if (flags)
        {
            SET_ATTRIB(EGL_CONTEXT_FLAGS_KHR, flags);
        }
    }
    else
    {
        if (ctxconfig->client == GRWL_OPENGL_ES_API)
        {
            SET_ATTRIB(EGL_CONTEXT_CLIENT_VERSION, ctxconfig->major);
        }
    }

    if (_grwl.egl.KHR_context_flush_control)
    {
        if (ctxconfig->release == GRWL_RELEASE_BEHAVIOR_NONE)
        {
            SET_ATTRIB(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR, EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR);
        }
        else if (ctxconfig->release == GRWL_RELEASE_BEHAVIOR_FLUSH)
        {
            SET_ATTRIB(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR, EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR);
        }
    }

    SET_ATTRIB(EGL_NONE, EGL_NONE);

    *context = eglCreateContext(_grwl.egl.display, eglConfig, share, attribs);

    if (*context == EGL_NO_CONTEXT)
    {
        _grwlInputError(GRWL_VERSION_UNAVAILABLE, "EGL: Failed to create context: %s",
                        getEGLErrorString(eglGetError()));
        return false;
    }

    return true;
}

// Create the OpenGL or OpenGL ES context
//
bool _grwlCreateContextEGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig)
{
    EGLNativeWindowType native;
    EGLint attribs[40];
    int index = 0;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &window->context.egl.config))
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "EGL: Failed to find a suitable EGLConfig");
        return false;
    }

    if (!_grwlCreateContextForConfigEGL(window->context.egl.config, ctxconfig, &window->context.egl.handle))
    {
        return false;
    }

    // Set up attributes for surface creation
    if (fbconfig->sRGB)
    {
        if (_grwl.egl.KHR_gl_colorspace)
        {
            SET_ATTRIB(EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR);
        }
    }

    if (!fbconfig->doublebuffer)
    {
        SET_ATTRIB(EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER);
    }

    if (_grwl.egl.EXT_present_opaque)
    {
        SET_ATTRIB(EGL_PRESENT_OPAQUE_EXT, !fbconfig->transparent);
    }

    SET_ATTRIB(EGL_NONE, EGL_NONE);

    native = _grwl.platform.getEGLNativeWindow(window);
    // HACK: ANGLE does not implement eglCreatePlatformWindowSurfaceEXT
    //       despite reporting EGL_EXT_platform_base
    if (_grwl.egl.platform && _grwl.egl.platform != EGL_PLATFORM_ANGLE_ANGLE)
    {
        window->context.egl.surface =
            eglCreatePlatformWindowSurfaceEXT(_grwl.egl.display, window->context.egl.config, native, attribs);
    }
    else
    {
        window->context.egl.surface =
            eglCreateWindowSurface(_grwl.egl.display, window->context.egl.config, native, attribs);
    }

    if (window->context.egl.surface == EGL_NO_SURFACE)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to create window surface: %s",
                        getEGLErrorString(eglGetError()));
        return false;
    }

    // Load the appropriate client library
    if (!_grwl.egl.KHR_get_all_proc_addresses)
    {
        const char** sonames;
        const char* es1sonames[] = {
#if defined(_GRWL_GLESV1_LIBRARY)
            _GRWL_GLESV1_LIBRARY,
#elif defined(_GRWL_WIN32)
            "GLESv1_CM.dll",
            "libGLES_CM.dll",
#elif defined(_GRWL_COCOA)
            "libGLESv1_CM.dylib",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGLESv1_CM.so",
#else
            "libGLESv1_CM.so.1",
            "libGLES_CM.so.1",
#endif
            nullptr
        };
        const char* es2sonames[] = {
#if defined(_GRWL_GLESV2_LIBRARY)
            _GRWL_GLESV2_LIBRARY,
#elif defined(_GRWL_WIN32)
            "GLESv2.dll",
            "libGLESv2.dll",
#elif defined(_GRWL_COCOA)
            "libGLESv2.dylib",
#elif defined(__CYGWIN__)
            "libGLESv2-2.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGLESv2.so",
#else
            "libGLESv2.so.2",
#endif
            nullptr
        };
        const char* glsonames[] = {
#if defined(_GRWL_OPENGL_LIBRARY)
            _GRWL_OPENGL_LIBRARY,
#elif defined(_GRWL_WIN32)
#elif defined(_GRWL_COCOA)
#elif defined(__OpenBSD__) || defined(__NetBSD__)
            "libGL.so",
#else
            "libOpenGL.so.0",
            "libGL.so.1",
#endif
            nullptr
        };

        if (ctxconfig->client == GRWL_OPENGL_ES_API)
        {
            if (ctxconfig->major == 1)
            {
                sonames = es1sonames;
            }
            else
            {
                sonames = es2sonames;
            }
        }
        else
        {
            sonames = glsonames;
        }

        for (int i = 0; sonames[i]; i++)
        {
            // HACK: Match presence of lib prefix to increase chance of finding
            //       a matching pair in the jungle that is Win32 EGL/GLES
            if (_grwl.egl.prefix != (strncmp(sonames[i], "lib", 3) == 0))
            {
                continue;
            }

            window->context.egl.client = _grwlPlatformLoadModule(sonames[i]);
            if (window->context.egl.client)
            {
                break;
            }
        }

        if (!window->context.egl.client)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "EGL: Failed to load client library");
            return false;
        }
    }

    window->context.makeCurrent = makeContextCurrentEGL;
    window->context.swapBuffers = swapBuffersEGL;
    window->context.swapInterval = swapIntervalEGL;
    window->context.extensionSupported = extensionSupportedEGL;
    window->context.getProcAddress = getProcAddressEGL;
    window->context.destroy = destroyContextEGL;

    return true;
}

#undef SET_ATTRIB

// Returns the Visual and depth of the chosen EGLConfig
//
#if defined(_GRWL_X11)
bool _grwlChooseVisualEGL(const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                          const _GRWLfbconfig* fbconfig, Visual** visual, int* depth)
{
    XVisualInfo* result;
    XVisualInfo desired;
    EGLConfig native;
    EGLint visualID = 0, count = 0;
    const long vimask = VisualScreenMask | VisualIDMask;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &native))
    {
        return false;
    }

    eglGetConfigAttrib(_grwl.egl.display, native, EGL_NATIVE_VISUAL_ID, &visualID);

    desired.screen = _grwl.x11.screen;
    desired.visualid = visualID;

    result = XGetVisualInfo(_grwl.x11.display, vimask, &desired, &count);
    if (!result)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to retrieve Visual for EGLConfig");
        return false;
    }

    *visual = result->visual;
    *depth = result->depth;

    XFree(result);
    return true;
}
#endif // _GRWL_X11

static void _grwlMakeUserContextCurrentEGL(_GRWLusercontext* context)
{
    if (!eglMakeCurrent(_grwl.egl.display, context->egl.surface, context->egl.surface, context->egl.handle))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to make user context current: %s",
                        getEGLErrorString(eglGetError()));
        _grwlPlatformSetTls(&_grwl.usercontextSlot, nullptr);
        return;
    }
    _grwlPlatformSetTls(&_grwl.usercontextSlot, context);
}

static void _grwlDestroyUserContextEGL(_GRWLusercontext* context)
{
    if (context->egl.surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(_grwl.egl.display, context->egl.surface);
    }

    eglDestroyContext(_grwl.egl.display, context->egl.handle);
    free(context);
}

_GRWLusercontext* _grwlCreateUserContextEGL(_GRWLwindow* window)
{
    _GRWLusercontext* context;
    _GRWLctxconfig ctxconfig;
    EGLint dummyConfigAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1, EGL_NONE
    };
    EGLint dummySurfaceAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    EGLint dummySurfaceNumConfigs;
    EGLConfig dummySurfaceConfig;

    context = (_GRWLusercontext*)calloc(1, sizeof(_GRWLusercontext));
    context->window = window;

    ctxconfig = _grwl.hints.context;
    ctxconfig.share = window;

    if (!_grwlCreateContextForConfigEGL(window->context.egl.config, &ctxconfig, &context->egl.handle))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to create user OpenGL context");
        free(context);
        return nullptr;
    }
    if (grwlExtensionSupported("EGL_KHR_surfaceless_context"))
    {
        context->egl.surface = EGL_NO_SURFACE;
    }
    else
    {
        eglChooseConfig(_grwl.egl.display, dummyConfigAttribs, &dummySurfaceConfig, 1, &dummySurfaceNumConfigs);
        if (!dummySurfaceNumConfigs)
        {
            eglDestroyContext(_grwl.egl.display, context->egl.handle);
            _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to find surface config for user context: %s",
                            getEGLErrorString(eglGetError()));
            free(context);
            return nullptr;
        }
        context->egl.surface = eglCreatePbufferSurface(_grwl.egl.display, dummySurfaceConfig, dummySurfaceAttribs);
        if (context->egl.surface == EGL_NO_SURFACE)
        {
            eglDestroyContext(_grwl.egl.display, context->egl.handle);
            _grwlInputError(GRWL_PLATFORM_ERROR, "EGL: Failed to create surface for user context: %s for %s",
                            getEGLErrorString(eglGetError()), eglQueryString(_grwl.egl.display, 0x3054));
            free(context);
            return nullptr;
        }
    }

    context->makeCurrent = _grwlMakeUserContextCurrentEGL;
    context->destroy = _grwlDestroyUserContextEGL;

    return context;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI EGLDisplay grwlGetEGLDisplay()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(EGL_NO_DISPLAY);
    return _grwl.egl.display;
}

GRWLAPI EGLContext grwlGetEGLContext(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(EGL_NO_CONTEXT);

    if (window->context.source != GRWL_EGL_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, nullptr);
        return EGL_NO_CONTEXT;
    }

    return window->context.egl.handle;
}

GRWLAPI EGLSurface grwlGetEGLSurface(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(EGL_NO_SURFACE);

    if (window->context.source != GRWL_EGL_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, nullptr);
        return EGL_NO_SURFACE;
    }

    return window->context.egl.surface;
}

GRWLAPI EGLConfig grwlGetEGLConfig(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(EGL_NO_SURFACE);

    if (window->context.source != GRWL_EGL_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, nullptr);
        return EGL_NO_SURFACE;
    }

    return window->context.egl.config;
}
