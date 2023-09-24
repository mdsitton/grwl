//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_COCOA)

    #include <cunistd>
    #include <cctype>
    #include <cstring>

    #include <mach/mach.h>
    #include <mach/mach_error.h>

    #include <CoreFoundation/CoreFoundation.h>
    #include <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>

// Joystick element information
//
typedef struct _GRWLjoyelementNS
{
    IOHIDElementRef native;
    uint32_t usage;
    int index;
    long minimum;
    long maximum;

} _GRWLjoyelementNS;

// Returns the value of the specified element of the specified joystick
//
static long getElementValue(_GRWLjoystick* js, _GRWLjoyelementNS* element)
{
    IOHIDValueRef valueRef;
    long value = 0;

    if (js->ns.device)
    {
        if (IOHIDDeviceGetValue(js->ns.device, element->native, &valueRef) == kIOReturnSuccess)
        {
            value = IOHIDValueGetIntegerValue(valueRef);
        }
    }

    return value;
}

// Comparison function for matching the SDL element order
//
static CFComparisonResult compareElements(const void* fp, const void* sp, void* user)
{
    const _GRWLjoyelementNS* fe = fp;
    const _GRWLjoyelementNS* se = sp;
    if (fe->usage < se->usage)
    {
        return kCFCompareLessThan;
    }
    if (fe->usage > se->usage)
    {
        return kCFCompareGreaterThan;
    }
    if (fe->index < se->index)
    {
        return kCFCompareLessThan;
    }
    if (fe->index > se->index)
    {
        return kCFCompareGreaterThan;
    }
    return kCFCompareEqualTo;
}

// Removes the specified joystick
//
static void closeJoystick(_GRWLjoystick* js)
{
    _grwlInputJoystick(js, GRWL_DISCONNECTED);

    for (int i = 0; i < CFArrayGetCount(js->ns.axes); i++)
    {
        _grwl_free((void*)CFArrayGetValueAtIndex(js->ns.axes, i));
    }
    CFRelease(js->ns.axes);

    for (int i = 0; i < CFArrayGetCount(js->ns.buttons); i++)
    {
        _grwl_free((void*)CFArrayGetValueAtIndex(js->ns.buttons, i));
    }
    CFRelease(js->ns.buttons);

    for (int i = 0; i < CFArrayGetCount(js->ns.hats); i++)
    {
        _grwl_free((void*)CFArrayGetValueAtIndex(js->ns.hats, i));
    }
    CFRelease(js->ns.hats);

    _grwlFreeJoystick(js);
}

