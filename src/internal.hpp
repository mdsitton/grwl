//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#pragma once

#if defined(_GRWL_USE_CONFIG_H)
    #include "grwl_config.h"
#endif

#if defined(GRWL_INCLUDE_GLCOREARB) || defined(GRWL_INCLUDE_ES1) || defined(GRWL_INCLUDE_ES2) || \
    defined(GRWL_INCLUDE_ES3) || defined(GRWL_INCLUDE_ES31) || defined(GRWL_INCLUDE_ES32) ||     \
    defined(GRWL_INCLUDE_NONE) || defined(GRWL_INCLUDE_GLEXT) || defined(GRWL_INCLUDE_GLU) ||    \
    defined(GRWL_INCLUDE_VULKAN) || defined(GRWL_DLL)
    #error "You must not define any header option macros when compiling GRWL"
#endif

#define GRWL_INCLUDE_NONE
#include "../include/GRWL/grwl.h"

#define _GRWL_INSERT_FIRST 0
#define _GRWL_INSERT_LAST 1

#define _GRWL_POLL_PRESENCE 0
#define _GRWL_POLL_AXES 1
#define _GRWL_POLL_BUTTONS 2
#define _GRWL_POLL_ALL (_GRWL_POLL_AXES | _GRWL_POLL_BUTTONS)

#define _GRWL_MESSAGE_SIZE 1024

typedef void (*GRWLproc)();

typedef struct _GRWLerror _GRWLerror;
typedef struct _GRWLinitconfig _GRWLinitconfig;
typedef struct _GRWLwndconfig _GRWLwndconfig;
typedef struct _GRWLctxconfig _GRWLctxconfig;
typedef struct _GRWLfbconfig _GRWLfbconfig;
typedef struct _GRWLcontext _GRWLcontext;
typedef struct _GRWLpreedit _GRWLpreedit;
typedef struct _GRWLpreeditcandidate _GRWLpreeditcandidate;
typedef struct _GRWLwindow _GRWLwindow;
typedef struct _GRWLplatform _GRWLplatform;
typedef struct _GRWLlibrary _GRWLlibrary;
typedef struct _GRWLmonitor _GRWLmonitor;
typedef struct _GRWLcursor _GRWLcursor;
typedef struct _GRWLmapelement _GRWLmapelement;
typedef struct _GRWLmapping _GRWLmapping;
typedef struct _GRWLusbinfo _GRWLusbinfo;
typedef struct _GRWLjoystick _GRWLjoystick;
typedef struct _GRWLtls _GRWLtls;
typedef struct _GRWLmutex _GRWLmutex;
typedef struct _GRWLusercontext _GRWLusercontext;

#define GL_VERSION 0x1f02
#define GL_NONE 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_UNSIGNED_BYTE 0x1401
#define GL_EXTENSIONS 0x1f03
#define GL_NUM_EXTENSIONS 0x821d
#define GL_CONTEXT_FLAGS 0x821e
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GL_NO_RESET_NOTIFICATION_ARB 0x8261
#define GL_CONTEXT_RELEASE_BEHAVIOR 0x82fb
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82fc
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x00000008

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;

typedef void(APIENTRY* PFNGLCLEARPROC)(GLbitfield);
typedef const GLubyte*(APIENTRY* PFNGLGETSTRINGPROC)(GLenum);
typedef void(APIENTRY* PFNGLGETINTEGERVPROC)(GLenum, GLint*);
typedef const GLubyte*(APIENTRY* PFNGLGETSTRINGIPROC)(GLenum, GLuint);

#if defined(_GRWL_WIN32)
    #define EGLAPIENTRY __stdcall
#else
    #define EGLAPIENTRY
#endif

