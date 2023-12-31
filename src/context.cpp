//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Checks whether the desired context attributes are valid
//
// This function checks things like whether the specified client API version
// exists and whether all relevant options have supported and non-conflicting
// values
//
bool _grwlIsValidContextConfig(const _GRWLctxconfig* ctxconfig)
{
    if (ctxconfig->source != GRWL_NATIVE_CONTEXT_API && ctxconfig->source != GRWL_EGL_CONTEXT_API &&
        ctxconfig->source != GRWL_OSMESA_CONTEXT_API)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid context creation API 0x%08X", ctxconfig->source);
        return false;
    }

    if (ctxconfig->client != GRWL_NO_API && ctxconfig->client != GRWL_OPENGL_API &&
        ctxconfig->client != GRWL_OPENGL_ES_API)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid client API 0x%08X", ctxconfig->client);
        return false;
    }

    if (ctxconfig->share)
    {
        if (ctxconfig->client == GRWL_NO_API || ctxconfig->share->context.client == GRWL_NO_API)
        {
            _grwlInputError(GRWL_NO_WINDOW_CONTEXT, nullptr);
            return false;
        }

        if (ctxconfig->source != ctxconfig->share->context.source)
        {
            _grwlInputError(GRWL_INVALID_ENUM, "Context creation APIs do not match between contexts");
            return false;
        }
    }

    if (ctxconfig->client == GRWL_OPENGL_API)
    {
        if ((ctxconfig->major < 1 || ctxconfig->minor < 0) || (ctxconfig->major == 1 && ctxconfig->minor > 5) ||
            (ctxconfig->major == 2 && ctxconfig->minor > 1) || (ctxconfig->major == 3 && ctxconfig->minor > 3))
        {
            // OpenGL 1.0 is the smallest valid version
            // OpenGL 1.x series ended with version 1.5
            // OpenGL 2.x series ended with version 2.1
            // OpenGL 3.x series ended with version 3.3
            // For now, let everything else through

            _grwlInputError(GRWL_INVALID_VALUE, "Invalid OpenGL version %i.%i", ctxconfig->major, ctxconfig->minor);
            return false;
        }

        if (ctxconfig->profile)
        {
            if (ctxconfig->profile != GRWL_OPENGL_CORE_PROFILE && ctxconfig->profile != GRWL_OPENGL_COMPAT_PROFILE)
            {
                _grwlInputError(GRWL_INVALID_ENUM, "Invalid OpenGL profile 0x%08X", ctxconfig->profile);
                return false;
            }

            if (ctxconfig->major <= 2 || (ctxconfig->major == 3 && ctxconfig->minor < 2))
            {
                // Desktop OpenGL context profiles are only defined for version 3.2
                // and above

                _grwlInputError(GRWL_INVALID_VALUE,
                                "Context profiles are only defined for OpenGL version 3.2 and above");
                return false;
            }
        }

        if (ctxconfig->forward && ctxconfig->major <= 2)
        {
            // Forward-compatible contexts are only defined for OpenGL version 3.0 and above
            _grwlInputError(GRWL_INVALID_VALUE,
                            "Forward-compatibility is only defined for OpenGL version 3.0 and above");
            return false;
        }
    }
    else if (ctxconfig->client == GRWL_OPENGL_ES_API)
    {
        if (ctxconfig->major < 1 || ctxconfig->minor < 0 || (ctxconfig->major == 1 && ctxconfig->minor > 1) ||
            (ctxconfig->major == 2 && ctxconfig->minor > 0))
        {
            // OpenGL ES 1.0 is the smallest valid version
            // OpenGL ES 1.x series ended with version 1.1
            // OpenGL ES 2.x series ended with version 2.0
            // For now, let everything else through

            _grwlInputError(GRWL_INVALID_VALUE, "Invalid OpenGL ES version %i.%i", ctxconfig->major, ctxconfig->minor);
            return false;
        }
    }

    if (ctxconfig->robustness)
    {
        if (ctxconfig->robustness != GRWL_NO_RESET_NOTIFICATION && ctxconfig->robustness != GRWL_LOSE_CONTEXT_ON_RESET)
        {
            _grwlInputError(GRWL_INVALID_ENUM, "Invalid context robustness mode 0x%08X", ctxconfig->robustness);
            return false;
        }
    }

    if (ctxconfig->release)
    {
        if (ctxconfig->release != GRWL_RELEASE_BEHAVIOR_NONE && ctxconfig->release != GRWL_RELEASE_BEHAVIOR_FLUSH)
        {
            _grwlInputError(GRWL_INVALID_ENUM, "Invalid context release behavior 0x%08X", ctxconfig->release);
            return false;
        }
    }

    return true;
}

