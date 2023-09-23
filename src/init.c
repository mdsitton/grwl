//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// NOTE: The global variables below comprise all mutable global data in GRWL
//       Any other mutable global variable is a bug

// This contains all mutable state shared between compilation units of GRWL
//
_GRWLlibrary _grwl = { GRWL_FALSE };

// These are outside of _grwl so they can be used before initialization and
// after termination without special handling when _grwl is cleared to zero
//
static _GRWLerror _grwlMainThreadError;
static GRWLerrorfun _grwlErrorCallback;
static GRWLallocator _grwlInitAllocator;
static _GRWLinitconfig _grwlInitHints = {
    GRWL_TRUE,                     // hat buttons
    GRWL_ANGLE_PLATFORM_TYPE_NONE, // ANGLE backend
    GRWL_ANY_PLATFORM,             // preferred platform
    GRWL_FALSE,                    // whether to manage preedit candidate
    NULL,                          // vkGetInstanceProcAddr function
    {
        GRWL_TRUE, // macOS menu bar
        GRWL_TRUE  // macOS bundle chdir
    },
    {
        GRWL_TRUE, // X11 XCB Vulkan surface
        GRWL_FALSE // X11 on-the-spot IM-style
    },
    {
        GRWL_WAYLAND_PREFER_LIBDECOR // Wayland libdecor mode
    },
};

// The allocation function used when no custom allocator is set
//
static void* defaultAllocate(size_t size, void* user)
{
    return malloc(size);
}

// The deallocation function used when no custom allocator is set
//
static void defaultDeallocate(void* block, void* user)
{
    free(block);
}

// The reallocation function used when no custom allocator is set
//
static void* defaultReallocate(void* block, size_t size, void* user)
{
    return realloc(block, size);
}

// Terminate the library
//
static void terminate(void)
{
    int i;

    memset(&_grwl.callbacks, 0, sizeof(_grwl.callbacks));

    while (_grwl.windowListHead)
    {
        grwlDestroyWindow((GRWLwindow*)_grwl.windowListHead);
    }

    while (_grwl.cursorListHead)
    {
        grwlDestroyCursor((GRWLcursor*)_grwl.cursorListHead);
    }

    for (i = 0; i < _grwl.monitorCount; i++)
    {
        _GRWLmonitor* monitor = _grwl.monitors[i];
        if (monitor->originalRamp.size)
        {
            _grwl.platform.setGammaRamp(monitor, &monitor->originalRamp);
        }
        _grwlFreeMonitor(monitor);
    }

    _grwl_free(_grwl.monitors);
    _grwl.monitors = NULL;
    _grwl.monitorCount = 0;

    _grwl_free(_grwl.mappings);
    _grwl.mappings = NULL;
    _grwl.mappingCount = 0;

    _grwlTerminateVulkan();
    _grwl.platform.terminateJoysticks();
    _grwl.platform.terminate();

    _grwl.initialized = GRWL_FALSE;

    while (_grwl.errorListHead)
    {
        _GRWLerror* error = _grwl.errorListHead;
        _grwl.errorListHead = error->next;
        _grwl_free(error);
    }

    _grwlPlatformDestroyTls(&_grwl.usercontextSlot);
    _grwlPlatformDestroyTls(&_grwl.contextSlot);
    _grwlPlatformDestroyTls(&_grwl.errorSlot);
    _grwlPlatformDestroyMutex(&_grwl.errorLock);

    memset(&_grwl, 0, sizeof(_grwl));
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Encode a Unicode code point to a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
size_t _grwlEncodeUTF8(char* s, uint32_t codepoint)
{
    size_t count = 0;

    if (codepoint < 0x80)
    {
        s[count++] = (char)codepoint;
    }
    else if (codepoint < 0x800)
    {
        s[count++] = (codepoint >> 6) | 0xc0;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x10000)
    {
        s[count++] = (codepoint >> 12) | 0xe0;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x110000)
    {
        s[count++] = (codepoint >> 18) | 0xf0;
        s[count++] = ((codepoint >> 12) & 0x3f) | 0x80;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }

    return count;
}

// Decode a Unicode code point from a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
uint32_t _grwlDecodeUTF8(const char** s)
{
    uint32_t codepoint = 0, count = 0;
    static const uint32_t offsets[] = { 0x00000000u, 0x00003080u, 0x000e2080u, 0x03c82080u, 0xfa082080u, 0x82082080u };

    do
    {
        codepoint = (codepoint << 6) + (unsigned char)**s;
        (*s)++;
        count++;
    } while ((**s & 0xc0) == 0x80);

    assert(count <= 6);
    return codepoint - offsets[count - 1];
}

// Splits and translates a text/uri-list into separate file paths
// NOTE: This function destroys the provided string
//
char** _grwlParseUriList(char* text, int* count)
{
    const char* prefix = "file://";
    char** paths = NULL;
    char* line;

    *count = 0;

    while ((line = strtok(text, "\r\n")))
    {
        char* path;

        text = NULL;

        if (line[0] == '#')
        {
            continue;
        }

        if (strncmp(line, prefix, strlen(prefix)) == 0)
        {
            line += strlen(prefix);
            // TODO: Validate hostname
            while (*line != '/')
            {
                line++;
            }
        }

        (*count)++;

        path = _grwl_calloc(strlen(line) + 1, 1);
        paths = _grwl_realloc(paths, *count * sizeof(char*));
        paths[*count - 1] = path;

        while (*line)
        {
            if (line[0] == '%' && line[1] && line[2])
            {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = (char)strtol(digits, NULL, 16);
                line += 2;
            }
            else
            {
                *path = *line;
            }

            path++;
            line++;
        }
    }

    return paths;
}

char* _grwl_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = _grwl_calloc(length + 1, 1);
    strcpy(result, source);
    return result;
}