#define EGL_SUCCESS 0x3000
#define EGL_NOT_INITIALIZED 0x3001
#define EGL_BAD_ACCESS 0x3002
#define EGL_BAD_ALLOC 0x3003
#define EGL_BAD_ATTRIBUTE 0x3004
#define EGL_BAD_CONFIG 0x3005
#define EGL_BAD_CONTEXT 0x3006
#define EGL_BAD_CURRENT_SURFACE 0x3007
#define EGL_BAD_DISPLAY 0x3008
#define EGL_BAD_MATCH 0x3009
#define EGL_BAD_NATIVE_PIXMAP 0x300a
#define EGL_BAD_NATIVE_WINDOW 0x300b
#define EGL_BAD_PARAMETER 0x300c
#define EGL_BAD_SURFACE 0x300d
#define EGL_CONTEXT_LOST 0x300e
#define EGL_COLOR_BUFFER_TYPE 0x303f
#define EGL_RGB_BUFFER 0x308e
#define EGL_SURFACE_TYPE 0x3033
#define EGL_WINDOW_BIT 0x0004
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_OPENGL_ES_BIT 0x0001
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_OPENGL_BIT 0x0008
#define EGL_ALPHA_SIZE 0x3021
#define EGL_BLUE_SIZE 0x3022
#define EGL_GREEN_SIZE 0x3023
#define EGL_RED_SIZE 0x3024
#define EGL_DEPTH_SIZE 0x3025
#define EGL_STENCIL_SIZE 0x3026
#define EGL_SAMPLES 0x3031
#define EGL_OPENGL_ES_API 0x30a0
#define EGL_OPENGL_API 0x30a2
#define EGL_NONE 0x3038
#define EGL_RENDER_BUFFER 0x3086
#define EGL_SINGLE_BUFFER 0x3085
#define EGL_EXTENSIONS 0x3055
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_NATIVE_VISUAL_ID 0x302e
#define EGL_NO_SURFACE ((EGLSurface)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_DEFAULT_DISPLAY ((EGLNativeDisplayType)0)
#define EGL_PBUFFER_BIT 0x0001
#define EGL_HEIGHT 0x3056
#define EGL_WIDTH 0x3057

#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR 0x00000002
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR 0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR 0x00000002
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR 0x00000001
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR 0x31bd
#define EGL_NO_RESET_NOTIFICATION_KHR 0x31be
#define EGL_LOSE_CONTEXT_ON_RESET_KHR 0x31bf
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR 0x00000004
#define EGL_CONTEXT_MAJOR_VERSION_KHR 0x3098
#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30fb
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR 0x30fd
#define EGL_CONTEXT_FLAGS_KHR 0x30fc
#define EGL_CONTEXT_OPENGL_NO_ERROR_KHR 0x31b3
#define EGL_GL_COLORSPACE_KHR 0x309d
#define EGL_GL_COLORSPACE_SRGB_KHR 0x3089
#define EGL_CONTEXT_RELEASE_BEHAVIOR_KHR 0x2097
#define EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR 0
#define EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR 0x2098
#define EGL_PLATFORM_X11_EXT 0x31d5
#define EGL_PLATFORM_WAYLAND_EXT 0x31d8
#define EGL_PRESENT_OPAQUE_EXT 0x31df
#define EGL_PLATFORM_ANGLE_ANGLE 0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE 0x3203
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE 0x320d
#define EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE 0x320e
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE 0x3207
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE 0x3208
#define EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE 0x3450
#define EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE 0x3489
#define EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE 0x348f

typedef int EGLint;
typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLDisplay;
typedef void* EGLSurface;

typedef void* EGLNativeDisplayType;
typedef void* EGLNativeWindowType;

// EGL function pointer typedefs
typedef EGLBoolean(EGLAPIENTRY* PFN_eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint, EGLint*);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglGetConfigs)(EGLDisplay, EGLConfig*, EGLint, EGLint*);
typedef EGLDisplay(EGLAPIENTRY* PFN_eglGetDisplay)(EGLNativeDisplayType);
typedef EGLint(EGLAPIENTRY* PFN_eglGetError)();
typedef EGLBoolean(EGLAPIENTRY* PFN_eglInitialize)(EGLDisplay, EGLint*, EGLint*);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglTerminate)(EGLDisplay);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglBindAPI)(EGLenum);
typedef EGLContext(EGLAPIENTRY* PFN_eglCreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglDestroySurface)(EGLDisplay, EGLSurface);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglDestroyContext)(EGLDisplay, EGLContext);
typedef EGLSurface(EGLAPIENTRY* PFN_eglCreateWindowSurface)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglSwapBuffers)(EGLDisplay, EGLSurface);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglSwapInterval)(EGLDisplay, EGLint);
typedef const char*(EGLAPIENTRY* PFN_eglQueryString)(EGLDisplay, EGLint);
typedef GRWLglproc(EGLAPIENTRY* PFN_eglGetProcAddress)(const char*);
typedef EGLSurface(EGLAPIENTRY* PFN_eglCreatePbufferSurface)(EGLDisplay, EGLConfig, const EGLint*);
typedef EGLBoolean(EGLAPIENTRY* PFN_eglChooseConfig)(EGLDisplay, EGLint const*, EGLConfig*, EGLint, EGLint*);
#define eglGetConfigAttrib _grwl.egl.GetConfigAttrib
#define eglGetConfigs _grwl.egl.GetConfigs
#define eglGetDisplay _grwl.egl.GetDisplay
#define eglGetError _grwl.egl.GetError
#define eglInitialize _grwl.egl.Initialize
#define eglTerminate _grwl.egl.Terminate
#define eglBindAPI _grwl.egl.BindAPI
#define eglCreateContext _grwl.egl.CreateContext
#define eglDestroySurface _grwl.egl.DestroySurface
#define eglDestroyContext _grwl.egl.DestroyContext
#define eglCreateWindowSurface _grwl.egl.CreateWindowSurface
#define eglMakeCurrent _grwl.egl.MakeCurrent
#define eglSwapBuffers _grwl.egl.SwapBuffers
#define eglSwapInterval _grwl.egl.SwapInterval
#define eglQueryString _grwl.egl.QueryString
#define eglGetProcAddress _grwl.egl.GetProcAddress
#define eglCreatePbufferSurface _grwl.egl.CreatePbufferSurface
#define eglChooseConfig _grwl.egl.ChooseConfig

