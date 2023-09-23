//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "internal.h"

static void makeContextCurrentOSMesa(_GRWLwindow* window)
{
    if (window)
    {
        int width, height;
        _grwl.platform.getFramebufferSize(window, &width, &height);

        // Check to see if we need to allocate a new buffer
        if ((window->context.osmesa.buffer == NULL) || (width != window->context.osmesa.width) ||
            (height != window->context.osmesa.height))
        {
            _grwl_free(window->context.osmesa.buffer);

            // Allocate the new buffer (width * height * 8-bit RGBA)
            window->context.osmesa.buffer = _grwl_calloc(4, (size_t)width * height);
            window->context.osmesa.width = width;
            window->context.osmesa.height = height;
        }

        if (!OSMesaMakeCurrent(window->context.osmesa.handle, window->context.osmesa.buffer, GL_UNSIGNED_BYTE, width,
                               height))
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "OSMesa: Failed to make context current");
            return;
        }
    }

    _grwlPlatformSetTls(&_grwl.contextSlot, window);
}

static GRWLglproc getProcAddressOSMesa(const char* procname)
{
    return (GRWLglproc)OSMesaGetProcAddress(procname);
}

static void destroyContextOSMesa(_GRWLwindow* window)
{
    if (window->context.osmesa.handle)
    {
        OSMesaDestroyContext(window->context.osmesa.handle);
        window->context.osmesa.handle = NULL;
    }

    if (window->context.osmesa.buffer)
    {
        _grwl_free(window->context.osmesa.buffer);
        window->context.osmesa.width = 0;
        window->context.osmesa.height = 0;
    }
}

static void swapBuffersOSMesa(_GRWLwindow* window)
{
    // No double buffering on OSMesa
}

static void swapIntervalOSMesa(int interval)
{
    // No swap interval on OSMesa
}

static int extensionSupportedOSMesa(const char* extension)
{
    // OSMesa does not have extensions
    return GRWL_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

GRWLbool _grwlInitOSMesa(void)
{
    int i;
    const char* sonames[] = {
#if defined(_GRWL_OSMESA_LIBRARY)
        _GRWL_OSMESA_LIBRARY,
#elif defined(_WIN32)
        "libOSMesa.dll",
        "OSMesa.dll",
#elif defined(__APPLE__)
        "libOSMesa.8.dylib",
#elif defined(__CYGWIN__)
        "libOSMesa-8.so",
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        "libOSMesa.so",
#else
        "libOSMesa.so.8",
        "libOSMesa.so.6",
#endif
        NULL
    };

    if (_grwl.osmesa.handle)
    {
        return GRWL_TRUE;
    }

    for (i = 0; sonames[i]; i++)
    {
        _grwl.osmesa.handle = _grwlPlatformLoadModule(sonames[i]);
        if (_grwl.osmesa.handle)
        {
            break;
        }
    }

    if (!_grwl.osmesa.handle)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "OSMesa: Library not found");
        return GRWL_FALSE;
    }

    _grwl.osmesa.CreateContextExt =
        (PFN_OSMesaCreateContextExt)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaCreateContextExt");
    _grwl.osmesa.CreateContextAttribs =
        (PFN_OSMesaCreateContextAttribs)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaCreateContextAttribs");
    _grwl.osmesa.DestroyContext =
        (PFN_OSMesaDestroyContext)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaDestroyContext");
    _grwl.osmesa.MakeCurrent =
        (PFN_OSMesaMakeCurrent)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaMakeCurrent");
    _grwl.osmesa.GetColorBuffer =
        (PFN_OSMesaGetColorBuffer)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaGetColorBuffer");
    _grwl.osmesa.GetDepthBuffer =
        (PFN_OSMesaGetDepthBuffer)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaGetDepthBuffer");
    _grwl.osmesa.GetProcAddress =
        (PFN_OSMesaGetProcAddress)_grwlPlatformGetModuleSymbol(_grwl.osmesa.handle, "OSMesaGetProcAddress");

    if (!_grwl.osmesa.CreateContextExt || !_grwl.osmesa.DestroyContext || !_grwl.osmesa.MakeCurrent ||
        !_grwl.osmesa.GetColorBuffer || !_grwl.osmesa.GetDepthBuffer || !_grwl.osmesa.GetProcAddress)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "OSMesa: Failed to load required entry points");

        _grwlTerminateOSMesa();
        return GRWL_FALSE;
    }

    return GRWL_TRUE;
}