// Chooses the framebuffer config that best matches the desired one
//
const _GRWLfbconfig* _grwlChooseFBConfig(const _GRWLfbconfig* desired, const _GRWLfbconfig* alternatives,
                                         unsigned int count)
{
    unsigned int missing, leastMissing = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    unsigned int extraDiff, leastExtraDiff = UINT_MAX;
    const _GRWLfbconfig* current;
    const _GRWLfbconfig* closest = nullptr;

    for (uint32_t i = 0; i < count; i++)
    {
        current = alternatives + i;

        if (desired->stereo == true && current->stereo == false)
        {
            // Stereo is a hard constraint
            continue;
        }

        // Count number of missing buffers
        {
            missing = 0;

            if (desired->alphaBits > 0 && current->alphaBits == 0)
            {
                missing++;
            }

            if (desired->depthBits > 0 && current->depthBits == 0)
            {
                missing++;
            }

            if (desired->stencilBits > 0 && current->stencilBits == 0)
            {
                missing++;
            }

            if (desired->auxBuffers > 0 && current->auxBuffers < desired->auxBuffers)
            {
                missing += desired->auxBuffers - current->auxBuffers;
            }

            if (desired->samples > 0 && current->samples == 0)
            {
                // Technically, several multisampling buffers could be
                // involved, but that's a lower level implementation detail and
                // not important to us here, so we count them as one
                missing++;
            }

            if (desired->transparent != current->transparent)
            {
                missing++;
            }
        }

        // These polynomials make many small channel size differences matter
        // less than one large channel size difference

        // Calculate color channel size difference value
        {
            colorDiff = 0;

            if (desired->redBits != GRWL_DONT_CARE)
            {
                colorDiff += (desired->redBits - current->redBits) * (desired->redBits - current->redBits);
            }

            if (desired->greenBits != GRWL_DONT_CARE)
            {
                colorDiff += (desired->greenBits - current->greenBits) * (desired->greenBits - current->greenBits);
            }

            if (desired->blueBits != GRWL_DONT_CARE)
            {
                colorDiff += (desired->blueBits - current->blueBits) * (desired->blueBits - current->blueBits);
            }
        }

        // Calculate non-color channel size difference value
        {
            extraDiff = 0;

            if (desired->alphaBits != GRWL_DONT_CARE)
            {
                extraDiff += (desired->alphaBits - current->alphaBits) * (desired->alphaBits - current->alphaBits);
            }

            if (desired->depthBits != GRWL_DONT_CARE)
            {
                extraDiff += (desired->depthBits - current->depthBits) * (desired->depthBits - current->depthBits);
            }

            if (desired->stencilBits != GRWL_DONT_CARE)
            {
                extraDiff +=
                    (desired->stencilBits - current->stencilBits) * (desired->stencilBits - current->stencilBits);
            }

            if (desired->accumRedBits != GRWL_DONT_CARE)
            {
                extraDiff +=
                    (desired->accumRedBits - current->accumRedBits) * (desired->accumRedBits - current->accumRedBits);
            }

            if (desired->accumGreenBits != GRWL_DONT_CARE)
            {
                extraDiff += (desired->accumGreenBits - current->accumGreenBits) *
                             (desired->accumGreenBits - current->accumGreenBits);
            }

            if (desired->accumBlueBits != GRWL_DONT_CARE)
            {
                extraDiff += (desired->accumBlueBits - current->accumBlueBits) *
                             (desired->accumBlueBits - current->accumBlueBits);
            }

            if (desired->accumAlphaBits != GRWL_DONT_CARE)
            {
                extraDiff += (desired->accumAlphaBits - current->accumAlphaBits) *
                             (desired->accumAlphaBits - current->accumAlphaBits);
            }

            if (desired->samples != GRWL_DONT_CARE)
            {
                extraDiff += (desired->samples - current->samples) * (desired->samples - current->samples);
            }

            if (desired->sRGB && !current->sRGB)
            {
                extraDiff++;
            }
        }

        // Figure out if the current one is better than the best one found so far
        // Least number of missing buffers is the most important heuristic,
        // then color buffer size match and lastly size match for other buffers

        if (missing < leastMissing)
        {
            closest = current;
        }
        else if (missing == leastMissing)
        {
            if ((colorDiff < leastColorDiff) || (colorDiff == leastColorDiff && extraDiff < leastExtraDiff))
            {
                closest = current;
            }
        }

        if (current == closest)
        {
            leastMissing = missing;
            leastColorDiff = colorDiff;
            leastExtraDiff = extraDiff;
        }
    }

    return closest;
}

