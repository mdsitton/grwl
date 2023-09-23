//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(_GRWL_X11)

    #include <string.h>
    #include <stdlib.h>
    #include <assert.h>

    #ifndef GLXBadProfileARB
        #define GLXBadProfileARB 13
    #endif

// Returns the specified attribute of the specified GLXFBConfig
//
static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib)
{
    int value;
    glXGetFBConfigAttrib(_grwl.x11.display, fbconfig, attrib, &value);
    return value;
}

// Return the GLXFBConfig most closely matching the specified hints
//
static GRWLbool chooseGLXFBConfig(const _GRWLfbconfig* desired, GLXFBConfig* result)
{
    GLXFBConfig* nativeConfigs;
    _GRWLfbconfig* usableConfigs;
    const _GRWLfbconfig* closest;
    int nativeCount, usableCount;
    const char* vendor;
    GRWLbool trustWindowBit = GRWL_TRUE;

    // HACK: This is a (hopefully temporary) workaround for Chromium
    //       (VirtualBox GL) not setting the window bit on any GLXFBConfigs
    vendor = glXGetClientString(_grwl.x11.display, GLX_VENDOR);
    if (vendor && strcmp(vendor, "Chromium") == 0)
    {
        trustWindowBit = GRWL_FALSE;
    }

    nativeConfigs = glXGetFBConfigs(_grwl.x11.display, _grwl.x11.screen, &nativeCount);
    if (!nativeConfigs || !nativeCount)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "GLX: No GLXFBConfigs returned");
        return GRWL_FALSE;
    }

    usableConfigs = _grwl_calloc(nativeCount, sizeof(_GRWLfbconfig));
    usableCount = 0;

    for (int i = 0; i < nativeCount; i++)
    {
        const GLXFBConfig n = nativeConfigs[i];
        _GRWLfbconfig* u = usableConfigs + usableCount;

        // Only consider RGBA GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT))
        {
            continue;
        }

        // Only consider window GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT))
        {
            if (trustWindowBit)
            {
                continue;
            }
        }

        if (getGLXFBConfigAttrib(n, GLX_DOUBLEBUFFER) != desired->doublebuffer)
        {
            continue;
        }

        if (desired->transparent)
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig(_grwl.x11.display, n);
            if (vi)
            {
                u->transparent = _grwlIsVisualTransparentX11(vi->visual);
                XFree(vi);
            }
        }

        u->redBits = getGLXFBConfigAttrib(n, GLX_RED_SIZE);
        u->greenBits = getGLXFBConfigAttrib(n, GLX_GREEN_SIZE);
        u->blueBits = getGLXFBConfigAttrib(n, GLX_BLUE_SIZE);

        u->alphaBits = getGLXFBConfigAttrib(n, GLX_ALPHA_SIZE);
        u->depthBits = getGLXFBConfigAttrib(n, GLX_DEPTH_SIZE);
        u->stencilBits = getGLXFBConfigAttrib(n, GLX_STENCIL_SIZE);

        u->accumRedBits = getGLXFBConfigAttrib(n, GLX_ACCUM_RED_SIZE);
        u->accumGreenBits = getGLXFBConfigAttrib(n, GLX_ACCUM_GREEN_SIZE);
        u->accumBlueBits = getGLXFBConfigAttrib(n, GLX_ACCUM_BLUE_SIZE);
        u->accumAlphaBits = getGLXFBConfigAttrib(n, GLX_ACCUM_ALPHA_SIZE);

        u->auxBuffers = getGLXFBConfigAttrib(n, GLX_AUX_BUFFERS);

        if (getGLXFBConfigAttrib(n, GLX_STEREO))
        {
            u->stereo = GRWL_TRUE;
        }

        if (_grwl.glx.ARB_multisample)
        {
            u->samples = getGLXFBConfigAttrib(n, GLX_SAMPLES);
        }

        if (_grwl.glx.ARB_framebuffer_sRGB || _grwl.glx.EXT_framebuffer_sRGB)
        {
            u->sRGB = getGLXFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);
        }

        u->handle = (uintptr_t)n;
        usableCount++;
    }

    closest = _grwlChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
    {
        *result = (GLXFBConfig)closest->handle;
    }

    XFree(nativeConfigs);
    _grwl_free(usableConfigs);

    return closest != NULL;
}