// Callback for user-initiated joystick addition
//
static void matchCallback(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
    int jid;
    char name[256];
    char guid[33];
    CFTypeRef property;
    uint32_t vendor = 0, product = 0, version = 0;
    _GRWLjoystick* js;
    CFMutableArrayRef axes, buttons, hats;

    for (jid = 0; jid <= GRWL_JOYSTICK_LAST; jid++)
    {
        if (_grwl.joysticks[jid].ns.device == device)
        {
            return;
        }
    }

    axes = CFArrayCreateMutable(NULL, 0, NULL);
    buttons = CFArrayCreateMutable(NULL, 0, NULL);
    hats = CFArrayCreateMutable(NULL, 0, NULL);

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
    if (property)
    {
        CFStringGetCString(property, name, sizeof(name), kCFStringEncodingUTF8);
    }
    else
    {
        strncpy(name, "Unknown", sizeof(name));
    }

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
    if (property)
    {
        CFNumberGetValue(property, kCFNumberSInt32Type, &vendor);
    }

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
    if (property)
    {
        CFNumberGetValue(property, kCFNumberSInt32Type, &product);
    }

    property = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVersionNumberKey));
    if (property)
    {
        CFNumberGetValue(property, kCFNumberSInt32Type, &version);
    }

    // Generate a joystick GUID that matches the SDL 2.0.5+ one
    if (vendor && product)
    {
        sprintf(guid, "03000000%02x%02x0000%02x%02x0000%02x%02x0000", (uint8_t)vendor, (uint8_t)(vendor >> 8),
                (uint8_t)product, (uint8_t)(product >> 8), (uint8_t)version, (uint8_t)(version >> 8));
    }
    else
    {
        sprintf(guid, "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00", name[0], name[1], name[2], name[3],
                name[4], name[5], name[6], name[7], name[8], name[9], name[10]);
    }

    CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);

    if (elements)
    {
        for (CFIndex i = 0; i < CFArrayGetCount(elements); i++)
        {
            IOHIDElementRef native = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
            if (CFGetTypeID(native) != IOHIDElementGetTypeID())
            {
                continue;
            }

            const IOHIDElementType type = IOHIDElementGetType(native);
            if ((type != kIOHIDElementTypeInput_Axis) && (type != kIOHIDElementTypeInput_Button) &&
                (type != kIOHIDElementTypeInput_Misc))
            {
                continue;
            }

            CFMutableArrayRef target = NULL;

            const uint32_t usage = IOHIDElementGetUsage(native);
            const uint32_t page = IOHIDElementGetUsagePage(native);
            if (page == kHIDPage_GenericDesktop)
            {
                switch (usage)
                {
                    case kHIDUsage_GD_X:
                    case kHIDUsage_GD_Y:
                    case kHIDUsage_GD_Z:
                    case kHIDUsage_GD_Rx:
                    case kHIDUsage_GD_Ry:
                    case kHIDUsage_GD_Rz:
                    case kHIDUsage_GD_Slider:
                    case kHIDUsage_GD_Dial:
                    case kHIDUsage_GD_Wheel:
                        target = axes;
                        break;
                    case kHIDUsage_GD_Hatswitch:
                        target = hats;
                        break;
                    case kHIDUsage_GD_DPadUp:
                    case kHIDUsage_GD_DPadRight:
                    case kHIDUsage_GD_DPadDown:
                    case kHIDUsage_GD_DPadLeft:
                    case kHIDUsage_GD_SystemMainMenu:
                    case kHIDUsage_GD_Select:
                    case kHIDUsage_GD_Start:
                        target = buttons;
                        break;
                }
            }
            else if (page == kHIDPage_Simulation)
            {
                switch (usage)
                {
                    case kHIDUsage_Sim_Accelerator:
                    case kHIDUsage_Sim_Brake:
                    case kHIDUsage_Sim_Throttle:
                    case kHIDUsage_Sim_Rudder:
                    case kHIDUsage_Sim_Steering:
                        target = axes;
                        break;
                }
            }
            else if (page == kHIDPage_Button || page == kHIDPage_Consumer)
            {
                target = buttons;
            }

            if (target)
            {
                _GRWLjoyelementNS* element = _grwl_calloc(1, sizeof(_GRWLjoyelementNS));
                element->native = native;
                element->usage = usage;
                element->index = (int)CFArrayGetCount(target);
                element->minimum = IOHIDElementGetLogicalMin(native);
                element->maximum = IOHIDElementGetLogicalMax(native);
                CFArrayAppendValue(target, element);
            }
        }

        CFRelease(elements);
    }

    CFArraySortValues(axes, CFRangeMake(0, CFArrayGetCount(axes)), compareElements, NULL);
    CFArraySortValues(buttons, CFRangeMake(0, CFArrayGetCount(buttons)), compareElements, NULL);
    CFArraySortValues(hats, CFRangeMake(0, CFArrayGetCount(hats)), compareElements, NULL);

    js = _grwlAllocJoystick(name, guid, (int)CFArrayGetCount(axes), (int)CFArrayGetCount(buttons),
                            (int)CFArrayGetCount(hats));

    js->ns.device = device;
    js->ns.axes = axes;
    js->ns.buttons = buttons;
    js->ns.hats = hats;

    _grwlInputJoystick(js, GRWL_CONNECTED);
}

