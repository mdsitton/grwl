//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_COCOA)

    #include <cstdlib>
    #include <climits>
    #include <cmath>

    #include <IOKit/graphics/IOGraphicsLib.h>
    #include <ApplicationServices/ApplicationServices.h>

// Get the name of the specified display, or nullptr
//
static char* getMonitorName(CGDirectDisplayID displayID, NSScreen* screen)
{
    // IOKit doesn't work on Apple Silicon anymore
    // Luckily, 10.15 introduced -[NSScreen localizedName].
    // Use it if available, and fall back to IOKit otherwise.
    if (screen)
    {
        if ([screen respondsToSelector:@selector(localizedName)])
        {
            NSString* name = [screen valueForKey:@"localizedName"];
            if (name)
            {
                return _grwl_strdup([name UTF8String]);
            }
        }
    }

    io_iterator_t it;
    io_service_t service;
    CFDictionaryRef info;

    if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("IODisplayConnect"), &it) != 0)
    {
        // This may happen if a desktop Mac is running headless
        return _grwl_strdup("Display");
    }

    while ((service = IOIteratorNext(it)) != 0)
    {
        info = IODisplayCreateInfoDictionary(service, kIODisplayOnlyPreferredName);

        CFNumberRef vendorIDRef = CFDictionaryGetValue(info, CFSTR(kDisplayVendorID));
        CFNumberRef productIDRef = CFDictionaryGetValue(info, CFSTR(kDisplayProductID));
        if (!vendorIDRef || !productIDRef)
        {
            CFRelease(info);
            continue;
        }

        unsigned int vendorID, productID;
        CFNumberGetValue(vendorIDRef, kCFNumberIntType, &vendorID);
        CFNumberGetValue(productIDRef, kCFNumberIntType, &productID);

        if (CGDisplayVendorNumber(displayID) == vendorID && CGDisplayModelNumber(displayID) == productID)
        {
            // Info dictionary is used and freed below
            break;
        }

        CFRelease(info);
    }

    IOObjectRelease(it);

    if (!service)
    {
        return _grwl_strdup("Display");
    }

    CFDictionaryRef names = CFDictionaryGetValue(info, CFSTR(kDisplayProductName));

    CFStringRef nameRef;

    if (!names || !CFDictionaryGetValueIfPresent(names, CFSTR("en_US"), (const void**)&nameRef))
    {
        // This may happen if a desktop Mac is running headless
        CFRelease(info);
        return _grwl_strdup("Display");
    }

    const CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(nameRef), kCFStringEncodingUTF8);
    char* name = _grwl_calloc(size + 1, 1);
    CFStringGetCString(nameRef, name, size, kCFStringEncodingUTF8);

    CFRelease(info);
    return name;
}

// Check whether the display mode should be included in enumeration
//
static bool modeIsGood(CGDisplayModeRef mode)
{
    uint32_t flags = CGDisplayModeGetIOFlags(mode);

    if (!(flags & kDisplayModeValidFlag) || !(flags & kDisplayModeSafeFlag))
    {
        return false;
    }
    if (flags & kDisplayModeInterlacedFlag)
    {
        return false;
    }
    if (flags & kDisplayModeStretchedFlag)
    {
        return false;
    }

    #if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
    CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) &&
        CFStringCompare(format, CFSTR(IO32BitDirectPixels), 0))
    {
        CFRelease(format);
        return false;
    }

    CFRelease(format);
    #endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
    return true;
}

// Convert Core Graphics display mode to GRWL video mode
//
static GRWLvidmode vidmodeFromCGDisplayMode(CGDisplayModeRef mode, double fallbackRefreshRate)
{
    GRWLvidmode result;
    result.width = (int)CGDisplayModeGetWidth(mode);
    result.height = (int)CGDisplayModeGetHeight(mode);
    result.refreshRate = (int)round(CGDisplayModeGetRefreshRate(mode));

    if (result.refreshRate == 0)
    {
        result.refreshRate = (int)round(fallbackRefreshRate);
    }

    #if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
    CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) == 0)
    {
        result.redBits = 5;
        result.greenBits = 5;
        result.blueBits = 5;
    }
    else
    #endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
    {
        result.redBits = 8;
        result.greenBits = 8;
        result.blueBits = 8;
    }

    #if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100
    CFRelease(format);
    #endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
    return result;
}

