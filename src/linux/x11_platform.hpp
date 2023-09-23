//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#if defined(_GRWL_X11)
    #include <unistd.h>
    #include <csignal>
    #include <cstdint>

    #include <X11/Xlib.h>
    #include <X11/keysym.h>
    #include <X11/Xatom.h>
    #include <X11/Xresource.h>
    #include <X11/Xcursor/Xcursor.h>

    // The XRandR extension provides mode setting and gamma control
    #include <X11/extensions/Xrandr.h>

    // The Xkb extension provides improved keyboard support
    #include <X11/XKBlib.h>

    // The Xinerama extension provides legacy monitor indices
    #include <X11/extensions/Xinerama.h>

    // The XInput extension provides raw mouse motion input
    #include <X11/extensions/XInput2.h>

    // The Shape extension provides custom window shapes
    #include <X11/extensions/shape.h>

    #define GLX_VENDOR 1
    #define GLX_RGBA_BIT 0x00000001
    #define GLX_WINDOW_BIT 0x00000001
    #define GLX_DRAWABLE_TYPE 0x8010
    #define GLX_RENDER_TYPE 0x8011
    #define GLX_RGBA_TYPE 0x8014
    #define GLX_DOUBLEBUFFER 5
    #define GLX_STEREO 6
    #define GLX_AUX_BUFFERS 7
    #define GLX_RED_SIZE 8
    #define GLX_GREEN_SIZE 9
    #define GLX_BLUE_SIZE 10
    #define GLX_ALPHA_SIZE 11
    #define GLX_DEPTH_SIZE 12
    #define GLX_STENCIL_SIZE 13
    #define GLX_ACCUM_RED_SIZE 14
    #define GLX_ACCUM_GREEN_SIZE 15
    #define GLX_ACCUM_BLUE_SIZE 16
    #define GLX_ACCUM_ALPHA_SIZE 17
    #define GLX_SAMPLES 0x186a1
    #define GLX_VISUAL_ID 0x800b

    #define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20b2
    #define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
    #define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
    #define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
    #define GLX_CONTEXT_FLAGS_ARB 0x2094
    #define GLX_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
    #define GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
    #define GLX_LOSE_CONTEXT_ON_RESET_ARB 0x8252
    #define GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
    #define GLX_NO_RESET_NOTIFICATION_ARB 0x8261
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
    #define GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
    #define GLX_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3

    #define STYLE_OVERTHESPOT (XIMPreeditNothing | XIMStatusNothing)
    #define STYLE_ONTHESPOT (XIMPreeditCallbacks | XIMStatusCallbacks)

typedef XID GLXWindow;
typedef XID GLXDrawable;
typedef struct __GLXFBConfig* GLXFBConfig;
typedef struct __GLXcontext* GLXContext;
typedef void (*__GLXextproc)(void);

typedef XClassHint* (*PFN_XAllocClassHint)(void);
typedef XSizeHints* (*PFN_XAllocSizeHints)(void);
typedef XWMHints* (*PFN_XAllocWMHints)(void);
typedef int (*PFN_XChangeProperty)(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);
typedef int (*PFN_XChangeWindowAttributes)(Display*, Window, unsigned long, XSetWindowAttributes*);
typedef Bool (*PFN_XCheckIfEvent)(Display*, XEvent*, Bool (*)(Display*, XEvent*, XPointer), XPointer);
typedef Bool (*PFN_XCheckTypedWindowEvent)(Display*, Window, int, XEvent*);
typedef int (*PFN_XCloseDisplay)(Display*);
typedef Status (*PFN_XCloseIM)(XIM);
typedef int (*PFN_XConvertSelection)(Display*, Atom, Atom, Atom, Window, Time);
typedef Colormap (*PFN_XCreateColormap)(Display*, Window, Visual*, int);
typedef Cursor (*PFN_XCreateFontCursor)(Display*, unsigned int);
typedef XIC (*PFN_XCreateIC)(XIM, ...);
typedef Region (*PFN_XCreateRegion)(void);
typedef Window (*PFN_XCreateWindow)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int,
                                    unsigned int, Visual*, unsigned long, XSetWindowAttributes*);
typedef int (*PFN_XDefineCursor)(Display*, Window, Cursor);
typedef int (*PFN_XDeleteContext)(Display*, XID, XContext);
typedef int (*PFN_XDeleteProperty)(Display*, Window, Atom);
typedef void (*PFN_XDestroyIC)(XIC);
typedef int (*PFN_XDestroyRegion)(Region);
typedef int (*PFN_XDestroyWindow)(Display*, Window);
typedef int (*PFN_XDisplayKeycodes)(Display*, int*, int*);
typedef int (*PFN_XEventsQueued)(Display*, int);
typedef Bool (*PFN_XFilterEvent)(XEvent*, Window);
typedef int (*PFN_XFindContext)(Display*, XID, XContext, XPointer*);
typedef int (*PFN_XFlush)(Display*);
typedef int (*PFN_XFree)(void*);
typedef int (*PFN_XFreeColormap)(Display*, Colormap);
typedef int (*PFN_XFreeCursor)(Display*, Cursor);
typedef void (*PFN_XFreeEventData)(Display*, XGenericEventCookie*);
typedef char* (*PFN_XGetAtomName)(Display*, Atom);
typedef int (*PFN_XGetErrorText)(Display*, int, char*, int);
typedef Bool (*PFN_XGetEventData)(Display*, XGenericEventCookie*);
typedef char* (*PFN_XGetICValues)(XIC, ...);
typedef char* (*PFN_XGetIMValues)(XIM, ...);
typedef int (*PFN_XGetInputFocus)(Display*, Window*, int*);
typedef KeySym* (*PFN_XGetKeyboardMapping)(Display*, KeyCode, int, int*);
typedef int (*PFN_XGetScreenSaver)(Display*, int*, int*, int*, int*);
typedef Window (*PFN_XGetSelectionOwner)(Display*, Atom);
typedef XVisualInfo* (*PFN_XGetVisualInfo)(Display*, long, XVisualInfo*, int*);
typedef Status (*PFN_XGetWMNormalHints)(Display*, Window, XSizeHints*, long*);
typedef Status (*PFN_XGetWindowAttributes)(Display*, Window, XWindowAttributes*);
typedef int (*PFN_XGetWindowProperty)(Display*, Window, Atom, long, long, Bool, Atom, Atom*, int*, unsigned long*,
                                      unsigned long*, unsigned char**);
