//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

// These construct a string literal from individual numeric constants
#define _GRWL_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define _GRWL_MAKE_VERSION(m, n, r) _GRWL_CONCAT_VERSION(m, n, r)

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

static const struct
{
    int ID;
    GRWLbool (*connect)(int, _GRWLplatform*);
} supportedPlatforms[] = {
#if defined(_GRWL_WIN32)
    { GRWL_PLATFORM_WIN32, _grwlConnectWin32 },
#endif
#if defined(_GRWL_COCOA)
    { GRWL_PLATFORM_COCOA, _grwlConnectCocoa },
#endif
#if defined(_GRWL_X11)
    { GRWL_PLATFORM_X11, _grwlConnectX11 },
#endif
#if defined(_GRWL_WAYLAND)
    { GRWL_PLATFORM_WAYLAND, _grwlConnectWayland },
#endif
};

GRWLbool _grwlSelectPlatform(int desiredID, _GRWLplatform* platform)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (desiredID != GRWL_ANY_PLATFORM && desiredID != GRWL_PLATFORM_WIN32 && desiredID != GRWL_PLATFORM_COCOA &&
        desiredID != GRWL_PLATFORM_WAYLAND && desiredID != GRWL_PLATFORM_X11 && desiredID != GRWL_PLATFORM_NULL)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid platform ID 0x%08X", desiredID);
        return GRWL_FALSE;
    }

    if (desiredID == GRWL_ANY_PLATFORM)
    {
        // If there is exactly one platform available for auto-selection, let it emit the
        // error on failure as the platform-specific error description may be more helpful
        if (count == 1)
        {
            return supportedPlatforms[0].connect(supportedPlatforms[0].ID, platform);
        }

        for (i = 0; i < count; i++)
        {
            if (supportedPlatforms[i].connect(desiredID, platform))
            {
                return GRWL_TRUE;
            }
        }

        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "Failed to detect any supported platform");
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (supportedPlatforms[i].ID == desiredID)
            {
                return supportedPlatforms[i].connect(desiredID, platform);
            }
        }

        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "The requested platform is not supported");
    }

    return GRWL_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI int grwlGetPlatform(void)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(0);
    return _grwl.platform.platformID;
}

GRWLAPI int grwlPlatformSupported(int platformID)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (platformID != GRWL_PLATFORM_WIN32 && platformID != GRWL_PLATFORM_COCOA && platformID != GRWL_PLATFORM_WAYLAND &&
        platformID != GRWL_PLATFORM_X11 && platformID != GRWL_PLATFORM_NULL)
    {
        _grwlInputError(GRWL_INVALID_ENUM, "Invalid platform ID 0x%08X", platformID);
        return GRWL_FALSE;
    }

    if (platformID == GRWL_PLATFORM_NULL)
    {
        return GRWL_TRUE;
    }

    for (i = 0; i < count; i++)
    {
        if (platformID == supportedPlatforms[i].ID)
        {
            return GRWL_TRUE;
        }
    }

    return GRWL_FALSE;
}

GRWLAPI const char* grwlGetVersionString(void)
{
    return _GRWL_MAKE_VERSION(GRWL_VERSION_MAJOR, GRWL_VERSION_MINOR, GRWL_VERSION_REVISION)
#if defined(_GRWL_WIN32)
        " Win32 WGL"
#endif
#if defined(_GRWL_COCOA)
        " Cocoa NSGL"
#endif
#if defined(_GRWL_WAYLAND)
        " Wayland"
#endif
#if defined(_GRWL_X11)
        " X11 GLX"
#endif
        " Null"
        " EGL"
        " OSMesa"
#if defined(__MINGW64_VERSION_MAJOR)
        " MinGW-w64"
#elif defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#endif
#if defined(_GRWL_USE_HYBRID_HPG) || defined(_GRWL_USE_OPTIMUS_HPG)
        " hybrid-GPU"
#endif
#if defined(_POSIX_MONOTONIC_CLOCK)
        " monotonic"
#endif
#if defined(_GRWL_BUILD_DLL)
    #if defined(_WIN32)
        " DLL"
    #elif defined(__APPLE__)
        " dynamic"
    #else
        " shared"
    #endif
#endif
        ;
}