// Starts reservation for display fading
//
static CGDisplayFadeReservationToken beginFadeReservation()
{
    CGDisplayFadeReservationToken token = kCGDisplayFadeReservationInvalidToken;

    if (CGAcquireDisplayFadeReservation(5, &token) == kCGErrorSuccess)
    {
        CGDisplayFade(token, 0.3, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, TRUE);
    }

    return token;
}

// Ends reservation for display fading
//
static void endFadeReservation(CGDisplayFadeReservationToken token)
{
    if (token != kCGDisplayFadeReservationInvalidToken)
    {
        CGDisplayFade(token, 0.5, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0.0, 0.0, 0.0, FALSE);
        CGReleaseDisplayFadeReservation(token);
    }
}

// Returns the display refresh rate queried from the I/O registry
//
static double getFallbackRefreshRate(CGDirectDisplayID displayID)
{
    double refreshRate = 60.0;

    io_iterator_t it;
    io_service_t service;

    if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("IOFramebuffer"), &it) != 0)
    {
        return refreshRate;
    }

    while ((service = IOIteratorNext(it)) != 0)
    {
        const CFNumberRef indexRef = IORegistryEntryCreateCFProperty(service, CFSTR("IOFramebufferOpenGLIndex"),
                                                                     kCFAllocatorDefault, kNilOptions);
        if (!indexRef)
        {
            continue;
        }

        uint32_t index = 0;
        CFNumberGetValue(indexRef, kCFNumberIntType, &index);
        CFRelease(indexRef);

        if (CGOpenGLDisplayMaskToDisplayID(1 << index) != displayID)
        {
            continue;
        }

        const CFNumberRef clockRef =
            IORegistryEntryCreateCFProperty(service, CFSTR("IOFBCurrentPixelClock"), kCFAllocatorDefault, kNilOptions);
        const CFNumberRef countRef =
            IORegistryEntryCreateCFProperty(service, CFSTR("IOFBCurrentPixelCount"), kCFAllocatorDefault, kNilOptions);

        uint32_t clock = 0, count = 0;

        if (clockRef)
        {
            CFNumberGetValue(clockRef, kCFNumberIntType, &clock);
            CFRelease(clockRef);
        }

        if (countRef)
        {
            CFNumberGetValue(countRef, kCFNumberIntType, &count);
            CFRelease(countRef);
        }

        if (clock > 0 && count > 0)
        {
            refreshRate = clock / (double)count;
        }

        break;
    }

    IOObjectRelease(it);
    return refreshRate;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _grwlPollMonitorsCocoa()
{
    uint32_t displayCount;
    CGGetOnlineDisplayList(0, nullptr, &displayCount);
    CGDirectDisplayID* displays = _grwl_calloc(displayCount, sizeof(CGDirectDisplayID));
    CGGetOnlineDisplayList(displayCount, displays, &displayCount);

    for (int i = 0; i < _grwl.monitorCount; i++)
    {
        _grwl.monitors[i]->ns.screen = nil;
    }

    _GRWLmonitor** disconnected = nullptr;
    uint32_t disconnectedCount = _grwl.monitorCount;
    if (disconnectedCount)
    {
        disconnected = _grwl_calloc(_grwl.monitorCount, sizeof(_GRWLmonitor*));
        memcpy(disconnected, _grwl.monitors, _grwl.monitorCount * sizeof(_GRWLmonitor*));
    }

    for (uint32_t i = 0; i < displayCount; i++)
    {
        if (CGDisplayIsAsleep(displays[i]))
        {
            continue;
        }

        const uint32_t unitNumber = CGDisplayUnitNumber(displays[i]);
        NSScreen* screen = nil;

        for (screen in [NSScreen screens])
        {
            NSNumber* screenNumber = [screen deviceDescription][@"NSScreenNumber"];

            // HACK: Compare unit numbers instead of display IDs to work around
            //       display replacement on machines with automatic graphics
            //       switching
            if (CGDisplayUnitNumber([screenNumber unsignedIntValue]) == unitNumber)
            {
                break;
            }
        }

        // HACK: Compare unit numbers instead of display IDs to work around
        //       display replacement on machines with automatic graphics
        //       switching
        uint32_t j;
        for (j = 0; j < disconnectedCount; j++)
        {
            if (disconnected[j] && disconnected[j]->ns.unitNumber == unitNumber)
            {
                disconnected[j]->ns.screen = screen;
                disconnected[j] = nullptr;
                break;
            }
        }

        if (j < disconnectedCount)
        {
            continue;
        }

        const CGSize size = CGDisplayScreenSize(displays[i]);
        char* name = getMonitorName(displays[i], screen);
        if (!name)
        {
            continue;
        }

        _GRWLmonitor* monitor = _grwlAllocMonitor(name, size.width, size.height);
        monitor->ns.displayID = displays[i];
        monitor->ns.unitNumber = unitNumber;
        monitor->ns.screen = screen;

        _grwl_free(name);

        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displays[i]);
        if (CGDisplayModeGetRefreshRate(mode) == 0.0)
        {
            monitor->ns.fallbackRefreshRate = getFallbackRefreshRate(displays[i]);
        }
        CGDisplayModeRelease(mode);

        _grwlInputMonitor(monitor, GRWL_CONNECTED, _GRWL_INSERT_LAST);
    }

    for (uint32_t i = 0; i < disconnectedCount; i++)
    {
        if (disconnected[i])
        {
            _grwlInputMonitor(disconnected[i], GRWL_DISCONNECTED, 0);
        }
    }

    _grwl_free(disconnected);
    _grwl_free(displays);
}

