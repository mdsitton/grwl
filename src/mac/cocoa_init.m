//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_COCOA)

    #include <sys/param.h> // For MAXPATHLEN

    // Needed for _NSGetProgname
    #include <crt_externs.h>

// Change to our application bundle's resources directory, if present
//
static void changeToResourcesDirectory()
{
    char resourcesPath[MAXPATHLEN];

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
    {
        return;
    }

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);

    CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);
    if (CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo)
    {
        CFRelease(last);
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(last);

    if (!CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8*)resourcesPath, MAXPATHLEN))
    {
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(resourcesURL);

    chdir(resourcesPath);
}

// Set up the menu bar (manually)
// This is nasty, nasty stuff -- calls to undocumented semi-private APIs that
// could go away at any moment, lots of stuff that really should be
// localize(d|able), etc.  Add a nib to save us this horror.
//
static void createMenuBar()
{
    NSString* appName = nil;
    NSDictionary* bundleInfo = [[NSBundle mainBundle] infoDictionary];
    NSString* nameKeys[] = {
        @"CFBundleDisplayName",
        @"CFBundleName",
        @"CFBundleExecutable",
    };

    // Try to figure out what the calling application is called

    for (size_t i = 0; i < sizeof(nameKeys) / sizeof(nameKeys[0]); i++)
    {
        id name = bundleInfo[nameKeys[i]];
        if (name && [name isKindOfClass:[NSString class]] && ![name isEqualToString:@""])
        {
            appName = name;
            break;
        }
    }

    if (!appName)
    {
        char** progname = _NSGetProgname();
        if (progname && *progname)
        {
            appName = @(*progname);
        }
        else
        {
            appName = @"GRWL Application";
        }
    }

    NSMenu* bar = [[NSMenu alloc] init];
    [NSApp setMainMenu:bar];

    NSMenuItem* appMenuItem = [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenu* servicesMenu = [[NSMenu alloc] init];
    [NSApp setServicesMenu:servicesMenu];
    [[appMenu addItemWithTitle:@"Services" action:NULL keyEquivalent:@""] setSubmenu:servicesMenu];
    [servicesMenu release];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [[appMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"]
        setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    [appMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

    NSMenuItem* windowMenuItem = [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    [bar release];
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [NSApp setWindowsMenu:windowMenu];
    [windowMenuItem setSubmenu:windowMenu];

    [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [windowMenu addItemWithTitle:@"Bring All to Front" action:@selector(arrangeInFront:) keyEquivalent:@""];

    // TODO: Make this appear at the bottom of the menu (for consistency)
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [[windowMenu addItemWithTitle:@"Enter Full Screen" action:@selector(toggleFullScreen:) keyEquivalent:@"f"]
        setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

    // Prior to Snow Leopard, we need to use this oddly-named semi-private API
    // to get the application menu working properly.
    SEL setAppleMenuSelector = NSSelectorFromString(@"setAppleMenu:");
    [NSApp performSelector:setAppleMenuSelector withObject:appMenu];
}

// Create key code translation tables
//
static void createKeyTables()
{
    memset(_grwl.ns.keycodes, -1, sizeof(_grwl.ns.keycodes));
    memset(_grwl.ns.scancodes, -1, sizeof(_grwl.ns.scancodes));

    _grwl.ns.keycodes[0x1D] = GRWL_KEY_0;
    _grwl.ns.keycodes[0x12] = GRWL_KEY_1;
    _grwl.ns.keycodes[0x13] = GRWL_KEY_2;
    _grwl.ns.keycodes[0x14] = GRWL_KEY_3;
    _grwl.ns.keycodes[0x15] = GRWL_KEY_4;
    _grwl.ns.keycodes[0x17] = GRWL_KEY_5;
    _grwl.ns.keycodes[0x16] = GRWL_KEY_6;
    _grwl.ns.keycodes[0x1A] = GRWL_KEY_7;
    _grwl.ns.keycodes[0x1C] = GRWL_KEY_8;
    _grwl.ns.keycodes[0x19] = GRWL_KEY_9;
    _grwl.ns.keycodes[0x00] = GRWL_KEY_A;
    _grwl.ns.keycodes[0x0B] = GRWL_KEY_B;
    _grwl.ns.keycodes[0x08] = GRWL_KEY_C;
    _grwl.ns.keycodes[0x02] = GRWL_KEY_D;
    _grwl.ns.keycodes[0x0E] = GRWL_KEY_E;
    _grwl.ns.keycodes[0x03] = GRWL_KEY_F;
    _grwl.ns.keycodes[0x05] = GRWL_KEY_G;
    _grwl.ns.keycodes[0x04] = GRWL_KEY_H;
    _grwl.ns.keycodes[0x22] = GRWL_KEY_I;
    _grwl.ns.keycodes[0x26] = GRWL_KEY_J;
    _grwl.ns.keycodes[0x28] = GRWL_KEY_K;
    _grwl.ns.keycodes[0x25] = GRWL_KEY_L;
    _grwl.ns.keycodes[0x2E] = GRWL_KEY_M;
    _grwl.ns.keycodes[0x2D] = GRWL_KEY_N;
    _grwl.ns.keycodes[0x1F] = GRWL_KEY_O;
    _grwl.ns.keycodes[0x23] = GRWL_KEY_P;
    _grwl.ns.keycodes[0x0C] = GRWL_KEY_Q;
    _grwl.ns.keycodes[0x0F] = GRWL_KEY_R;
    _grwl.ns.keycodes[0x01] = GRWL_KEY_S;
    _grwl.ns.keycodes[0x11] = GRWL_KEY_T;
    _grwl.ns.keycodes[0x20] = GRWL_KEY_U;
    _grwl.ns.keycodes[0x09] = GRWL_KEY_V;
    _grwl.ns.keycodes[0x0D] = GRWL_KEY_W;
    _grwl.ns.keycodes[0x07] = GRWL_KEY_X;
    _grwl.ns.keycodes[0x10] = GRWL_KEY_Y;
    _grwl.ns.keycodes[0x06] = GRWL_KEY_Z;

    _grwl.ns.keycodes[0x27] = GRWL_KEY_APOSTROPHE;
    _grwl.ns.keycodes[0x2A] = GRWL_KEY_BACKSLASH;
    _grwl.ns.keycodes[0x2B] = GRWL_KEY_COMMA;
    _grwl.ns.keycodes[0x18] = GRWL_KEY_EQUAL;
    _grwl.ns.keycodes[0x32] = GRWL_KEY_GRAVE_ACCENT;
    _grwl.ns.keycodes[0x21] = GRWL_KEY_LEFT_BRACKET;
    _grwl.ns.keycodes[0x1B] = GRWL_KEY_MINUS;
    _grwl.ns.keycodes[0x2F] = GRWL_KEY_PERIOD;
    _grwl.ns.keycodes[0x1E] = GRWL_KEY_RIGHT_BRACKET;
    _grwl.ns.keycodes[0x29] = GRWL_KEY_SEMICOLON;
    _grwl.ns.keycodes[0x2C] = GRWL_KEY_SLASH;
    _grwl.ns.keycodes[0x0A] = GRWL_KEY_WORLD_1;

    _grwl.ns.keycodes[0x33] = GRWL_KEY_BACKSPACE;
    _grwl.ns.keycodes[0x39] = GRWL_KEY_CAPS_LOCK;
    _grwl.ns.keycodes[0x75] = GRWL_KEY_DELETE;
    _grwl.ns.keycodes[0x7D] = GRWL_KEY_DOWN;
    _grwl.ns.keycodes[0x77] = GRWL_KEY_END;
    _grwl.ns.keycodes[0x24] = GRWL_KEY_ENTER;
    _grwl.ns.keycodes[0x35] = GRWL_KEY_ESCAPE;
    _grwl.ns.keycodes[0x7A] = GRWL_KEY_F1;
    _grwl.ns.keycodes[0x78] = GRWL_KEY_F2;
    _grwl.ns.keycodes[0x63] = GRWL_KEY_F3;
    _grwl.ns.keycodes[0x76] = GRWL_KEY_F4;
    _grwl.ns.keycodes[0x60] = GRWL_KEY_F5;
    _grwl.ns.keycodes[0x61] = GRWL_KEY_F6;
    _grwl.ns.keycodes[0x62] = GRWL_KEY_F7;
    _grwl.ns.keycodes[0x64] = GRWL_KEY_F8;
    _grwl.ns.keycodes[0x65] = GRWL_KEY_F9;
    _grwl.ns.keycodes[0x6D] = GRWL_KEY_F10;
    _grwl.ns.keycodes[0x67] = GRWL_KEY_F11;
    _grwl.ns.keycodes[0x6F] = GRWL_KEY_F12;
    _grwl.ns.keycodes[0x69] = GRWL_KEY_PRINT_SCREEN;
    _grwl.ns.keycodes[0x6B] = GRWL_KEY_F14;
    _grwl.ns.keycodes[0x71] = GRWL_KEY_F15;
    _grwl.ns.keycodes[0x6A] = GRWL_KEY_F16;
    _grwl.ns.keycodes[0x40] = GRWL_KEY_F17;
    _grwl.ns.keycodes[0x4F] = GRWL_KEY_F18;
    _grwl.ns.keycodes[0x50] = GRWL_KEY_F19;
    _grwl.ns.keycodes[0x5A] = GRWL_KEY_F20;
    _grwl.ns.keycodes[0x73] = GRWL_KEY_HOME;
    _grwl.ns.keycodes[0x72] = GRWL_KEY_INSERT;
    _grwl.ns.keycodes[0x7B] = GRWL_KEY_LEFT;
    _grwl.ns.keycodes[0x3A] = GRWL_KEY_LEFT_ALT;
    _grwl.ns.keycodes[0x3B] = GRWL_KEY_LEFT_CONTROL;
    _grwl.ns.keycodes[0x38] = GRWL_KEY_LEFT_SHIFT;
    _grwl.ns.keycodes[0x37] = GRWL_KEY_LEFT_SUPER;
    _grwl.ns.keycodes[0x6E] = GRWL_KEY_MENU;
    _grwl.ns.keycodes[0x47] = GRWL_KEY_NUM_LOCK;
    _grwl.ns.keycodes[0x79] = GRWL_KEY_PAGE_DOWN;
    _grwl.ns.keycodes[0x74] = GRWL_KEY_PAGE_UP;
    _grwl.ns.keycodes[0x7C] = GRWL_KEY_RIGHT;
    _grwl.ns.keycodes[0x3D] = GRWL_KEY_RIGHT_ALT;
    _grwl.ns.keycodes[0x3E] = GRWL_KEY_RIGHT_CONTROL;
    _grwl.ns.keycodes[0x3C] = GRWL_KEY_RIGHT_SHIFT;
    _grwl.ns.keycodes[0x36] = GRWL_KEY_RIGHT_SUPER;
    _grwl.ns.keycodes[0x31] = GRWL_KEY_SPACE;
    _grwl.ns.keycodes[0x30] = GRWL_KEY_TAB;
    _grwl.ns.keycodes[0x7E] = GRWL_KEY_UP;

    _grwl.ns.keycodes[0x52] = GRWL_KEY_KP_0;
    _grwl.ns.keycodes[0x53] = GRWL_KEY_KP_1;
    _grwl.ns.keycodes[0x54] = GRWL_KEY_KP_2;
    _grwl.ns.keycodes[0x55] = GRWL_KEY_KP_3;
    _grwl.ns.keycodes[0x56] = GRWL_KEY_KP_4;
    _grwl.ns.keycodes[0x57] = GRWL_KEY_KP_5;
    _grwl.ns.keycodes[0x58] = GRWL_KEY_KP_6;
    _grwl.ns.keycodes[0x59] = GRWL_KEY_KP_7;
    _grwl.ns.keycodes[0x5B] = GRWL_KEY_KP_8;
    _grwl.ns.keycodes[0x5C] = GRWL_KEY_KP_9;
    _grwl.ns.keycodes[0x45] = GRWL_KEY_KP_ADD;
    _grwl.ns.keycodes[0x41] = GRWL_KEY_KP_DECIMAL;
    _grwl.ns.keycodes[0x4B] = GRWL_KEY_KP_DIVIDE;
    _grwl.ns.keycodes[0x4C] = GRWL_KEY_KP_ENTER;
    _grwl.ns.keycodes[0x51] = GRWL_KEY_KP_EQUAL;
    _grwl.ns.keycodes[0x43] = GRWL_KEY_KP_MULTIPLY;
    _grwl.ns.keycodes[0x4E] = GRWL_KEY_KP_SUBTRACT;

    for (int scancode = 0; scancode < 256; scancode++)
    {
        // Store the reverse translation for faster key name lookup
        if (_grwl.ns.keycodes[scancode] >= 0)
        {
            _grwl.ns.scancodes[_grwl.ns.keycodes[scancode]] = scancode;
        }
    }
}

// Load HIToolbox.framework and the TIS symbols we need from it
//
static bool initializeTIS()
{
    // This works only because Cocoa has already loaded it properly
    _grwl.ns.tis.bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.HIToolbox"));
    if (!_grwl.ns.tis.bundle)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to load HIToolbox.framework");
        return false;
    }

    CFStringRef* kCategoryKeyboardInputSource =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISCategoryKeyboardInputSource"));
    CFStringRef* kPropertyInputSourceCategory =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyInputSourceCategory"));
    CFStringRef* kPropertyInputSourceID =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyInputSourceID"));
    CFStringRef* kPropertyInputSourceIsSelectCapable =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyInputSourceIsSelectCapable"));
    CFStringRef* kPropertyInputSourceType =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyInputSourceType"));
    CFStringRef* kPropertyUnicodeKeyLayoutData =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyUnicodeKeyLayoutData"));
    CFStringRef* kPropertyInputSourceID =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyInputSourceID"));
    CFStringRef* kPropertyLocalizedName =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISPropertyLocalizedName"));
    CFStringRef* kTypeKeyboardInputMethodModeEnabled =
        CFBundleGetDataPointerForName(_grwl.ns.tis.bundle, CFSTR("kTISTypeKeyboardInputMethodModeEnabled"));
    _grwl.ns.tis.CopyCurrentASCIICapableKeyboardInputSource =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISCopyCurrentASCIICapableKeyboardInputSource"));
    _grwl.ns.tis.CopyCurrentKeyboardInputSource =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISCopyCurrentKeyboardInputSource"));
    _grwl.ns.tis.CopyCurrentKeyboardLayoutInputSource =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISCopyCurrentKeyboardLayoutInputSource"));
    _grwl.ns.tis.CopyInputSourceForLanguage =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISCopyInputSourceForLanguage"));
    _grwl.ns.tis.CreateASCIICapableInputSourceList =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISCreateASCIICapableInputSourceList"));
    _grwl.ns.tis.CreateInputSourceList =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISCreateInputSourceList"));
    _grwl.ns.tis.GetInputSourceProperty =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISGetInputSourceProperty"));
    _grwl.ns.tis.SelectInputSource =
        CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("TISSelectInputSource"));
    _grwl.ns.tis.GetKbdType = CFBundleGetFunctionPointerForName(_grwl.ns.tis.bundle, CFSTR("LMGetKbdType"));

    if (!kCategoryKeyboardInputSource || !kPropertyInputSourceCategory || !kPropertyInputSourceID ||
        !kPropertyLocalizedName || !kPropertyInputSourceIsSelectCapable || !kPropertyInputSourceType ||
        !kPropertyUnicodeKeyLayoutData || !kTypeKeyboardInputMethodModeEnabled ||
        !TISCopyCurrentASCIICapableKeyboardInputSource || !TISCopyCurrentKeyboardInputSource ||
        !TISCopyCurrentKeyboardLayoutInputSource || !TISCopyInputSourceForLanguage ||
        !TISCreateASCIICapableInputSourceList || !TISCreateInputSourceList || !TISGetInputSourceProperty ||
        !TISSelectInputSource || !LMGetKbdType)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to load TIS API symbols");
        return false;
    }

    _grwl.ns.tis.kCategoryKeyboardInputSource = *kCategoryKeyboardInputSource;
    _grwl.ns.tis.kPropertyInputSourceCategory = *kPropertyInputSourceCategory;
    _grwl.ns.tis.kPropertyInputSourceID = *kPropertyInputSourceID;
    _grwl.ns.tis.kPropertyInputSourceIsSelectCapable = *kPropertyInputSourceIsSelectCapable;
    _grwl.ns.tis.kPropertyInputSourceType = *kPropertyInputSourceType;
    _grwl.ns.tis.kPropertyUnicodeKeyLayoutData = *kPropertyUnicodeKeyLayoutData;
    _grwl.ns.tis.kPropertyInputSourceID = *kPropertyInputSourceID;
    _grwl.ns.tis.kPropertyLocalizedName = *kPropertyLocalizedName;
    _grwl.ns.tis.kTypeKeyboardInputMethodModeEnabled = *kTypeKeyboardInputMethodModeEnabled;

    _grwl.ns.inputSource = TISCopyCurrentKeyboardInputSource();
    _grwl.ns.keyboardLayout = TISCopyCurrentKeyboardLayoutInputSource();
    _grwl.ns.unicodeData = TISGetInputSourceProperty(_grwl.ns.keyboardLayout, kTISPropertyUnicodeKeyLayoutData);

    return true;
}

@interface GRWLHelper: NSObject
@end

@implementation GRWLHelper

- ()selectedKeyboardInputSourceChanged:(NSObject*)object
{
    // The keyboard layout is needed for Unicode data which is the source of
    // GRWL key names on Cocoa (the generic input source may not have this)
    CFRelease(_grwl.ns.keyboardLayout);
    _grwl.ns.keyboardLayout = TISCopyCurrentKeyboardLayoutInputSource();
    _grwl.ns.unicodeData = TISGetInputSourceProperty(_grwl.ns.keyboardLayout, kTISPropertyUnicodeKeyLayoutData);

    // The generic input source may be something higher level than a keyboard
    // layout and if so will provide a better layout name than the layout source
    const TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
    const CFStringRef newID = TISGetInputSourceProperty(source, kTISPropertyInputSourceID);
    const CFStringRef oldID = TISGetInputSourceProperty(_grwl.ns.inputSource, kTISPropertyInputSourceID);
    const CFComparisonResult result = CFStringCompare(oldID, newID, 0);
    CFRelease(_grwl.ns.inputSource);
    _grwl.ns.inputSource = source;

    // Filter events as we may receive more than one per input source switch
    if (result != kCFCompareEqualTo)
    {
        _grwlInputKeyboardLayout();
    }
}

- ()doNothing:(id)object
{
}

@end // GRWLHelper

@interface GRWLApplicationDelegate: NSObject <NSApplicationDelegate>
@end

@implementation GRWLApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    for (_GRWLwindow* window = _grwl.windowListHead; window; window = window->next)
    {
        _grwlInputWindowCloseRequest(window);
    }

    return NSTerminateCancel;
}

- ()applicationDidChangeScreenParameters:(NSNotification*)notification
{
    for (_GRWLwindow* window = _grwl.windowListHead; window; window = window->next)
    {
        if (window->context.client != GRWL_NO_API)
        {
            [window->context.nsgl.object update];
        }
    }

    _grwlPollMonitorsCocoa();
}

- ()applicationWillFinishLaunching:(NSNotification*)notification
{
    if (_grwl.hints.init.ns.menubar)
    {
        // Menu bar setup must go between sharedApplication and finishLaunching
        // in order to properly emulate the behavior of NSApplicationMain

        if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"])
        {
            [[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:NSApp topLevelObjects:&_grwl.ns.nibObjects];
        }
        else
        {
            createMenuBar();
        }
    }
}

- ()applicationDidFinishLaunching:(NSNotification*)notification
{
    _grwlPostEmptyEventCocoa();
    [NSApp stop:nil];
}

- ()applicationDidHide:(NSNotification*)notification
{
    for (int i = 0; i < _grwl.monitorCount; i++)
    {
        _grwlRestoreVideoModeCocoa(_grwl.monitors[i]);
    }
}

@end // GRWLApplicationDelegate

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

void* _grwlLoadLocalVulkanLoaderCocoa()
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
    {
        return NULL;
    }

    CFURLRef frameworksUrl = CFBundleCopyPrivateFrameworksURL(bundle);
    if (!frameworksUrl)
    {
        return NULL;
    }

    CFURLRef loaderUrl =
        CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, frameworksUrl, CFSTR("libvulkan.1.dylib"), false);
    if (!loaderUrl)
    {
        CFRelease(frameworksUrl);
        return NULL;
    }

    char path[PATH_MAX];
    void* handle = NULL;

    if (CFURLGetFileSystemRepresentation(loaderUrl, true, (UInt8*)path, sizeof(path) - 1))
    {
        handle = _grwlPlatformLoadModule(path);
    }

    CFRelease(loaderUrl);
    CFRelease(frameworksUrl);
    return handle;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlConnectCocoa(int platformID, _GRWLplatform* platform)
{
    const _GRWLplatform cocoa = {
        GRWL_PLATFORM_COCOA,
        _grwlInitCocoa,
        _grwlTerminateCocoa,
        _grwlGetCursorPosCocoa,
        _grwlSetCursorPosCocoa,
        _grwlSetCursorModeCocoa,
        _grwlSetRawMouseMotionCocoa,
        _grwlRawMouseMotionSupportedCocoa,
        _grwlCreateCursorCocoa,
        _grwlCreateStandardCursorCocoa,
        _grwlDestroyCursorCocoa,
        _grwlSetCursorCocoa,
        _grwlGetScancodeNameCocoa,
        _grwlGetKeyScancodeCocoa,
        _grwlGetKeyboardLayoutNameCocoa,
        _grwlSetClipboardStringCocoa,
        _grwlGetClipboardStringCocoa,
        _grwlUpdatePreeditCursorRectangleCocoa,
        _grwlResetPreeditTextCocoa,
        _grwlSetIMEStatusCocoa,
        _grwlGetIMEStatusCocoa,
        _grwlInitJoysticksCocoa,
        _grwlTerminateJoysticksCocoa,
        _grwlPollJoystickCocoa,
        _grwlGetMappingNameCocoa,
        _grwlUpdateGamepadGUIDCocoa,
        _grwlFreeMonitorCocoa,
        _grwlGetMonitorPosCocoa,
        _grwlGetMonitorContentScaleCocoa,
        _grwlGetMonitorWorkareaCocoa,
        _grwlGetVideoModesCocoa,
        _grwlGetVideoModeCocoa,
        _grwlGetGammaRampCocoa,
        _grwlSetGammaRampCocoa,
        _grwlCreateWindowCocoa,
        _grwlDestroyWindowCocoa,
        _grwlSetWindowTitleCocoa,
        _grwlSetWindowIconCocoa,
        _grwlSetWindowProgressIndicatorCocoa,
        _grwlSetWindowBadgeCocoa,
        _grwlSetWindowBadgeStringCocoa,
        _grwlGetWindowPosCocoa,
        _grwlSetWindowPosCocoa,
        _grwlGetWindowSizeCocoa,
        _grwlSetWindowSizeCocoa,
        _grwlSetWindowSizeLimitsCocoa,
        _grwlSetWindowAspectRatioCocoa,
        _grwlGetFramebufferSizeCocoa,
        _grwlGetWindowFrameSizeCocoa,
        _grwlGetWindowContentScaleCocoa,
        _grwlIconifyWindowCocoa,
        _grwlRestoreWindowCocoa,
        _grwlMaximizeWindowCocoa,
        _grwlShowWindowCocoa,
        _grwlHideWindowCocoa,
        _grwlRequestWindowAttentionCocoa,
        _grwlFocusWindowCocoa,
        _grwlSetWindowMonitorCocoa,
        _grwlWindowFocusedCocoa,
        _grwlWindowIconifiedCocoa,
        _grwlWindowVisibleCocoa,
        _grwlWindowMaximizedCocoa,
        _grwlWindowHoveredCocoa,
        _grwlFramebufferTransparentCocoa,
        _grwlGetWindowOpacityCocoa,
        _grwlSetWindowResizableCocoa,
        _grwlSetWindowDecoratedCocoa,
        _grwlSetWindowFloatingCocoa,
        _grwlSetWindowOpacityCocoa,
        _grwlSetWindowMousePassthroughCocoa,
        _grwlPollEventsCocoa,
        _grwlWaitEventsCocoa,
        _grwlWaitEventsTimeoutCocoa,
        _grwlPostEmptyEventCocoa,
        _grwlCreateUserContextCocoa,
        _grwlGetEGLPlatformCocoa,
        _grwlGetEGLNativeDisplayCocoa,
        _grwlGetEGLNativeWindowCocoa,
        _grwlGetRequiredInstanceExtensionsCocoa,
        _grwlGetPhysicalDevicePresentationSupportCocoa,
        _grwlCreateWindowSurfaceCocoa,
    };

    *platform = cocoa;
    return true;
}

int _grwlInitCocoa()
{
    @autoreleasepool
    {

        _grwl.ns.helper = [[GRWLHelper alloc] init];

        [NSThread detachNewThreadSelector:@selector(doNothing:) toTarget:_grwl.ns.helper withObject:nil];

        [NSApplication sharedApplication];

        _grwl.ns.delegate = [[GRWLApplicationDelegate alloc] init];
        if (_grwl.ns.delegate == nil)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to create application delegate");
            return false;
        }

        [NSApp setDelegate:_grwl.ns.delegate];

        NSEvent* (^block)(NSEvent*) = ^NSEvent*(NSEvent* event) {
          if ([event modifierFlags] & NSEventModifierFlagCommand)
          {
              [[NSApp keyWindow] sendEvent:event];
          }

          return event;
        };

        _grwl.ns.keyUpMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp handler:block];

        if (_grwl.hints.init.ns.chdir)
        {
            changeToResourcesDirectory();
        }

        // Press and Hold prevents some keys from emitting repeated characters
        NSDictionary* defaults = @{@"ApplePressAndHoldEnabled" : @NO};
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];

        [[NSNotificationCenter defaultCenter] addObserver:_grwl.ns.helper
                                                 selector:@selector(selectedKeyboardInputSourceChanged:)
                                                     name:NSTextInputContextKeyboardSelectionDidChangeNotification
                                                   object:nil];

        createKeyTables();

        _grwl.ns.eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
        if (!_grwl.ns.eventSource)
        {
            return false;
        }

        CGEventSourceSetLocalEventsSuppressionInterval(_grwl.ns.eventSource, 0.0);

        if (!initializeTIS())
        {
            return false;
        }

        _grwlPollMonitorsCocoa();

        if (![[NSRunningApplication currentApplication] isFinishedLaunching])
        {
            [NSApp run];
        }

        // In case we are unbundled, make us a proper UI application
        if (_grwl.hints.init.ns.menubar)
        {
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        }

        return true;

    } // autoreleasepool
}