typedef int (*PFN_XGrabPointer)(Display*, Window, Bool, unsigned int, int, int, Window, Cursor, Time);
typedef Status (*PFN_XIconifyWindow)(Display*, Window, int);
typedef Status (*PFN_XInitThreads)(void);
typedef Atom (*PFN_XInternAtom)(Display*, const char*, Bool);
typedef int (*PFN_XLookupString)(XKeyEvent*, char*, int, KeySym*, XComposeStatus*);
typedef int (*PFN_XMapRaised)(Display*, Window);
typedef int (*PFN_XMapWindow)(Display*, Window);
typedef int (*PFN_XMoveResizeWindow)(Display*, Window, int, int, unsigned int, unsigned int);
typedef int (*PFN_XMoveWindow)(Display*, Window, int, int);
typedef int (*PFN_XNextEvent)(Display*, XEvent*);
typedef Display* (*PFN_XOpenDisplay)(const char*);
typedef XIM (*PFN_XOpenIM)(Display*, XrmDatabase*, char*, char*);
typedef int (*PFN_XPeekEvent)(Display*, XEvent*);
typedef int (*PFN_XPending)(Display*);
typedef Bool (*PFN_XQueryExtension)(Display*, const char*, int*, int*, int*);
typedef Bool (*PFN_XQueryPointer)(Display*, Window, Window*, Window*, int*, int*, int*, int*, unsigned int*);
typedef int (*PFN_XRaiseWindow)(Display*, Window);
typedef Bool (*PFN_XRegisterIMInstantiateCallback)(Display*, void*, char*, char*, XIDProc, XPointer);
typedef int (*PFN_XResizeWindow)(Display*, Window, unsigned int, unsigned int);
typedef char* (*PFN_XResourceManagerString)(Display*);
typedef int (*PFN_XSaveContext)(Display*, XID, XContext, const char*);
typedef int (*PFN_XSelectInput)(Display*, Window, long);
typedef Status (*PFN_XSendEvent)(Display*, Window, Bool, long, XEvent*);
typedef int (*PFN_XSetClassHint)(Display*, Window, XClassHint*);
typedef XErrorHandler (*PFN_XSetErrorHandler)(XErrorHandler);
typedef void (*PFN_XSetICFocus)(XIC);
typedef char* (*PFN_XSetICValues)(XIC, ...);
typedef char* (*PFN_XSetIMValues)(XIM, ...);
typedef int (*PFN_XSetInputFocus)(Display*, Window, int, Time);
typedef char* (*PFN_XSetLocaleModifiers)(const char*);
typedef int (*PFN_XSetScreenSaver)(Display*, int, int, int, int);
typedef int (*PFN_XSetSelectionOwner)(Display*, Atom, Window, Time);
typedef int (*PFN_XSetWMHints)(Display*, Window, XWMHints*);
typedef void (*PFN_XSetWMNormalHints)(Display*, Window, XSizeHints*);
typedef Status (*PFN_XSetWMProtocols)(Display*, Window, Atom*, int);
typedef Bool (*PFN_XSupportsLocale)(void);
typedef int (*PFN_XSync)(Display*, Bool);
typedef Bool (*PFN_XTranslateCoordinates)(Display*, Window, Window, int, int, int*, int*, Window*);
typedef int (*PFN_XUndefineCursor)(Display*, Window);
typedef int (*PFN_XUngrabPointer)(Display*, Time);
typedef int (*PFN_XUnmapWindow)(Display*, Window);
typedef void (*PFN_XUnsetICFocus)(XIC);
typedef XVaNestedList (*PFN_XVaCreateNestedList)(int, ...);
typedef VisualID (*PFN_XVisualIDFromVisual)(Visual*);
typedef int (*PFN_XWarpPointer)(Display*, Window, Window, int, int, unsigned int, unsigned int, int, int);
typedef void (*PFN_XkbFreeKeyboard)(XkbDescPtr, unsigned int, Bool);
typedef void (*PFN_XkbFreeNames)(XkbDescPtr, unsigned int, Bool);
typedef XkbDescPtr (*PFN_XkbAllocKeyboard)(void);
typedef XkbDescPtr (*PFN_XkbGetMap)(Display*, unsigned int, unsigned int);
typedef Status (*PFN_XkbGetNames)(Display*, unsigned int, XkbDescPtr);
typedef Status (*PFN_XkbGetState)(Display*, unsigned int, XkbStatePtr);
typedef KeySym (*PFN_XkbKeycodeToKeysym)(Display*, KeyCode, int, int);
typedef Bool (*PFN_XkbQueryExtension)(Display*, int*, int*, int*, int*, int*);
typedef Bool (*PFN_XkbSelectEventDetails)(Display*, unsigned int, unsigned int, unsigned long, unsigned long);
typedef Bool (*PFN_XkbSetDetectableAutoRepeat)(Display*, Bool, Bool*);
typedef char* (*PFN_XmbResetIC)(XIC);
typedef void (*PFN_XrmDestroyDatabase)(XrmDatabase);
typedef Bool (*PFN_XrmGetResource)(XrmDatabase, const char*, const char*, char**, XrmValue*);
typedef XrmDatabase (*PFN_XrmGetStringDatabase)(const char*);
typedef void (*PFN_XrmInitialize)(void);
typedef XrmQuark (*PFN_XrmUniqueQuark)(void);
typedef Bool (*PFN_XUnregisterIMInstantiateCallback)(Display*, void*, char*, char*, XIDProc, XPointer);
typedef int (*PFN_Xutf8LookupString)(XIC, XKeyPressedEvent*, char*, int, KeySym*, Status*);
typedef void (*PFN_Xutf8SetWMProperties)(Display*, Window, const char*, const char*, char**, int, XSizeHints*,
                                         XWMHints*, XClassHint*);
    #define XAllocClassHint _grwl.x11.xlib.AllocClassHint
    #define XAllocSizeHints _grwl.x11.xlib.AllocSizeHints
    #define XAllocWMHints _grwl.x11.xlib.AllocWMHints
    #define XChangeProperty _grwl.x11.xlib.ChangeProperty
    #define XChangeWindowAttributes _grwl.x11.xlib.ChangeWindowAttributes
    #define XCheckIfEvent _grwl.x11.xlib.CheckIfEvent
    #define XCheckTypedWindowEvent _grwl.x11.xlib.CheckTypedWindowEvent
    #define XCloseDisplay _grwl.x11.xlib.CloseDisplay
    #define XCloseIM _grwl.x11.xlib.CloseIM
    #define XConvertSelection _grwl.x11.xlib.ConvertSelection
    #define XCreateColormap _grwl.x11.xlib.CreateColormap
    #define XCreateFontCursor _grwl.x11.xlib.CreateFontCursor
    #define XCreateIC _grwl.x11.xlib.CreateIC
    #define XCreateRegion _grwl.x11.xlib.CreateRegion
    #define XCreateWindow _grwl.x11.xlib.CreateWindow
    #define XDefineCursor _grwl.x11.xlib.DefineCursor
    #define XDeleteContext _grwl.x11.xlib.DeleteContext
    #define XDeleteProperty _grwl.x11.xlib.DeleteProperty
    #define XDestroyIC _grwl.x11.xlib.DestroyIC
    #define XDestroyRegion _grwl.x11.xlib.DestroyRegion
    #define XDestroyWindow _grwl.x11.xlib.DestroyWindow
    #define XDisplayKeycodes _grwl.x11.xlib.DisplayKeycodes
    #define XEventsQueued _grwl.x11.xlib.EventsQueued
    #define XFilterEvent _grwl.x11.xlib.FilterEvent
    #define XFindContext _grwl.x11.xlib.FindContext
    #define XFlush _grwl.x11.xlib.Flush
    #define XFree _grwl.x11.xlib.Free
    #define XFreeColormap _grwl.x11.xlib.FreeColormap
    #define XFreeCursor _grwl.x11.xlib.FreeCursor
    #define XFreeEventData _grwl.x11.xlib.FreeEventData
    #define XGetAtomName _grwl.x11.xlib.GetAtomName
    #define XGetErrorText _grwl.x11.xlib.GetErrorText
    #define XGetEventData _grwl.x11.xlib.GetEventData
    #define XGetICValues _grwl.x11.xlib.GetICValues
    #define XGetIMValues _grwl.x11.xlib.GetIMValues
    #define XGetInputFocus _grwl.x11.xlib.GetInputFocus
    #define XGetKeyboardMapping _grwl.x11.xlib.GetKeyboardMapping
    #define XGetScreenSaver _grwl.x11.xlib.GetScreenSaver
    #define XGetSelectionOwner _grwl.x11.xlib.GetSelectionOwner
    #define XGetVisualInfo _grwl.x11.xlib.GetVisualInfo
    #define XGetWMNormalHints _grwl.x11.xlib.GetWMNormalHints
    #define XGetWindowAttributes _grwl.x11.xlib.GetWindowAttributes
    #define XGetWindowProperty _grwl.x11.xlib.GetWindowProperty
    #define XGrabPointer _grwl.x11.xlib.GrabPointer
    #define XIconifyWindow _grwl.x11.xlib.IconifyWindow
    #define XInternAtom _grwl.x11.xlib.InternAtom
    #define XLookupString _grwl.x11.xlib.LookupString
    #define XMapRaised _grwl.x11.xlib.MapRaised
    #define XMapWindow _grwl.x11.xlib.MapWindow
    #define XMoveResizeWindow _grwl.x11.xlib.MoveResizeWindow
    #define XMoveWindow _grwl.x11.xlib.MoveWindow
    #define XNextEvent _grwl.x11.xlib.NextEvent
    #define XOpenIM _grwl.x11.xlib.OpenIM
    #define XPeekEvent _grwl.x11.xlib.PeekEvent
    #define XPending _grwl.x11.xlib.Pending
    #define XQueryExtension _grwl.x11.xlib.QueryExtension
    #define XQueryPointer _grwl.x11.xlib.QueryPointer
    #define XRaiseWindow _grwl.x11.xlib.RaiseWindow
    #define XRegisterIMInstantiateCallback _grwl.x11.xlib.RegisterIMInstantiateCallback
    #define XResizeWindow _grwl.x11.xlib.ResizeWindow
    #define XResourceManagerString _grwl.x11.xlib.ResourceManagerString
    #define XSaveContext _grwl.x11.xlib.SaveContext
    #define XSelectInput _grwl.x11.xlib.SelectInput
    #define XSendEvent _grwl.x11.xlib.SendEvent
    #define XSetClassHint _grwl.x11.xlib.SetClassHint
    #define XSetErrorHandler _grwl.x11.xlib.SetErrorHandler
    #define XSetICFocus _grwl.x11.xlib.SetICFocus
    #define XSetICValues _grwl.x11.xlib.SetICValues
    #define XSetIMValues _grwl.x11.xlib.SetIMValues
    #define XSetInputFocus _grwl.x11.xlib.SetInputFocus
    #define XSetLocaleModifiers _grwl.x11.xlib.SetLocaleModifiers
    #define XSetScreenSaver _grwl.x11.xlib.SetScreenSaver
    #define XSetSelectionOwner _grwl.x11.xlib.SetSelectionOwner
    #define XSetWMHints _grwl.x11.xlib.SetWMHints
    #define XSetWMNormalHints _grwl.x11.xlib.SetWMNormalHints
    #define XSetWMProtocols _grwl.x11.xlib.SetWMProtocols
    #define XSupportsLocale _grwl.x11.xlib.SupportsLocale
    #define XSync _grwl.x11.xlib.Sync
    #define XTranslateCoordinates _grwl.x11.xlib.TranslateCoordinates
    #define XUndefineCursor _grwl.x11.xlib.UndefineCursor
    #define XUngrabPointer _grwl.x11.xlib.UngrabPointer
    #define XUnmapWindow _grwl.x11.xlib.UnmapWindow
    #define XUnsetICFocus _grwl.x11.xlib.UnsetICFocus
    #define XVaCreateNestedList _grwl.x11.xlib.VaCreateNestedList
    #define XVisualIDFromVisual _grwl.x11.xlib.VisualIDFromVisual
    #define XWarpPointer _grwl.x11.xlib.WarpPointer
    #define XkbAllocKeyboard _grwl.x11.xkb.AllocKeyboard
    #define XkbFreeKeyboard _grwl.x11.xkb.FreeKeyboard
    #define XkbFreeNames _grwl.x11.xkb.FreeNames
    #define XkbGetMap _grwl.x11.xkb.GetMap
    #define XkbGetNames _grwl.x11.xkb.GetNames
    #define XkbGetState _grwl.x11.xkb.GetState
    #define XkbKeycodeToKeysym _grwl.x11.xkb.KeycodeToKeysym
    #define XkbQueryExtension _grwl.x11.xkb.QueryExtension
    #define XkbSelectEventDetails _grwl.x11.xkb.SelectEventDetails
    #define XkbSetDetectableAutoRepeat _grwl.x11.xkb.SetDetectableAutoRepeat
    #define XmbResetIC _grwl.x11.xlib.mbResetIC
    #define XrmDestroyDatabase _grwl.x11.xrm.DestroyDatabase
    #define XrmGetResource _grwl.x11.xrm.GetResource
    #define XrmGetStringDatabase _grwl.x11.xrm.GetStringDatabase
    #define XrmUniqueQuark _grwl.x11.xrm.UniqueQuark
    #define XUnregisterIMInstantiateCallback _grwl.x11.xlib.UnregisterIMInstantiateCallback
    #define Xutf8LookupString _grwl.x11.xlib.utf8LookupString
    #define Xutf8SetWMProperties _grwl.x11.xlib.utf8SetWMProperties

