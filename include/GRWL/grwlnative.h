/*************************************************************************
 * GRWL (formerly from GLFW) - Graphics Windowing Library
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
 * Copyright (c) 2023 Matthew Sitton <matthewsitton@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#ifndef _grwl_native_h_
#define _grwl_native_h_

#ifdef __cplusplus
extern "C"
{
#endif

    /*************************************************************************
     * Doxygen documentation
     *************************************************************************/

    /*! @file grwlnative.h
     *  @brief The header of the native access functions.
     *
     *  This is the header file of the native access functions.  See @ref native for
     *  more information.
     */
    /*! @defgroup native Native access
     *  @brief Functions related to accessing native handles.
     *
     *  **By using the native access functions you assert that you know what you're
     *  doing and how to fix problems caused by using them.  If you don't, you
     *  shouldn't be using them.**
     *
     *  Before the inclusion of @ref grwlnative.h, you may define zero or more
     *  window system API macro and zero or more context creation API macros.
     *
     *  The chosen backends must match those the library was compiled for.  Failure
     *  to do this will cause a link-time error.
     *
     *  The available window API macros are:
     *  * `GRWL_EXPOSE_NATIVE_WIN32`
     *  * `GRWL_EXPOSE_NATIVE_COCOA`
     *  * `GRWL_EXPOSE_NATIVE_X11`
     *  * `GRWL_EXPOSE_NATIVE_WAYLAND`
     *
     *  The available context API macros are:
     *  * `GRWL_EXPOSE_NATIVE_WGL`
     *  * `GRWL_EXPOSE_NATIVE_NSGL`
     *  * `GRWL_EXPOSE_NATIVE_GLX`
     *  * `GRWL_EXPOSE_NATIVE_EGL`
     *  * `GRWL_EXPOSE_NATIVE_OSMESA`
     *
     *  These macros select which of the native access functions that are declared
     *  and which platform-specific headers to include.  It is then up your (by
     *  definition platform-specific) code to handle which of these should be
     *  defined.
     *
     *  If you do not want the platform-specific headers to be included, define
     *  `GRWL_NATIVE_INCLUDE_NONE` before including the @ref grwlnative.h header.
     *
     *  @code
     *  #define GRWL_EXPOSE_NATIVE_WIN32
     *  #define GRWL_EXPOSE_NATIVE_WGL
     *  #define GRWL_NATIVE_INCLUDE_NONE
     *  #include <GRWL/grwlnative.h>
     *  @endcode
     */

    /*************************************************************************
     * System headers and types
     *************************************************************************/

#if !defined(GRWL_NATIVE_INCLUDE_NONE)

    #if defined(GRWL_EXPOSE_NATIVE_WIN32) || defined(GRWL_EXPOSE_NATIVE_WGL)
        /* This is a workaround for the fact that grwl.h needs to export APIENTRY (for
         * example to allow applications to correctly declare a GL_KHR_debug callback)
         * but windows.h assumes no one will define APIENTRY before it does
         */
        #if defined(GRWL_APIENTRY_DEFINED)
            #undef APIENTRY
            #undef GRWL_APIENTRY_DEFINED
        #endif
        #include <windows.h>
    #endif

    #if defined(GRWL_EXPOSE_NATIVE_COCOA) || defined(GRWL_EXPOSE_NATIVE_NSGL)
        #if defined(__OBJC__)
            #import <Cocoa/Cocoa.h>
        #else
            #include <ApplicationServices/ApplicationServices.h>
            #include <objc/objc.h>
        #endif
    #endif

    #if defined(GRWL_EXPOSE_NATIVE_X11) || defined(GRWL_EXPOSE_NATIVE_GLX)
        #include <X11/Xlib.h>
        #include <X11/extensions/Xrandr.h>
    #endif

    #if defined(GRWL_EXPOSE_NATIVE_WAYLAND)
        #include <wayland-client.h>
    #endif

    #if defined(GRWL_EXPOSE_NATIVE_WGL)
            /* WGL is declared by windows.h */
    #endif
    #if defined(GRWL_EXPOSE_NATIVE_NSGL)
            /* NSGL is declared by Cocoa.h */
    #endif
    #if defined(GRWL_EXPOSE_NATIVE_GLX)
        /* This is a workaround for the fact that grwl.h defines GLAPIENTRY because by
         * default it also acts as an OpenGL header
         * However, glx.h will include gl.h, which will define it unconditionally
         */
        #if defined(GRWL_GLAPIENTRY_DEFINED)
            #undef GLAPIENTRY
            #undef GRWL_GLAPIENTRY_DEFINED
        #endif
        #include <GL/glx.h>
    #endif
    #if defined(GRWL_EXPOSE_NATIVE_EGL)
        #include <EGL/egl.h>
    #endif
    #if defined(GRWL_EXPOSE_NATIVE_OSMESA)
        /* This is a workaround for the fact that grwl.h defines GLAPIENTRY because by
         * default it also acts as an OpenGL header
         * However, osmesa.h will include gl.h, which will define it unconditionally
         */
        #if defined(GRWL_GLAPIENTRY_DEFINED)
            #undef GLAPIENTRY
            #undef GRWL_GLAPIENTRY_DEFINED
        #endif
        #include <GL/osmesa.h>
    #endif

