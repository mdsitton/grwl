//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_COCOA)

    #include <unistd.h>
    #include <cmath>

static void makeContextCurrentNSGL(_GRWLwindow* window)
{
    @autoreleasepool
    {

        if (window)
        {
            [window->context.nsgl.object makeCurrentContext];
        }
        else
        {
            [NSOpenGLContext clearCurrentContext];
        }

        _grwlPlatformSetTls(&_grwl.contextSlot, window);

    } // autoreleasepool
}

static void swapBuffersNSGL(_GRWLwindow* window)
{
    @autoreleasepool
    {

        // HACK: Simulate vsync with usleep as NSGL swap interval does not apply to
        //       windows with a non-visible occlusion state
        if (window->ns.occluded)
        {
            int interval = 0;
            [window->context.nsgl.object getValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];

            if (interval > 0)
            {
                const double framerate = 60.0;
                const uint64_t frequency = _grwlPlatformGetTimerFrequency();
                const uint64_t value = _grwlPlatformGetTimerValue();

                const double elapsed = value / (double)frequency;
                const double period = 1.0 / framerate;
                const double delay = period - fmod(elapsed, period);

                usleep(floorl(delay * 1e6));
            }
        }

        [window->context.nsgl.object flushBuffer];

    } // autoreleasepool
}

static void swapIntervalNSGL(int interval)
{
    @autoreleasepool
    {

        _GRWLwindow* window = _grwlPlatformGetTls(&_grwl.contextSlot);
        if (window)
        {
            [window->context.nsgl.object setValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
        }

    } // autoreleasepool
}

static int extensionSupportedNSGL(const char* extension)
{
    // There are no NSGL extensions
    return false;
}

static GRWLglproc getProcAddressNSGL(const char* procname)
{
    CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, procname, kCFStringEncodingASCII);

    GRWLglproc symbol = CFBundleGetFunctionPointerForName(_grwl.nsgl.framework, symbolName);

    CFRelease(symbolName);

    return symbol;
}

static void destroyContextNSGL(_GRWLwindow* window)
{
    @autoreleasepool
    {

        [window->context.nsgl.pixelFormat release];
        window->context.nsgl.pixelFormat = nil;

        [window->context.nsgl.object release];
        window->context.nsgl.object = nil;

    } // autoreleasepool
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize OpenGL support
//
bool _grwlInitNSGL()
{
    if (_grwl.nsgl.framework)
    {
        return true;
    }

    _grwl.nsgl.framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (_grwl.nsgl.framework == NULL)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "NSGL: Failed to locate OpenGL framework");
        return false;
    }

    return true;
}

// Terminate OpenGL support
//
void _grwlTerminateNSGL()
{
}