typedef XRRCrtcGamma* (*PFN_XRRAllocGamma)(int);
typedef void (*PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);
typedef void (*PFN_XRRFreeGamma)(XRRCrtcGamma*);
typedef void (*PFN_XRRFreeOutputInfo)(XRROutputInfo*);
typedef void (*PFN_XRRFreeScreenResources)(XRRScreenResources*);
typedef XRRCrtcGamma* (*PFN_XRRGetCrtcGamma)(Display*, RRCrtc);
typedef int (*PFN_XRRGetCrtcGammaSize)(Display*, RRCrtc);
typedef XRRCrtcInfo* (*PFN_XRRGetCrtcInfo)(Display*, XRRScreenResources*, RRCrtc);
typedef XRROutputInfo* (*PFN_XRRGetOutputInfo)(Display*, XRRScreenResources*, RROutput);
typedef RROutput (*PFN_XRRGetOutputPrimary)(Display*, Window);
typedef XRRScreenResources* (*PFN_XRRGetScreenResourcesCurrent)(Display*, Window);
typedef Bool (*PFN_XRRQueryExtension)(Display*, int*, int*);
typedef Status (*PFN_XRRQueryVersion)(Display*, int*, int*);
typedef void (*PFN_XRRSelectInput)(Display*, Window, int);
typedef Status (*PFN_XRRSetCrtcConfig)(Display*, XRRScreenResources*, RRCrtc, Time, int, int, RRMode, Rotation,
                                       RROutput*, int);