typedef EGLDisplay(EGLAPIENTRY* PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum, void*, const EGLint*);
typedef EGLSurface(EGLAPIENTRY* PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)(EGLDisplay, EGLConfig, void*, const EGLint*);
#define eglGetPlatformDisplayEXT _grwl.egl.GetPlatformDisplayEXT
#define eglCreatePlatformWindowSurfaceEXT _grwl.egl.CreatePlatformWindowSurfaceEXT

#define VK_NULL_HANDLE 0

typedef void* VkInstance;
typedef void* VkPhysicalDevice;
typedef uint64_t VkSurfaceKHR;
typedef uint32_t VkFlags;
typedef uint32_t VkBool32;

typedef enum VkStructureType
{
    VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR = 1000004000,
    VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR = 1000005000,
    VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR = 1000006000,
    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR = 1000009000,
    VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK = 1000123000,
    VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT = 1000217000,
    VK_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} VkStructureType;

typedef enum VkResult
{
    VK_SUCCESS = 0,
    VK_NOT_READY = 1,
    VK_TIMEOUT = 2,
    VK_EVENT_SET = 3,
    VK_EVENT_RESET = 4,
    VK_INCOMPLETE = 5,
    VK_ERROR_OUT_OF_HOST_MEMORY = -1,
    VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
    VK_ERROR_INITIALIZATION_FAILED = -3,
    VK_ERROR_DEVICE_LOST = -4,
    VK_ERROR_MEMORY_MAP_FAILED = -5,
    VK_ERROR_LAYER_NOT_PRESENT = -6,
    VK_ERROR_EXTENSION_NOT_PRESENT = -7,
    VK_ERROR_FEATURE_NOT_PRESENT = -8,
    VK_ERROR_INCOMPATIBLE_DRIVER = -9,
    VK_ERROR_TOO_MANY_OBJECTS = -10,
    VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
    VK_ERROR_SURFACE_LOST_KHR = -1000000000,
    VK_SUBOPTIMAL_KHR = 1000001003,
    VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
    VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
    VK_ERROR_NATIVE_WINDOW_IN_USE_KHR = -1000000001,
    VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
    VK_RESULT_MAX_ENUM = 0x7FFFFFFF
} VkResult;

typedef struct VkAllocationCallbacks VkAllocationCallbacks;

typedef struct VkExtensionProperties
{
    char extensionName[256];
    uint32_t specVersion;
} VkExtensionProperties;

typedef void(APIENTRY* PFN_vkVoidFunction)();

typedef PFN_vkVoidFunction(APIENTRY* PFN_vkGetInstanceProcAddr)(VkInstance, const char*);
typedef VkResult(APIENTRY* PFN_vkEnumerateInstanceExtensionProperties)(const char*, uint32_t*, VkExtensionProperties*);
#define vkGetInstanceProcAddr _grwl.vk.GetInstanceProcAddr

#include "platform.hpp"

// Checks for whether the library has been initialized
#define _GRWL_REQUIRE_INIT()                            \
    if (!_grwl.initialized)                             \
    {                                                   \
        _grwlInputError(GRWL_NOT_INITIALIZED, nullptr); \
        return;                                         \
    }
#define _GRWL_REQUIRE_INIT_OR_RETURN(x)                 \
    if (!_grwl.initialized)                             \
    {                                                   \
        _grwlInputError(GRWL_NOT_INITIALIZED, nullptr); \
        return x;                                       \
    }

// Swaps the provided pointers
#define _GRWL_SWAP(type, x, y) \
    {                          \
        type t;                \
        t = x;                 \
        x = y;                 \
        y = t;                 \
    }

// Per-thread error structure
//
struct _GRWLerror
{
    _GRWLerror* next;
    int code;
    char description[_GRWL_MESSAGE_SIZE];
};

// Initialization configuration
//
// Parameters relating to the initialization of the library
//
struct _GRWLinitconfig
{
    bool hatButtons;
    int angleType;
    int platformID;
    bool managePreeditCandidate;
    PFN_vkGetInstanceProcAddr vulkanLoader;

    struct
    {
        bool menubar;
        bool chdir;
    } ns;