// Retrieves the attributes of the current context
//
bool _grwlRefreshContextAttribs(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig)
{
    _GRWLwindow* previous;
    const char* version;
    const char* prefixes[] = { "OpenGL ES-CM ", "OpenGL ES-CL ", "OpenGL ES ", nullptr };

    window->context.source = ctxconfig->source;
    window->context.client = GRWL_OPENGL_API;

    previous = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);
    grwlMakeContextCurrent((GRWLwindow*)window);

    window->context.GetIntegerv = (PFNGLGETINTEGERVPROC)window->context.getProcAddress("glGetIntegerv");
    window->context.GetString = (PFNGLGETSTRINGPROC)window->context.getProcAddress("glGetString");
    if (!window->context.GetIntegerv || !window->context.GetString)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Entry point retrieval is broken");
        grwlMakeContextCurrent((GRWLwindow*)previous);
        return false;
    }

    version = (const char*)window->context.GetString(GL_VERSION);
    if (!version)
    {
        if (ctxconfig->client == GRWL_OPENGL_API)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "OpenGL version string retrieval is broken");
        }
        else
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "OpenGL ES version string retrieval is broken");
        }

        grwlMakeContextCurrent((GRWLwindow*)previous);
        return false;
    }

    for (int i = 0; prefixes[i]; i++)
    {
        const size_t length = strlen(prefixes[i]);

        if (strncmp(version, prefixes[i], length) == 0)
        {
            version += length;
            window->context.client = GRWL_OPENGL_ES_API;
            break;
        }
    }

    if (!sscanf(version, "%d.%d.%d", &window->context.major, &window->context.minor, &window->context.revision))
    {
        if (window->context.client == GRWL_OPENGL_API)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "No version found in OpenGL version string");
        }
        else
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "No version found in OpenGL ES version string");
        }

        grwlMakeContextCurrent((GRWLwindow*)previous);
        return false;
    }

    if (window->context.major < ctxconfig->major ||
        (window->context.major == ctxconfig->major && window->context.minor < ctxconfig->minor))
    {
        // The desired OpenGL version is greater than the actual version
        // This only happens if the machine lacks {GLX|WGL}_ARB_create_context
        // /and/ the user has requested an OpenGL version greater than 1.0

        // For API consistency, we emulate the behavior of the
        // {GLX|WGL}_ARB_create_context extension and fail here

        if (window->context.client == GRWL_OPENGL_API)
        {
            _grwlInputError(GRWL_VERSION_UNAVAILABLE, "Requested OpenGL version %i.%i, got version %i.%i",
                            ctxconfig->major, ctxconfig->minor, window->context.major, window->context.minor);
        }
        else
        {
            _grwlInputError(GRWL_VERSION_UNAVAILABLE, "Requested OpenGL ES version %i.%i, got version %i.%i",
                            ctxconfig->major, ctxconfig->minor, window->context.major, window->context.minor);
        }

        grwlMakeContextCurrent((GRWLwindow*)previous);
        return false;
    }

    if (window->context.major >= 3)
    {
        // OpenGL 3.0+ uses a different function for extension string retrieval
        // We cache it here instead of in grwlExtensionSupported mostly to alert
        // users as early as possible that their build may be broken

        window->context.GetStringi = (PFNGLGETSTRINGIPROC)window->context.getProcAddress("glGetStringi");
        if (!window->context.GetStringi)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Entry point retrieval is broken");
            grwlMakeContextCurrent((GRWLwindow*)previous);
            return false;
        }
    }

    if (window->context.client == GRWL_OPENGL_API)
    {
        // Read back context flags (OpenGL 3.0 and above)
        if (window->context.major >= 3)
        {
            GLint flags;
            window->context.GetIntegerv(GL_CONTEXT_FLAGS, &flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
            {
                window->context.forward = true;
            }

            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
            {
                window->context.debug = true;
            }
            else if (grwlExtensionSupported("GL_ARB_debug_output") && ctxconfig->debug)
            {
                // HACK: This is a workaround for older drivers (pre KHR_debug)
                //       not setting the debug bit in the context flags for
                //       debug contexts
                window->context.debug = true;
            }

            if (flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)
            {
                window->context.noerror = true;
            }
        }

        // Read back OpenGL context profile (OpenGL 3.2 and above)
        if (window->context.major >= 4 || (window->context.major == 3 && window->context.minor >= 2))
        {
            GLint mask;
            window->context.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

            if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
            {
                window->context.profile = GRWL_OPENGL_COMPAT_PROFILE;
            }
            else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
            {
                window->context.profile = GRWL_OPENGL_CORE_PROFILE;
            }
            else if (grwlExtensionSupported("GL_ARB_compatibility"))
            {
                // HACK: This is a workaround for the compatibility profile bit
                //       not being set in the context flags if an OpenGL 3.2+
                //       context was created without having requested a specific
                //       version
                window->context.profile = GRWL_OPENGL_COMPAT_PROFILE;
            }
        }

        // Read back robustness strategy
        if (grwlExtensionSupported("GL_ARB_robustness"))
        {
            // NOTE: We avoid using the context flags for detection, as they are
            //       only present from 3.0 while the extension applies from 1.1

            GLint strategy;
            window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
            {
                window->context.robustness = GRWL_LOSE_CONTEXT_ON_RESET;
            }
            else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
            {
                window->context.robustness = GRWL_NO_RESET_NOTIFICATION;
            }
        }
    }
    else
    {
        // Read back robustness strategy
        if (grwlExtensionSupported("GL_EXT_robustness"))
        {
            // NOTE: The values of these constants match those of the OpenGL ARB
            //       one, so we can reuse them here

            GLint strategy;
            window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
            {
                window->context.robustness = GRWL_LOSE_CONTEXT_ON_RESET;
            }
            else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
            {
                window->context.robustness = GRWL_NO_RESET_NOTIFICATION;
            }
        }
    }

    if (grwlExtensionSupported("GL_KHR_context_flush_control"))
    {
        GLint behavior;
        window->context.GetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &behavior);

        if (behavior == GL_NONE)
        {
            window->context.release = GRWL_RELEASE_BEHAVIOR_NONE;
        }
        else if (behavior == GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH)
        {
            window->context.release = GRWL_RELEASE_BEHAVIOR_FLUSH;
        }
    }

    // Clearing the front buffer to black to avoid garbage pixels left over from
    // previous uses of our bit of VRAM
    {
        PFNGLCLEARPROC glClear = (PFNGLCLEARPROC)window->context.getProcAddress("glClear");
        glClear(GL_COLOR_BUFFER_BIT);

        if (window->doublebuffer)
        {
            window->context.swapBuffers(window);
        }
    }

    grwlMakeContextCurrent((GRWLwindow*)previous);
    return true;
}