typedef void (*PFN_XRRSetCrtcGamma)(Display*, RRCrtc, XRRCrtcGamma*);
typedef int (*PFN_XRRUpdateConfiguration)(XEvent*);
    #define XRRAllocGamma _grwl.x11.randr.AllocGamma
    #define XRRFreeCrtcInfo _grwl.x11.randr.FreeCrtcInfo
    #define XRRFreeGamma _grwl.x11.randr.FreeGamma
    #define XRRFreeOutputInfo _grwl.x11.randr.FreeOutputInfo
    #define XRRFreeScreenResources _grwl.x11.randr.FreeScreenResources
    #define XRRGetCrtcGamma _grwl.x11.randr.GetCrtcGamma
    #define XRRGetCrtcGammaSize _grwl.x11.randr.GetCrtcGammaSize
    #define XRRGetCrtcInfo _grwl.x11.randr.GetCrtcInfo
    #define XRRGetOutputInfo _grwl.x11.randr.GetOutputInfo
    #define XRRGetOutputPrimary _grwl.x11.randr.GetOutputPrimary
    #define XRRGetScreenResourcesCurrent _grwl.x11.randr.GetScreenResourcesCurrent
    #define XRRQueryExtension _grwl.x11.randr.QueryExtension
    #define XRRQueryVersion _grwl.x11.randr.QueryVersion
    #define XRRSelectInput _grwl.x11.randr.SelectInput
    #define XRRSetCrtcConfig _grwl.x11.randr.SetCrtcConfig
    #define XRRSetCrtcGamma _grwl.x11.randr.SetCrtcGamma
    #define XRRUpdateConfiguration _grwl.x11.randr.UpdateConfiguration

typedef XcursorImage* (*PFN_XcursorImageCreate)(int, int);
typedef void (*PFN_XcursorImageDestroy)(XcursorImage*);
typedef Cursor (*PFN_XcursorImageLoadCursor)(Display*, const XcursorImage*);
typedef char* (*PFN_XcursorGetTheme)(Display*);
typedef int (*PFN_XcursorGetDefaultSize)(Display*);
typedef XcursorImage* (*PFN_XcursorLibraryLoadImage)(const char*, const char*, int);
    #define XcursorImageCreate _grwl.x11.xcursor.ImageCreate
    #define XcursorImageDestroy _grwl.x11.xcursor.ImageDestroy
    #define XcursorImageLoadCursor _grwl.x11.xcursor.ImageLoadCursor
    #define XcursorGetTheme _grwl.x11.xcursor.GetTheme
    #define XcursorGetDefaultSize _grwl.x11.xcursor.GetDefaultSize
    #define XcursorLibraryLoadImage _grwl.x11.xcursor.LibraryLoadImage

typedef Bool (*PFN_XineramaIsActive)(Display*);
typedef Bool (*PFN_XineramaQueryExtension)(Display*, int*, int*);
typedef XineramaScreenInfo* (*PFN_XineramaQueryScreens)(Display*, int*);
    #define XineramaIsActive _grwl.x11.xinerama.IsActive
    #define XineramaQueryExtension _grwl.x11.xinerama.QueryExtension
    #define XineramaQueryScreens _grwl.x11.xinerama.QueryScreens

typedef XID xcb_window_t;
typedef XID xcb_visualid_t;
typedef struct xcb_connection_t xcb_connection_t;
typedef xcb_connection_t* (*PFN_XGetXCBConnection)(Display*);
    #define XGetXCBConnection _grwl.x11.x11xcb.GetXCBConnection

typedef Bool (*PFN_XF86VidModeQueryExtension)(Display*, int*, int*);
typedef Bool (*PFN_XF86VidModeGetGammaRamp)(Display*, int, int, unsigned short*, unsigned short*, unsigned short*);
typedef Bool (*PFN_XF86VidModeSetGammaRamp)(Display*, int, int, unsigned short*, unsigned short*, unsigned short*);
typedef Bool (*PFN_XF86VidModeGetGammaRampSize)(Display*, int, int*);
    #define XF86VidModeQueryExtension _grwl.x11.vidmode.QueryExtension
    #define XF86VidModeGetGammaRamp _grwl.x11.vidmode.GetGammaRamp
    #define XF86VidModeSetGammaRamp _grwl.x11.vidmode.SetGammaRamp
    #define XF86VidModeGetGammaRampSize _grwl.x11.vidmode.GetGammaRampSize

typedef Status (*PFN_XIQueryVersion)(Display*, int*, int*);
typedef int (*PFN_XISelectEvents)(Display*, Window, XIEventMask*, int);
    #define XIQueryVersion _grwl.x11.xi.QueryVersion
    #define XISelectEvents _grwl.x11.xi.SelectEvents

typedef Bool (*PFN_XRenderQueryExtension)(Display*, int*, int*);
typedef Status (*PFN_XRenderQueryVersion)(Display* dpy, int*, int*);
typedef XRenderPictFormat* (*PFN_XRenderFindVisualFormat)(Display*, Visual const*);
    #define XRenderQueryExtension _grwl.x11.xrender.QueryExtension
    #define XRenderQueryVersion _grwl.x11.xrender.QueryVersion
    #define XRenderFindVisualFormat _grwl.x11.xrender.FindVisualFormat

typedef Bool (*PFN_XShapeQueryExtension)(Display*, int*, int*);
typedef Status (*PFN_XShapeQueryVersion)(Display* dpy, int*, int*);
typedef void (*PFN_XShapeCombineRegion)(Display*, Window, int, int, int, Region, int);
typedef void (*PFN_XShapeCombineMask)(Display*, Window, int, int, int, Pixmap, int);

    #define XShapeQueryExtension _grwl.x11.xshape.QueryExtension
    #define XShapeQueryVersion _grwl.x11.xshape.QueryVersion
    #define XShapeCombineRegion _grwl.x11.xshape.ShapeCombineRegion
    #define XShapeCombineMask _grwl.x11.xshape.ShapeCombineMask

typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display*, GLXFBConfig, int, int*);
typedef const char* (*PFNGLXGETCLIENTSTRINGPROC)(Display*, int);
typedef Bool (*PFNGLXQUERYEXTENSIONPROC)(Display*, int*, int*);
typedef Bool (*PFNGLXQUERYVERSIONPROC)(Display*, int*, int*);
typedef void (*PFNGLXDESTROYCONTEXTPROC)(Display*, GLXContext);
typedef Bool (*PFNGLXMAKECURRENTPROC)(Display*, GLXDrawable, GLXContext);
typedef void (*PFNGLXSWAPBUFFERSPROC)(Display*, GLXDrawable);
typedef const char* (*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*, int);
typedef GLXFBConfig* (*PFNGLXGETFBCONFIGSPROC)(Display*, int, int*);
typedef GLXContext (*PFNGLXCREATENEWCONTEXTPROC)(Display*, GLXFBConfig, int, GLXContext, Bool);
typedef __GLXextproc (*PFNGLXGETPROCADDRESSPROC)(const GLubyte* procName);
typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*, GLXDrawable, int);
typedef XVisualInfo* (*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*, GLXFBConfig);
typedef GLXWindow (*PFNGLXCREATEWINDOWPROC)(Display*, GLXFBConfig, Window, const int*);
typedef void (*PFNGLXDESTROYWINDOWPROC)(Display*, GLXWindow);