// Change the current video mode
//
void _grwlSetVideoModeCocoa(_GRWLmonitor* monitor, const GRWLvidmode* desired)
{
    GRWLvidmode current;
    _grwlGetVideoModeCocoa(monitor, &current);

    const GRWLvidmode* best = _grwlChooseVideoMode(monitor, desired);
    if (_grwlCompareVideoModes(&current, best) == 0)
    {
        return;
    }

    CFArrayRef modes = CGDisplayCopyAllDisplayModes(monitor->ns.displayID, nullptr);
    const CFIndex count = CFArrayGetCount(modes);
    CGDisplayModeRef native = nullptr;

    for (CFIndex i = 0; i < count; i++)
    {
        CGDisplayModeRef dm = (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, i);
        if (!modeIsGood(dm))
        {
            continue;
        }

        const GRWLvidmode mode = vidmodeFromCGDisplayMode(dm, monitor->ns.fallbackRefreshRate);
        if (_grwlCompareVideoModes(best, &mode) == 0)
        {
            native = dm;
            break;
        }
    }

    if (native)
    {
        if (monitor->ns.previousMode == nullptr)
        {
            monitor->ns.previousMode = CGDisplayCopyDisplayMode(monitor->ns.displayID);
        }

        CGDisplayFadeReservationToken token = beginFadeReservation();
        CGDisplaySetDisplayMode(monitor->ns.displayID, native, nullptr);
        endFadeReservation(token);
    }

    CFRelease(modes);
}

// Restore the previously saved (original) video mode
//
void _grwlRestoreVideoModeCocoa(_GRWLmonitor* monitor)
{
    if (monitor->ns.previousMode)
    {
        CGDisplayFadeReservationToken token = beginFadeReservation();
        CGDisplaySetDisplayMode(monitor->ns.displayID, monitor->ns.previousMode, nullptr);
        endFadeReservation(token);

        CGDisplayModeRelease(monitor->ns.previousMode);
        monitor->ns.previousMode = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlFreeMonitorCocoa(_GRWLmonitor* monitor)
{
}

void _grwlGetMonitorPosCocoa(_GRWLmonitor* monitor, int* xpos, int* ypos)
{
    @autoreleasepool
    {

        const CGRect bounds = CGDisplayBounds(monitor->ns.displayID);

        if (xpos)
        {
            *xpos = (int)bounds.origin.x;
        }
        if (ypos)
        {
            *ypos = (int)bounds.origin.y;
        }

    } // autoreleasepool
}

void _grwlGetMonitorContentScaleCocoa(_GRWLmonitor* monitor, float* xscale, float* yscale)
{
    @autoreleasepool
    {

        if (!monitor->ns.screen)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Cannot query content scale without screen");
        }

        const NSRect points = [monitor->ns.screen frame];
        const NSRect pixels = [monitor->ns.screen convertRectToBacking:points];

        if (xscale)
        {
            *xscale = (float)(pixels.size.width / points.size.width);
        }
        if (yscale)
        {
            *yscale = (float)(pixels.size.height / points.size.height);
        }

    } // autoreleasepool
}

void _grwlGetMonitorWorkareaCocoa(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height)
{
    @autoreleasepool
    {

        if (!monitor->ns.screen)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Cannot query workarea without screen");
        }

        const NSRect frameRect = [monitor->ns.screen visibleFrame];

        if (xpos)
        {
            *xpos = frameRect.origin.x;
        }
        if (ypos)
        {
            *ypos = _grwlTransformYCocoa(frameRect.origin.y + frameRect.size.height - 1);
        }
        if (width)
        {
            *width = frameRect.size.width;
        }
        if (height)
        {
            *height = frameRect.size.height;
        }

    } // autoreleasepool
}