    struct
    {
        bool xcbVulkanSurface;
        bool onTheSpotIMStyle;
    } x11;

    struct
    {
        int libdecorMode;
    } wl;
};

// Window configuration
//
// Parameters relating to the creation of the window but not directly related
// to the framebuffer.  This is used to pass window creation parameters from
// shared code to the platform API.
//
struct _GRWLwndconfig
{
    int xpos;
    int ypos;
    int width;
    int height;
    const char* title;
    bool resizable;
    bool visible;
    bool decorated;
    bool focused;
    bool autoIconify;
    bool floating;
    bool maximized;
    bool centerCursor;
    bool focusOnShow;
    bool mousePassthrough;
    bool scaleToMonitor;
    bool softFullscreen;

    struct
    {
        bool retina;
        char frameName[256];
    } ns;

    struct
    {
        char className[256];
        char instanceName[256];
    } x11;

    struct
    {
        bool keymenu;
        bool genericBadge;
    } win32;

    struct
    {
        char appId[256];
    } wl;
};

// Context configuration
//
// Parameters relating to the creation of the context but not directly related
// to the framebuffer.  This is used to pass context creation parameters from
// shared code to the platform API.
//
struct _GRWLctxconfig
{
    int client;
    int source;
    int major;
    int minor;
    bool forward;
    bool debug;
    bool noerror;
    int profile;
    int robustness;
    int release;
    _GRWLwindow* share;

    struct
    {
        bool offline;
    } nsgl;
};

// Framebuffer configuration
//
// This describes buffers and their sizes.  It also contains
// a platform-specific ID used to map back to the backend API object.
//
// It is used to pass framebuffer parameters from shared code to the platform
// API and also to enumerate and select available framebuffer configs.
//
struct _GRWLfbconfig
{
    int redBits;
    int greenBits;
    int blueBits;
    int alphaBits;
    int depthBits;
    int stencilBits;
    int accumRedBits;
    int accumGreenBits;
    int accumBlueBits;
    int accumAlphaBits;
    int auxBuffers;
    bool stereo;
    int samples;
    bool sRGB;
    bool doublebuffer;
    bool transparent;
    uintptr_t handle;
};

// Context structure
//
struct _GRWLcontext
{
    int client;
    int source;
    int major, minor, revision;
    bool forward, debug, noerror;
    int profile;
    int robustness;
    int release;

    PFNGLGETSTRINGIPROC GetStringi;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLGETSTRINGPROC GetString;

    void (*makeCurrent)(_GRWLwindow*);
    void (*swapBuffers)(_GRWLwindow*);
    void (*swapInterval)(int);
    int (*extensionSupported)(const char*);
    GRWLglproc (*getProcAddress)(const char*);
    void (*destroy)(_GRWLwindow*);

    struct
    {
        EGLConfig config;
        EGLContext handle;
        EGLSurface surface;
        void* client;
    } egl;

    // This is defined in platform.h
    GRWL_PLATFORM_CONTEXT_STATE
};

// Preedit structure for Input Method Editor/Engine
//
struct _GRWLpreedit
{
    unsigned int* text;
    int textCount;
    int textBufferCount;
    int* blockSizes;
    int blockSizesCount;
    int blockSizesBufferCount;
    int focusedBlockIndex;
    int caretIndex;
    int cursorPosX, cursorPosY, cursorWidth, cursorHeight;

    // Used only when apps display candidates by themselves.
    // Usually, OS displays them, so apps don't need to do it.
    _GRWLpreeditcandidate* candidates;
    int candidateCount;
    int candidateBufferCount;
    int candidateSelection;
    int candidatePageStart;
    int candidatePageSize;
};

// Preedit candidate structure
//
struct _GRWLpreeditcandidate
{
    unsigned int* text;
    int textCount;
    int textBufferCount;
};

// User Context structure
//
struct _GRWLusercontext
{
    _GRWLwindow* window;

    void (*makeCurrent)(_GRWLusercontext* context);
    void (*destroy)(_GRWLusercontext* context);

    struct
    {
        EGLContext handle;
        EGLSurface surface;
    } egl;

    // This is defined in platform.h
    GRWL_PLATFORM_USER_CONTEXT_STATE
};

// Window and context structure
//
struct _GRWLwindow
{
    struct _GRWLwindow* next;

    // Window settings and state
    bool resizable;
    bool decorated;
    bool autoIconify;
    bool floating;
    bool focusOnShow;
    bool mousePassthrough;
    bool shouldClose;
    void* userPointer;
    bool doublebuffer;
    GRWLvidmode videoMode;
    _GRWLmonitor* monitor;
    _GRWLcursor* cursor;

    int minwidth, minheight;
    int maxwidth, maxheight;
    int numer, denom;