// Searches an extension string for the specified extension
//
bool _grwlStringInExtensionString(const char* string, const char* extensions)
{
    const char* start = extensions;

    for (;;)
    {
        const char* where;
        const char* terminator;

        where = strstr(start, string);
        if (!where)
        {
            return false;
        }

        terminator = where + strlen(string);
        if (where == start || *(where - 1) == ' ')
        {
            if (*terminator == ' ' || *terminator == '\0')
            {
                break;
            }
        }

        start = terminator;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI void grwlMakeContextCurrent(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWLwindow* previous;

    _GRWL_REQUIRE_INIT();

    _grwlPlatformSetTls(&_grwl.usercontextSlot, nullptr);
    previous = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);

    if (window && window->context.client == GRWL_NO_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT,
                        "Cannot make current with a window that has no OpenGL or OpenGL ES context");
        return;
    }

    if (previous)
    {
        if (!window || window->context.source != previous->context.source)
        {
            previous->context.makeCurrent(nullptr);
        }
    }

    if (window)
    {
        window->context.makeCurrent(window);
    }
}

GRWLAPI GRWLwindow* grwlGetCurrentContext()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);
    return (GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);
}

GRWLAPI void grwlSwapBuffers(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(window != nullptr);

    _GRWL_REQUIRE_INIT();

    if (window->context.client == GRWL_NO_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT,
                        "Cannot swap buffers of a window that has no OpenGL or OpenGL ES context");
        return;
    }

    window->context.swapBuffers(window);
}