GRWLvidmode* _grwlGetVideoModesCocoa(_GRWLmonitor* monitor, int* count)
{
    @autoreleasepool
    {

        *count = 0;

        CFArrayRef modes = CGDisplayCopyAllDisplayModes(monitor->ns.displayID, nullptr);
        const CFIndex found = CFArrayGetCount(modes);
        GRWLvidmode* result = _grwl_calloc(found, sizeof(GRWLvidmode));

        for (CFIndex i = 0; i < found; i++)
        {
            CGDisplayModeRef dm = (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, i);
            if (!modeIsGood(dm))
            {
                continue;
            }

            const GRWLvidmode mode = vidmodeFromCGDisplayMode(dm, monitor->ns.fallbackRefreshRate);
            CFIndex j;

            for (j = 0; j < *count; j++)
            {
                if (_grwlCompareVideoModes(result + j, &mode) == 0)
                {
                    break;
                }
            }

            // Skip duplicate modes
            if (j < *count)
            {
                continue;
            }

            (*count)++;
            result[*count - 1] = mode;
        }

        CFRelease(modes);
        return result;

    } // autoreleasepool
}

void _grwlGetVideoModeCocoa(_GRWLmonitor* monitor, GRWLvidmode* mode)
{
    @autoreleasepool
    {

        CGDisplayModeRef native = CGDisplayCopyDisplayMode(monitor->ns.displayID);
        *mode = vidmodeFromCGDisplayMode(native, monitor->ns.fallbackRefreshRate);
        CGDisplayModeRelease(native);

    } // autoreleasepool
}

bool _grwlGetGammaRampCocoa(_GRWLmonitor* monitor, GRWLgammaramp* ramp)
{
    @autoreleasepool
    {

        uint32_t size = CGDisplayGammaTableCapacity(monitor->ns.displayID);
        CGGammaValue* values = _grwl_calloc(size * 3, sizeof(CGGammaValue));

        CGGetDisplayTransferByTable(monitor->ns.displayID, size, values, values + size, values + size * 2, &size);

        _grwlAllocGammaArrays(ramp, size);

        for (uint32_t i = 0; i < size; i++)
        {
            ramp->red[i] = (unsigned short)(values[i] * 65535);
            ramp->green[i] = (unsigned short)(values[i + size] * 65535);
            ramp->blue[i] = (unsigned short)(values[i + size * 2] * 65535);
        }

        _grwl_free(values);
        return true;

    } // autoreleasepool
}

void _grwlSetGammaRampCocoa(_GRWLmonitor* monitor, const GRWLgammaramp* ramp)
{
    @autoreleasepool
    {

        CGGammaValue* values = _grwl_calloc(ramp->size * 3, sizeof(CGGammaValue));

        for (unsigned int i = 0; i < ramp->size; i++)
        {
            values[i] = ramp->red[i] / 65535.f;
            values[i + ramp->size] = ramp->green[i] / 65535.f;
            values[i + ramp->size * 2] = ramp->blue[i] / 65535.f;
        }

        CGSetDisplayTransferByTable(monitor->ns.displayID, ramp->size, values, values + ramp->size,
                                    values + ramp->size * 2);

        _grwl_free(values);

    } // autoreleasepool
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI CGDirectDisplayID grwlGetCocoaMonitor(GRWLmonitor* handle)
{
    _GRWLmonitor* monitor = (_GRWLmonitor*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(kCGNullDirectDisplay);
    return monitor->ns.displayID;
}

#endif // _GRWL_COCOA