    bool stickyKeys;
    bool stickyMouseButtons;
    bool lockKeyMods;
    int cursorMode;
    char mouseButtons[GRWL_MOUSE_BUTTON_LAST + 1];
    char keys[GRWL_KEY_LAST + 1];
    // Virtual cursor position when cursor is disabled
    double virtualCursorPosX, virtualCursorPosY;
    bool rawMouseMotion;

    _GRWLcontext context;

    _GRWLpreedit preedit;

    struct
    {
        GRWLwindowposfun pos;
        GRWLwindowsizefun size;
        GRWLwindowclosefun close;
        GRWLwindowrefreshfun refresh;
        GRWLwindowfocusfun focus;
        GRWLwindowiconifyfun iconify;
        GRWLwindowmaximizefun maximize;
        GRWLframebuffersizefun fbsize;
        GRWLwindowcontentscalefun scale;
        GRWLmousebuttonfun mouseButton;
        GRWLcursorposfun cursorPos;
        GRWLcursorenterfun cursorEnter;
        GRWLscrollfun scroll;
        GRWLkeyfun key;
        GRWLcharfun character;
        GRWLcharmodsfun charmods;
        GRWLpreeditfun preedit;
        GRWLimestatusfun imestatus;
        GRWLpreeditcandidatefun preeditCandidate;
        GRWLdropfun drop;
    } callbacks;

    // This is defined in platform.h
    GRWL_PLATFORM_WINDOW_STATE
};

// Monitor structure
//
struct _GRWLmonitor
{
    char name[128];
    void* userPointer;

    // Physical dimensions in millimeters.
    int widthMM, heightMM;

    // The window whose video mode is current on this monitor
    _GRWLwindow* window;

    GRWLvidmode* modes;
    int modeCount;
    GRWLvidmode currentMode;

    // This is defined in platform.h
    GRWL_PLATFORM_MONITOR_STATE
};

// Cursor structure
//
struct _GRWLcursor
{
    _GRWLcursor* next;
    // This is defined in platform.h
    GRWL_PLATFORM_CURSOR_STATE
};

// Gamepad mapping element structure
//
struct _GRWLmapelement
{
    uint8_t type;
    uint8_t index;
    int8_t axisScale;
    int8_t axisOffset;
};

// Gamepad mapping structure
//
struct _GRWLmapping
{
    char name[128];
    char guid[33];
    _GRWLmapelement buttons[15];
    _GRWLmapelement axes[6];
};

// USB vendor, product, version
//
struct _GRWLusbinfo
{
    uint16_t bustype;
    uint16_t vendor;
    uint16_t product;
    uint16_t version;
};

// Joystick structure
//
struct _GRWLjoystick
{
    bool allocated;
    bool connected;
    float* axes;
    int axisCount;
    unsigned char* buttons;
    int buttonCount;
    unsigned char* hats;
    int hatCount;
    char name[128];
    void* userPointer;
    char guid[33];
    _GRWLusbinfo usbInfo;
    _GRWLmapping* mapping;

    // This is defined in platform.h
    GRWL_PLATFORM_JOYSTICK_STATE
};

// Thread local storage structure
//
struct _GRWLtls
{
    // This is defined in platform.h
    GRWL_PLATFORM_TLS_STATE
};

// Mutex structure
//
struct _GRWLmutex
{
    // This is defined in platform.h
    GRWL_PLATFORM_MUTEX_STATE
};