// Create the OpenGL context using legacy API
//
static GLXContext createLegacyContextGLX(_GRWLwindow* window, GLXFBConfig fbconfig, GLXContext share)
{
    return glXCreateNewContext(_grwl.x11.display, fbconfig, GLX_RGBA_TYPE, share, True);
}

static void makeContextCurrentGLX(_GRWLwindow* window)
{
    if (window)
    {
        if (!glXMakeCurrent(_grwl.x11.display, window->context.glx.window, window->context.glx.handle))
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to make context current");
            return;
        }
    }
    else
    {
        if (!glXMakeCurrent(_grwl.x11.display, None, NULL))
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to clear current context");
            return;
        }
    }

    _grwlPlatformSetTls(&_grwl.contextSlot, window);
}

static void swapBuffersGLX(_GRWLwindow* window)
{
    glXSwapBuffers(_grwl.x11.display, window->context.glx.window);
}

static void swapIntervalGLX(int interval)
{
    if (_grwl.glx.EXT_swap_control)
    {
        _GRWLwindow* window = _grwlPlatformGetTls(&_grwl.contextSlot);
        if (window)
        {
            _grwl.glx.SwapIntervalEXT(_grwl.x11.display, window->context.glx.window, interval);
        }
    }
    else if (_grwl.glx.MESA_swap_control)
    {
        _grwl.glx.SwapIntervalMESA(interval);
    }
    else if (_grwl.glx.SGI_swap_control)
    {
        if (interval > 0)
        {
            _grwl.glx.SwapIntervalSGI(interval);
        }
    }
}

static int extensionSupportedGLX(const char* extension)
{
    const char* extensions = glXQueryExtensionsString(_grwl.x11.display, _grwl.x11.screen);
    if (extensions)
    {
        if (_grwlStringInExtensionString(extension, extensions))
        {
            return GRWL_TRUE;
        }
    }

    return GRWL_FALSE;
}

static GRWLglproc getProcAddressGLX(const char* procname)
{
    if (_grwl.glx.GetProcAddress)
    {
        return _grwl.glx.GetProcAddress((const GLubyte*)procname);
    }
    else if (_grwl.glx.GetProcAddressARB)
    {
        return _grwl.glx.GetProcAddressARB((const GLubyte*)procname);
    }
    else
    {
        // NOTE: glvnd provides GLX 1.4, so this can only happen with libGL
        return _grwlPlatformGetModuleSymbol(_grwl.glx.handle, procname);
    }
}