typedef int (*PFNGLXSWAPINTERVALMESAPROC)(int);
typedef int (*PFNGLXSWAPINTERVALSGIPROC)(int);
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

    // libGL.so function pointer typedefs
    #define glXGetFBConfigs _grwl.glx.GetFBConfigs
    #define glXGetFBConfigAttrib _grwl.glx.GetFBConfigAttrib
    #define glXGetClientString _grwl.glx.GetClientString
    #define glXQueryExtension _grwl.glx.QueryExtension
    #define glXQueryVersion _grwl.glx.QueryVersion
    #define glXDestroyContext _grwl.glx.DestroyContext
    #define glXMakeCurrent _grwl.glx.MakeCurrent
    #define glXSwapBuffers _grwl.glx.SwapBuffers
    #define glXQueryExtensionsString _grwl.glx.QueryExtensionsString
    #define glXCreateNewContext _grwl.glx.CreateNewContext
    #define glXGetVisualFromFBConfig _grwl.glx.GetVisualFromFBConfig
    #define glXCreateWindow _grwl.glx.CreateWindow
    #define glXDestroyWindow _grwl.glx.DestroyWindow

typedef VkFlags VkXlibSurfaceCreateFlagsKHR;
typedef VkFlags VkXcbSurfaceCreateFlagsKHR;

typedef struct VkXlibSurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkXlibSurfaceCreateFlagsKHR flags;
    Display* dpy;
    Window window;
} VkXlibSurfaceCreateInfoKHR;

typedef struct VkXcbSurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkXcbSurfaceCreateFlagsKHR flags;
    xcb_connection_t* connection;
    xcb_window_t window;
} VkXcbSurfaceCreateInfoKHR;