void _grwlTerminateOSMesa(void)
{
    if (_grwl.osmesa.handle)
    {
        _grwlPlatformFreeModule(_grwl.osmesa.handle);
        _grwl.osmesa.handle = NULL;
    }
}

#define SET_ATTRIB(a, v)                                                    \
    {                                                                       \
        assert(((size_t)index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
        attribs[index++] = a;                                               \
        attribs[index++] = v;                                               \
    }

GRWLbool _grwlCreateContextForConfigOSMesa(const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig,
                                           OSMesaContext* context)
{
    OSMesaContext share = NULL;
    const int accumBits =
        fbconfig->accumRedBits + fbconfig->accumGreenBits + fbconfig->accumBlueBits + fbconfig->accumAlphaBits;

    if (ctxconfig->client == GRWL_OPENGL_ES_API)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "OSMesa: OpenGL ES is not available on OSMesa");
        return GRWL_FALSE;
    }

    if (ctxconfig->share)
    {
        share = ctxconfig->share->context.osmesa.handle;
    }

    if (OSMesaCreateContextAttribs)
    {
        int index = 0, attribs[40];

        SET_ATTRIB(OSMESA_FORMAT, OSMESA_RGBA);
        SET_ATTRIB(OSMESA_DEPTH_BITS, fbconfig->depthBits);
        SET_ATTRIB(OSMESA_STENCIL_BITS, fbconfig->stencilBits);
        SET_ATTRIB(OSMESA_ACCUM_BITS, accumBits);

        if (ctxconfig->profile == GRWL_OPENGL_CORE_PROFILE)
        {
            SET_ATTRIB(OSMESA_PROFILE, OSMESA_CORE_PROFILE);
        }
        else if (ctxconfig->profile == GRWL_OPENGL_COMPAT_PROFILE)
        {
            SET_ATTRIB(OSMESA_PROFILE, OSMESA_COMPAT_PROFILE);
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            SET_ATTRIB(OSMESA_CONTEXT_MAJOR_VERSION, ctxconfig->major);
            SET_ATTRIB(OSMESA_CONTEXT_MINOR_VERSION, ctxconfig->minor);
        }

        if (ctxconfig->forward)
        {
            _grwlInputError(GRWL_VERSION_UNAVAILABLE, "OSMesa: Forward-compatible contexts not supported");
            return GRWL_FALSE;
        }

        SET_ATTRIB(0, 0);

        *context = OSMesaCreateContextAttribs(attribs, share);
    }
    else
    {
        if (ctxconfig->profile)
        {
            _grwlInputError(GRWL_VERSION_UNAVAILABLE, "OSMesa: OpenGL profiles unavailable");
            return GRWL_FALSE;
        }

        *context = OSMesaCreateContextExt(OSMESA_RGBA, fbconfig->depthBits, fbconfig->stencilBits, accumBits, share);
    }

    if (*context == NULL)
    {
        _grwlInputError(GRWL_VERSION_UNAVAILABLE, "OSMesa: Failed to create context");
        return GRWL_FALSE;
    }

    return GRWL_TRUE;
}

#undef setAttrib