// Callback for user-initiated joystick removal
//
static void removeCallback(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
    for (int jid = 0; jid <= GRWL_JOYSTICK_LAST; jid++)
    {
        if (_grwl.joysticks[jid].connected && _grwl.joysticks[jid].ns.device == device)
        {
            closeJoystick(&_grwl.joysticks[jid]);
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlInitJoysticksCocoa()
{
    CFMutableArrayRef matching;
    const long usages[] = { kHIDUsage_GD_Joystick, kHIDUsage_GD_GamePad, kHIDUsage_GD_MultiAxisController };

    _grwl.ns.hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    matching = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (!matching)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to create array");
        return false;
    }

    for (size_t i = 0; i < sizeof(usages) / sizeof(long); i++)
    {
        const long page = kHIDPage_GenericDesktop;

        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks);
        if (!dict)
        {
            continue;
        }

        CFNumberRef pageRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &page);
        CFNumberRef usageRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &usages[i]);
        if (pageRef && usageRef)
        {
            CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), pageRef);
            CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usageRef);
            CFArrayAppendValue(matching, dict);
        }

        if (pageRef)
        {
            CFRelease(pageRef);
        }
        if (usageRef)
        {
            CFRelease(usageRef);
        }

        CFRelease(dict);
    }

    IOHIDManagerSetDeviceMatchingMultiple(_grwl.ns.hidManager, matching);
    CFRelease(matching);

    IOHIDManagerRegisterDeviceMatchingCallback(_grwl.ns.hidManager, &matchCallback, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(_grwl.ns.hidManager, &removeCallback, NULL);
    IOHIDManagerScheduleWithRunLoop(_grwl.ns.hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    IOHIDManagerOpen(_grwl.ns.hidManager, kIOHIDOptionsTypeNone);

    // Execute the run loop once in order to register any initially-attached
    // joysticks
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    return true;
}

void _grwlTerminateJoysticksCocoa()
{
    for (int jid = 0; jid <= GRWL_JOYSTICK_LAST; jid++)
    {
        if (_grwl.joysticks[jid].connected)
        {
            closeJoystick(&_grwl.joysticks[jid]);
        }
    }

    if (_grwl.ns.hidManager)
    {
        CFRelease(_grwl.ns.hidManager);
        _grwl.ns.hidManager = NULL;
    }
}

bool _grwlPollJoystickCocoa(_GRWLjoystick* js, int mode)
{
    if (mode & _GRWL_POLL_AXES)
    {
        for (CFIndex i = 0; i < CFArrayGetCount(js->ns.axes); i++)
        {
            _GRWLjoyelementNS* axis = (_GRWLjoyelementNS*)CFArrayGetValueAtIndex(js->ns.axes, i);

            const long raw = getElementValue(js, axis);
            // Perform auto calibration
            if (raw < axis->minimum)
            {
                axis->minimum = raw;
            }
            if (raw > axis->maximum)
            {
                axis->maximum = raw;
            }

            const long size = axis->maximum - axis->minimum;
            if (size == 0)
            {
                _grwlInputJoystickAxis(js, (int)i, 0.f);
            }
            else
            {
                const float value = (2.f * (raw - axis->minimum) / size) - 1.f;
                _grwlInputJoystickAxis(js, (int)i, value);
            }
        }
    }

    if (mode & _GRWL_POLL_BUTTONS)
    {
        for (CFIndex i = 0; i < CFArrayGetCount(js->ns.buttons); i++)
        {
            _GRWLjoyelementNS* button = (_GRWLjoyelementNS*)CFArrayGetValueAtIndex(js->ns.buttons, i);
            const char value = getElementValue(js, button) - button->minimum;
            const int state = (value > 0) ? GRWL_PRESS : GRWL_RELEASE;
            _grwlInputJoystickButton(js, (int)i, state);
        }

        for (CFIndex i = 0; i < CFArrayGetCount(js->ns.hats); i++)
        {
            const int states[9] = { GRWL_HAT_UP,         GRWL_HAT_RIGHT_UP, GRWL_HAT_RIGHT,
                                    GRWL_HAT_RIGHT_DOWN, GRWL_HAT_DOWN,     GRWL_HAT_LEFT_DOWN,
                                    GRWL_HAT_LEFT,       GRWL_HAT_LEFT_UP,  GRWL_HAT_CENTERED };

            _GRWLjoyelementNS* hat = (_GRWLjoyelementNS*)CFArrayGetValueAtIndex(js->ns.hats, i);
            long state = getElementValue(js, hat) - hat->minimum;
            if (state < 0 || state > 8)
            {
                state = 8;
            }

            _grwlInputJoystickHat(js, (int)i, states[state]);
        }
    }

    return js->connected;
}

const char* _grwlGetMappingNameCocoa()
{
    return "Mac OS X";
}

void _grwlUpdateGamepadGUIDCocoa(char* guid)
{
    if ((strncmp(guid + 4, "000000000000", 12) == 0) && (strncmp(guid + 20, "000000000000", 12) == 0))
    {
        char original[33];
        strncpy(original, guid, sizeof(original) - 1);
        sprintf(guid, "03000000%.4s0000%.4s000000000000", original, original + 16);
    }
}

#endif // _GRWL_COCOA
