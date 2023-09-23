//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
#pragma once

#if defined(_GRWL_COCOA)
    #include <cstdint>

    #include <Carbon/Carbon.h>
    #include <IOKit/hid/IOHIDLib.h>

    // NOTE: All of NSGL was deprecated in the 10.14 SDK
    //       This disables the pointless warnings for every symbol we use
    #ifndef GL_SILENCE_DEPRECATION
        #define GL_SILENCE_DEPRECATION
    #endif

    #if defined(__OBJC__)
        #import <Cocoa/Cocoa.h>
    #else
typedef void* id;
    #endif

// NOTE: Many Cocoa enum values have been renamed and we need to build across
//       SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the base SDK does not provide the newer names.

    #if MAC_OS_X_VERSION_MAX_ALLOWED < 101400
        #define NSOpenGLContextParameterSwapInterval NSOpenGLCPSwapInterval
        #define NSOpenGLContextParameterSurfaceOpacity NSOpenGLCPSurfaceOpacity
    #endif

    #if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
        #define NSBitmapFormatAlphaNonpremultiplied NSAlphaNonpremultipliedBitmapFormat
        #define NSEventMaskAny NSAnyEventMask
        #define NSEventMaskKeyUp NSKeyUpMask
        #define NSEventModifierFlagCapsLock NSAlphaShiftKeyMask
        #define NSEventModifierFlagCommand NSCommandKeyMask
        #define NSEventModifierFlagControl NSControlKeyMask
        #define NSEventModifierFlagDeviceIndependentFlagsMask NSDeviceIndependentModifierFlagsMask
        #define NSEventModifierFlagOption NSAlternateKeyMask
        #define NSEventModifierFlagShift NSShiftKeyMask
        #define NSEventTypeApplicationDefined NSApplicationDefined
        #define NSWindowStyleMaskBorderless NSBorderlessWindowMask
        #define NSWindowStyleMaskClosable NSClosableWindowMask
        #define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
        #define NSWindowStyleMaskResizable NSResizableWindowMask
        #define NSWindowStyleMaskTitled NSTitledWindowMask
    #endif

// NOTE: Many Cocoa dynamically linked constants have been renamed and we need
//       to build across SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the deployment target is older than the newer names.

    #if MAC_OS_X_VERSION_MIN_REQUIRED < 101300
        #define NSPasteboardTypeURL NSURLPboardType
    #endif

typedef VkFlags VkMacOSSurfaceCreateFlagsMVK;
typedef VkFlags VkMetalSurfaceCreateFlagsEXT;

typedef struct VkMacOSSurfaceCreateInfoMVK
{
    VkStructureType sType;
    const void* pNext;
    VkMacOSSurfaceCreateFlagsMVK flags;
    const void* pView;
} VkMacOSSurfaceCreateInfoMVK;