// Platform API structure
//
struct _GRWLplatform
{
    int platformID;
    // init
    bool (*init)();
    void (*terminate)();
    // input
    void (*getCursorPos)(_GRWLwindow*, double*, double*);
    void (*setCursorPos)(_GRWLwindow*, double, double);
    void (*setCursorMode)(_GRWLwindow*, int);
    void (*setRawMouseMotion)(_GRWLwindow*, bool);
    bool (*rawMouseMotionSupported)();
    bool (*createCursor)(_GRWLcursor*, const GRWLimage*, int, int);
    bool (*createStandardCursor)(_GRWLcursor*, int);
    void (*destroyCursor)(_GRWLcursor*);
    void (*setCursor)(_GRWLwindow*, _GRWLcursor*);
    const char* (*getScancodeName)(int);
    int (*getKeyScancode)(int);
    const char* (*getKeyboardLayoutName)();
    void (*setClipboardString)(const char*);
    const char* (*getClipboardString)();
    void (*updatePreeditCursorRectangle)(_GRWLwindow*);
    void (*resetPreeditText)(_GRWLwindow*);
    void (*setIMEStatus)(_GRWLwindow*, int);
    int (*getIMEStatus)(_GRWLwindow*);
    bool (*initJoysticks)();
    void (*terminateJoysticks)();
    bool (*pollJoystick)(_GRWLjoystick*, int);
    const char* (*getMappingName)();
    void (*updateGamepadGUID)(char*);
    // monitor
    void (*freeMonitor)(_GRWLmonitor*);
    void (*getMonitorPos)(_GRWLmonitor*, int*, int*);
    void (*getMonitorContentScale)(_GRWLmonitor*, float*, float*);
    void (*getMonitorWorkarea)(_GRWLmonitor*, int*, int*, int*, int*);
    GRWLvidmode* (*getVideoModes)(_GRWLmonitor*, int*);
    void (*getVideoMode)(_GRWLmonitor*, GRWLvidmode*);
    // window
    bool (*createWindow)(_GRWLwindow*, const _GRWLwndconfig*, const _GRWLctxconfig*, const _GRWLfbconfig*);
    void (*destroyWindow)(_GRWLwindow*);
    void (*setWindowTitle)(_GRWLwindow*, const char*);
    void (*setWindowIcon)(_GRWLwindow*, int, const GRWLimage*);
    void (*setWindowProgressIndicator)(_GRWLwindow*, const int, double);
    void (*setWindowBadge)(_GRWLwindow*, int);
    void (*setWindowBadgeString)(_GRWLwindow*, const char* string);
    void (*getWindowPos)(_GRWLwindow*, int*, int*);
    void (*setWindowPos)(_GRWLwindow*, int, int);
    void (*getWindowSize)(_GRWLwindow*, int*, int*);
    void (*setWindowSize)(_GRWLwindow*, int, int);
    void (*setWindowSizeLimits)(_GRWLwindow*, int, int, int, int);
    void (*setWindowAspectRatio)(_GRWLwindow*, int, int);
    void (*getFramebufferSize)(_GRWLwindow*, int*, int*);
    void (*getWindowFrameSize)(_GRWLwindow*, int*, int*, int*, int*);
    void (*getWindowContentScale)(_GRWLwindow*, float*, float*);
    void (*iconifyWindow)(_GRWLwindow*);
    void (*restoreWindow)(_GRWLwindow*);
    void (*maximizeWindow)(_GRWLwindow*);
    void (*showWindow)(_GRWLwindow*);
    void (*hideWindow)(_GRWLwindow*);
    void (*requestWindowAttention)(_GRWLwindow*);
    void (*focusWindow)(_GRWLwindow*);
    void (*setWindowMonitor)(_GRWLwindow*, _GRWLmonitor*, int, int, int, int, int);
    bool (*windowFocused)(_GRWLwindow*);
    bool (*windowIconified)(_GRWLwindow*);
    bool (*windowVisible)(_GRWLwindow*);
    bool (*windowMaximized)(_GRWLwindow*);
    bool (*windowHovered)(_GRWLwindow*);
    bool (*framebufferTransparent)(_GRWLwindow*);
    float (*getWindowOpacity)(_GRWLwindow*);
    void (*setWindowResizable)(_GRWLwindow*, bool);
    void (*setWindowDecorated)(_GRWLwindow*, bool);
    void (*setWindowFloating)(_GRWLwindow*, bool);
    void (*setWindowOpacity)(_GRWLwindow*, float);
    void (*setWindowMousePassthrough)(_GRWLwindow*, bool);
    void (*pollEvents)();
    void (*waitEvents)();
    void (*waitEventsTimeout)(double);
    void (*postEmptyEvent)();
    _GRWLusercontext* (*createUserContext)(_GRWLwindow*);
    // EGL
    EGLenum (*getEGLPlatform)(EGLint**);
    EGLNativeDisplayType (*getEGLNativeDisplay)();
    EGLNativeWindowType (*getEGLNativeWindow)(_GRWLwindow*);
    // vulkan
    void (*getRequiredInstanceExtensions)(char**);
    bool (*getPhysicalDevicePresentationSupport)(VkInstance, VkPhysicalDevice, uint32_t);
    VkResult (*createWindowSurface)(VkInstance, _GRWLwindow*, const VkAllocationCallbacks*, VkSurfaceKHR*);
};

// Library global data
//
struct _GRWLlibrary
{
    bool initialized;
    GRWLallocator allocator;

    _GRWLplatform platform;

    struct
    {
        _GRWLinitconfig init;
        _GRWLfbconfig framebuffer;
        _GRWLwndconfig window;
        _GRWLctxconfig context;
        int refreshRate;
    } hints;

    _GRWLerror* errorListHead;
    _GRWLcursor* cursorListHead;
    _GRWLwindow* windowListHead;

    _GRWLmonitor** monitors;
    int monitorCount;

    bool joysticksInitialized;
    _GRWLjoystick joysticks[GRWL_JOYSTICK_LAST + 1];
    _GRWLmapping* mappings;
    int mappingCount;