static void destroyContextGLX(_GRWLwindow* window)
{
    if (window->context.glx.window)
    {
        glXDestroyWindow(_grwl.x11.display, window->context.glx.window);
        window->context.glx.window = None;
    }

    if (window->context.glx.handle)
    {
        glXDestroyContext(_grwl.x11.display, window->context.glx.handle);
        window->context.glx.handle = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize GLX
//
GRWLbool _grwlInitGLX(void)
{
    const char* sonames[] = {
    #if defined(_GRWL_GLX_LIBRARY)
        _GRWL_GLX_LIBRARY,
    #elif defined(__CYGWIN__)
        "libGL-1.so",
    #elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libGL.so",
    #else
        "libGLX.so.0",
        "libGL.so.1",
        "libGL.so",
    #endif
        NULL
    };

    if (_grwl.glx.handle)
    {
        return GRWL_TRUE;
    }

    for (int i = 0; sonames[i]; i++)
    {
        _grwl.glx.handle = _grwlPlatformLoadModule(sonames[i]);
        if (_grwl.glx.handle)
        {
            break;
        }
    }

    if (!_grwl.glx.handle)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "GLX: Failed to load GLX");
        return GRWL_FALSE;
    }

    _grwl.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXGetFBConfigs");
    _grwl.glx.GetFBConfigAttrib =
        (PFNGLXGETFBCONFIGATTRIBPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXGetFBConfigAttrib");
    _grwl.glx.GetClientString =
        (PFNGLXGETCLIENTSTRINGPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXGetClientString");
    _grwl.glx.QueryExtension =
        (PFNGLXQUERYEXTENSIONPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXQueryExtension");
    _grwl.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXQueryVersion");
    _grwl.glx.DestroyContext =
        (PFNGLXDESTROYCONTEXTPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXDestroyContext");
    _grwl.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXMakeCurrent");
    _grwl.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXSwapBuffers");
    _grwl.glx.QueryExtensionsString =
        (PFNGLXQUERYEXTENSIONSSTRINGPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXQueryExtensionsString");
    _grwl.glx.CreateNewContext =
        (PFNGLXCREATENEWCONTEXTPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXCreateNewContext");
    _grwl.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXCreateWindow");
    _grwl.glx.DestroyWindow =
        (PFNGLXDESTROYWINDOWPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXDestroyWindow");
    _grwl.glx.GetVisualFromFBConfig =
        (PFNGLXGETVISUALFROMFBCONFIGPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXGetVisualFromFBConfig");

    if (!_grwl.glx.GetFBConfigs || !_grwl.glx.GetFBConfigAttrib || !_grwl.glx.GetClientString ||
        !_grwl.glx.QueryExtension || !_grwl.glx.QueryVersion || !_grwl.glx.DestroyContext || !_grwl.glx.MakeCurrent ||
        !_grwl.glx.SwapBuffers || !_grwl.glx.QueryExtensionsString || !_grwl.glx.CreateNewContext ||
        !_grwl.glx.CreateWindow || !_grwl.glx.DestroyWindow || !_grwl.glx.GetVisualFromFBConfig)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to load required entry points");
        return GRWL_FALSE;
    }

    // NOTE: Unlike GLX 1.3 entry points these are not required to be present
    _grwl.glx.GetProcAddress =
        (PFNGLXGETPROCADDRESSPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXGetProcAddress");
    _grwl.glx.GetProcAddressARB =
        (PFNGLXGETPROCADDRESSPROC)_grwlPlatformGetModuleSymbol(_grwl.glx.handle, "glXGetProcAddressARB");

    if (!glXQueryExtension(_grwl.x11.display, &_grwl.glx.errorBase, &_grwl.glx.eventBase))
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "GLX: GLX extension not found");
        return GRWL_FALSE;
    }

    if (!glXQueryVersion(_grwl.x11.display, &_grwl.glx.major, &_grwl.glx.minor))
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "GLX: Failed to query GLX version");
        return GRWL_FALSE;
    }

    if (_grwl.glx.major == 1 && _grwl.glx.minor < 3)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "GLX: GLX version 1.3 is required");
        return GRWL_FALSE;
    }

    if (extensionSupportedGLX("GLX_EXT_swap_control"))
    {
        _grwl.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)getProcAddressGLX("glXSwapIntervalEXT");

        if (_grwl.glx.SwapIntervalEXT)
        {
            _grwl.glx.EXT_swap_control = GRWL_TRUE;
        }
    }

    if (extensionSupportedGLX("GLX_SGI_swap_control"))
    {
        _grwl.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)getProcAddressGLX("glXSwapIntervalSGI");

        if (_grwl.glx.SwapIntervalSGI)
        {
            _grwl.glx.SGI_swap_control = GRWL_TRUE;
        }
    }

    if (extensionSupportedGLX("GLX_MESA_swap_control"))
    {
        _grwl.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)getProcAddressGLX("glXSwapIntervalMESA");

        if (_grwl.glx.SwapIntervalMESA)
        {
            _grwl.glx.MESA_swap_control = GRWL_TRUE;
        }
    }

    if (extensionSupportedGLX("GLX_ARB_multisample"))
    {
        _grwl.glx.ARB_multisample = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_framebuffer_sRGB"))
    {
        _grwl.glx.ARB_framebuffer_sRGB = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_EXT_framebuffer_sRGB"))
    {
        _grwl.glx.EXT_framebuffer_sRGB = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_create_context"))
    {
        _grwl.glx.CreateContextAttribsARB =
            (PFNGLXCREATECONTEXTATTRIBSARBPROC)getProcAddressGLX("glXCreateContextAttribsARB");

        if (_grwl.glx.CreateContextAttribsARB)
        {
            _grwl.glx.ARB_create_context = GRWL_TRUE;
        }
    }

    if (extensionSupportedGLX("GLX_ARB_create_context_robustness"))
    {
        _grwl.glx.ARB_create_context_robustness = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_create_context_profile"))
    {
        _grwl.glx.ARB_create_context_profile = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_EXT_create_context_es2_profile"))
    {
        _grwl.glx.EXT_create_context_es2_profile = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_create_context_no_error"))
    {
        _grwl.glx.ARB_create_context_no_error = GRWL_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_context_flush_control"))
    {
        _grwl.glx.ARB_context_flush_control = GRWL_TRUE;
    }

    return GRWL_TRUE;
}

// Terminate GLX
//
void _grwlTerminateGLX(void)
{
    // NOTE: This function must not call any X11 functions, as it is called
    //       after XCloseDisplay (see _grwlTerminateX11 for details)

    if (_grwl.glx.handle)
    {
        _grwlPlatformFreeModule(_grwl.glx.handle);
        _grwl.glx.handle = NULL;
    }
}

    #define SET_ATTRIB(a, v)                                                    \
        {                                                                       \
            assert(((size_t)index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
            attribs[index++] = a;                                               \
            attribs[index++] = v;                                               \
        }

// Create the OpenGL or OpenGL ES context for the window fbConfig
//
GRWLbool _grwlCreateContextForFBGLX(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, GLXContext* context)
{
    int attribs[40];
    GLXContext share = NULL;

    if (ctxconfig->share)
    {
        share = ctxconfig->share->context.glx.handle;
    }

    if (ctxconfig->client == GRWL_OPENGL_ES_API)
    {
        if (!_grwl.glx.ARB_create_context || !_grwl.glx.ARB_create_context_profile ||
            !_grwl.glx.EXT_create_context_es2_profile)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE,
                            "GLX: OpenGL ES requested but GLX_EXT_create_context_es2_profile is unavailable");
            return GRWL_FALSE;
        }
    }

    if (ctxconfig->forward)
    {
        if (!_grwl.glx.ARB_create_context)
        {
            _grwlInputError(GRWL_VERSION_UNAVAILABLE,
                            "GLX: Forward compatibility requested but GLX_ARB_create_context_profile is unavailable");
            return GRWL_FALSE;
        }
    }

    if (ctxconfig->profile)
    {
        if (!_grwl.glx.ARB_create_context || !_grwl.glx.ARB_create_context_profile)
        {
            _grwlInputError(GRWL_VERSION_UNAVAILABLE,
                            "GLX: An OpenGL profile requested but GLX_ARB_create_context_profile is unavailable");
            return GRWL_FALSE;
        }
    }

    _grwlGrabErrorHandlerX11();

    if (_grwl.glx.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == GRWL_OPENGL_API)
        {
            if (ctxconfig->forward)
            {
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            }

            if (ctxconfig->profile == GRWL_OPENGL_CORE_PROFILE)
            {
                mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            }
            else if (ctxconfig->profile == GRWL_OPENGL_COMPAT_PROFILE)
            {
                mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            }
        }
        else
        {
            mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;
        }

        if (ctxconfig->debug)
        {
            flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
        }

        if (ctxconfig->robustness)
        {
            if (_grwl.glx.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == GRWL_NO_RESET_NOTIFICATION)
                {
                    SET_ATTRIB(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, GLX_NO_RESET_NOTIFICATION_ARB);
                }
                else if (ctxconfig->robustness == GRWL_LOSE_CONTEXT_ON_RESET)
                {
                    SET_ATTRIB(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, GLX_LOSE_CONTEXT_ON_RESET_ARB);
                }

                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_grwl.glx.ARB_context_flush_control)
            {
                if (ctxconfig->release == GRWL_RELEASE_BEHAVIOR_NONE)
                {
                    SET_ATTRIB(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB, GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                }
                else if (ctxconfig->release == GRWL_RELEASE_BEHAVIOR_FLUSH)
                {
                    SET_ATTRIB(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB, GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror)
        {
            if (_grwl.glx.ARB_create_context_no_error)
            {
                SET_ATTRIB(GLX_CONTEXT_OPENGL_NO_ERROR_ARB, GRWL_TRUE);
            }
        }

        // NOTE: Only request an explicitly versioned context when necessary, as
        //       explicitly requesting version 1.0 does not always return the
        //       highest version supported by the driver
        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            SET_ATTRIB(GLX_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
            SET_ATTRIB(GLX_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
        }

        if (mask)
        {
            SET_ATTRIB(GLX_CONTEXT_PROFILE_MASK_ARB, mask);
        }

        if (flags)
        {
            SET_ATTRIB(GLX_CONTEXT_FLAGS_ARB, flags);
        }

        SET_ATTRIB(None, None);

        *context =
            _grwl.glx.CreateContextAttribsARB(_grwl.x11.display, window->context.glx.fbconfig, share, True, attribs);

        // HACK: This is a fallback for broken versions of the Mesa
        //       implementation of GLX_ARB_create_context_profile that fail
        //       default 1.0 context creation with a GLXBadProfileARB error in
        //       violation of the extension spec
        if (!(*context))
        {
            if (_grwl.x11.errorCode == _grwl.glx.errorBase + GLXBadProfileARB && ctxconfig->client == GRWL_OPENGL_API &&
                ctxconfig->profile == GRWL_OPENGL_ANY_PROFILE && ctxconfig->forward == GRWL_FALSE)
            {
                *context = createLegacyContextGLX(window, window->context.glx.fbconfig, share);
            }
        }
    }
    else
    {
        *context = createLegacyContextGLX(window, window->context.glx.fbconfig, share);
    }

    _grwlReleaseErrorHandlerX11();

    if (!(*context))
    {
        _grwlInputErrorX11(GRWL_VERSION_UNAVAILABLE, "GLX: Failed to create context");
        return GRWL_FALSE;
    }

    return GRWL_TRUE;
}

// Create the OpenGL or OpenGL ES context
//
GRWLbool _grwlCreateContextGLX(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig)
{

    if (!chooseGLXFBConfig(fbconfig, &window->context.glx.fbconfig))
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "GLX: Failed to find a suitable GLXFBConfig");
        return GRWL_FALSE;
    }

    if (!_grwlCreateContextForFBGLX(window, ctxconfig, &window->context.glx.handle))
    {
        return GRWL_FALSE;
    }

    window->context.glx.window =
        glXCreateWindow(_grwl.x11.display, window->context.glx.fbconfig, window->x11.handle, NULL);
    if (!window->context.glx.window)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to create window");
        return GRWL_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers = swapBuffersGLX;
    window->context.swapInterval = swapIntervalGLX;
    window->context.extensionSupported = extensionSupportedGLX;
    window->context.getProcAddress = getProcAddressGLX;
    window->context.destroy = destroyContextGLX;

    return GRWL_TRUE;
}

    #undef SET_ATTRIB

// Returns the Visual and depth of the chosen GLXFBConfig
//
GRWLbool _grwlChooseVisualGLX(const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                              const _GRWLfbconfig* fbconfig, Visual** visual, int* depth)
{
    GLXFBConfig native;
    XVisualInfo* result;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "GLX: Failed to find a suitable GLXFBConfig");
        return GRWL_FALSE;
    }

    result = glXGetVisualFromFBConfig(_grwl.x11.display, native);
    if (!result)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to retrieve Visual for GLXFBConfig");
        return GRWL_FALSE;
    }

    *visual = result->visual;
    *depth = result->depth;

    XFree(result);
    return GRWL_TRUE;
}

static void _grwlMakeUserContextCurrentGLX(_GRWLusercontext* context)
{
    if (!glXMakeCurrent(_grwl.x11.display, context->window->context.glx.window, context->glx.handle))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to make user context current");
        _grwlPlatformSetTls(&_grwl.usercontextSlot, NULL);
        return;
    }
    _grwlPlatformSetTls(&_grwl.usercontextSlot, context);
}

static void _grwlDestroyUserContextGLX(_GRWLusercontext* context)
{
    glXDestroyContext(_grwl.x11.display, context->glx.handle);
    free(context);
}

_GRWLusercontext* _grwlCreateUserContextGLX(_GRWLwindow* window)
{
    _GRWLusercontext* context;
    _GRWLctxconfig ctxconfig;

    context = calloc(1, sizeof(_GRWLusercontext));
    context->window = window;

    ctxconfig = _grwl.hints.context;
    ctxconfig.share = window;

    if (!_grwlCreateContextForFBGLX(window, &ctxconfig, &context->glx.handle))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "GLX: Failed to create user OpenGL context");
        free(context);
        return NULL;
    }

    context->makeCurrent = _grwlMakeUserContextCurrentGLX;
    context->destroy = _grwlDestroyUserContextGLX;

    return context;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI GLXContext grwlGetGLXContext(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (_grwl.platform.platformID != GRWL_PLATFORM_X11)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized");
        return NULL;
    }

    if (window->context.source != GRWL_NATIVE_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.glx.handle;
}

GRWLAPI GLXWindow grwlGetGLXWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(None);

    if (_grwl.platform.platformID != GRWL_PLATFORM_X11)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized");
        return None;
    }

    if (window->context.source != GRWL_NATIVE_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, NULL);
        return None;
    }

    return window->context.glx.window;
}

#endif // _GRWL_X11
