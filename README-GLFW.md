
# GLFW

[![Build status](https://github.com/glfw/glfw/actions/workflows/build.yml/badge.svg)](https://github.com/glfw/glfw/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)
[![Coverity Scan](https://scan.coverity.com/projects/4884/badge.svg)](https://scan.coverity.com/projects/glfw-glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both X11 and Wayland are supported.

GLFW is licensed under the [zlib/libpng
license](https://www.glfw.org/license.html).

You can [download](https://www.glfw.org/download.html) the latest stable release
as source or Windows binaries, or fetch the `latest` branch from GitHub.  Each
release starting with 3.0 also has a corresponding [annotated
tag](https://github.com/glfw/glfw/releases) with source and binary archives.

The [documentation](https://www.glfw.org/docs/latest/) is available online and is
included in all source and binary archives.  See the [release
notes](https://www.glfw.org/docs/latest/news.html) for new features, caveats and
deprecations in the latest release.  For more details see the [version
history](https://www.glfw.org/changelog.html).

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms, although details of newly added features may
change until they have been included in a release.  New features and many bug
fixes live in [other branches](https://github.com/glfw/glfw/branches/all) until
they are stable enough to merge.

If you are new to GLFW, you may find the
[tutorial](https://www.glfw.org/docs/latest/quick.html) for GLFW 3 useful.  If
you have used GLFW 2 in the past, there is a [transition
guide](https://www.glfw.org/docs/latest/moving.html) for moving to the GLFW
3 API.

GLFW exists because of the contributions of [many people](CONTRIBUTORS.md)
around the world, whether by reporting bugs, providing community support, adding
features, reviewing or testing code, debugging, proofreading docs, suggesting
features or fixing bugs.


## Compiling GLFW

GLFW itself requires only the headers and libraries for your OS and window
system.  It does not need the headers for any context creation API (WGL, GLX,
EGL, NSGL, OSMesa) or rendering API (OpenGL, OpenGL ES, Vulkan) to enable
support for them.

GLFW supports compilation on Windows with Visual C++ 2010 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled Windows binaries](https://www.glfw.org/download.html)
available for all supported compilers.

See the [compilation guide](https://www.glfw.org/docs/latest/compile.html) for
more information about how to compile GLFW yourself.


## Using GLFW

See the [documentation](https://www.glfw.org/docs/latest/) for tutorials, guides
and the API reference.


## Contributing to GLFW

See the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
more information.


## System requirements

GLFW supports Windows XP and later and macOS 10.8 and later.  Linux and other
Unix-like systems running the X Window System are supported even without
a desktop environment or modern extensions, although some features require
a running window or clipboard manager.  The OSMesa backend requires Mesa 6.3.

See the [compatibility guide](https://www.glfw.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

GLFW itself needs only CMake 3.1 or later and the headers and libraries for your
OS and window system.

The documentation is generated with [Doxygen](https://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/glfw/glfw/issues).
Please check the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog


 - Added OpenGL and OpenGL ES user contexts for multiple window contexts via
         `GLFWusercontext`, `glfwCreateUserContext`, `glfwDestroyUserContext`,
         `glfwMakeUserContextCurrent`, `glfwGetCurrentUserContext` (#1687)
 - Added `GRWL_PLATFORM` init hint for runtime platform selection (#1958)
 - Added `GRWL_ANGLE_PLATFORM_TYPE` init hint and `GRWL_ANGLE_PLATFORM_TYPE_*`
   values to select ANGLE backend (#1380)
 - Added `GRWL_ANY_PLATFORM`, `GRWL_PLATFORM_WIN32`, `GRWL_PLATFORM_COCOA`,
   `GRWL_PLATFORM_WAYLAND`, `GRWL_PLATFORM_X11` and `GRWL_PLATFORM_NULL` symbols
   to specify the desired platform (#1958)
 - Added `GRWL_ANY_POSITION` hint value for letting the window manager choose
   (#1603,#1747)
 - Added `GRWL_BUILD_COCOA` CMake option for enabling Cocoa support (#1958)
 - Added `GRWL_BUILD_WIN32` CMake option for enabling Win32 support (#1958)
 - Added `GRWL_BUILD_X11` CMake option for enabling X11 support (#1958)
 - Added `GRWL_CURSOR_CAPTURED` cursor mode to confine the cursor to the window
   content area (#58)
 - Added `GRWL_FEATURE_UNAVAILABLE` error for platform limitations (#1692)
 - Added `GRWL_FEATURE_UNIMPLEMENTED` error for incomplete backends (#1692)
 - Added `GRWL_IME` input mode for `glfwGetInputMode` and `glfwSetInputMode`
   (#2130)
 - Added `GRWL_LIBRARY_TYPE` CMake variable for overriding the library type
   (#279,#1307,#1497,#1574,#1928)
 - Added `GRWL_MANAGE_PREEDIT_CANDIDATE` init hint for displaying preedit
   candidates on the application side (supported only on Windows currently)
   (#2130)
 - Added `GRWL_MOUSE_PASSTHROUGH` window hint for letting mouse input pass
   through the window (#1236,#1568)
 - Added `GRWL_NATIVE_INCLUDE_NONE` for disabling inclusion of native headers
   (#1348)
 - Added `GRWL_PKG_CONFIG_REQUIRES_PRIVATE` and `GRWL_PKG_CONFIG_LIBS_PRIVATE`
   CMake variables exposing pkg-config dependencies (#1307)
 - Added `GRWL_PLATFORM_UNAVAILABLE` error for platform detection failures
   (#1958)
 - Added `GRWL_PLATFORM` init hint for runtime platform selection (#1958)
 - Added `GRWL_POINTING_HAND_CURSOR` alias for `GRWL_HAND_CURSOR` (#427)
 - Added `GRWL_POSITION_X` and `GRWL_POSITION_Y` window hints for initial
   position (#1603,#1747)
 - Added `GRWL_RESIZE_EW_CURSOR` alias for `GRWL_HRESIZE_CURSOR` (#427)
 - Added `GRWL_RESIZE_NS_CURSOR` alias for `GRWL_VRESIZE_CURSOR` (#427)
 - Added `GRWL_RESIZE_NWSE_CURSOR`, `GRWL_RESIZE_NESW_CURSOR`,
   `GRWL_RESIZE_ALL_CURSOR` and `GRWL_NOT_ALLOWED_CURSOR` cursor shapes (#427)
 - Added `GRWL_WAYLAND_APP_ID` window hint string for Wayland app\_id selection
   (#2121,#2122)
 - Added `GRWL_X11_ONTHESPOT` init hint for using on-the-spot input method style
   on X11 (#2130)
 - Added `GRWL_X11_XCB_VULKAN_SURFACE` init hint for selecting X11 Vulkan
   surface extension (#1793)
 - Added `GLFWallocator` struct and `GLFWallocatefun`, `GLFWreallocatefun` and
   `GLFWdeallocatefun` types (#544,#1628,#1947)
 - Added `glfwGetKeyboardLayoutName` for querying the name of the current
   keyboard layout (#1201)
 - Added `glfwGetPlatform` function to query what platform was selected
   (#1655,#1958)
 - Added `glfwGetPreeditCandidate` function to get a preeidt candidate text
   (#2130)
 - Added `glfwGetPreeditCursorRectangle` function to get the preedit cursor area
   (#2130)
 - Added `glfwInitAllocator` for setting a custom memory allocator
   (#544,#1628,#1947)
 - Added `glfwInitVulkanLoader` for using a non-default Vulkan loader
   (#1374,#1890)
 - Added `glfwPlatformSupported` function to query if a platform is supported
   (#1655,#1958)
 - Added `glfwRequestWindowAttention` implementation for Wayland using
   xdg-activation-v1 protocol (#2287)
 - Added `glfwResetPreeditText` function to reset preedit of input method
   (#2130)
 - Added `glfwSetIMEStatusCallback` function and `GLFWimestatusfun` type for
   status of input method (#2130)
 - Added `glfwSetKeyboardLayoutCallback` and `GLFWkeyboardlayoutfun` for
   receiving keyboard layout events (#1201)
 - Added `glfwSetPreeditCallback` function and `GLFWpreeditfun` type for preedit
   of input method (#2130)
 - Added `glfwSetPreeditCandidateCallback` function and
   `GLFWpreeditcandidatefun` type for preedit candidates (#2130)
 - Added `glfwSetPreeditCursorRectangle` function to set the preedit cursor area
   that is used to decide the position of the candidate window of input method
   (#2130)
 - Added `glfwSetWindowBadge` and `glfwSetWindowBadgeString` for displaying
   window application badge (#2248)
 - Added `glfwSetWindowProgressIndicator` for displaying progress on the dock or
   taskbar (#2286,#1183)
 - Bugfix: `glfwGetJoystickUserPointer` returned `NULL` during disconnection
   (#2092)
 - Bugfix: `glfwMakeContextCurrent` would access TLS slot before initialization
 - Bugfix: `glfwSetGammaRamp` could emit `GRWL_INVALID_VALUE` before
   initialization
 - Bugfix: Buffers were swapped at creation on single-buffered windows (#1873)
 - Bugfix: Built-in mappings failed because some OEMs re-used VID/PID (#1583)
 - Bugfix: Compiling with -Wextra-semi caused warnings (#1440)
 - Bugfix: Gamepad mapping updates could spam `GRWL_INVALID_VALUE` due to
   incompatible controllers sharing hardware ID (#1763)
 - Bugfix: Native access functions for context handles did not check that the
   API matched
 - Bugfix: Some extension loader headers did not prevent default OpenGL header
   inclusion (#1695)
 - Bugfix: The CMake config-file package used an absolute path and was not
   relocatable (#1470)
 - Bugfix: Video modes with a duplicate screen area were discarded (#1555,#1556)
 - Disabled tests and examples by default when built as a CMake subdirectory
 - Made `GRWL_DOUBLEBUFFER` a read-only window attribute
 - Made joystick subsystem initialize at first use (#1284,#1646)
 - Removed `GRWL_USE_OSMESA` CMake option enabling the Null platform (#1958)
 - Removed CMake generated configuration header
 - Renamed `GRWL_USE_WAYLAND` CMake option to `GRWL_BUILD_WAYLAND` (#1958)
 - Updated gamepad mappings from upstream
 - Updated the minimum required CMake version to 3.1
 - [Cocoa] Added locating the Vulkan loader at runtime in an application bundle
 - [Cocoa] Added support for `VK_EXT_metal_surface` (#1619)
 - [Cocoa] Bugfix: `GRWL_MAXIMIZED` was always true when `GRWL_RESIZABLE` was
   false
 - [Cocoa] Bugfix: `glfwSetWindowSize` used a bottom-left anchor point (#1553)
 - [Cocoa] Bugfix: `kIOMasterPortDefault` was deprecated in macOS 12.0 (#1980)
 - [Cocoa] Bugfix: `kUTTypeURL` was deprecated in macOS 12.0 (#2003)
 - [Cocoa] Bugfix: A connected Apple AirPlay would emit a useless error (#1791)
 - [Cocoa] Bugfix: A connected joystick, for which input monitoring permissions
   have not been granted, would cause a segfault (#2320).
 - [Cocoa] Bugfix: Changing `GRWL_DECORATED` in macOS fullscreen would abort
   application (#1886)
 - [Cocoa] Bugfix: Duplicate video modes were not filtered out (#1830)
 - [Cocoa] Bugfix: Event processing before window creation would assert (#1543)
 - [Cocoa] Bugfix: Failing to retrieve the refresh rate of built-in displays
   could leak memory
 - [Cocoa] Bugfix: Menu bar was not clickable on macOS 10.15+ until it lost and
   regained focus (#1648,#1802)
 - [Cocoa] Bugfix: Monitor name query could segfault on macOS 11 (#1809,#1833)
 - [Cocoa] Bugfix: Moving the cursor programmatically would freeze it for a
   fraction of a second (#1962)
 - [Cocoa] Bugfix: Non-BMP Unicode codepoint input was reported as UTF-16
   (#1635)
 - [Cocoa] Bugfix: Objective-C files were compiled as C with CMake 3.19 (#1787)
 - [Cocoa] Bugfix: Setting a monitor from macOS fullscreen would abort
   application (#2110)
 - [Cocoa] Bugfix: The EGL and OSMesa libraries were not unloaded on termination
 - [Cocoa] Bugfix: The install name of the installed dylib was relative (#1504)
 - [Cocoa] Bugfix: The MoltenVK layer contents scale was updated only after
   related events were emitted
 - [Cocoa] Bugfix: The Vulkan loader was not loaded from the `Frameworks` bundle
   subdirectory (#2113,#2120)
 - [Cocoa] Bugfix: Touching event queue from secondary thread before main thread
   would abort (#1649)
 - [Cocoa] Bugfix: Undecorated windows could not be iconified on recent macOS
 - [Cocoa] Bugfix: Window remained on screen after destruction until event poll
   (#1412)
 - [Cocoa] Changed `EGLNativeWindowType` from `NSView` to `CALayer` (#1169)
 - [Cocoa] Changed F13 key to report Print Screen for cross-platform consistency
   (#1786)
 - [Cocoa] Disabled macOS fullscreen when `GRWL_RESIZABLE` is false
 - [Cocoa] Moved main menu creation to GLFW initialization time (#1649)
 - [Cocoa] Removed dependency on the CoreVideo framework
 - [EGL] Added ANGLE backend selection via `EGL_ANGLE_platform_angle` extension
   (#1380)
 - [EGL] Added loading of glvnd `libOpenGL.so.0` where available for OpenGL
 - [EGL] Added platform selection via the `EGL_EXT_platform_base` extension
   (#442)
 - [EGL] Bugfix: The `GRWL_DOUBLEBUFFER` context attribute was ignored (#1843)
 - [EGL] Made it possible to query the `EGLConfig` that was chosen to create a
   given window via `glfwGetEGLConfig`
 - [GLX] Added loading of glvnd `libGLX.so.0` where available
 - [GLX] Bugfix: Context creation failed if GLX 1.4 was not exported by GLX
   library
 - [Linux] Bugfix: Joysticks without buttons were ignored (#2042,#2043)
 - [NSGL] Bugfix: `GRWL_COCOA_RETINA_FRAMEBUFFER` had no effect on newer macOS
   versions (#1442)
 - [NSGL] Bugfix: Defining `GL_SILENCE_DEPRECATION` externally caused a
   duplicate definition warning (#1840)
 - [NSGL] Bugfix: Workaround for swap interval on 10.14 broke on 10.12 (#1483)
 - [NSGL] Removed enforcement of forward-compatible flag for core contexts
 - [POSIX] Bugfix: `CLOCK_MONOTONIC` was not correctly tested for or enabled
 - [POSIX] Removed use of deprecated function `gettimeofday`
 - [Wayland] Added dynamic loading of all Wayland libraries
 - [Wayland] Added improved fallback window decorations via libdecor
   (#1639,#1693)
 - [Wayland] Added support for file path drop events (#2040)
 - [Wayland] Added support for key names via xkbcommon
 - [Wayland] Added support for more human-readable monitor names where available
 - [Wayland] Bugfix: `CLOCK_MONOTONIC` was not correctly enabled
 - [Wayland] Bugfix: `GRWL_DECORATED` was ignored when showing a window with XDG
   decorations
 - [Wayland] Bugfix: `GRWL_MAXIMIZED` window hint had no effect
 - [Wayland] Bugfix: `glfwCreateWindow` could emit `GRWL_FEATURE_UNAVAILABLE`
 - [Wayland] Bugfix: `glfwPostEmptyEvent` sometimes had no effect (#1520,#1521)
 - [Wayland] Bugfix: `glfwRestoreWindow` assumed it was always in windowed mode
 - [Wayland] Bugfix: `glfwRestoreWindow` had no effect before first show
 - [Wayland] Bugfix: `glfwRestoreWindow` would make a full screen window
   windowed
 - [Wayland] Bugfix: `glfwSetClipboardString` would fail if set to result of
   `glfwGetClipboardString`
 - [Wayland] Bugfix: `glfwSetWindowAspectRatio` reported an error instead of
   applying the specified ratio
 - [Wayland] Bugfix: `glfwSetWindowMonitor` did not update windowed mode size
 - [Wayland] Bugfix: `glfwSetWindowSize` would resize a full screen window
 - [Wayland] Bugfix: `glfwTerminate` would segfault if any monitor had changed
   scale
 - [Wayland] Bugfix: A key being repeated was not released when window lost
   focus
 - [Wayland] Bugfix: A monitor would be reported as connected again if its scale
   changed
 - [Wayland] Bugfix: A window content scale event would be emitted every time
   the window resized
 - [Wayland] Bugfix: A window leaving full screen mode ignored its desired size
 - [Wayland] Bugfix: A window leaving full screen mode would be iconified
   (#1995)
 - [Wayland] Bugfix: A window maximized or restored by the user would enter an
   inconsistent state
 - [Wayland] Bugfix: Activating a window would emit two input focus events
 - [Wayland] Bugfix: Client-Side Decorations were destroyed in the wrong order
   (#1798)
 - [Wayland] Bugfix: Connecting a mouse after `glfwInit` would segfault (#1450)
 - [Wayland] Bugfix: Data source creation error would cause double free at
   termination
 - [Wayland] Bugfix: Disable key repeat mechanism when window loses input focus
 - [Wayland] Bugfix: Drag and drop data was misinterpreted as clipboard string
 - [Wayland] Bugfix: Fallback decorations emitted `GRWL_CURSOR_UNAVAILABLE`
   errors
 - [Wayland] Bugfix: Full screen window creation did not ignore `GRWL_VISIBLE`
 - [Wayland] Bugfix: Hiding and then showing a window caused program abort on
   wlroots compositors (#1268)
 - [Wayland] Bugfix: If `glfwInit` failed it would close stdin
 - [Wayland] Bugfix: Joysticks connected after `glfwInit` were not detected
   (#2198)
 - [Wayland] Bugfix: Key repeat could lead to a race condition (#1710)
 - [Wayland] Bugfix: Lock key modifier bits were only set when lock keys were
   pressed
 - [Wayland] Bugfix: Manual resizing with fallback decorations behaved
   erratically (#1991,#2115,#2127)
 - [Wayland] Bugfix: MIME type matching was not performed for clipboard string
 - [Wayland] Bugfix: Monitors physical size could report zero (#1784,#1792)
 - [Wayland] Bugfix: Non-arrow cursors are offset from the hotspot (#1706,#1899)
 - [Wayland] Bugfix: Partial writes of clipboard string would cause beginning to
   repeat
 - [Wayland] Bugfix: Repeated keys could be reported with `NULL` window (#1704)
 - [Wayland] Bugfix: Retrieving partial framebuffer size would segfault
 - [Wayland] Bugfix: Scrolling offsets were inverted compared to other platforms
   (#1463)
 - [Wayland] Bugfix: Showing a hidden window did not emit a window refresh event
 - [Wayland] Bugfix: Size limits included frame size for fallback decorations
 - [Wayland] Bugfix: Some errors would cause clipboard string transfer to hang
 - [Wayland] Bugfix: Some keys were not repeating in Wayland (#1908)
 - [Wayland] Bugfix: Some keys were reported as wrong key or `GRWL_KEY_UNKNOWN`
 - [Wayland] Bugfix: Text input did not repeat along with key repeat
 - [Wayland] Bugfix: The `GRWL_HAND_CURSOR` shape used the wrong image (#1432)
 - [Wayland] Bugfix: The `O_CLOEXEC` flag was not defined on FreeBSD
 - [Wayland] Bugfix: The OSMesa library was not unloaded on termination
 - [Wayland] Bugfix: Updating `GRWL_DECORATED` had no effect on server-side
   decorations
 - [Wayland] Bugfix: Window content scale events were not emitted when monitor
   scale changed
 - [Wayland] Bugfix: Window hiding and showing did not work (#1492,#1731)
 - [Wayland] Bugfix: Window maximization events were not emitted
 - [Wayland] Disabled alpha channel for opaque windows on systems lacking
   `EGL_EXT_present_opaque` (#1895)
 - [Wayland] Removed support for `wl_shell` (#1443)
 - [WGL] Disabled the DWM swap interval hack for Windows 8 and later (#1072)
 - [Win32] Added a version info resource to the GLFW DLL
 - [Win32] Added the `GRWL_WIN32_KEYBOARD_MENU` window hint for enabling access
   to the window menu
 - [Win32] Bugfix: `Alt+PrtSc` would emit `GRWL_KEY_UNKNOWN` and a different
   scancode than `PrtSc` (#1993)
 - [Win32] Bugfix: `GRWL_INCLUDE_VULKAN` plus `VK_USE_PLATFORM_WIN32_KHR` caused
   symbol redefinition (#1524)
 - [Win32] Bugfix: `GRWL_KEY_PAUSE` scancode from `glfwGetKeyScancode` did not
   match event scancode (#1993)
 - [Win32] Bugfix: `GRWL_SCALE_TO_MONITOR` had no effect on systems older than
   Windows 10 version 1703 (#1511)
 - [Win32] Bugfix: `glfwGetKeyName` could access out of bounds and return an
   invalid pointer
 - [Win32] Bugfix: `glfwMaximizeWindow` would make a hidden window visible
 - [Win32] Bugfix: `USE_MSVC_RUNTIME_LIBRARY_DLL` had no effect on CMake 3.15 or
   later (#1783,#1796)
 - [Win32] Bugfix: A window created maximized and undecorated would cover the
   whole monitor (#1806)
 - [Win32] Bugfix: Compilation with LLVM for Windows failed (#1807,#1824,#1874)
 - [Win32] Bugfix: Content scale queries could fail silently (#1615)
 - [Win32] Bugfix: Content scales could have garbage values if monitor was
   recently disconnected (#1615)
 - [Win32] Bugfix: Disabled cursor mode interfered with some non-client actions
 - [Win32] Bugfix: Duplicate size events were not filtered (#1610)
 - [Win32] Bugfix: Full screen windows were incorrectly resized by DPI changes
   (#1582)
 - [Win32] Bugfix: GLFW refuses to go fullscreen with custom resolution refresh
   rate (#1904)
 - [Win32] Bugfix: Initialization would segfault on Windows 8 (not 8.1) (#1775)
 - [Win32] Bugfix: Instance-local operations used executable instance
   (#469,#1296,#1395)
 - [Win32] Bugfix: Monitor functions could return invalid values after
   configuration change (#1761)
 - [Win32] Bugfix: Non-BMP Unicode codepoint input was reported as UTF-16
 - [Win32] Bugfix: Right shift emitted `GRWL_KEY_UNKNOWN` when using a CJK IME
   (#2050)
 - [Win32] Bugfix: Some synthetic key events were reported as `GRWL_KEY_UNKNOWN`
   (#1623)
 - [Win32] Bugfix: Super key was not released after Win+V hotkey (#1622)
 - [Win32] Bugfix: The cursor position event was emitted before its cursor enter
   event (#1490)
 - [Win32] Bugfix: The default restored window position was lost when creating a
   maximized window
 - [Win32] Bugfix: The foreground lock timeout was overridden, ignoring the user
 - [Win32] Bugfix: The OSMesa library was not unloaded on termination
 - [Win32] Bugfix: The window hint `GRWL_MAXIMIZED` did not move or resize the
   window (#1499)
 - [Win32] Bugfix: Worker threads are now able to correctly detect if the GLFW
   window has focus by getting the `GRWL_FOCUSED` attribute.
 - [Win32] Disabled framebuffer transparency on Windows 7 when DWM windows are
   opaque (#1512)
 - [Win32] Made hidden helper window use its own window class
 - [Win32] Added support for dark title bar (#2228)
 - [X11] Bugfix: `glfwFocusWindow` could terminate on older WMs or without a WM
 - [X11] Bugfix: `glfwMaximizeWindow` had no effect on hidden windows
 - [X11] Bugfix: `glfwPostEmptyEvent` could be ignored due to race condition
   (#379,#1281,#1285,#2033)
 - [X11] Bugfix: `glfwWaitEvents*` did not continue for joystick events
 - [X11] Bugfix: A handle race condition could cause a `BadWindow` error (#1633)
 - [X11] Bugfix: A malformed response during selection transfer could cause a
   segfault
 - [X11] Bugfix: Any IM started after initialization would not be detected
 - [X11] Bugfix: Changing `GRWL_FLOATING` could leak memory
 - [X11] Bugfix: Changing `GRWL_FLOATING` on a hidden window could silently fail
 - [X11] Bugfix: Clearing `GRWL_FLOATING` on a hidden window caused invalid read
 - [X11] Bugfix: Content scale fallback value could be inconsistent (#1578)
 - [X11] Bugfix: Decorations could not be enabled after window creation (#1566)
 - [X11] Bugfix: Disabled cursor mode was interrupted by indicator windows
 - [X11] Bugfix: Dynamic loading on NetBSD failed due to soname differences
 - [X11] Bugfix: Dynamic loading on OpenBSD failed due to soname differences
 - [X11] Bugfix: Function keys were mapped to `GRWL_KEY_UNKNOWN` for some layout
   combinations (#1598)
 - [X11] Bugfix: Icon pixel format conversion worked only by accident, relying
   on undefined behavior (#1986)
 - [X11] Bugfix: IME input of CJK was broken for "C" locale (#1587,#1636)
 - [X11] Bugfix: Joystick events could lead to busy-waiting (#1872)
 - [X11] Bugfix: Key names were not updated when the keyboard layout changed
   (#1462,#1528)
 - [X11] Bugfix: Keys pressed simultaneously with others were not always
   reported (#1112,#1415,#1472,#1616)
 - [X11] Bugfix: Left shift of int constant relied on undefined behavior (#1951)
 - [X11] Bugfix: Monitor physical dimensions could be reported as zero mm
 - [X11] Bugfix: Querying a disconnected monitor could segfault (#1602)
 - [X11] Bugfix: Some calls would reset Xlib to the default error handler
   (#2108)
 - [X11] Bugfix: Some window attributes were not applied on leaving fullscreen
   (#1863)
 - [X11] Bugfix: Termination would segfault if the IM had been destroyed
 - [X11] Bugfix: The CMake files did not check for the XInput headers (#1480)
 - [X11] Bugfix: The OSMesa libray was not unloaded on termination
 - [X11] Bugfix: Waiting for events would fail if file descriptor was too large
   (#2024)
 - [X11] Bugfix: Window position events were not emitted during resizing (#1613)
 - [X11] Bugfix: XKB path used keysyms instead of physical locations for
   non-printable keys (#1598)
 - [X11] Bugfix: Xlib errors caused by other parts of the application could be
   reported as GLFW errors


## Contact

On [glfw.org](https://www.glfw.org/) you can find the latest version of GLFW, as
well as news, documentation and other information about the project.

If you have questions related to the use of GLFW, we have a
[forum](https://discourse.glfw.org/), and the `#glfw` IRC channel on
[Libera.Chat](https://libera.chat/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/glfw/glfw/issues) on GitHub.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, join us on the forum, GitHub or IRC.