    _GRWLtls errorSlot;
    _GRWLtls contextSlot;
    _GRWLtls usercontextSlot;
    _GRWLmutex errorLock;

    struct
    {
        uint64_t offset;
        // This is defined in platform.h
        GRWL_PLATFORM_LIBRARY_TIMER_STATE
    } timer;

    struct
    {
        EGLenum platform;
        EGLDisplay display;
        EGLint major, minor;
        bool prefix;

        bool KHR_create_context;
        bool KHR_create_context_no_error;
        bool KHR_gl_colorspace;
        bool KHR_get_all_proc_addresses;
        bool KHR_context_flush_control;
        bool EXT_client_extensions;
        bool EXT_platform_base;
        bool EXT_platform_x11;
        bool EXT_platform_wayland;
        bool EXT_present_opaque;
        bool ANGLE_platform_angle;
        bool ANGLE_platform_angle_opengl;
        bool ANGLE_platform_angle_d3d;
        bool ANGLE_platform_angle_vulkan;
        bool ANGLE_platform_angle_metal;

        void* handle;

        PFN_eglGetConfigAttrib GetConfigAttrib;
        PFN_eglGetConfigs GetConfigs;
        PFN_eglGetDisplay GetDisplay;
        PFN_eglGetError GetError;
        PFN_eglInitialize Initialize;
        PFN_eglTerminate Terminate;
        PFN_eglBindAPI BindAPI;
        PFN_eglCreateContext CreateContext;
        PFN_eglDestroySurface DestroySurface;
        PFN_eglDestroyContext DestroyContext;
        PFN_eglCreateWindowSurface CreateWindowSurface;
        PFN_eglMakeCurrent MakeCurrent;
        PFN_eglSwapBuffers SwapBuffers;
        PFN_eglSwapInterval SwapInterval;
        PFN_eglQueryString QueryString;
        PFN_eglGetProcAddress GetProcAddress;
        PFN_eglCreatePbufferSurface CreatePbufferSurface;
        PFN_eglChooseConfig ChooseConfig;

        PFNEGLGETPLATFORMDISPLAYEXTPROC GetPlatformDisplayEXT;
        PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC CreatePlatformWindowSurfaceEXT;
    } egl;

    struct
    {
        bool available;
        void* handle;
        char* extensions[2];
        PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
        bool KHR_surface;
        bool KHR_win32_surface;
        bool MVK_macos_surface;
        bool EXT_metal_surface;
        bool KHR_xlib_surface;
        bool KHR_xcb_surface;
        bool KHR_wayland_surface;
    } vk;

    struct
    {
        GRWLmonitorfun monitor;
        GRWLjoystickfun joystick;
        GRWLkeyboardlayoutfun layout;
        GRWLjoystickaxisfun joystick_axis;
        GRWLjoystickbuttonfun joystick_button;
        GRWLjoystickhatfun joystick_hat;
        GRWLgamepadstatefun gamepad_state;
    } callbacks;

    // These are defined in platform.h
    GRWL_PLATFORM_LIBRARY_WINDOW_STATE
    GRWL_PLATFORM_LIBRARY_CONTEXT_STATE
    GRWL_PLATFORM_LIBRARY_JOYSTICK_STATE
    GRWL_PLATFORM_LIBRARY_DBUS_STATE
};

// Global state shared between compilation units of GRWL
//
extern _GRWLlibrary _grwl;

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _grwlPlatformInitTimer();
uint64_t _grwlPlatformGetTimerValue();
uint64_t _grwlPlatformGetTimerFrequency();

bool _grwlPlatformCreateTls(_GRWLtls* tls);
void _grwlPlatformDestroyTls(_GRWLtls* tls);
void* _grwlPlatformGetTls(_GRWLtls* tls);
void _grwlPlatformSetTls(_GRWLtls* tls, void* value);

bool _grwlPlatformCreateMutex(_GRWLmutex* mutex);
void _grwlPlatformDestroyMutex(_GRWLmutex* mutex);
void _grwlPlatformLockMutex(_GRWLmutex* mutex);
void _grwlPlatformUnlockMutex(_GRWLmutex* mutex);

void* _grwlPlatformLoadModule(const char* path);
void _grwlPlatformFreeModule(void* module);
GRWLproc _grwlPlatformGetModuleSymbol(void* module, const char* name);

//////////////////////////////////////////////////////////////////////////
//////                         GRWL event API                       //////
//////////////////////////////////////////////////////////////////////////