int _grwl_min(int a, int b)
{
    return a < b ? a : b;
}

int _grwl_max(int a, int b)
{
    return a > b ? a : b;
}

float _grwl_fminf(float a, float b)
{
    if (a != a)
    {
        return b;
    }
    else if (b != b)
    {
        return a;
    }
    else if (a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

float _grwl_fmaxf(float a, float b)
{
    if (a != a)
    {
        return b;
    }
    else if (b != b)
    {
        return a;
    }
    else if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

void* _grwl_calloc(size_t count, size_t size)
{
    if (count && size)
    {
        void* block;

        if (count > SIZE_MAX / size)
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Allocation size overflow");
            return NULL;
        }

        block = _grwl.allocator.allocate(count * size, _grwl.allocator.user);
        if (block)
        {
            return memset(block, 0, count * size);
        }
        else
        {
            _grwlInputError(GRWL_OUT_OF_MEMORY, NULL);
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

void* _grwl_realloc(void* block, size_t size)
{
    if (block && size)
    {
        void* resized = _grwl.allocator.reallocate(block, size, _grwl.allocator.user);
        if (resized)
        {
            return resized;
        }
        else
        {
            _grwlInputError(GRWL_OUT_OF_MEMORY, NULL);
            return NULL;
        }
    }
    else if (block)
    {
        _grwl_free(block);
        return NULL;
    }
    else
    {
        return _grwl_calloc(1, size);
    }
}

void _grwl_free(void* block)
{
    if (block)
    {
        _grwl.allocator.deallocate(block, _grwl.allocator.user);
    }
}

//////////////////////////////////////////////////////////////////////////
//////                         GRWL event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of an error
//
void _grwlInputError(int code, const char* format, ...)
{
    _GRWLerror* error;
    char description[_GRWL_MESSAGE_SIZE];

    if (format)
    {
        va_list vl;

        va_start(vl, format);
        vsnprintf(description, sizeof(description), format, vl);
        va_end(vl);

        description[sizeof(description) - 1] = '\0';
    }
    else
    {
        if (code == GRWL_NOT_INITIALIZED)
        {
            strcpy(description, "The GRWL library is not initialized");
        }
        else if (code == GRWL_NO_CURRENT_CONTEXT)
        {
            strcpy(description, "There is no current context");
        }
        else if (code == GRWL_INVALID_ENUM)
        {
            strcpy(description, "Invalid argument for enum parameter");
        }
        else if (code == GRWL_INVALID_VALUE)
        {
            strcpy(description, "Invalid value for parameter");
        }
        else if (code == GRWL_OUT_OF_MEMORY)
        {
            strcpy(description, "Out of memory");
        }
        else if (code == GRWL_API_UNAVAILABLE)
        {
            strcpy(description, "The requested API is unavailable");
        }
        else if (code == GRWL_VERSION_UNAVAILABLE)
        {
            strcpy(description, "The requested API version is unavailable");
        }
        else if (code == GRWL_PLATFORM_ERROR)
        {
            strcpy(description, "A platform-specific error occurred");
        }
        else if (code == GRWL_FORMAT_UNAVAILABLE)
        {
            strcpy(description, "The requested format is unavailable");
        }
        else if (code == GRWL_NO_WINDOW_CONTEXT)
        {
            strcpy(description, "The specified window has no context");
        }
        else if (code == GRWL_CURSOR_UNAVAILABLE)
        {
            strcpy(description, "The specified cursor shape is unavailable");
        }
        else if (code == GRWL_FEATURE_UNAVAILABLE)
        {
            strcpy(description, "The requested feature cannot be implemented for this platform");
        }
        else if (code == GRWL_FEATURE_UNIMPLEMENTED)
        {
            strcpy(description, "The requested feature has not yet been implemented for this platform");
        }
        else if (code == GRWL_PLATFORM_UNAVAILABLE)
        {
            strcpy(description, "The requested platform is unavailable");
        }
        else
        {
            strcpy(description, "ERROR: UNKNOWN GRWL ERROR");
        }
    }

    if (_grwl.initialized)
    {
        error = _grwlPlatformGetTls(&_grwl.errorSlot);
        if (!error)
        {
            error = _grwl_calloc(1, sizeof(_GRWLerror));
            _grwlPlatformSetTls(&_grwl.errorSlot, error);
            _grwlPlatformLockMutex(&_grwl.errorLock);
            error->next = _grwl.errorListHead;
            _grwl.errorListHead = error;
            _grwlPlatformUnlockMutex(&_grwl.errorLock);
        }
    }
    else
    {
        error = &_grwlMainThreadError;
    }

    error->code = code;
    strcpy(error->description, description);

    if (_grwlErrorCallback)
    {
        _grwlErrorCallback(code, description);
    }
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI int grwlInit(void)
{
    if (_grwl.initialized)
    {
        return GRWL_TRUE;
    }

    memset(&_grwl, 0, sizeof(_grwl));
    _grwl.hints.init = _grwlInitHints;

    _grwl.allocator = _grwlInitAllocator;
    if (!_grwl.allocator.allocate)
    {
        _grwl.allocator.allocate = defaultAllocate;
        _grwl.allocator.reallocate = defaultReallocate;
        _grwl.allocator.deallocate = defaultDeallocate;
    }

    if (!_grwlSelectPlatform(_grwl.hints.init.platformID, &_grwl.platform))
    {
        return GRWL_FALSE;
    }

    if (!_grwl.platform.init())
    {
        terminate();
        return GRWL_FALSE;
    }

    if (!_grwlPlatformCreateMutex(&_grwl.errorLock) || !_grwlPlatformCreateTls(&_grwl.errorSlot) ||
        !_grwlPlatformCreateTls(&_grwl.contextSlot) || !_grwlPlatformCreateTls(&_grwl.usercontextSlot))
    {
        terminate();
        return GRWL_FALSE;
    }

    _grwlPlatformSetTls(&_grwl.errorSlot, &_grwlMainThreadError);

    _grwlInitGamepadMappings();

    _grwlPlatformInitTimer();
    _grwl.timer.offset = _grwlPlatformGetTimerValue();

    _grwl.initialized = GRWL_TRUE;

    grwlDefaultWindowHints();
    return GRWL_TRUE;
}

GRWLAPI void grwlTerminate(void)
{
    if (!_grwl.initialized)
    {
        return;
    }

    terminate();
}

GRWLAPI void grwlInitHint(int hint, int value)
{
    switch (hint)
    {
        case GRWL_JOYSTICK_HAT_BUTTONS:
            _grwlInitHints.hatButtons = value;
            return;
        case GRWL_ANGLE_PLATFORM_TYPE:
            _grwlInitHints.angleType = value;
            return;
        case GRWL_PLATFORM:
            _grwlInitHints.platformID = value;
            return;
        case GRWL_MANAGE_PREEDIT_CANDIDATE:
            _grwlInitHints.managePreeditCandidate = value;
            return;
        case GRWL_COCOA_CHDIR_RESOURCES:
            _grwlInitHints.ns.chdir = value;
            return;
        case GRWL_COCOA_MENUBAR:
            _grwlInitHints.ns.menubar = value;
            return;
        case GRWL_X11_XCB_VULKAN_SURFACE:
            _grwlInitHints.x11.xcbVulkanSurface = value;
            return;
        case GRWL_X11_ONTHESPOT:
            _grwlInitHints.x11.onTheSpotIMStyle = value;
            return;
        case GRWL_WAYLAND_LIBDECOR:
            _grwlInitHints.wl.libdecorMode = value;
            return;
    }

    _grwlInputError(GRWL_INVALID_ENUM, "Invalid init hint 0x%08X", hint);
}

GRWLAPI void grwlInitAllocator(const GRWLallocator* allocator)
{
    if (allocator)
    {
        if (allocator->allocate && allocator->reallocate && allocator->deallocate)
        {
            _grwlInitAllocator = *allocator;
        }
        else
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Missing function in allocator");
        }
    }
    else
    {
        memset(&_grwlInitAllocator, 0, sizeof(GRWLallocator));
    }
}

GRWLAPI void grwlInitVulkanLoader(PFN_vkGetInstanceProcAddr loader)
{
    _grwlInitHints.vulkanLoader = loader;
}

GRWLAPI void grwlGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL)
    {
        *major = GRWL_VERSION_MAJOR;
    }
    if (minor != NULL)
    {
        *minor = GRWL_VERSION_MINOR;
    }
    if (rev != NULL)
    {
        *rev = GRWL_VERSION_REVISION;
    }
}

GRWLAPI int grwlGetError(const char** description)
{
    _GRWLerror* error;
    int code = GRWL_NO_ERROR;

    if (description)
    {
        *description = NULL;
    }

    if (_grwl.initialized)
    {
        error = _grwlPlatformGetTls(&_grwl.errorSlot);
    }
    else
    {
        error = &_grwlMainThreadError;
    }

    if (error)
    {
        code = error->code;
        error->code = GRWL_NO_ERROR;
        if (description && code)
        {
            *description = error->description;
        }
    }

    return code;
}

GRWLAPI int grwlIsInitialized(void)
{
    return _grwl.initialized;
}

GRWLAPI GRWLerrorfun grwlSetErrorCallback(GRWLerrorfun cbfun)
{
    _GRWL_SWAP(GRWLerrorfun, _grwlErrorCallback, cbfun);
    return cbfun;
}