void _grwlTerminateCocoa()
{
    @autoreleasepool
    {

        if (_grwl.ns.dockProgressIndicator.view != nil)
        {
            [_grwl.ns.dockProgressIndicator.view removeFromSuperview];
            [_grwl.ns.dockProgressIndicator.view release];
        }

        if (_grwl.ns.keyboardLayout)
        {
            CFRelease(_grwl.ns.keyboardLayout);
            _grwl.ns.keyboardLayout = NULL;
            _grwl.ns.unicodeData = NULL;
        }

        if (_grwl.ns.inputSource)
        {
            CFRelease(_grwl.ns.inputSource);
            _grwl.ns.inputSource = NULL;
        }

        if (_grwl.ns.eventSource)
        {
            CFRelease(_grwl.ns.eventSource);
            _grwl.ns.eventSource = NULL;
        }

        if (_grwl.ns.delegate)
        {
            [NSApp setDelegate:nil];
            [_grwl.ns.delegate release];
            _grwl.ns.delegate = nil;
        }

        if (_grwl.ns.helper)
        {
            [[NSNotificationCenter defaultCenter]
                removeObserver:_grwl.ns.helper
                          name:NSTextInputContextKeyboardSelectionDidChangeNotification
                        object:nil];
            [[NSNotificationCenter defaultCenter] removeObserver:_grwl.ns.helper];
            [_grwl.ns.helper release];
            _grwl.ns.helper = nil;
        }

        if (_grwl.ns.keyUpMonitor)
        {
            [NSEvent removeMonitor:_grwl.ns.keyUpMonitor];
        }

        _grwl_free(_grwl.ns.clipboardString);
        _grwl_free(_grwl.ns.keyboardLayoutName);

        _grwlTerminateNSGL();
        _grwlTerminateEGL();
        _grwlTerminateOSMesa();

    } // autoreleasepool
}

#endif // _GRWL_COCOA