#endif /*GRWL_NATIVE_INCLUDE_NONE*/

    /*************************************************************************
     * Functions
     *************************************************************************/

#if defined(GRWL_EXPOSE_NATIVE_WIN32)
    /*! @brief Returns the adapter device name of the specified monitor.
     *
     *  @return The UTF-8 encoded adapter device name (for example `\\.\DISPLAY1`)
     *  of the specified monitor, or `NULL` if an [error](@ref error_handling)
     *  occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI const char* grwlGetWin32Adapter(GRWLmonitor* monitor);

    /*! @brief Returns the display device name of the specified monitor.
     *
     *  @return The UTF-8 encoded display device name (for example
     *  `\\.\DISPLAY1\Monitor0`) of the specified monitor, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI const char* grwlGetWin32Monitor(GRWLmonitor* monitor);

    /*! @brief Returns the `HWND` of the specified window.
     *
     *  @return The `HWND` of the specified window, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @remark The `HDC` associated with the window can be queried with the
     *  [GetDC](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc)
     *  function.
     *  @code
     *  HDC dc = GetDC(grwlGetWin32Window(window));
     *  @endcode
     *  This DC is private and does not need to be released.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI HWND grwlGetWin32Window(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_WGL)
    /*! @brief Returns the `HGLRC` of the specified window.
     *
     *  @return The `HGLRC` of the specified window, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @remark The `HDC` associated with the window can be queried with the
     *  [GetDC](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc)
     *  function.
     *  @code
     *  HDC dc = GetDC(grwlGetWin32Window(window));
     *  @endcode
     *  This DC is private and does not need to be released.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI HGLRC grwlGetWGLContext(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_COCOA)
    /*! @brief Returns the `CGDirectDisplayID` of the specified monitor.
     *
     *  @return The `CGDirectDisplayID` of the specified monitor, or
     *  `kCGNullDirectDisplay` if an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI CGDirectDisplayID grwlGetCocoaMonitor(GRWLmonitor* monitor);

    /*! @brief Returns the `NSWindow` of the specified window.
     *
     *  @return The `NSWindow` of the specified window, or `nil` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI id grwlGetCocoaWindow(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_NSGL)
    /*! @brief Returns the `NSOpenGLContext` of the specified window.
     *
     *  @return The `NSOpenGLContext` of the specified window, or `nil` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI id grwlGetNSGLContext(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_X11)
    /*! @brief Returns the `Display` used by GRWL.
     *
     *  @return The `Display` used by GRWL, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI Display* grwlGetX11Display();

    /*! @brief Returns the `RRCrtc` of the specified monitor.
     *
     *  @return The `RRCrtc` of the specified monitor, or `None` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI RRCrtc grwlGetX11Adapter(GRWLmonitor* monitor);

    /*! @brief Returns the `RROutput` of the specified monitor.
     *
     *  @return The `RROutput` of the specified monitor, or `None` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI RROutput grwlGetX11Monitor(GRWLmonitor* monitor);

    /*! @brief Returns the `Window` of the specified window.
     *
     *  @return The `Window` of the specified window, or `None` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI Window grwlGetX11Window(GRWLwindow* window);

    /*! @brief Sets the current primary selection to the specified string.
     *
     *  @param[in] string A UTF-8 encoded string.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The specified string is copied before this function
     *  returns.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref clipboard
     *  @sa grwlGetX11SelectionString
     *  @sa grwlSetClipboardString
     *
     *  @ingroup native
     */
    GRWLAPI void grwlSetX11SelectionString(const char* string);

    /*! @brief Returns the contents of the current primary selection as a string.
     *
     *  If the selection is empty or if its contents cannot be converted, `NULL`
     *  is returned and a @ref GRWL_FORMAT_UNAVAILABLE error is generated.
     *
     *  @return The contents of the selection as a UTF-8 encoded string, or `NULL`
     *  if an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL. You
     *  should not free it yourself. It is valid until the next call to @ref
     *  grwlGetX11SelectionString or @ref grwlSetX11SelectionString, or until the
     *  library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref clipboard
     *  @sa grwlSetX11SelectionString
     *  @sa grwlGetClipboardString
     *
     *  @ingroup native
     */
    GRWLAPI const char* grwlGetX11SelectionString();