typedef struct VkMetalSurfaceCreateInfoEXT
{
    VkStructureType sType;
    const void* pNext;
    VkMetalSurfaceCreateFlagsEXT flags;
    const void* pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult(APIENTRY* PFN_vkCreateMacOSSurfaceMVK)(VkInstance, const VkMacOSSurfaceCreateInfoMVK*,
                                                        const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef VkResult(APIENTRY* PFN_vkCreateMetalSurfaceEXT)(VkInstance, const VkMetalSurfaceCreateInfoEXT*,
                                                        const VkAllocationCallbacks*, VkSurfaceKHR*);

    #define GRWL_COCOA_WINDOW_STATE _GRWLwindowNS ns;
    #define GRWL_COCOA_LIBRARY_WINDOW_STATE _GRWLlibraryNS ns;
    #define GRWL_COCOA_MONITOR_STATE _GRWLmonitorNS ns;
    #define GRWL_COCOA_CURSOR_STATE _GRWLcursorNS ns;

    #define GRWL_NSGL_CONTEXT_STATE _GRWLcontextNSGL nsgl;
    #define GRWL_NSGL_LIBRARY_CONTEXT_STATE _GRWLlibraryNSGL nsgl;
    #define GRWL_NSGL_USER_CONTEXT_STATE _GRWLusercontextNSGL nsgl;

    // HIToolbox.framework pointer typedefs
    #define kTISCategoryKeyboardInputSource _grwl.ns.tis.kCategoryKeyboardInputSource
    #define kTISPropertyInputSourceCategory _grwl.ns.tis.kPropertyInputSourceCategory
    #define kTISPropertyInputSourceID _grwl.ns.tis.kPropertyInputSourceID
    #define kTISPropertyInputSourceIsSelectCapable _grwl.ns.tis.kPropertyInputSourceIsSelectCapable
    #define kTISPropertyInputSourceType _grwl.ns.tis.kPropertyInputSourceType
    #define kTISPropertyUnicodeKeyLayoutData _grwl.ns.tis.kPropertyUnicodeKeyLayoutData
    #define kTISPropertyInputSourceID _grwl.ns.tis.kPropertyInputSourceID
    #define kTISPropertyLocalizedName _grwl.ns.tis.kPropertyLocalizedName
    #define kTISTypeKeyboardInputMethodModeEnabled _grwl.ns.tis.kTypeKeyboardInputMethodModeEnabled
typedef TISInputSourceRef (*PFN_TISCopyCurrentASCIICapableKeyboardInputSource)(void);
    #define TISCopyCurrentASCIICapableKeyboardInputSource _grwl.ns.tis.CopyCurrentASCIICapableKeyboardInputSource
typedef TISInputSourceRef (*PFN_TISCopyCurrentKeyboardInputSource)(void);
    #define TISCopyCurrentKeyboardInputSource _grwl.ns.tis.CopyCurrentKeyboardInputSource
typedef TISInputSourceRef (*PFN_TISCopyCurrentKeyboardLayoutInputSource)(void);
    #define TISCopyCurrentKeyboardLayoutInputSource _grwl.ns.tis.CopyCurrentKeyboardLayoutInputSource
typedef TISInputSourceRef (*PFN_TISCopyInputSourceForLanguage)(CFStringRef);
    #define TISCopyInputSourceForLanguage _grwl.ns.tis.CopyInputSourceForLanguage
typedef CFArrayRef (*PFN_TISCreateASCIICapableInputSourceList)(void);
    #define TISCreateASCIICapableInputSourceList _grwl.ns.tis.CreateASCIICapableInputSourceList
typedef CFArrayRef (*PEN_TISCreateInputSourceList)(CFDictionaryRef, Boolean);
    #define TISCreateInputSourceList _grwl.ns.tis.CreateInputSourceList
typedef void* (*PFN_TISGetInputSourceProperty)(TISInputSourceRef, CFStringRef);
    #define TISGetInputSourceProperty _grwl.ns.tis.GetInputSourceProperty
typedef OSStatus (*PFN_TISSelectInputSource)(TISInputSourceRef);
    #define TISSelectInputSource _grwl.ns.tis.SelectInputSource
typedef UInt8 (*PFN_LMGetKbdType)(void);
    #define LMGetKbdType _grwl.ns.tis.GetKbdType

// NSGL-specific per-context data
//
typedef struct _GRWLcontextNSGL
{
    id pixelFormat;
    id object;
} _GRWLcontextNSGL;

// NSGL-specific global data
//
typedef struct _GRWLlibraryNSGL
{
    // dlopen handle for OpenGL.framework (for grwlGetProcAddress)
    CFBundleRef framework;
} _GRWLlibraryNSGL;

// NSGL-specific per usercontext data
//
typedef struct _GRWLusercontextNSGL
{
    id object;

} _GRWLusercontextNSGL;

// Cocoa-specific per-window data
//
typedef struct _GRWLwindowNS
{
    id object;
    id delegate;
    id view;
    id layer;

    GRWLbool maximized;
    GRWLbool occluded;
    GRWLbool retina;

    // Cached window properties to filter out duplicate events
    int width, height;
    int fbWidth, fbHeight;
    float xscale, yscale;

    // The total sum of the distances the cursor has been warped
    // since the last cursor motion event was processed
    // This is kept to counteract Cocoa doing the same internally
    double cursorWarpDeltaX, cursorWarpDeltaY;

    struct
    {
        int state;
        double value;
    } dockProgressIndicator;
} _GRWLwindowNS;

// Cocoa-specific global data
//
typedef struct _GRWLlibraryNS
{
    CGEventSourceRef eventSource;
    id delegate;
    GRWLbool cursorHidden;
    TISInputSourceRef inputSource;
    TISInputSourceRef keyboardLayout;
    IOHIDManagerRef hidManager;
    void* unicodeData;
    id helper;
    id keyUpMonitor;
    id nibObjects;

    char keynames[GRWL_KEY_LAST + 1][17];
    short int keycodes[256];
    short int scancodes[GRWL_KEY_LAST + 1];
    char* clipboardString;
    char* keyboardLayoutName;
    CGPoint cascadePoint;
    // Where to place the cursor when re-enabled
    double restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _GRWLwindow* disabledCursorWindow;

    struct
    {
        CFBundleRef bundle;
        PFN_TISCopyCurrentASCIICapableKeyboardInputSource CopyCurrentASCIICapableKeyboardInputSource;
        PFN_TISCopyCurrentKeyboardInputSource CopyCurrentKeyboardInputSource;
        PFN_TISCopyCurrentKeyboardLayoutInputSource CopyCurrentKeyboardLayoutInputSource;
        PFN_TISCopyInputSourceForLanguage CopyInputSourceForLanguage;
        PFN_TISCreateASCIICapableInputSourceList CreateASCIICapableInputSourceList;
        PEN_TISCreateInputSourceList CreateInputSourceList;
        PFN_TISGetInputSourceProperty GetInputSourceProperty;
        PFN_TISSelectInputSource SelectInputSource;
        PFN_LMGetKbdType GetKbdType;
        CFStringRef kCategoryKeyboardInputSource;
        CFStringRef kPropertyInputSourceCategory;
        CFStringRef kPropertyInputSourceID;
        CFStringRef kPropertyInputSourceIsSelectCapable;
        CFStringRef kPropertyInputSourceType;
        CFStringRef kPropertyUnicodeKeyLayoutData;
        CFStringRef kPropertyInputSourceID;
        CFStringRef kPropertyLocalizedName;
        CFStringRef kTypeKeyboardInputMethodModeEnabled;
    } tis;

    struct
    {
        id view;
        int windowCount;
        int indeterminateCount;
        double totalValue;
    } dockProgressIndicator;
} _GRWLlibraryNS;

// Cocoa-specific per-monitor data
//
typedef struct _GRWLmonitorNS
{
    CGDirectDisplayID displayID;
    CGDisplayModeRef previousMode;
    uint32_t unitNumber;
    id screen;
    double fallbackRefreshRate;
} _GRWLmonitorNS;

// Cocoa-specific per-cursor data
//
typedef struct _GRWLcursorNS
{
    id object;
} _GRWLcursorNS;

GRWLbool _grwlConnectCocoa(int platformID, _GRWLplatform* platform);
int _grwlInitCocoa(void);
void _grwlTerminateCocoa(void);

GRWLbool _grwlCreateWindowCocoa(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                                const _GRWLfbconfig* fbconfig);
void _grwlDestroyWindowCocoa(_GRWLwindow* window);
void _grwlSetWindowTitleCocoa(_GRWLwindow* window, const char* title);
void _grwlSetWindowIconCocoa(_GRWLwindow* window, int count, const GRWLimage* images);
void _grwlSetWindowProgressIndicatorCocoa(_GRWLwindow* window, int progressState, double value);
void _grwlSetWindowBadgeCocoa(_GRWLwindow* window, int count);
void _grwlSetWindowBadgeStringCocoa(_GRWLwindow* window, const char* string);
void _grwlGetWindowPosCocoa(_GRWLwindow* window, int* xpos, int* ypos);
void _grwlSetWindowPosCocoa(_GRWLwindow* window, int xpos, int ypos);
void _grwlGetWindowSizeCocoa(_GRWLwindow* window, int* width, int* height);
void _grwlSetWindowSizeCocoa(_GRWLwindow* window, int width, int height);
void _grwlSetWindowSizeLimitsCocoa(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _grwlSetWindowAspectRatioCocoa(_GRWLwindow* window, int numer, int denom);
void _grwlGetFramebufferSizeCocoa(_GRWLwindow* window, int* width, int* height);
void _grwlGetWindowFrameSizeCocoa(_GRWLwindow* window, int* left, int* top, int* right, int* bottom);
void _grwlGetWindowContentScaleCocoa(_GRWLwindow* window, float* xscale, float* yscale);
void _grwlIconifyWindowCocoa(_GRWLwindow* window);
void _grwlRestoreWindowCocoa(_GRWLwindow* window);
void _grwlMaximizeWindowCocoa(_GRWLwindow* window);
void _grwlShowWindowCocoa(_GRWLwindow* window);
void _grwlHideWindowCocoa(_GRWLwindow* window);
void _grwlRequestWindowAttentionCocoa(_GRWLwindow* window);
void _grwlFocusWindowCocoa(_GRWLwindow* window);
void _grwlSetWindowMonitorCocoa(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                                int refreshRate);
GRWLbool _grwlWindowFocusedCocoa(_GRWLwindow* window);
GRWLbool _grwlWindowIconifiedCocoa(_GRWLwindow* window);
GRWLbool _grwlWindowVisibleCocoa(_GRWLwindow* window);
GRWLbool _grwlWindowMaximizedCocoa(_GRWLwindow* window);
GRWLbool _grwlWindowHoveredCocoa(_GRWLwindow* window);
GRWLbool _grwlFramebufferTransparentCocoa(_GRWLwindow* window);
void _grwlSetWindowResizableCocoa(_GRWLwindow* window, GRWLbool enabled);
void _grwlSetWindowDecoratedCocoa(_GRWLwindow* window, GRWLbool enabled);
void _grwlSetWindowFloatingCocoa(_GRWLwindow* window, GRWLbool enabled);
float _grwlGetWindowOpacityCocoa(_GRWLwindow* window);
void _grwlSetWindowOpacityCocoa(_GRWLwindow* window, float opacity);
void _grwlSetWindowMousePassthroughCocoa(_GRWLwindow* window, GRWLbool enabled);

void _grwlSetRawMouseMotionCocoa(_GRWLwindow* window, GRWLbool enabled);
GRWLbool _grwlRawMouseMotionSupportedCocoa(void);

void _grwlPollEventsCocoa(void);
void _grwlWaitEventsCocoa(void);
void _grwlWaitEventsTimeoutCocoa(double timeout);
void _grwlPostEmptyEventCocoa(void);

void _grwlGetCursorPosCocoa(_GRWLwindow* window, double* xpos, double* ypos);
void _grwlSetCursorPosCocoa(_GRWLwindow* window, double xpos, double ypos);
void _grwlSetCursorModeCocoa(_GRWLwindow* window, int mode);
const char* _grwlGetScancodeNameCocoa(int scancode);
int _grwlGetKeyScancodeCocoa(int key);
const char* _grwlGetKeyboardLayoutNameCocoa(void);
GRWLbool _grwlCreateCursorCocoa(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot);
GRWLbool _grwlCreateStandardCursorCocoa(_GRWLcursor* cursor, int shape);
void _grwlDestroyCursorCocoa(_GRWLcursor* cursor);
void _grwlSetCursorCocoa(_GRWLwindow* window, _GRWLcursor* cursor);
void _grwlSetClipboardStringCocoa(const char* string);
const char* _grwlGetClipboardStringCocoa(void);

void _grwlUpdatePreeditCursorRectangleCocoa(_GRWLwindow* window);
void _grwlResetPreeditTextCocoa(_GRWLwindow* window);
void _grwlSetIMEStatusCocoa(_GRWLwindow* window, int active);
int _grwlGetIMEStatusCocoa(_GRWLwindow* window);

EGLenum _grwlGetEGLPlatformCocoa(EGLint** attribs);
EGLNativeDisplayType _grwlGetEGLNativeDisplayCocoa(void);
EGLNativeWindowType _grwlGetEGLNativeWindowCocoa(_GRWLwindow* window);

void _grwlGetRequiredInstanceExtensionsCocoa(char** extensions);
GRWLbool _grwlGetPhysicalDevicePresentationSupportCocoa(VkInstance instance, VkPhysicalDevice device,
                                                        uint32_t queuefamily);
VkResult _grwlCreateWindowSurfaceCocoa(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR* surface);

void _grwlFreeMonitorCocoa(_GRWLmonitor* monitor);
void _grwlGetMonitorPosCocoa(_GRWLmonitor* monitor, int* xpos, int* ypos);
void _grwlGetMonitorContentScaleCocoa(_GRWLmonitor* monitor, float* xscale, float* yscale);
void _grwlGetMonitorWorkareaCocoa(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GRWLvidmode* _grwlGetVideoModesCocoa(_GRWLmonitor* monitor, int* count);
void _grwlGetVideoModeCocoa(_GRWLmonitor* monitor, GRWLvidmode* mode);
GRWLbool _grwlGetGammaRampCocoa(_GRWLmonitor* monitor, GRWLgammaramp* ramp);
void _grwlSetGammaRampCocoa(_GRWLmonitor* monitor, const GRWLgammaramp* ramp);

void _grwlPollMonitorsCocoa(void);
void _grwlSetVideoModeCocoa(_GRWLmonitor* monitor, const GRWLvidmode* desired);
void _grwlRestoreVideoModeCocoa(_GRWLmonitor* monitor);

float _grwlTransformYCocoa(float y);

void* _grwlLoadLocalVulkanLoaderCocoa(void);

GRWLbool _grwlInitNSGL(void);
void _grwlTerminateNSGL(void);
GRWLbool _grwlCreateContextNSGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig);
void _grwlDestroyContextNSGL(_GRWLwindow* window);

_GRWLusercontext* _grwlCreateUserContextCocoa(_GRWLwindow* window);
_GRWLusercontext* _grwlCreateUserContextNSGL(_GRWLwindow* window);

#endif