GRWLAPI void grwlSwapInterval(int interval)
{
    _GRWLwindow* window;

    _GRWL_REQUIRE_INIT();

    window = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);
    if (!window)
    {
        _grwlInputError(GRWL_NO_CURRENT_CONTEXT,
                        "Cannot set swap interval without a current OpenGL or OpenGL ES context");
        return;
    }

    window->context.swapInterval(interval);
}

GRWLAPI int grwlExtensionSupported(const char* extension)
{
    _GRWLwindow* window;
    assert(extension != nullptr);

    _GRWL_REQUIRE_INIT_OR_RETURN(false);

    window = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);
    if (!window)
    {
        _grwlInputError(GRWL_NO_CURRENT_CONTEXT,
                        "Cannot query extension without a current OpenGL or OpenGL ES context");
        return false;
    }

    if (*extension == '\0')
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Extension name cannot be an empty string");
        return false;
    }

    if (window->context.major >= 3)
    {
        GLint count;

        // Check if extension is in the modern OpenGL extensions string list

        window->context.GetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (int i = 0; i < count; i++)
        {
            const char* en = (const char*)window->context.GetStringi(GL_EXTENSIONS, i);
            if (!en)
            {
                _grwlInputError(GRWL_PLATFORM_ERROR, "Extension string retrieval is broken");
                return false;
            }

            if (strcmp(en, extension) == 0)
            {
                return true;
            }
        }
    }
    else
    {
        // Check if extension is in the old style OpenGL extensions string

        const char* extensions = (const char*)window->context.GetString(GL_EXTENSIONS);
        if (!extensions)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Extension string retrieval is broken");
            return false;
        }

        if (_grwlStringInExtensionString(extension, extensions))
        {
            return true;
        }
    }

    // Check if extension is in the platform-specific string
    return window->context.extensionSupported(extension);
}

GRWLAPI GRWLglproc grwlGetProcAddress(const char* procname)
{
    _GRWLwindow* window;
    assert(procname != nullptr);

    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    window = (_GRWLwindow*)_grwlPlatformGetTls(&_grwl.contextSlot);
    if (!window)
    {
        _grwlInputError(GRWL_NO_CURRENT_CONTEXT,
                        "Cannot query entry point without a current OpenGL or OpenGL ES context");
        return nullptr;
    }

    return window->context.getProcAddress(procname);
}

GRWLAPI GRWLusercontext* grwlCreateUserContext(GRWLwindow* handle)
{
    _GRWLusercontext* context;
    _GRWLwindow* window = (_GRWLwindow*)handle;

    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    if (!window)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Cannot create a user context without a valid window handle");
        return nullptr;
    }

    if (window->context.client == GRWL_NO_API)
    {
        _grwlInputError(GRWL_NO_WINDOW_CONTEXT,
                        "Cannot create a user context for a window that has no OpenGL or OpenGL ES context");
        return nullptr;
    }

    context = _grwl.platform.createUserContext(window);

    return (GRWLusercontext*)context;
}

GRWLAPI void grwlDestroyUserContext(GRWLusercontext* handle)
{
    _GRWLusercontext* context = (_GRWLusercontext*)handle;
    _GRWLusercontext* current = (_GRWLusercontext*)_grwlPlatformGetTls(&_grwl.usercontextSlot);

    _GRWL_REQUIRE_INIT();

    if (context)
    {
        if (current == context)
        {
            grwlMakeContextCurrent(nullptr);
        }

        context->destroy(context);
    }
}

GRWLAPI void grwlMakeUserContextCurrent(GRWLusercontext* handle)
{
    _GRWLusercontext* context = (_GRWLusercontext*)handle;

    _GRWL_REQUIRE_INIT();

    // Call grwlMakeContextCurrent(nullptr) to both clear context TLS and set
    // context to nullptr if required by platform & context, and this
    // handles case of calling grwlMakeUserContextCurrent(nullptr)
    grwlMakeContextCurrent(nullptr);

    if (context)
    {
        context->makeCurrent(context);
    }
}

GRWLAPI GRWLusercontext* grwlGetCurrentUserContext()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);
    return (GRWLusercontext*)_grwlPlatformGetTls(&_grwl.usercontextSlot);
}