#endif

#if defined(GRWL_EXPOSE_NATIVE_GLX)
    /*! @brief Returns the `GLXContext` of the specified window.
     *
     *  @return The `GLXContext` of the specified window, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI GLXContext grwlGetGLXContext(GRWLwindow* window);

    /*! @brief Returns the `GLXWindow` of the specified window.
     *
     *  @return The `GLXWindow` of the specified window, or `None` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI GLXWindow grwlGetGLXWindow(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_WAYLAND)
    /*! @brief Returns the `struct wl_display*` used by GRWL.
     *
     *  @return The `struct wl_display*` used by GRWL, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI struct wl_display* grwlGetWaylandDisplay();

    /*! @brief Returns the `struct wl_output*` of the specified monitor.
     *
     *  @return The `struct wl_output*` of the specified monitor, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI struct wl_output* grwlGetWaylandMonitor(GRWLmonitor* monitor);

    /*! @brief Returns the main `struct wl_surface*` of the specified window.
     *
     *  @return The main `struct wl_surface*` of the specified window, or `NULL` if
     *  an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI struct wl_surface* grwlGetWaylandWindow(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_EGL)
    /*! @brief Returns the `EGLDisplay` used by GRWL.
     *
     *  @return The `EGLDisplay` used by GRWL, or `EGL_NO_DISPLAY` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @remark Because EGL is initialized on demand, this function will return
     *  `EGL_NO_DISPLAY` until the first context has been created via EGL.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI EGLDisplay grwlGetEGLDisplay();

    /*! @brief Returns the `EGLContext` of the specified window.
     *
     *  @return The `EGLContext` of the specified window, or `EGL_NO_CONTEXT` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI EGLContext grwlGetEGLContext(GRWLwindow* window);

    /*! @brief Returns the `EGLSurface` of the specified window.
     *
     *  @return The `EGLSurface` of the specified window, or `EGL_NO_SURFACE` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI EGLSurface grwlGetEGLSurface(GRWLwindow* window);

    /*! @brief Returns the `EGLConfig` of the specified window.
     *
     *  @return The `EGLConfig` of the specified window, or `EGL_NO_SURFACE` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI EGLConfig grwlGetEGLConfig(GRWLwindow* window);
#endif

#if defined(GRWL_EXPOSE_NATIVE_OSMESA)
    /*! @brief Retrieves the color buffer associated with the specified window.
     *
     *  @param[in] window The window whose color buffer to retrieve.
     *  @param[out] width Where to store the width of the color buffer, or `NULL`.
     *  @param[out] height Where to store the height of the color buffer, or `NULL`.
     *  @param[out] format Where to store the OSMesa pixel format of the color
     *  buffer, or `NULL`.
     *  @param[out] buffer Where to store the address of the color buffer, or
     *  `NULL`.
     *  @return `true` if successful, or `false` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI int grwlGetOSMesaColorBuffer(GRWLwindow* window, int* width, int* height, int* format, void** buffer);

    /*! @brief Retrieves the depth buffer associated with the specified window.
     *
     *  @param[in] window The window whose depth buffer to retrieve.
     *  @param[out] width Where to store the width of the depth buffer, or `NULL`.
     *  @param[out] height Where to store the height of the depth buffer, or `NULL`.
     *  @param[out] bytesPerValue Where to store the number of bytes per depth
     *  buffer element, or `NULL`.
     *  @param[out] buffer Where to store the address of the depth buffer, or
     *  `NULL`.
     *  @return `true` if successful, or `false` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI int grwlGetOSMesaDepthBuffer(GRWLwindow* window, int* width, int* height, int* bytesPerValue,
                                         void** buffer);

    /*! @brief Returns the `OSMesaContext` of the specified window.
     *
     *  @return The `OSMesaContext` of the specified window, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NO_WINDOW_CONTEXT and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @ingroup native
     */
    GRWLAPI OSMesaContext grwlGetOSMesaContext(GRWLwindow* window);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _grwl_native_h_ */