// Create the OpenGL context
//
bool _grwlCreateContextNSGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig)
{
    if (ctxconfig->client == GRWL_OPENGL_ES_API)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "NSGL: OpenGL ES is not available on macOS");
        return false;
    }

    if (ctxconfig->major > 2)
    {
        if (ctxconfig->major == 3 && ctxconfig->minor < 2)
        {
            _grwlInputError(
                GRWL_VERSION_UNAVAILABLE,
                "NSGL: The targeted version of macOS does not support OpenGL 3.0 or 3.1 but may support 3.2 and above");
            return false;
        }
    }

    // Context robustness modes (GL_KHR_robustness) are not yet supported by
    // macOS but are not a hard constraint, so ignore and continue

    // Context release behaviors (GL_KHR_context_flush_control) are not yet
    // supported by macOS but are not a hard constraint, so ignore and continue

    // Debug contexts (GL_KHR_debug) are not yet supported by macOS but are not
    // a hard constraint, so ignore and continue

    // No-error contexts (GL_KHR_no_error) are not yet supported by macOS but
    // are not a hard constraint, so ignore and continue

    #define ADD_ATTRIB(a)                                                 \
        {                                                                 \
            assert((size_t)index < sizeof(attribs) / sizeof(attribs[0])); \
            attribs[index++] = a;                                         \
        }
    #define SET_ATTRIB(a, v) \
        {                    \
            ADD_ATTRIB(a);   \
            ADD_ATTRIB(v);   \
        }

    NSOpenGLPixelFormatAttribute attribs[40];
    int index = 0;

    ADD_ATTRIB(NSOpenGLPFAAccelerated);
    ADD_ATTRIB(NSOpenGLPFAClosestPolicy);

    if (ctxconfig->nsgl.offline)
    {
        ADD_ATTRIB(NSOpenGLPFAAllowOfflineRenderers);
        // NOTE: This replaces the NSSupportsAutomaticGraphicsSwitching key in
        //       Info.plist for unbundled applications
        // HACK: This assumes that NSOpenGLPixelFormat will remain
        //       a straightforward wrapper of its CGL counterpart
        ADD_ATTRIB(kCGLPFASupportsAutomaticGraphicsSwitching);
    }

    #if MAC_OS_X_VERSION_MAX_ALLOWED >= 101000
    if (ctxconfig->major >= 4)
    {
        SET_ATTRIB(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core);
    }
    else
    #endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
        if (ctxconfig->major >= 3)
        {
            SET_ATTRIB(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core);
        }

    if (ctxconfig->major <= 2)
    {
        if (fbconfig->auxBuffers != GRWL_DONT_CARE)
        {
            SET_ATTRIB(NSOpenGLPFAAuxBuffers, fbconfig->auxBuffers);
        }

        if (fbconfig->accumRedBits != GRWL_DONT_CARE && fbconfig->accumGreenBits != GRWL_DONT_CARE &&
            fbconfig->accumBlueBits != GRWL_DONT_CARE && fbconfig->accumAlphaBits != GRWL_DONT_CARE)
        {
            const int accumBits =
                fbconfig->accumRedBits + fbconfig->accumGreenBits + fbconfig->accumBlueBits + fbconfig->accumAlphaBits;

            SET_ATTRIB(NSOpenGLPFAAccumSize, accumBits);
        }
    }

    if (fbconfig->redBits != GRWL_DONT_CARE && fbconfig->greenBits != GRWL_DONT_CARE &&
        fbconfig->blueBits != GRWL_DONT_CARE)
    {
        int colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;

        // macOS needs non-zero color size, so set reasonable values
        if (colorBits == 0)
        {
            colorBits = 24;
        }
        else if (colorBits < 15)
        {
            colorBits = 15;
        }

        SET_ATTRIB(NSOpenGLPFAColorSize, colorBits);
    }

    if (fbconfig->alphaBits != GRWL_DONT_CARE)
    {
        SET_ATTRIB(NSOpenGLPFAAlphaSize, fbconfig->alphaBits);
    }

    if (fbconfig->depthBits != GRWL_DONT_CARE)
    {
        SET_ATTRIB(NSOpenGLPFADepthSize, fbconfig->depthBits);
    }

    if (fbconfig->stencilBits != GRWL_DONT_CARE)
    {
        SET_ATTRIB(NSOpenGLPFAStencilSize, fbconfig->stencilBits);
    }

    if (fbconfig->stereo)
    {
    #if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "NSGL: Stereo rendering is deprecated");
        return false;
    #else
        ADD_ATTRIB(NSOpenGLPFAStereo);
    #endif
    }

    if (fbconfig->doublebuffer)
    {
        ADD_ATTRIB(NSOpenGLPFADoubleBuffer);
    }

    if (fbconfig->samples != GRWL_DONT_CARE)
    {
        if (fbconfig->samples == 0)
        {
            SET_ATTRIB(NSOpenGLPFASampleBuffers, 0);
        }
        else
        {
            SET_ATTRIB(NSOpenGLPFASampleBuffers, 1);
            SET_ATTRIB(NSOpenGLPFASamples, fbconfig->samples);
        }
    }

    // NOTE: All NSOpenGLPixelFormats on the relevant cards support sRGB
    //       framebuffer, so there's no need (and no way) to request it

    ADD_ATTRIB(0);

    #undef ADD_ATTRIB
    #undef SET_ATTRIB

    window->context.nsgl.pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    if (window->context.nsgl.pixelFormat == nil)
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "NSGL: Failed to find a suitable pixel format");
        return false;
    }

    NSOpenGLContext* share = nil;

    if (ctxconfig->share)
    {
        share = ctxconfig->share->context.nsgl.object;
    }

    window->context.nsgl.object = [[NSOpenGLContext alloc] initWithFormat:window->context.nsgl.pixelFormat
                                                             shareContext:share];
    if (window->context.nsgl.object == nil)
    {
        _grwlInputError(GRWL_VERSION_UNAVAILABLE, "NSGL: Failed to create OpenGL context");
        return false;
    }

    if (fbconfig->transparent)
    {
        GLint opaque = 0;
        [window->context.nsgl.object setValues:&opaque forParameter:NSOpenGLContextParameterSurfaceOpacity];
    }

    [window->ns.view setWantsBestResolutionOpenGLSurface:window->ns.retina];

    [window->context.nsgl.object setView:window->ns.view];

    window->context.makeCurrent = makeContextCurrentNSGL;
    window->context.swapBuffers = swapBuffersNSGL;
    window->context.swapInterval = swapIntervalNSGL;
    window->context.extensionSupported = extensionSupportedNSGL;
    window->context.getProcAddress = getProcAddressNSGL;
    window->context.destroy = destroyContextNSGL;

    return true;
}

static void _grwlMakeUserContextCurrentNSGL(_GRWLusercontext* context)
{
    @autoreleasepool
    {

        [context->nsgl.object makeCurrentContext];

        _grwlPlatformSetTls(&_grwl.usercontextSlot, context);

    } // autoreleasepool
}

static void _grwlDestroyUserContextNSGL(_GRWLusercontext* context)
{
    @autoreleasepool
    {

        [context->nsgl.object release];

    } // autoreleasepool
    free(context);
}

_GRWLusercontext* _grwlCreateUserContextNSGL(_GRWLwindow* window)
{
    _GRWLusercontext* context;

    context = calloc(1, sizeof(_GRWLusercontext));
    context->window = window;

    context->nsgl.object = [[NSOpenGLContext alloc] initWithFormat:window->context.nsgl.pixelFormat
                                                      shareContext:window->context.nsgl.object];
    if (window->context.nsgl.object == nil)
    {
        _grwlInputError(GRWL_VERSION_UNAVAILABLE, "NSGL: Failed to create OpenGL user context");
        free(context);
        return NULL;
    }

    context->makeCurrent = _grwlMakeUserContextCurrentNSGL;
    context->destroy = _grwlDestroyUserContextNSGL;

    return context;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI id grwlGetNSGLContext(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(nil);

    if (_grwl.platform.platformID != GRWL_PLATFORM_COCOA)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "NSGL: Platform not initialized");
        return nil;
    }

    if (window->context.source != GRWL_NATIVE_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, NULL);
        return nil;
    }

    return window->context.nsgl.object;
}

#endif // _GRWL_COCOA