typedef VkResult(APIENTRY* PFN_vkCreateXlibSurfaceKHR)(VkInstance, const VkXlibSurfaceCreateInfoKHR*,
                                                       const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef VkBool32(APIENTRY* PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)(VkPhysicalDevice, uint32_t, Display*,
                                                                              VisualID);
typedef VkResult(APIENTRY* PFN_vkCreateXcbSurfaceKHR)(VkInstance, const VkXcbSurfaceCreateInfoKHR*,
                                                      const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef VkBool32(APIENTRY* PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)(VkPhysicalDevice, uint32_t,
                                                                             xcb_connection_t*, xcb_visualid_t);

    #include "xkb_unicode.h"
    #include "posix_poll.h"

    #define GRWL_X11_WINDOW_STATE _GRWLwindowX11 x11;
    #define GRWL_X11_LIBRARY_WINDOW_STATE _GRWLlibraryX11 x11;
    #define GRWL_X11_MONITOR_STATE _GRWLmonitorX11 x11;
    #define GRWL_X11_CURSOR_STATE _GRWLcursorX11 x11;

    #define GRWL_GLX_CONTEXT_STATE _GRWLcontextGLX glx;
    #define GRWL_GLX_LIBRARY_CONTEXT_STATE _GRWLlibraryGLX glx;
    #define GRWL_GLX_USER_CONTEXT_STATE _GRWLusercontextGLX glx;

// GLX-specific per-context data
//
typedef struct _GRWLcontextGLX
{
    GLXContext handle;
    GLXWindow window;
    GLXFBConfig fbconfig;
} _GRWLcontextGLX;

// GLX-specific global data
//
typedef struct _GRWLlibraryGLX
{
    int major, minor;
    int eventBase;
    int errorBase;

    void* handle;

    // GLX 1.3 functions
    PFNGLXGETFBCONFIGSPROC GetFBConfigs;
    PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib;
    PFNGLXGETCLIENTSTRINGPROC GetClientString;
    PFNGLXQUERYEXTENSIONPROC QueryExtension;
    PFNGLXQUERYVERSIONPROC QueryVersion;
    PFNGLXDESTROYCONTEXTPROC DestroyContext;
    PFNGLXMAKECURRENTPROC MakeCurrent;
    PFNGLXSWAPBUFFERSPROC SwapBuffers;
    PFNGLXQUERYEXTENSIONSSTRINGPROC QueryExtensionsString;
    PFNGLXCREATENEWCONTEXTPROC CreateNewContext;
    PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig;
    PFNGLXCREATEWINDOWPROC CreateWindow;
    PFNGLXDESTROYWINDOWPROC DestroyWindow;

    // GLX 1.4 and extension functions
    PFNGLXGETPROCADDRESSPROC GetProcAddress;
    PFNGLXGETPROCADDRESSPROC GetProcAddressARB;
    PFNGLXSWAPINTERVALSGIPROC SwapIntervalSGI;
    PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT;
    PFNGLXSWAPINTERVALMESAPROC SwapIntervalMESA;
    PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;
    GRWLbool SGI_swap_control;
    GRWLbool EXT_swap_control;
    GRWLbool MESA_swap_control;
    GRWLbool ARB_multisample;
    GRWLbool ARB_framebuffer_sRGB;
    GRWLbool EXT_framebuffer_sRGB;
    GRWLbool ARB_create_context;
    GRWLbool ARB_create_context_profile;
    GRWLbool ARB_create_context_robustness;
    GRWLbool EXT_create_context_es2_profile;
    GRWLbool ARB_create_context_no_error;
    GRWLbool ARB_context_flush_control;
} _GRWLlibraryGLX;

// GLX-specific per usercontext data
//
typedef struct _GRWLusercontextGLX
{
    GLXContext handle;
} _GRWLusercontextGLX;

// X11-specific per-window data
//
typedef struct _GRWLwindowX11
{
    Colormap colormap;
    Window handle;
    Window parent;
    XIC ic;

    GRWLbool overrideRedirect;
    GRWLbool iconified;
    GRWLbool maximized;

    // Whether the visual supports framebuffer transparency
    GRWLbool transparent;

    // Cached position and size used to filter out duplicate events
    int width, height;
    int xpos, ypos;

    // The last received cursor position, regardless of source
    int lastCursorPosX, lastCursorPosY;
    // The last position the cursor was warped to by GRWL
    int warpCursorPosX, warpCursorPosY;

    // The time of the last KeyPress event per keycode, for discarding
    // duplicate key events generated for some keys by ibus
    Time keyPressTimes[256];

    // Preedit callbacks
    XIMCallback preeditStartCallback;
    XIMCallback preeditDoneCallback;
    XIMCallback preeditDrawCallback;
    XIMCallback preeditCaretCallback;
    XIMCallback statusStartCallback;
    XIMCallback statusDoneCallback;
    XIMCallback statusDrawCallback;

    int imeFocus;
} _GRWLwindowX11;

// X11-specific global data
//
typedef struct _GRWLlibraryX11
{
    Display* display;
    int screen;
    Window root;

    // System content scale
    float contentScaleX, contentScaleY;
    // Helper window for IPC
    Window helperWindowHandle;
    // Invisible cursor for hidden cursor mode
    Cursor hiddenCursorHandle;
    // Context for mapping window XIDs to _GRWLwindow pointers
    XContext context;
    // XIM input method
    XIM im;
    // XIM input method style
    XIMStyle imStyle;
    // The previous X error handler, to be restored later
    XErrorHandler errorHandler;
    // Most recent error code received by X error handler
    int errorCode;
    // Primary selection string (while the primary selection is owned)
    char* primarySelectionString;
    // Clipboard string (while the selection is owned)
    char* clipboardString;
    // Key name string
    char keynames[GRWL_KEY_LAST + 1][5];
    // X11 keycode to GRWL key LUT
    short int keycodes[256];
    // GRWL key to X11 keycode LUT
    short int scancodes[GRWL_KEY_LAST + 1];
    char* keyboardLayoutName;
    // Where to place the cursor when re-enabled
    double restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _GRWLwindow* disabledCursorWindow;
    int emptyEventPipe[2];

    // Window manager atoms
    Atom NET_SUPPORTED;
    Atom NET_SUPPORTING_WM_CHECK;
    Atom WM_PROTOCOLS;
    Atom WM_STATE;
    Atom WM_DELETE_WINDOW;
    Atom NET_WM_NAME;
    Atom NET_WM_ICON_NAME;
    Atom NET_WM_ICON;
    Atom NET_WM_PID;
    Atom NET_WM_PING;
    Atom NET_WM_WINDOW_TYPE;
    Atom NET_WM_WINDOW_TYPE_NORMAL;
    Atom NET_WM_STATE;
    Atom NET_WM_STATE_ABOVE;
    Atom NET_WM_STATE_FULLSCREEN;
    Atom NET_WM_STATE_MAXIMIZED_VERT;
    Atom NET_WM_STATE_MAXIMIZED_HORZ;
    Atom NET_WM_STATE_DEMANDS_ATTENTION;
    Atom NET_WM_BYPASS_COMPOSITOR;
    Atom NET_WM_FULLSCREEN_MONITORS;
    Atom NET_WM_WINDOW_OPACITY;
    Atom NET_WM_CM_Sx;
    Atom NET_WORKAREA;
    Atom NET_CURRENT_DESKTOP;
    Atom NET_ACTIVE_WINDOW;
    Atom NET_FRAME_EXTENTS;
    Atom NET_REQUEST_FRAME_EXTENTS;
    Atom MOTIF_WM_HINTS;

    // Xdnd (drag and drop) atoms
    Atom XdndAware;
    Atom XdndEnter;
    Atom XdndPosition;
    Atom XdndStatus;
    Atom XdndActionCopy;
    Atom XdndDrop;
    Atom XdndFinished;
    Atom XdndSelection;
    Atom XdndTypeList;
    Atom text_uri_list;

    // Selection (clipboard) atoms
    Atom TARGETS;
    Atom MULTIPLE;
    Atom INCR;
    Atom CLIPBOARD;
    Atom PRIMARY;
    Atom CLIPBOARD_MANAGER;
    Atom SAVE_TARGETS;
    Atom NULL_;
    Atom UTF8_STRING;
    Atom COMPOUND_STRING;
    Atom ATOM_PAIR;
    Atom GRWL_SELECTION;

    struct
    {
        void* handle;
        GRWLbool utf8;
        PFN_XAllocClassHint AllocClassHint;
        PFN_XAllocSizeHints AllocSizeHints;
        PFN_XAllocWMHints AllocWMHints;
        PFN_XChangeProperty ChangeProperty;
        PFN_XChangeWindowAttributes ChangeWindowAttributes;
        PFN_XCheckIfEvent CheckIfEvent;
        PFN_XCheckTypedWindowEvent CheckTypedWindowEvent;
        PFN_XCloseDisplay CloseDisplay;
        PFN_XCloseIM CloseIM;
        PFN_XConvertSelection ConvertSelection;
        PFN_XCreateColormap CreateColormap;
        PFN_XCreateFontCursor CreateFontCursor;
        PFN_XCreateIC CreateIC;
        PFN_XCreateRegion CreateRegion;
        PFN_XCreateWindow CreateWindow;
        PFN_XDefineCursor DefineCursor;
        PFN_XDeleteContext DeleteContext;
        PFN_XDeleteProperty DeleteProperty;
        PFN_XDestroyIC DestroyIC;
        PFN_XDestroyRegion DestroyRegion;
        PFN_XDestroyWindow DestroyWindow;
        PFN_XDisplayKeycodes DisplayKeycodes;
        PFN_XEventsQueued EventsQueued;
        PFN_XFilterEvent FilterEvent;
        PFN_XFindContext FindContext;
        PFN_XFlush Flush;
        PFN_XFree Free;
        PFN_XFreeColormap FreeColormap;
        PFN_XFreeCursor FreeCursor;
        PFN_XFreeEventData FreeEventData;
        PFN_XGetAtomName GetAtomName;
        PFN_XGetErrorText GetErrorText;
        PFN_XGetEventData GetEventData;
        PFN_XGetICValues GetICValues;
        PFN_XGetIMValues GetIMValues;
        PFN_XGetInputFocus GetInputFocus;
        PFN_XGetKeyboardMapping GetKeyboardMapping;
        PFN_XGetScreenSaver GetScreenSaver;
        PFN_XGetSelectionOwner GetSelectionOwner;
        PFN_XGetVisualInfo GetVisualInfo;
        PFN_XGetWMNormalHints GetWMNormalHints;
        PFN_XGetWindowAttributes GetWindowAttributes;
        PFN_XGetWindowProperty GetWindowProperty;
        PFN_XGrabPointer GrabPointer;
        PFN_XIconifyWindow IconifyWindow;
        PFN_XInternAtom InternAtom;
        PFN_XLookupString LookupString;
        PFN_XMapRaised MapRaised;
        PFN_XMapWindow MapWindow;
        PFN_XMoveResizeWindow MoveResizeWindow;
        PFN_XMoveWindow MoveWindow;
        PFN_XNextEvent NextEvent;
        PFN_XOpenIM OpenIM;
        PFN_XPeekEvent PeekEvent;
        PFN_XPending Pending;
        PFN_XQueryExtension QueryExtension;
        PFN_XQueryPointer QueryPointer;
        PFN_XRaiseWindow RaiseWindow;
        PFN_XRegisterIMInstantiateCallback RegisterIMInstantiateCallback;
        PFN_XResizeWindow ResizeWindow;
        PFN_XResourceManagerString ResourceManagerString;
        PFN_XSaveContext SaveContext;
        PFN_XSelectInput SelectInput;
        PFN_XSendEvent SendEvent;
        PFN_XSetClassHint SetClassHint;
        PFN_XSetErrorHandler SetErrorHandler;
        PFN_XSetICFocus SetICFocus;
        PFN_XSetICValues SetICValues;
        PFN_XSetIMValues SetIMValues;
        PFN_XSetInputFocus SetInputFocus;
        PFN_XSetLocaleModifiers SetLocaleModifiers;
        PFN_XSetScreenSaver SetScreenSaver;
        PFN_XSetSelectionOwner SetSelectionOwner;
        PFN_XSetWMHints SetWMHints;
        PFN_XSetWMNormalHints SetWMNormalHints;
        PFN_XSetWMProtocols SetWMProtocols;
        PFN_XSupportsLocale SupportsLocale;
        PFN_XSync Sync;
        PFN_XTranslateCoordinates TranslateCoordinates;
        PFN_XUndefineCursor UndefineCursor;
        PFN_XUngrabPointer UngrabPointer;
        PFN_XUnmapWindow UnmapWindow;
        PFN_XUnsetICFocus UnsetICFocus;
        PFN_XVaCreateNestedList VaCreateNestedList;
        PFN_XVisualIDFromVisual VisualIDFromVisual;
        PFN_XWarpPointer WarpPointer;
        PFN_XUnregisterIMInstantiateCallback UnregisterIMInstantiateCallback;
        PFN_XmbResetIC mbResetIC;
        PFN_Xutf8LookupString utf8LookupString;
        PFN_Xutf8SetWMProperties utf8SetWMProperties;
    } xlib;

    struct
    {
        PFN_XrmDestroyDatabase DestroyDatabase;
        PFN_XrmGetResource GetResource;
        PFN_XrmGetStringDatabase GetStringDatabase;
        PFN_XrmUniqueQuark UniqueQuark;
    } xrm;

    struct
    {
        GRWLbool available;
        void* handle;
        int eventBase;
        int errorBase;
        int major;
        int minor;
        GRWLbool gammaBroken;
        GRWLbool monitorBroken;
        PFN_XRRAllocGamma AllocGamma;
        PFN_XRRFreeCrtcInfo FreeCrtcInfo;
        PFN_XRRFreeGamma FreeGamma;
        PFN_XRRFreeOutputInfo FreeOutputInfo;
        PFN_XRRFreeScreenResources FreeScreenResources;
        PFN_XRRGetCrtcGamma GetCrtcGamma;
        PFN_XRRGetCrtcGammaSize GetCrtcGammaSize;
        PFN_XRRGetCrtcInfo GetCrtcInfo;
        PFN_XRRGetOutputInfo GetOutputInfo;
        PFN_XRRGetOutputPrimary GetOutputPrimary;
        PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent;
        PFN_XRRQueryExtension QueryExtension;
        PFN_XRRQueryVersion QueryVersion;
        PFN_XRRSelectInput SelectInput;
        PFN_XRRSetCrtcConfig SetCrtcConfig;
        PFN_XRRSetCrtcGamma SetCrtcGamma;
        PFN_XRRUpdateConfiguration UpdateConfiguration;
    } randr;

    struct
    {
        GRWLbool available;
        GRWLbool detectable;
        int majorOpcode;
        int eventBase;
        int errorBase;
        int major;
        int minor;
        unsigned int group;
        PFN_XkbAllocKeyboard AllocKeyboard;
        PFN_XkbFreeKeyboard FreeKeyboard;
        PFN_XkbFreeNames FreeNames;
        PFN_XkbGetMap GetMap;
        PFN_XkbGetNames GetNames;
        PFN_XkbGetState GetState;
        PFN_XkbKeycodeToKeysym KeycodeToKeysym;
        PFN_XkbQueryExtension QueryExtension;
        PFN_XkbSelectEventDetails SelectEventDetails;
        PFN_XkbSetDetectableAutoRepeat SetDetectableAutoRepeat;
    } xkb;

    struct
    {
        int count;
        int timeout;
        int interval;
        int blanking;
        int exposure;
    } saver;

    struct
    {
        int version;
        Window source;
        Atom format;
    } xdnd;

    struct
    {
        void* handle;
        PFN_XcursorImageCreate ImageCreate;
        PFN_XcursorImageDestroy ImageDestroy;
        PFN_XcursorImageLoadCursor ImageLoadCursor;
        PFN_XcursorGetTheme GetTheme;
        PFN_XcursorGetDefaultSize GetDefaultSize;
        PFN_XcursorLibraryLoadImage LibraryLoadImage;
    } xcursor;

    struct
    {
        GRWLbool available;
        void* handle;
        int major;
        int minor;
        PFN_XineramaIsActive IsActive;
        PFN_XineramaQueryExtension QueryExtension;
        PFN_XineramaQueryScreens QueryScreens;
    } xinerama;

    struct
    {
        void* handle;
        PFN_XGetXCBConnection GetXCBConnection;
    } x11xcb;

    struct
    {
        GRWLbool available;
        void* handle;
        int eventBase;
        int errorBase;
        PFN_XF86VidModeQueryExtension QueryExtension;
        PFN_XF86VidModeGetGammaRamp GetGammaRamp;
        PFN_XF86VidModeSetGammaRamp SetGammaRamp;
        PFN_XF86VidModeGetGammaRampSize GetGammaRampSize;
    } vidmode;

    struct
    {
        GRWLbool available;
        void* handle;
        int majorOpcode;
        int eventBase;
        int errorBase;
        int major;
        int minor;
        PFN_XIQueryVersion QueryVersion;
        PFN_XISelectEvents SelectEvents;
    } xi;

    struct
    {
        GRWLbool available;
        void* handle;
        int major;
        int minor;
        int eventBase;
        int errorBase;
        PFN_XRenderQueryExtension QueryExtension;
        PFN_XRenderQueryVersion QueryVersion;
        PFN_XRenderFindVisualFormat FindVisualFormat;
    } xrender;

    struct
    {
        GRWLbool available;
        void* handle;
        int major;
        int minor;
        int eventBase;
        int errorBase;
        PFN_XShapeQueryExtension QueryExtension;
        PFN_XShapeCombineRegion ShapeCombineRegion;
        PFN_XShapeQueryVersion QueryVersion;
        PFN_XShapeCombineMask ShapeCombineMask;
    } xshape;
} _GRWLlibraryX11;

// X11-specific per-monitor data
//
typedef struct _GRWLmonitorX11
{
    RROutput output;
    RRCrtc crtc;
    RRMode oldMode;

    // Index of corresponding Xinerama screen,
    // for EWMH full screen window placement
    int index;
} _GRWLmonitorX11;

// X11-specific per-cursor data
//
typedef struct _GRWLcursorX11
{
    Cursor handle;
} _GRWLcursorX11;

GRWLbool _grwlConnectX11(int platformID, _GRWLplatform* platform);
int _grwlInitX11(void);
void _grwlTerminateX11(void);

GRWLbool _grwlCreateWindowX11(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                              const _GRWLfbconfig* fbconfig);
void _grwlDestroyWindowX11(_GRWLwindow* window);
void _grwlSetWindowTitleX11(_GRWLwindow* window, const char* title);
void _grwlSetWindowIconX11(_GRWLwindow* window, int count, const GRWLimage* images);
void _grwlSetWindowProgressIndicatorX11(_GRWLwindow* window, int progressState, double value);
void _grwlSetWindowBadgeX11(_GRWLwindow* window, int count);
void _grwlSetWindowBadgeStringX11(_GRWLwindow* window, const char* string);
void _grwlGetWindowPosX11(_GRWLwindow* window, int* xpos, int* ypos);
void _grwlSetWindowPosX11(_GRWLwindow* window, int xpos, int ypos);
void _grwlGetWindowSizeX11(_GRWLwindow* window, int* width, int* height);
void _grwlSetWindowSizeX11(_GRWLwindow* window, int width, int height);
void _grwlSetWindowSizeLimitsX11(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _grwlSetWindowAspectRatioX11(_GRWLwindow* window, int numer, int denom);
void _grwlGetFramebufferSizeX11(_GRWLwindow* window, int* width, int* height);
void _grwlGetWindowFrameSizeX11(_GRWLwindow* window, int* left, int* top, int* right, int* bottom);
void _grwlGetWindowContentScaleX11(_GRWLwindow* window, float* xscale, float* yscale);
void _grwlIconifyWindowX11(_GRWLwindow* window);
void _grwlRestoreWindowX11(_GRWLwindow* window);
void _grwlMaximizeWindowX11(_GRWLwindow* window);
void _grwlShowWindowX11(_GRWLwindow* window);
void _grwlHideWindowX11(_GRWLwindow* window);
void _grwlRequestWindowAttentionX11(_GRWLwindow* window);
void _grwlFocusWindowX11(_GRWLwindow* window);
void _grwlSetWindowMonitorX11(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                              int refreshRate);
GRWLbool _grwlWindowFocusedX11(_GRWLwindow* window);
GRWLbool _grwlWindowIconifiedX11(_GRWLwindow* window);
GRWLbool _grwlWindowVisibleX11(_GRWLwindow* window);
GRWLbool _grwlWindowMaximizedX11(_GRWLwindow* window);
GRWLbool _grwlWindowHoveredX11(_GRWLwindow* window);
GRWLbool _grwlFramebufferTransparentX11(_GRWLwindow* window);
void _grwlSetWindowResizableX11(_GRWLwindow* window, GRWLbool enabled);
void _grwlSetWindowDecoratedX11(_GRWLwindow* window, GRWLbool enabled);
void _grwlSetWindowFloatingX11(_GRWLwindow* window, GRWLbool enabled);
float _grwlGetWindowOpacityX11(_GRWLwindow* window);
void _grwlSetWindowOpacityX11(_GRWLwindow* window, float opacity);
void _grwlSetWindowMousePassthroughX11(_GRWLwindow* window, GRWLbool enabled);

void _grwlSetRawMouseMotionX11(_GRWLwindow* window, GRWLbool enabled);
GRWLbool _grwlRawMouseMotionSupportedX11(void);

void _grwlPollEventsX11(void);
void _grwlWaitEventsX11(void);
void _grwlWaitEventsTimeoutX11(double timeout);
void _grwlPostEmptyEventX11(void);

void _grwlGetCursorPosX11(_GRWLwindow* window, double* xpos, double* ypos);
void _grwlSetCursorPosX11(_GRWLwindow* window, double xpos, double ypos);
void _grwlSetCursorModeX11(_GRWLwindow* window, int mode);
const char* _grwlGetScancodeNameX11(int scancode);
int _grwlGetKeyScancodeX11(int key);
const char* _grwlGetKeyboardLayoutNameX11(void);
GRWLbool _grwlCreateCursorX11(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot);
GRWLbool _grwlCreateStandardCursorX11(_GRWLcursor* cursor, int shape);
void _grwlDestroyCursorX11(_GRWLcursor* cursor);
void _grwlSetCursorX11(_GRWLwindow* window, _GRWLcursor* cursor);
void _grwlSetClipboardStringX11(const char* string);
const char* _grwlGetClipboardStringX11(void);

void _grwlUpdatePreeditCursorRectangleX11(_GRWLwindow* window);
void _grwlResetPreeditTextX11(_GRWLwindow* window);
void _grwlSetIMEStatusX11(_GRWLwindow* window, int active);
int _grwlGetIMEStatusX11(_GRWLwindow* window);

EGLenum _grwlGetEGLPlatformX11(EGLint** attribs);
EGLNativeDisplayType _grwlGetEGLNativeDisplayX11(void);
EGLNativeWindowType _grwlGetEGLNativeWindowX11(_GRWLwindow* window);

void _grwlGetRequiredInstanceExtensionsX11(char** extensions);
GRWLbool _grwlGetPhysicalDevicePresentationSupportX11(VkInstance instance, VkPhysicalDevice device,
                                                      uint32_t queuefamily);
VkResult _grwlCreateWindowSurfaceX11(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                     VkSurfaceKHR* surface);

void _grwlFreeMonitorX11(_GRWLmonitor* monitor);
void _grwlGetMonitorPosX11(_GRWLmonitor* monitor, int* xpos, int* ypos);
void _grwlGetMonitorContentScaleX11(_GRWLmonitor* monitor, float* xscale, float* yscale);
void _grwlGetMonitorWorkareaX11(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GRWLvidmode* _grwlGetVideoModesX11(_GRWLmonitor* monitor, int* count);
void _grwlGetVideoModeX11(_GRWLmonitor* monitor, GRWLvidmode* mode);
GRWLbool _grwlGetGammaRampX11(_GRWLmonitor* monitor, GRWLgammaramp* ramp);
void _grwlSetGammaRampX11(_GRWLmonitor* monitor, const GRWLgammaramp* ramp);

void _grwlPollMonitorsX11(void);
void _grwlSetVideoModeX11(_GRWLmonitor* monitor, const GRWLvidmode* desired);
void _grwlRestoreVideoModeX11(_GRWLmonitor* monitor);

Cursor _grwlCreateNativeCursorX11(const GRWLimage* image, int xhot, int yhot);

unsigned long _grwlGetWindowPropertyX11(Window window, Atom property, Atom type, unsigned char** value);
GRWLbool _grwlIsVisualTransparentX11(Visual* visual);

void _grwlGrabErrorHandlerX11(void);
void _grwlReleaseErrorHandlerX11(void);
void _grwlInputErrorX11(int error, const char* message);

void _grwlPushSelectionToManagerX11(void);
void _grwlCreateInputContextX11(_GRWLwindow* window);

GRWLbool _grwlInitGLX(void);
void _grwlTerminateGLX(void);
GRWLbool _grwlCreateContextGLX(_GRWLwindow* window, const _GRWLctxconfig* ctxconfig, const _GRWLfbconfig* fbconfig);
void _grwlDestroyContextGLX(_GRWLwindow* window);
GRWLbool _grwlChooseVisualGLX(const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                              const _GRWLfbconfig* fbconfig, Visual** visual, int* depth);

_GRWLusercontext* _grwlCreateUserContextX11(_GRWLwindow* window);
_GRWLusercontext* _grwlCreateUserContextGLX(_GRWLwindow* window);

#endif