GRWLbool _grwlCreateContextOSMesa(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig)
{
    if (!_grwlCreateContextForConfigOSMesa(ctxconfig, fbconfig, &window->context.osmesa.handle))
    {
        return GRWL_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentOSMesa;
    window->context.swapBuffers = swapBuffersOSMesa;
    window->context.swapInterval = swapIntervalOSMesa;
    window->context.extensionSupported = extensionSupportedOSMesa;
    window->context.getProcAddress = getProcAddressOSMesa;
    window->context.destroy = destroyContextOSMesa;

    return GRWL_TRUE;
}

static void _grwlMakeUserContextCurrentOSMesa(_GRWLusercontext* context)
{
    if (!OSMesaMakeCurrent(context->osmesa.handle, context->window->context.osmesa.buffer, GL_UNSIGNED_BYTE,
                           context->window->context.osmesa.width, context->window->context.osmesa.height))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "OSMesa: Failed to make user context current");
        _grwlPlatformSetTls(&_grwl.usercontextSlot, NULL);
        return;
    }
    _grwlPlatformSetTls(&_grwl.usercontextSlot, context);
}

static void _grwlDestroyUserContextOSMesa(_GRWLusercontext* context)
{
    if (context->osmesa.handle)
    {
        OSMesaDestroyContext(context->osmesa.handle);
    }
    free(context);
}

_GRWLusercontext* _grwlCreateUserContextOSMesa(_GRWLwindow* window)
{
    _GRWLusercontext* context;
    _GRWLctxconfig ctxconfig;
    _GRWLfbconfig fbconfig;

    context = calloc(1, sizeof(_GRWLusercontext));
    context->window = window;

    ctxconfig = _grwl.hints.context;
    ctxconfig.share = window;

    fbconfig = _grwl.hints.framebuffer;

    if (!_grwlCreateContextForConfigOSMesa(&ctxconfig, &fbconfig, &context->osmesa.handle))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "OSMesa: Failed to create user OpenGL context");
        free(context);
        return NULL;
    }

    context->makeCurrent = _grwlMakeUserContextCurrentOSMesa;
    context->destroy = _grwlDestroyUserContextOSMesa;

    return context;
}

#undef SET_ATTRIB

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI int grwlGetOSMesaColorBuffer(GRWLwindow* handle, int* width, int* height, int* format, void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaFormat;
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(GRWL_FALSE);

    if (window->context.source != GRWL_OSMESA_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, NULL);
        return GRWL_FALSE;
    }

    if (!OSMesaGetColorBuffer(window->context.osmesa.handle, &mesaWidth, &mesaHeight, &mesaFormat, &mesaBuffer))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "OSMesa: Failed to retrieve color buffer");
        return GRWL_FALSE;
    }

    if (width)
    {
        *width = mesaWidth;
    }
    if (height)
    {
        *height = mesaHeight;
    }
    if (format)
    {
        *format = mesaFormat;
    }
    if (buffer)
    {
        *buffer = mesaBuffer;
    }

    return GRWL_TRUE;
}

GRWLAPI int grwlGetOSMesaDepthBuffer(GRWLwindow* handle, int* width, int* height, int* bytesPerValue, void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaBytes;
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(GRWL_FALSE);

    if (window->context.source != GRWL_OSMESA_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, NULL);
        return GRWL_FALSE;
    }

    if (!OSMesaGetDepthBuffer(window->context.osmesa.handle, &mesaWidth, &mesaHeight, &mesaBytes, &mesaBuffer))
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "OSMesa: Failed to retrieve depth buffer");
        return GRWL_FALSE;
    }

    if (width)
    {
        *width = mesaWidth;
    }
    if (height)
    {
        *height = mesaHeight;
    }
    if (bytesPerValue)
    {
        *bytesPerValue = mesaBytes;
    }
    if (buffer)
    {
        *buffer = mesaBuffer;
    }

    return GRWL_TRUE;
}

GRWLAPI OSMesaContext grwlGetOSMesaContext(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != GRWL_OSMESA_CONTEXT_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.osmesa.handle;
}