void _grwlInputWindowFocus(_GRWLwindow* window, bool focused);
void _grwlInputWindowPos(_GRWLwindow* window, int xpos, int ypos);
void _grwlInputWindowSize(_GRWLwindow* window, int width, int height);
void _grwlInputFramebufferSize(_GRWLwindow* window, int width, int height);
void _grwlInputWindowContentScale(_GRWLwindow* window, float xscale, float yscale);
void _grwlInputWindowIconify(_GRWLwindow* window, bool iconified);
void _grwlInputWindowMaximize(_GRWLwindow* window, bool maximized);
void _grwlInputWindowDamage(_GRWLwindow* window);
void _grwlInputWindowCloseRequest(_GRWLwindow* window);
void _grwlInputWindowMonitor(_GRWLwindow* window, _GRWLmonitor* monitor);

void _grwlInputKeyboardLayout();
void _grwlInputKey(_GRWLwindow* window, int key, int scancode, int action, int mods);
void _grwlInputChar(_GRWLwindow* window, uint32_t codepoint, int mods, bool plain);
void _grwlInputPreedit(_GRWLwindow* window);
void _grwlInputIMEStatus(_GRWLwindow* window);
void _grwlInputPreeditCandidate(_GRWLwindow* window);
void _grwlInputScroll(_GRWLwindow* window, double xoffset, double yoffset);
void _grwlInputMouseClick(_GRWLwindow* window, int button, int action, int mods);
void _grwlInputCursorPos(_GRWLwindow* window, double xpos, double ypos);
void _grwlInputCursorEnter(_GRWLwindow* window, bool entered);
void _grwlInputDrop(_GRWLwindow* window, int count, const char** names);
void _grwlInputJoystick(_GRWLjoystick* js, int event);
void _grwlInputJoystickAxis(_GRWLjoystick* js, int axis, float value);
void _grwlInputJoystickButton(_GRWLjoystick* js, int button, char value);
void _grwlInputJoystickHat(_GRWLjoystick* js, int hat, char value);

void _grwlInputMonitor(_GRWLmonitor* monitor, int action, int placement);
void _grwlInputMonitorWindow(_GRWLmonitor* monitor, _GRWLwindow* window);

#if defined(__GNUC__)
void _grwlInputError(int code, const char* format, ...) __attribute__((format(printf, 2, 3)));
#else
void _grwlInputError(int code, const char* format, ...);
#endif

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlSelectPlatform(int platformID, _GRWLplatform* platform);

bool _grwlStringInExtensionString(const char* string, const char* extensions);
const _GRWLfbconfig* _grwlChooseFBConfig(const _GRWLfbconfig* desired, const _GRWLfbconfig* alternatives,
                                         unsigned int count);
bool _grwlRefreshContextAttribs(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig);
bool _grwlIsValidContextConfig(const _GRWLctxconfig* ctxconfig);

const GRWLvidmode* _grwlChooseVideoMode(_GRWLmonitor* monitor, const GRWLvidmode* desired);
int _grwlCompareVideoModes(const GRWLvidmode* first, const GRWLvidmode* second);
_GRWLmonitor* _grwlAllocMonitor(const char* name, int widthMM, int heightMM);
void _grwlFreeMonitor(_GRWLmonitor* monitor);
void _grwlSplitBPP(int bpp, int* red, int* green, int* blue);

void _grwlInitGamepadMappings();
_GRWLjoystick* _grwlAllocJoystick(const char* name, const char* guid, int axisCount, int buttonCount, int hatCount);
void _grwlFreeJoystick(_GRWLjoystick* js);
void _grwlCenterCursorInContentArea(_GRWLwindow* window);
void _grwlPollAllJoysticks();

bool _grwlInitEGL();
void _grwlTerminateEGL();
bool _grwlCreateContextEGL(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig);
_GRWLusercontext* _grwlCreateUserContextEGL(_GRWLwindow* window);
#if defined(_GRWL_X11)
bool _grwlChooseVisualEGL(const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                          const _GRWLfbconfig* fbconfig, Visual** visual, int* depth);
#endif /*_GRWL_X11*/

bool _grwlInitOSMesa();
void _grwlTerminateOSMesa();
bool _grwlCreateContextOSMesa(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig);
_GRWLusercontext* _grwlCreateUserContextOSMesa(_GRWLwindow* window);

bool _grwlInitVulkan(int mode);
void _grwlTerminateVulkan();
const char* _grwlGetVulkanResultString(VkResult result);

size_t _grwlEncodeUTF8(char* s, uint32_t codepoint);
uint32_t _grwlDecodeUTF8(const char** s);
char** _grwlParseUriList(char* text, int* count);

char* _grwl_strdup(const char* source);
int _grwl_min(int a, int b);
int _grwl_max(int a, int b);
float _grwl_fminf(float a, float b);
float _grwl_fmaxf(float a, float b);

void* _grwl_calloc(size_t count, size_t size);
void* _grwl_realloc(void* pointer, size_t size);
void _grwl_free(void* pointer);
