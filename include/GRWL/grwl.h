/*************************************************************************
 * GRWL (formerly from GLFW) - Graphics Windowing Library
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
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

#ifndef _grwl_h_
#define _grwl_h_

#ifdef __cplusplus
extern "C"
{
#endif

/*************************************************************************
 * Doxygen documentation
 *************************************************************************/

/*! @file grwl.h
 *  @brief The header of the GRWL 3 API.
 *
 *  This is the header file of the GRWL 3 API.  It defines all its types and
 *  declares all its functions.
 *
 *  For more information about how to use this file, see @ref build_include.
 */
/*! @defgroup context Context reference
 *  @brief Functions and types related to OpenGL and OpenGL ES contexts.
 *
 *  This is the reference documentation for OpenGL and OpenGL ES context related
 *  functions.  For more task-oriented information, see the @ref context_guide.
 */
/*! @defgroup vulkan Vulkan support reference
 *  @brief Functions and types related to Vulkan.
 *
 *  This is the reference documentation for Vulkan related functions and types.
 *  For more task-oriented information, see the @ref vulkan_guide.
 */
/*! @defgroup init Initialization, version and error reference
 *  @brief Functions and types related to initialization and error handling.
 *
 *  This is the reference documentation for initialization and termination of
 *  the library, version management and error handling.  For more task-oriented
 *  information, see the @ref intro_guide.
 */
/*! @defgroup input Input reference
 *  @brief Functions and types related to input handling.
 *
 *  This is the reference documentation for input related functions and types.
 *  For more task-oriented information, see the @ref input_guide.
 */
/*! @defgroup monitor Monitor reference
 *  @brief Functions and types related to monitors.
 *
 *  This is the reference documentation for monitor related functions and types.
 *  For more task-oriented information, see the @ref monitor_guide.
 */
/*! @defgroup window Window reference
 *  @brief Functions and types related to windows.
 *
 *  This is the reference documentation for window related functions and types,
 *  including creation, deletion and event polling.  For more task-oriented
 *  information, see the @ref window_guide.
 */

/*************************************************************************
 * Compiler- and platform-specific preprocessor work
 *************************************************************************/

/* If we are we on Windows, we want a single define for it.
 */
#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
    #define _WIN32
#endif /* _WIN32 */

/* Include because most Windows GLU headers need wchar_t and
 * the macOS OpenGL header blocks the definition of ptrdiff_t by glext.h.
 * Include it unconditionally to avoid surprising side-effects.
 */
#include <stddef.h>

/* Include because it is needed by Vulkan and related functions.
 * Include it unconditionally to avoid surprising side-effects.
 */
#include <stdint.h>

#if defined(GRWL_INCLUDE_VULKAN)
    #include <vulkan/vulkan.h>
#endif /* Vulkan header */

/* The Vulkan header may have indirectly included windows.h (because of
 * VK_USE_PLATFORM_WIN32_KHR) so we offer our replacement symbols after it.
 */

/* It is customary to use APIENTRY for OpenGL function pointer declarations on
 * all platforms.  Additionally, the Windows OpenGL header needs APIENTRY.
 */
#if !defined(APIENTRY)
    #if defined(_WIN32)
        #define APIENTRY __stdcall
    #else
        #define APIENTRY
    #endif
    #define GRWL_APIENTRY_DEFINED
#endif /* APIENTRY */

/* Some Windows OpenGL headers need this.
 */
#if !defined(WINGDIAPI) && defined(_WIN32)
    #define WINGDIAPI __declspec(dllimport)
    #define GRWL_WINGDIAPI_DEFINED
#endif /* WINGDIAPI */

/* Some Windows GLU headers need this.
 */
#if !defined(CALLBACK) && defined(_WIN32)
    #define CALLBACK __stdcall
    #define GRWL_CALLBACK_DEFINED
#endif /* CALLBACK */

/* Include the chosen OpenGL or OpenGL ES headers.
 */
#if defined(GRWL_INCLUDE_ES1)

    #include <GLES/gl.h>
    #if defined(GRWL_INCLUDE_GLEXT)
        #include <GLES/glext.h>
    #endif

#elif defined(GRWL_INCLUDE_ES2)

    #include <GLES2/gl2.h>
    #if defined(GRWL_INCLUDE_GLEXT)
        #include <GLES2/gl2ext.h>
    #endif

#elif defined(GRWL_INCLUDE_ES3)

    #include <GLES3/gl3.h>
    #if defined(GRWL_INCLUDE_GLEXT)
        #include <GLES2/gl2ext.h>
    #endif

#elif defined(GRWL_INCLUDE_ES31)

    #include <GLES3/gl31.h>
    #if defined(GRWL_INCLUDE_GLEXT)
        #include <GLES2/gl2ext.h>
    #endif

#elif defined(GRWL_INCLUDE_ES32)

    #include <GLES3/gl32.h>
    #if defined(GRWL_INCLUDE_GLEXT)
        #include <GLES2/gl2ext.h>
    #endif

#elif defined(GRWL_INCLUDE_GLCOREARB)

    #if defined(__APPLE__)

        #include <OpenGL/gl3.h>
        #if defined(GRWL_INCLUDE_GLEXT)
            #include <OpenGL/gl3ext.h>
        #endif /*GRWL_INCLUDE_GLEXT*/

    #else /*__APPLE__*/

        #include <GL/glcorearb.h>
        #if defined(GRWL_INCLUDE_GLEXT)
            #include <GL/glext.h>
        #endif

    #endif /*__APPLE__*/

#elif defined(GRWL_INCLUDE_GLU)

    #if defined(__APPLE__)

        #if defined(GRWL_INCLUDE_GLU)
            #include <OpenGL/glu.h>
        #endif

    #else /*__APPLE__*/

        #if defined(GRWL_INCLUDE_GLU)
            #include <GL/glu.h>
        #endif

    #endif /*__APPLE__*/

#elif !defined(GRWL_INCLUDE_NONE) && !defined(__gl_h_) && !defined(__gles1_gl_h_) && !defined(__gles2_gl2_h_) && \
    !defined(__gles2_gl3_h_) && !defined(__gles2_gl31_h_) && !defined(__gles2_gl32_h_) &&                        \
    !defined(__gl_glcorearb_h_) && !defined(__gl2_h_) /*legacy*/ && !defined(__gl3_h_) /*legacy*/ &&             \
    !defined(__gl31_h_) /*legacy*/ && !defined(__gl32_h_) /*legacy*/ && !defined(__glcorearb_h_) /*legacy*/ &&   \
    !defined(__GL_H__) /*non-standard*/ && !defined(__gltypes_h_) /*non-standard*/ &&                            \
    !defined(__glee_h_) /*non-standard*/

    #if defined(__APPLE__)

        #if !defined(GRWL_INCLUDE_GLEXT)
            #define GL_GLEXT_LEGACY
        #endif
        #include <OpenGL/gl.h>

    #else /*__APPLE__*/

        #include <GL/gl.h>
        #if defined(GRWL_INCLUDE_GLEXT)
            #include <GL/glext.h>
        #endif

    #endif /*__APPLE__*/

#endif /* OpenGL and OpenGL ES headers */

#if defined(GRWL_INCLUDE_WEBGPU)
    #include <webgpu/webgpu.h>
#endif /* webgpu header */

#if defined(GRWL_DLL) && defined(_GRWL_BUILD_DLL)
    /* GRWL_DLL must be defined by applications that are linking against the DLL
     * version of the GRWL library.  _GRWL_BUILD_DLL is defined by the GRWL
     * configuration header when compiling the DLL version of the library.
     */
    #error "You must not have both GRWL_DLL and _GRWL_BUILD_DLL defined"
#endif

/* GRWLAPI is used to declare public API functions for export
 * from the DLL / shared library / dynamic library.
 */
#if defined(_WIN32) && defined(_GRWL_BUILD_DLL)
    /* We are building GRWL as a Win32 DLL */
    #define GRWLAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(GRWL_DLL)
    /* We are calling a GRWL Win32 DLL */
    #define GRWLAPI __declspec(dllimport)
#elif defined(__GNUC__) && defined(_GRWL_BUILD_DLL)
    /* We are building GRWL as a Unix shared library */
    #define GRWLAPI __attribute__((visibility("default")))
#else
    #define GRWLAPI
#endif

/*************************************************************************
 * GRWL API tokens
 *************************************************************************/

/*! @name GRWL version macros
 *  @{ */
/*! @brief The major version number of the GRWL header.
 *
 *  The major version number of the GRWL header.  This is incremented when the
 *  API is changed in non-compatible ways.
 *  @ingroup init
 */
#define GRWL_VERSION_MAJOR 0
/*! @brief The minor version number of the GRWL header.
 *
 *  The minor version number of the GRWL header.  This is incremented when
 *  features are added to the API but it remains backward-compatible.
 *  @ingroup init
 */
#define GRWL_VERSION_MINOR 1
/*! @brief The revision number of the GRWL header.
 *
 *  The revision number of the GRWL header.  This is incremented when a bug fix
 *  release is made that does not contain any API changes.
 *  @ingroup init
 */
#define GRWL_VERSION_REVISION 0
/*! @} */

/*! @name Key and button actions
 *  @{ */
/*! @brief The key or mouse button was released.
 *
 *  The key or mouse button was released.
 *
 *  @ingroup input
 */
#define GRWL_RELEASE 0
/*! @brief The key or mouse button was pressed.
 *
 *  The key or mouse button was pressed.
 *
 *  @ingroup input
 */
#define GRWL_PRESS 1
/*! @brief The key was held down until it repeated.
 *
 *  The key was held down until it repeated.
 *
 *  @ingroup input
 */
#define GRWL_REPEAT 2
/*! @} */

/*! @defgroup hat_state Joystick hat states
 *  @brief Joystick hat states.
 *
 *  See [joystick hat input](@ref joystick_hat) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define GRWL_HAT_CENTERED 0
#define GRWL_HAT_UP 1
#define GRWL_HAT_RIGHT 2
#define GRWL_HAT_DOWN 4
#define GRWL_HAT_LEFT 8
#define GRWL_HAT_RIGHT_UP (GRWL_HAT_RIGHT | GRWL_HAT_UP)
#define GRWL_HAT_RIGHT_DOWN (GRWL_HAT_RIGHT | GRWL_HAT_DOWN)
#define GRWL_HAT_LEFT_UP (GRWL_HAT_LEFT | GRWL_HAT_UP)
#define GRWL_HAT_LEFT_DOWN (GRWL_HAT_LEFT | GRWL_HAT_DOWN)
/*! @} */

/*! @defgroup keys Keyboard keys
 *  @brief Keyboard key IDs.
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  These key codes are inspired by the _USB HID Usage Tables v1.12_ (p. 53-60),
 *  but re-arranged to map to 7-bit ASCII for printable keys (function keys are
 *  put in the 256+ range).
 *
 *  The naming of the key codes follow these rules:
 *   - The US keyboard layout is used
 *   - Names of printable alphanumeric characters are used (e.g. "A", "R",
 *     "3", etc.)
 *   - For non-alphanumeric characters, Unicode:ish names are used (e.g.
 *     "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
 *     correspond to the Unicode standard (usually for brevity)
 *   - Keys that lack a clear US mapping are named "WORLD_x"
 *   - For non-printable keys, custom names are used (e.g. "F4",
 *     "BACKSPACE", etc.)
 *
 *  @ingroup input
 *  @{
 */

/* The unknown key */
#define GRWL_KEY_UNKNOWN -1

/* Printable keys */
#define GRWL_KEY_SPACE 32
#define GRWL_KEY_APOSTROPHE 39 /* ' */
#define GRWL_KEY_COMMA 44      /* , */
#define GRWL_KEY_MINUS 45      /* - */
#define GRWL_KEY_PERIOD 46     /* . */
#define GRWL_KEY_SLASH 47      /* / */
#define GRWL_KEY_0 48
#define GRWL_KEY_1 49
#define GRWL_KEY_2 50
#define GRWL_KEY_3 51
#define GRWL_KEY_4 52
#define GRWL_KEY_5 53
#define GRWL_KEY_6 54
#define GRWL_KEY_7 55
#define GRWL_KEY_8 56
#define GRWL_KEY_9 57
#define GRWL_KEY_SEMICOLON 59 /* ; */
#define GRWL_KEY_EQUAL 61     /* = */
#define GRWL_KEY_A 65
#define GRWL_KEY_B 66
#define GRWL_KEY_C 67
#define GRWL_KEY_D 68
#define GRWL_KEY_E 69
#define GRWL_KEY_F 70
#define GRWL_KEY_G 71
#define GRWL_KEY_H 72
#define GRWL_KEY_I 73
#define GRWL_KEY_J 74
#define GRWL_KEY_K 75
#define GRWL_KEY_L 76
#define GRWL_KEY_M 77
#define GRWL_KEY_N 78
#define GRWL_KEY_O 79
#define GRWL_KEY_P 80
#define GRWL_KEY_Q 81
#define GRWL_KEY_R 82
#define GRWL_KEY_S 83
#define GRWL_KEY_T 84
#define GRWL_KEY_U 85
#define GRWL_KEY_V 86
#define GRWL_KEY_W 87
#define GRWL_KEY_X 88
#define GRWL_KEY_Y 89
#define GRWL_KEY_Z 90
#define GRWL_KEY_LEFT_BRACKET 91  /* [ */
#define GRWL_KEY_BACKSLASH 92     /* \ */
#define GRWL_KEY_RIGHT_BRACKET 93 /* ] */
#define GRWL_KEY_GRAVE_ACCENT 96  /* ` */
#define GRWL_KEY_WORLD_1 161      /* non-US #1 */
#define GRWL_KEY_WORLD_2 162      /* non-US #2 */

/* Function keys */
#define GRWL_KEY_ESCAPE 256
#define GRWL_KEY_ENTER 257
#define GRWL_KEY_TAB 258
#define GRWL_KEY_BACKSPACE 259
#define GRWL_KEY_INSERT 260
#define GRWL_KEY_DELETE 261
#define GRWL_KEY_RIGHT 262
#define GRWL_KEY_LEFT 263
#define GRWL_KEY_DOWN 264
#define GRWL_KEY_UP 265
#define GRWL_KEY_PAGE_UP 266
#define GRWL_KEY_PAGE_DOWN 267
#define GRWL_KEY_HOME 268
#define GRWL_KEY_END 269
#define GRWL_KEY_CAPS_LOCK 280
#define GRWL_KEY_SCROLL_LOCK 281
#define GRWL_KEY_NUM_LOCK 282
#define GRWL_KEY_PRINT_SCREEN 283
#define GRWL_KEY_PAUSE 284
#define GRWL_KEY_F1 290
#define GRWL_KEY_F2 291
#define GRWL_KEY_F3 292
#define GRWL_KEY_F4 293
#define GRWL_KEY_F5 294
#define GRWL_KEY_F6 295
#define GRWL_KEY_F7 296
#define GRWL_KEY_F8 297
#define GRWL_KEY_F9 298
#define GRWL_KEY_F10 299
#define GRWL_KEY_F11 300
#define GRWL_KEY_F12 301
#define GRWL_KEY_F13 302
#define GRWL_KEY_F14 303
#define GRWL_KEY_F15 304
#define GRWL_KEY_F16 305
#define GRWL_KEY_F17 306
#define GRWL_KEY_F18 307
#define GRWL_KEY_F19 308
#define GRWL_KEY_F20 309
#define GRWL_KEY_F21 310
#define GRWL_KEY_F22 311
#define GRWL_KEY_F23 312
#define GRWL_KEY_F24 313
#define GRWL_KEY_F25 314
#define GRWL_KEY_KP_0 320
#define GRWL_KEY_KP_1 321
#define GRWL_KEY_KP_2 322
#define GRWL_KEY_KP_3 323
#define GRWL_KEY_KP_4 324
#define GRWL_KEY_KP_5 325
#define GRWL_KEY_KP_6 326
#define GRWL_KEY_KP_7 327
#define GRWL_KEY_KP_8 328
#define GRWL_KEY_KP_9 329
#define GRWL_KEY_KP_DECIMAL 330
#define GRWL_KEY_KP_DIVIDE 331
#define GRWL_KEY_KP_MULTIPLY 332
#define GRWL_KEY_KP_SUBTRACT 333
#define GRWL_KEY_KP_ADD 334
#define GRWL_KEY_KP_ENTER 335
#define GRWL_KEY_KP_EQUAL 336
#define GRWL_KEY_LEFT_SHIFT 340
#define GRWL_KEY_LEFT_CONTROL 341
#define GRWL_KEY_LEFT_ALT 342
#define GRWL_KEY_LEFT_SUPER 343
#define GRWL_KEY_RIGHT_SHIFT 344
#define GRWL_KEY_RIGHT_CONTROL 345
#define GRWL_KEY_RIGHT_ALT 346
#define GRWL_KEY_RIGHT_SUPER 347
#define GRWL_KEY_MENU 348

#define GRWL_KEY_LAST GRWL_KEY_MENU

/*! @} */

/*! @defgroup mods Modifier key flags
 *  @brief Modifier key flags.
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  @ingroup input
 *  @{ */

/*! @brief If this bit is set one or more Shift keys were held down.
 *
 *  If this bit is set one or more Shift keys were held down.
 */
#define GRWL_MOD_SHIFT 0x0001
/*! @brief If this bit is set one or more Control keys were held down.
 *
 *  If this bit is set one or more Control keys were held down.
 */
#define GRWL_MOD_CONTROL 0x0002
/*! @brief If this bit is set one or more Alt keys were held down.
 *
 *  If this bit is set one or more Alt keys were held down.
 */
#define GRWL_MOD_ALT 0x0004
/*! @brief If this bit is set one or more Super keys were held down.
 *
 *  If this bit is set one or more Super keys were held down.
 */
#define GRWL_MOD_SUPER 0x0008
/*! @brief If this bit is set the Caps Lock key is enabled.
 *
 *  If this bit is set the Caps Lock key is enabled and the @ref
 *  GRWL_LOCK_KEY_MODS input mode is set.
 */
#define GRWL_MOD_CAPS_LOCK 0x0010
/*! @brief If this bit is set the Num Lock key is enabled.
 *
 *  If this bit is set the Num Lock key is enabled and the @ref
 *  GRWL_LOCK_KEY_MODS input mode is set.
 */
#define GRWL_MOD_NUM_LOCK 0x0020

/*! @} */

/*! @defgroup buttons Mouse buttons
 *  @brief Mouse button IDs.
 *
 *  See [mouse button input](@ref input_mouse_button) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define GRWL_MOUSE_BUTTON_1 0
#define GRWL_MOUSE_BUTTON_2 1
#define GRWL_MOUSE_BUTTON_3 2
#define GRWL_MOUSE_BUTTON_4 3
#define GRWL_MOUSE_BUTTON_5 4
#define GRWL_MOUSE_BUTTON_6 5
#define GRWL_MOUSE_BUTTON_7 6
#define GRWL_MOUSE_BUTTON_8 7
#define GRWL_MOUSE_BUTTON_LAST GRWL_MOUSE_BUTTON_8
#define GRWL_MOUSE_BUTTON_LEFT GRWL_MOUSE_BUTTON_1
#define GRWL_MOUSE_BUTTON_RIGHT GRWL_MOUSE_BUTTON_2
#define GRWL_MOUSE_BUTTON_MIDDLE GRWL_MOUSE_BUTTON_3
/*! @} */

/*! @defgroup joysticks Joysticks
 *  @brief Joystick IDs.
 *
 *  See [joystick input](@ref joystick) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define GRWL_JOYSTICK_1 0
#define GRWL_JOYSTICK_2 1
#define GRWL_JOYSTICK_3 2
#define GRWL_JOYSTICK_4 3
#define GRWL_JOYSTICK_5 4
#define GRWL_JOYSTICK_6 5
#define GRWL_JOYSTICK_7 6
#define GRWL_JOYSTICK_8 7
#define GRWL_JOYSTICK_9 8
#define GRWL_JOYSTICK_10 9
#define GRWL_JOYSTICK_11 10
#define GRWL_JOYSTICK_12 11
#define GRWL_JOYSTICK_13 12
#define GRWL_JOYSTICK_14 13
#define GRWL_JOYSTICK_15 14
#define GRWL_JOYSTICK_16 15
#define GRWL_JOYSTICK_LAST GRWL_JOYSTICK_16
/*! @} */

/*! @defgroup gamepad_buttons Gamepad buttons
 *  @brief Gamepad buttons.
 *
 *  See @ref gamepad for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define GRWL_GAMEPAD_BUTTON_A 0
#define GRWL_GAMEPAD_BUTTON_B 1
#define GRWL_GAMEPAD_BUTTON_X 2
#define GRWL_GAMEPAD_BUTTON_Y 3
#define GRWL_GAMEPAD_BUTTON_LEFT_BUMPER 4
#define GRWL_GAMEPAD_BUTTON_RIGHT_BUMPER 5
#define GRWL_GAMEPAD_BUTTON_BACK 6
#define GRWL_GAMEPAD_BUTTON_START 7
#define GRWL_GAMEPAD_BUTTON_GUIDE 8
#define GRWL_GAMEPAD_BUTTON_LEFT_THUMB 9
#define GRWL_GAMEPAD_BUTTON_RIGHT_THUMB 10
#define GRWL_GAMEPAD_BUTTON_DPAD_UP 11
#define GRWL_GAMEPAD_BUTTON_DPAD_RIGHT 12
#define GRWL_GAMEPAD_BUTTON_DPAD_DOWN 13
#define GRWL_GAMEPAD_BUTTON_DPAD_LEFT 14
#define GRWL_GAMEPAD_BUTTON_LAST GRWL_GAMEPAD_BUTTON_DPAD_LEFT

#define GRWL_GAMEPAD_BUTTON_CROSS GRWL_GAMEPAD_BUTTON_A
#define GRWL_GAMEPAD_BUTTON_CIRCLE GRWL_GAMEPAD_BUTTON_B
#define GRWL_GAMEPAD_BUTTON_SQUARE GRWL_GAMEPAD_BUTTON_X
#define GRWL_GAMEPAD_BUTTON_TRIANGLE GRWL_GAMEPAD_BUTTON_Y
/*! @} */

/*! @defgroup gamepad_axes Gamepad axes
 *  @brief Gamepad axes.
 *
 *  See @ref gamepad for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define GRWL_GAMEPAD_AXIS_LEFT_X 0
#define GRWL_GAMEPAD_AXIS_LEFT_Y 1
#define GRWL_GAMEPAD_AXIS_RIGHT_X 2
#define GRWL_GAMEPAD_AXIS_RIGHT_Y 3
#define GRWL_GAMEPAD_AXIS_LEFT_TRIGGER 4
#define GRWL_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define GRWL_GAMEPAD_AXIS_LAST GRWL_GAMEPAD_AXIS_RIGHT_TRIGGER
/*! @} */

/*! @defgroup errors Error codes
 *  @brief Error codes.
 *
 *  See [error handling](@ref error_handling) for how these are used.
 *
 *  @ingroup init
 *  @{ */
/*! @brief No error has occurred.
 *
 *  No error has occurred.
 *
 *  @analysis Yay.
 */
#define GRWL_NO_ERROR 0
/*! @brief GRWL has not been initialized.
 *
 *  This occurs if a GRWL function was called that must not be called unless the
 *  library is [initialized](@ref intro_init).
 *
 *  @analysis Application programmer error.  Initialize GRWL before calling any
 *  function that requires initialization.
 */
#define GRWL_NOT_INITIALIZED 0x00010001
/*! @brief No context is current for this thread.
 *
 *  This occurs if a GRWL function was called that needs and operates on the
 *  current OpenGL or OpenGL ES context but no context is current on the calling
 *  thread.  One such function is @ref grwlSwapInterval.
 *
 *  @analysis Application programmer error.  Ensure a context is current before
 *  calling functions that require a current context.
 */
#define GRWL_NO_CURRENT_CONTEXT 0x00010002
/*! @brief One of the arguments to the function was an invalid enum value.
 *
 *  One of the arguments to the function was an invalid enum value, for example
 *  requesting @ref GRWL_RED_BITS with @ref grwlGetWindowAttrib.
 *
 *  @analysis Application programmer error.  Fix the offending call.
 */
#define GRWL_INVALID_ENUM 0x00010003
/*! @brief One of the arguments to the function was an invalid value.
 *
 *  One of the arguments to the function was an invalid value, for example
 *  requesting a non-existent OpenGL or OpenGL ES version like 2.7.
 *
 *  Requesting a valid but unavailable OpenGL or OpenGL ES version will instead
 *  result in a @ref GRWL_VERSION_UNAVAILABLE error.
 *
 *  @analysis Application programmer error.  Fix the offending call.
 */
#define GRWL_INVALID_VALUE 0x00010004
/*! @brief A memory allocation failed.
 *
 *  A memory allocation failed.
 *
 *  @analysis A bug in GRWL or the underlying operating system.  Report the bug
 *  to our [issue tracker](https://github.com/grwl/grwl/issues).
 */
#define GRWL_OUT_OF_MEMORY 0x00010005
/*! @brief GRWL could not find support for the requested API on the system.
 *
 *  GRWL could not find support for the requested API on the system.
 *
 *  @analysis The installed graphics driver does not support the requested
 *  API, or does not support it via the chosen context creation API.
 *  Below are a few examples.
 *
 *  @par
 *  Some pre-installed Windows graphics drivers do not support OpenGL.  AMD only
 *  supports OpenGL ES via EGL, while Nvidia and Intel only support it via
 *  a WGL or GLX extension.  macOS does not provide OpenGL ES at all.  The Mesa
 *  EGL, OpenGL and OpenGL ES libraries do not interface with the Nvidia binary
 *  driver.  Older graphics drivers do not support Vulkan.
 */
#define GRWL_API_UNAVAILABLE 0x00010006
/*! @brief The requested OpenGL or OpenGL ES version is not available.
 *
 *  The requested OpenGL or OpenGL ES version (including any requested context
 *  or framebuffer hints) is not available on this machine.
 *
 *  @analysis The machine does not support your requirements.  If your
 *  application is sufficiently flexible, downgrade your requirements and try
 *  again.  Otherwise, inform the user that their machine does not match your
 *  requirements.
 *
 *  @par
 *  Future invalid OpenGL and OpenGL ES versions, for example OpenGL 4.8 if 5.0
 *  comes out before the 4.x series gets that far, also fail with this error and
 *  not @ref GRWL_INVALID_VALUE, because GRWL cannot know what future versions
 *  will exist.
 */
#define GRWL_VERSION_UNAVAILABLE 0x00010007
/*! @brief A platform-specific error occurred that does not match any of the
 *  more specific categories.
 *
 *  A platform-specific error occurred that does not match any of the more
 *  specific categories.
 *
 *  @analysis A bug or configuration error in GRWL, the underlying operating
 *  system or its drivers, or a lack of required resources.  Report the issue to
 *  our [issue tracker](https://github.com/grwl/grwl/issues).
 */
#define GRWL_PLATFORM_ERROR 0x00010008
/*! @brief The requested format is not supported or available.
 *
 *  If emitted during window creation, the requested pixel format is not
 *  supported.
 *
 *  If emitted when querying the clipboard, the contents of the clipboard could
 *  not be converted to the requested format.
 *
 *  @analysis If emitted during window creation, one or more
 *  [hard constraints](@ref window_hints_hard) did not match any of the
 *  available pixel formats.  If your application is sufficiently flexible,
 *  downgrade your requirements and try again.  Otherwise, inform the user that
 *  their machine does not match your requirements.
 *
 *  @par
 *  If emitted when querying the clipboard, ignore the error or report it to
 *  the user, as appropriate.
 */
#define GRWL_FORMAT_UNAVAILABLE 0x00010009
/*! @brief The specified window does not have an OpenGL or OpenGL ES context.
 *
 *  A window that does not have an OpenGL or OpenGL ES context was passed to
 *  a function that requires it to have one.
 *
 *  @analysis Application programmer error.  Fix the offending call.
 */
#define GRWL_NO_WINDOW_CONTEXT 0x0001000A
/*! @brief The specified cursor shape is not available.
 *
 *  The specified standard cursor shape is not available, either because the
 *  current platform cursor theme does not provide it or because it is not
 *  available on the platform.
 *
 *  @analysis Platform or system settings limitation.  Pick another
 *  [standard cursor shape](@ref shapes) or create a
 *  [custom cursor](@ref cursor_custom).
 */
#define GRWL_CURSOR_UNAVAILABLE 0x0001000B
/*! @brief The requested feature is not provided by the platform.
 *
 *  The requested feature is not provided by the platform, so GRWL is unable to
 *  implement it.  The documentation for each function notes if it could emit
 *  this error.
 *
 *  @analysis Platform or platform version limitation.  The error can be ignored
 *  unless the feature is critical to the application.
 *
 *  @par
 *  A function call that emits this error has no effect other than the error and
 *  updating any existing out parameters.
 */
#define GRWL_FEATURE_UNAVAILABLE 0x0001000C
/*! @brief The requested feature is not implemented for the platform.
 *
 *  The requested feature has not yet been implemented in GRWL for this platform.
 *
 *  @analysis An incomplete implementation of GRWL for this platform, hopefully
 *  fixed in a future release.  The error can be ignored unless the feature is
 *  critical to the application.
 *
 *  @par
 *  A function call that emits this error has no effect other than the error and
 *  updating any existing out parameters.
 */
#define GRWL_FEATURE_UNIMPLEMENTED 0x0001000D
/*! @brief Platform unavailable or no matching platform was found.
 *
 *  If emitted during initialization, no matching platform was found.  If @ref
 *  GRWL_PLATFORM is set to `GRWL_ANY_PLATFORM`, GRWL could not detect any of the
 *  platforms supported by this library binary, except for the Null platform.  If set to
 *  a specific platform, it is either not supported by this library binary or GRWL was not
 *  able to detect it.
 *
 *  If emitted by a native access function, GRWL was initialized for a different platform
 *  than the function is for.
 *
 *  @analysis Failure to detect any platform usually only happens on non-macOS Unix
 *  systems, either when no window system is running or the program was run from
 *  a terminal that does not have the necessary environment variables.  Fall back to
 *  a different platform if possible or notify the user that no usable platform was
 *  detected.
 *
 *  Failure to detect a specific platform may have the same cause as above or be because
 *  support for that platform was not compiled in.  Call @ref grwlPlatformSupported to
 *  check whether a specific platform is supported by a library binary.
 */
#define GRWL_PLATFORM_UNAVAILABLE 0x0001000E
/*! @} */

/*! @addtogroup window
 *  @{ */
/*! @brief Input focus window hint and attribute
 *
 *  Input focus [window hint](@ref GRWL_FOCUSED_hint) or
 *  [window attribute](@ref GRWL_FOCUSED_attrib).
 */
#define GRWL_FOCUSED 0x00020001
/*! @brief Window iconification window attribute
 *
 *  Window iconification [window attribute](@ref GRWL_ICONIFIED_attrib).
 */
#define GRWL_ICONIFIED 0x00020002
/*! @brief Window resize-ability window hint and attribute
 *
 *  Window resize-ability [window hint](@ref GRWL_RESIZABLE_hint) and
 *  [window attribute](@ref GRWL_RESIZABLE_attrib).
 */
#define GRWL_RESIZABLE 0x00020003
/*! @brief Window visibility window hint and attribute
 *
 *  Window visibility [window hint](@ref GRWL_VISIBLE_hint) and
 *  [window attribute](@ref GRWL_VISIBLE_attrib).
 */
#define GRWL_VISIBLE 0x00020004
/*! @brief Window decoration window hint and attribute
 *
 *  Window decoration [window hint](@ref GRWL_DECORATED_hint) and
 *  [window attribute](@ref GRWL_DECORATED_attrib).
 */
#define GRWL_DECORATED 0x00020005
/*! @brief Window auto-iconification window hint and attribute
 *
 *  Window auto-iconification [window hint](@ref GRWL_AUTO_ICONIFY_hint) and
 *  [window attribute](@ref GRWL_AUTO_ICONIFY_attrib).
 */
#define GRWL_AUTO_ICONIFY 0x00020006
/*! @brief Window decoration window hint and attribute
 *
 *  Window decoration [window hint](@ref GRWL_FLOATING_hint) and
 *  [window attribute](@ref GRWL_FLOATING_attrib).
 */
#define GRWL_FLOATING 0x00020007
/*! @brief Window maximization window hint and attribute
 *
 *  Window maximization [window hint](@ref GRWL_MAXIMIZED_hint) and
 *  [window attribute](@ref GRWL_MAXIMIZED_attrib).
 */
#define GRWL_MAXIMIZED 0x00020008
/*! @brief Cursor centering window hint
 *
 *  Cursor centering [window hint](@ref GRWL_CENTER_CURSOR_hint).
 */
#define GRWL_CENTER_CURSOR 0x00020009
/*! @brief Window framebuffer transparency hint and attribute
 *
 *  Window framebuffer transparency
 *  [window hint](@ref GRWL_TRANSPARENT_FRAMEBUFFER_hint) and
 *  [window attribute](@ref GRWL_TRANSPARENT_FRAMEBUFFER_attrib).
 */
#define GRWL_TRANSPARENT_FRAMEBUFFER 0x0002000A
/*! @brief Mouse cursor hover window attribute.
 *
 *  Mouse cursor hover [window attribute](@ref GRWL_HOVERED_attrib).
 */
#define GRWL_HOVERED 0x0002000B
/*! @brief Input focus on calling show window hint and attribute
 *
 *  Input focus [window hint](@ref GRWL_FOCUS_ON_SHOW_hint) or
 *  [window attribute](@ref GRWL_FOCUS_ON_SHOW_attrib).
 */
#define GRWL_FOCUS_ON_SHOW 0x0002000C

/*! @brief Mouse input transparency window hint and attribute
 *
 *  Mouse input transparency [window hint](@ref GRWL_MOUSE_PASSTHROUGH_hint) or
 *  [window attribute](@ref GRWL_MOUSE_PASSTHROUGH_attrib).
 */
#define GRWL_MOUSE_PASSTHROUGH 0x0002000D

/*! @brief Initial position x-coordinate window hint.
 *
 *  Initial position x-coordinate [window hint](@ref GRWL_POSITION_X).
 */
#define GRWL_POSITION_X 0x0002000E

/*! @brief Initial position y-coordinate window hint.
 *
 *  Initial position y-coordinate [window hint](@ref GRWL_POSITION_Y).
 */
#define GRWL_POSITION_Y 0x0002000F

/*! @brief Soft fullscreen window hint
 *
 *  Soft fullscreen [window hint](@ref GRWL_SOFT_FULLSCREEN_hint).
 */
#define GRWL_SOFT_FULLSCREEN 0x00020010

/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_RED_BITS).
 */
#define GRWL_RED_BITS 0x00021001
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_GREEN_BITS).
 */
#define GRWL_GREEN_BITS 0x00021002
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_BLUE_BITS).
 */
#define GRWL_BLUE_BITS 0x00021003
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_ALPHA_BITS).
 */
#define GRWL_ALPHA_BITS 0x00021004
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_DEPTH_BITS).
 */
#define GRWL_DEPTH_BITS 0x00021005
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_STENCIL_BITS).
 */
#define GRWL_STENCIL_BITS 0x00021006
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_ACCUM_RED_BITS).
 */
#define GRWL_ACCUM_RED_BITS 0x00021007
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_ACCUM_GREEN_BITS).
 */
#define GRWL_ACCUM_GREEN_BITS 0x00021008
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_ACCUM_BLUE_BITS).
 */
#define GRWL_ACCUM_BLUE_BITS 0x00021009
/*! @brief Framebuffer bit depth hint.
 *
 *  Framebuffer bit depth [hint](@ref GRWL_ACCUM_ALPHA_BITS).
 */
#define GRWL_ACCUM_ALPHA_BITS 0x0002100A
/*! @brief Framebuffer auxiliary buffer hint.
 *
 *  Framebuffer auxiliary buffer [hint](@ref GRWL_AUX_BUFFERS).
 */
#define GRWL_AUX_BUFFERS 0x0002100B
/*! @brief OpenGL stereoscopic rendering hint.
 *
 *  OpenGL stereoscopic rendering [hint](@ref GRWL_STEREO).
 */
#define GRWL_STEREO 0x0002100C
/*! @brief Framebuffer MSAA samples hint.
 *
 *  Framebuffer MSAA samples [hint](@ref GRWL_SAMPLES).
 */
#define GRWL_SAMPLES 0x0002100D
/*! @brief Framebuffer sRGB hint.
 *
 *  Framebuffer sRGB [hint](@ref GRWL_SRGB_CAPABLE).
 */
#define GRWL_SRGB_CAPABLE 0x0002100E
/*! @brief Monitor refresh rate hint.
 *
 *  Monitor refresh rate [hint](@ref GRWL_REFRESH_RATE).
 */
#define GRWL_REFRESH_RATE 0x0002100F
/*! @brief Framebuffer double buffering hint and attribute.
 *
 *  Framebuffer double buffering [hint](@ref GRWL_DOUBLEBUFFER_hint) and
 *  [attribute](@ref GRWL_DOUBLEBUFFER_attrib).
 */
#define GRWL_DOUBLEBUFFER 0x00021010

/*! @brief Context client API hint and attribute.
 *
 *  Context client API [hint](@ref GRWL_CLIENT_API_hint) and
 *  [attribute](@ref GRWL_CLIENT_API_attrib).
 */
#define GRWL_CLIENT_API 0x00022001
/*! @brief Context client API major version hint and attribute.
 *
 *  Context client API major version [hint](@ref GRWL_CONTEXT_VERSION_MAJOR_hint)
 *  and [attribute](@ref GRWL_CONTEXT_VERSION_MAJOR_attrib).
 */
#define GRWL_CONTEXT_VERSION_MAJOR 0x00022002
/*! @brief Context client API minor version hint and attribute.
 *
 *  Context client API minor version [hint](@ref GRWL_CONTEXT_VERSION_MINOR_hint)
 *  and [attribute](@ref GRWL_CONTEXT_VERSION_MINOR_attrib).
 */
#define GRWL_CONTEXT_VERSION_MINOR 0x00022003
/*! @brief Context client API revision number attribute.
 *
 *  Context client API revision number
 *  [attribute](@ref GRWL_CONTEXT_REVISION_attrib).
 */
#define GRWL_CONTEXT_REVISION 0x00022004
/*! @brief Context robustness hint and attribute.
 *
 *  Context client API revision number [hint](@ref GRWL_CONTEXT_ROBUSTNESS_hint)
 *  and [attribute](@ref GRWL_CONTEXT_ROBUSTNESS_attrib).
 */
#define GRWL_CONTEXT_ROBUSTNESS 0x00022005
/*! @brief OpenGL forward-compatibility hint and attribute.
 *
 *  OpenGL forward-compatibility [hint](@ref GRWL_OPENGL_FORWARD_COMPAT_hint)
 *  and [attribute](@ref GRWL_OPENGL_FORWARD_COMPAT_attrib).
 */
#define GRWL_OPENGL_FORWARD_COMPAT 0x00022006
/*! @brief Debug mode context hint and attribute.
 *
 *  Debug mode context [hint](@ref GRWL_CONTEXT_DEBUG_hint) and
 *  [attribute](@ref GRWL_CONTEXT_DEBUG_attrib).
 */
#define GRWL_CONTEXT_DEBUG 0x00022007
/*! @brief Legacy name for compatibility.
 *
 *  This is an alias for compatibility with earlier versions.
 */
#define GRWL_OPENGL_DEBUG_CONTEXT GRWL_CONTEXT_DEBUG
/*! @brief OpenGL profile hint and attribute.
 *
 *  OpenGL profile [hint](@ref GRWL_OPENGL_PROFILE_hint) and
 *  [attribute](@ref GRWL_OPENGL_PROFILE_attrib).
 */
#define GRWL_OPENGL_PROFILE 0x00022008
/*! @brief Context flush-on-release hint and attribute.
 *
 *  Context flush-on-release [hint](@ref GRWL_CONTEXT_RELEASE_BEHAVIOR_hint) and
 *  [attribute](@ref GRWL_CONTEXT_RELEASE_BEHAVIOR_attrib).
 */
#define GRWL_CONTEXT_RELEASE_BEHAVIOR 0x00022009
/*! @brief Context error suppression hint and attribute.
 *
 *  Context error suppression [hint](@ref GRWL_CONTEXT_NO_ERROR_hint) and
 *  [attribute](@ref GRWL_CONTEXT_NO_ERROR_attrib).
 */
#define GRWL_CONTEXT_NO_ERROR 0x0002200A
/*! @brief Context creation API hint and attribute.
 *
 *  Context creation API [hint](@ref GRWL_CONTEXT_CREATION_API_hint) and
 *  [attribute](@ref GRWL_CONTEXT_CREATION_API_attrib).
 */
#define GRWL_CONTEXT_CREATION_API 0x0002200B
/*! @brief Window content area scaling window
 *  [window hint](@ref GRWL_SCALE_TO_MONITOR).
 */
#define GRWL_SCALE_TO_MONITOR 0x0002200C
/*! @brief macOS specific
 *  [window hint](@ref GRWL_COCOA_RETINA_FRAMEBUFFER_hint).
 */
#define GRWL_COCOA_RETINA_FRAMEBUFFER 0x00023001
/*! @brief macOS specific
 *  [window hint](@ref GRWL_COCOA_FRAME_NAME_hint).
 */
#define GRWL_COCOA_FRAME_NAME 0x00023002
/*! @brief macOS specific
 *  [window hint](@ref GRWL_COCOA_GRAPHICS_SWITCHING_hint).
 */
#define GRWL_COCOA_GRAPHICS_SWITCHING 0x00023003
/*! @brief X11 specific
 *  [window hint](@ref GRWL_X11_CLASS_NAME_hint).
 */
#define GRWL_X11_CLASS_NAME 0x00024001
/*! @brief X11 specific
 *  [window hint](@ref GRWL_X11_CLASS_NAME_hint).
 */
#define GRWL_X11_INSTANCE_NAME 0x00024002
#define GRWL_WIN32_KEYBOARD_MENU 0x00025001
#define GRWL_WIN32_GENERIC_BADGE 0x00025002
/*! @brief Wayland specific
 *  [window hint](@ref GRWL_WAYLAND_APP_ID_hint).
 *
 *  Allows specification of the Wayland app_id.
 */
#define GRWL_WAYLAND_APP_ID 0x00026001
    /*! @} */

#define GRWL_NO_API 0
#define GRWL_OPENGL_API 0x00030001
#define GRWL_OPENGL_ES_API 0x00030002

#define GRWL_NO_ROBUSTNESS 0
#define GRWL_NO_RESET_NOTIFICATION 0x00031001
#define GRWL_LOSE_CONTEXT_ON_RESET 0x00031002

#define GRWL_OPENGL_ANY_PROFILE 0
#define GRWL_OPENGL_CORE_PROFILE 0x00032001
#define GRWL_OPENGL_COMPAT_PROFILE 0x00032002

#define GRWL_CURSOR 0x00033001
#define GRWL_STICKY_KEYS 0x00033002
#define GRWL_STICKY_MOUSE_BUTTONS 0x00033003
#define GRWL_LOCK_KEY_MODS 0x00033004
#define GRWL_RAW_MOUSE_MOTION 0x00033005
#define GRWL_IME 0x00033006

#define GRWL_CURSOR_NORMAL 0x00034001
#define GRWL_CURSOR_HIDDEN 0x00034002
#define GRWL_CURSOR_DISABLED 0x00034003
#define GRWL_CURSOR_CAPTURED 0x00034004

#define GRWL_ANY_RELEASE_BEHAVIOR 0
#define GRWL_RELEASE_BEHAVIOR_FLUSH 0x00035001
#define GRWL_RELEASE_BEHAVIOR_NONE 0x00035002

#define GRWL_NATIVE_CONTEXT_API 0x00036001
#define GRWL_EGL_CONTEXT_API 0x00036002
#define GRWL_OSMESA_CONTEXT_API 0x00036003

#define GRWL_ANGLE_PLATFORM_TYPE_NONE 0x00037001
#define GRWL_ANGLE_PLATFORM_TYPE_OPENGL 0x00037002
#define GRWL_ANGLE_PLATFORM_TYPE_OPENGLES 0x00037003
#define GRWL_ANGLE_PLATFORM_TYPE_D3D9 0x00037004
#define GRWL_ANGLE_PLATFORM_TYPE_D3D11 0x00037005
#define GRWL_ANGLE_PLATFORM_TYPE_VULKAN 0x00037007
#define GRWL_ANGLE_PLATFORM_TYPE_METAL 0x00037008

#define GRWL_WAYLAND_PREFER_LIBDECOR 0x00038001
#define GRWL_WAYLAND_DISABLE_LIBDECOR 0x00038002

#define GRWL_ANY_POSITION 0x80000000

/*! @defgroup shapes Standard cursor shapes
 *  @brief Standard system cursor shapes.
 *
 *  These are the [standard cursor shapes](@ref cursor_standard) that can be
 *  requested from the platform (window system).
 *
 *  @ingroup input
 *  @{ */

/*! @brief The regular arrow cursor shape.
 *
 *  The regular arrow cursor shape.
 */
#define GRWL_ARROW_CURSOR 0x00036001
/*! @brief The text input I-beam cursor shape.
 *
 *  The text input I-beam cursor shape.
 */
#define GRWL_IBEAM_CURSOR 0x00036002
/*! @brief The crosshair cursor shape.
 *
 *  The crosshair cursor shape.
 */
#define GRWL_CROSSHAIR_CURSOR 0x00036003
/*! @brief The pointing hand cursor shape.
 *
 *  The pointing hand cursor shape.
 */
#define GRWL_POINTING_HAND_CURSOR 0x00036004
/*! @brief The horizontal resize/move arrow shape.
 *
 *  The horizontal resize/move arrow shape.  This is usually a horizontal
 *  double-headed arrow.
 */
#define GRWL_RESIZE_EW_CURSOR 0x00036005
/*! @brief The vertical resize/move arrow shape.
 *
 *  The vertical resize/move shape.  This is usually a vertical double-headed
 *  arrow.
 */
#define GRWL_RESIZE_NS_CURSOR 0x00036006
/*! @brief The top-left to bottom-right diagonal resize/move arrow shape.
 *
 *  The top-left to bottom-right diagonal resize/move shape.  This is usually
 *  a diagonal double-headed arrow.
 *
 *  @note @macos This shape is provided by a private system API and may fail
 *  with @ref GRWL_CURSOR_UNAVAILABLE in the future.
 *
 *  @note @x11 This shape is provided by a newer standard not supported by all
 *  cursor themes.
 *
 *  @note @wayland This shape is provided by a newer standard not supported by
 *  all cursor themes.
 */
#define GRWL_RESIZE_NWSE_CURSOR 0x00036007
/*! @brief The top-right to bottom-left diagonal resize/move arrow shape.
 *
 *  The top-right to bottom-left diagonal resize/move shape.  This is usually
 *  a diagonal double-headed arrow.
 *
 *  @note @macos This shape is provided by a private system API and may fail
 *  with @ref GRWL_CURSOR_UNAVAILABLE in the future.
 *
 *  @note @x11 This shape is provided by a newer standard not supported by all
 *  cursor themes.
 *
 *  @note @wayland This shape is provided by a newer standard not supported by
 *  all cursor themes.
 */
#define GRWL_RESIZE_NESW_CURSOR 0x00036008
/*! @brief The omni-directional resize/move cursor shape.
 *
 *  The omni-directional resize cursor/move shape.  This is usually either
 *  a combined horizontal and vertical double-headed arrow or a grabbing hand.
 */
#define GRWL_RESIZE_ALL_CURSOR 0x00036009
/*! @brief The operation-not-allowed shape.
 *
 *  The operation-not-allowed shape.  This is usually a circle with a diagonal
 *  line through it.
 *
 *  @note @x11 This shape is provided by a newer standard not supported by all
 *  cursor themes.
 *
 *  @note @wayland This shape is provided by a newer standard not supported by
 *  all cursor themes.
 */
#define GRWL_NOT_ALLOWED_CURSOR 0x0003600A
/*! @brief Legacy name for compatibility.
 *
 *  This is an alias for compatibility with earlier versions.
 */
#define GRWL_HRESIZE_CURSOR GRWL_RESIZE_EW_CURSOR
/*! @brief Legacy name for compatibility.
 *
 *  This is an alias for compatibility with earlier versions.
 */
#define GRWL_VRESIZE_CURSOR GRWL_RESIZE_NS_CURSOR
/*! @brief Legacy name for compatibility.
 *
 *  This is an alias for compatibility with earlier versions.
 */
#define GRWL_HAND_CURSOR GRWL_POINTING_HAND_CURSOR
/*! @} */

/*! @addtogroup window
 *  @{ */
/*! @brief Disable the progress bar.
 *
 *  Disable the progress bar.
 *
 *  Used by @ref window_progress_indicator.
 */
#define GRWL_PROGRESS_INDICATOR_DISABLED 0
/*! @brief Display the progress bar in an indeterminate state.
 *
 *  Display the progress bar in an indeterminate state.
 *
 *  @remark @win32 This displays the progress bar animation cycling repeatedly.
 *
 *  @remark @x11 @wayland This behaves like @ref GRWL_PROGRESS_INDICATOR_NORMAL.
 *
 *  @remark @macos This displays a standard indeterminate `NSProgressIndicator`.
 *
 *  Used by @ref window_progress_indicator.
 */
#define GRWL_PROGRESS_INDICATOR_INDETERMINATE 1
/*! @brief Display the normal progress bar.
 *
 *  Display the normal progress bar.
 *
 *  Used by @ref window_progress_indicator.
 */
#define GRWL_PROGRESS_INDICATOR_NORMAL 2
/*! @brief Display the progress bar in an error state.
 *
 *  Display the progress bar in an error state.
 *
 *  @remark @win32 This displays a red progress bar.
 *
 *  @remark @x11 @wayland @macos This behaves like @ref GRWL_PROGRESS_INDICATOR_NORMAL.
 *
 *  Used by @ref window_progress_indicator.
 */
#define GRWL_PROGRESS_INDICATOR_ERROR 3
/*! @brief Display the progress bar in a paused state.
 *
 *  Display the progress bar in a paused state.
 *
 *  @remark @win32 This displays a yellow progress bar.
 *
 *  @remark @x11 @wayland @macos This behaves like @ref GRWL_PROGRESS_INDICATOR_NORMAL.
 *
 *  Used by @ref window_progress_indicator.
 */
#define GRWL_PROGRESS_INDICATOR_PAUSED 4
    /*! @} */

#define GRWL_CONNECTED 0x00040001
#define GRWL_DISCONNECTED 0x00040002

/*! @addtogroup init
 *  @{ */
/*! @brief Joystick hat buttons init hint.
 *
 *  Joystick hat buttons [init hint](@ref GRWL_JOYSTICK_HAT_BUTTONS).
 */
#define GRWL_JOYSTICK_HAT_BUTTONS 0x00050001
/*! @brief ANGLE rendering backend init hint.
 *
 *  ANGLE rendering backend [init hint](@ref GRWL_ANGLE_PLATFORM_TYPE_hint).
 */
#define GRWL_ANGLE_PLATFORM_TYPE 0x00050002
/*! @brief Platform selection init hint.
 *
 *  Platform selection [init hint](@ref GRWL_PLATFORM).
 */
#define GRWL_PLATFORM 0x00050003
/*! @brief Preedit candidate init hint.
 *
 *  Preedit candidate [init hint](@ref GRWL_MANAGE_PREEDIT_CANDIDATE_hint).
 */
#define GRWL_MANAGE_PREEDIT_CANDIDATE 0x00050004
/*! @brief macOS specific init hint.
 *
 *  macOS specific [init hint](@ref GRWL_COCOA_CHDIR_RESOURCES_hint).
 */
#define GRWL_COCOA_CHDIR_RESOURCES 0x00051001
/*! @brief macOS specific init hint.
 *
 *  macOS specific [init hint](@ref GRWL_COCOA_MENUBAR_hint).
 */
#define GRWL_COCOA_MENUBAR 0x00051002
/*! @brief X11 specific init hint.
 *
 *  X11 specific [init hint](@ref GRWL_X11_XCB_VULKAN_SURFACE_hint).
 */
#define GRWL_X11_XCB_VULKAN_SURFACE 0x00052001
/*! @brief X11 specific init hint.
 *
 *  X11 specific [init hint](@ref GRWL_X11_ONTHESPOT_hint).
 */
#define GRWL_X11_ONTHESPOT 0x00052002
/*! @brief Wayland specific init hint.
 *
 *  Wayland specific [init hint](@ref GRWL_WAYLAND_LIBDECOR_hint).
 */
#define GRWL_WAYLAND_LIBDECOR 0x00053001
/*! @} */

/*! @addtogroup init
 *  @{ */
/*! @brief Hint value that enables automatic platform selection.
 *
 *  Hint value for @ref GRWL_PLATFORM that enables automatic platform selection.
 */
#define GRWL_ANY_PLATFORM 0x00060000
#define GRWL_PLATFORM_WIN32 0x00060001
#define GRWL_PLATFORM_COCOA 0x00060002
#define GRWL_PLATFORM_WAYLAND 0x00060003
#define GRWL_PLATFORM_X11 0x00060004
#define GRWL_PLATFORM_NULL 0x00060005
    /*! @} */

#define GRWL_DONT_CARE -1

    /*************************************************************************
     * GRWL API types
     *************************************************************************/

    /*! @brief Client API function pointer type.
     *
     *  Generic function pointer used for returning client API function pointers
     *  without forcing a cast from a regular pointer.
     *
     *  @sa @ref context_glext
     *  @sa @ref grwlGetProcAddress
     *
     *  @ingroup context
     */
    typedef void (*GRWLglproc)();

    /*! @brief Vulkan API function pointer type.
     *
     *  Generic function pointer used for returning Vulkan API function pointers
     *  without forcing a cast from a regular pointer.
     *
     *  @sa @ref vulkan_proc
     *  @sa @ref grwlGetInstanceProcAddress
     *
     *  @ingroup vulkan
     */
    typedef void (*GRWLvkproc)();

    /*! @brief Opaque monitor object.
     *
     *  Opaque monitor object.
     *
     *  @see @ref monitor_object
     *
     *  @ingroup monitor
     */
    typedef struct GRWLmonitor GRWLmonitor;

    /*! @brief Opaque window object.
     *
     *  Opaque window object.
     *
     *  @see @ref window_object
     *
     *  @ingroup window
     */
    typedef struct GRWLwindow GRWLwindow;

    /*! @brief Opaque user OpenGL & OpenGL ES context object.
     *
     *  Opaque user OpenGL OpenGL ES context object.
     *
     *  @see @ref context_user
     *
     *  @ingroup window
     */
    typedef struct GRWLusercontext GRWLusercontext;

    /*! @brief Opaque cursor object.
     *
     *  Opaque cursor object.
     *
     *  @see @ref cursor_object
     *
     *  @ingroup input
     */
    typedef struct GRWLcursor GRWLcursor;

    /*! @brief The function pointer type for memory allocation callbacks.
     *
     *  This is the function pointer type for memory allocation callbacks.  A memory
     *  allocation callback function has the following signature:
     *  @code
     *  void* function_name(size_t size, void* user)
     *  @endcode
     *
     *  This function must return either a memory block at least `size` bytes long,
     *  or `NULL` if allocation failed.  Note that not all parts of GRWL handle allocation
     *  failures gracefully yet.
     *
     *  This function may be called during @ref grwlInit but before the library is
     *  flagged as initialized, as well as during @ref grwlTerminate after the
     *  library is no longer flagged as initialized.
     *
     *  Any memory allocated by this function will be deallocated during library
     *  termination or earlier.
     *
     *  The size will always be greater than zero.  Allocations of size zero are filtered out
     *  before reaching the custom allocator.
     *
     *  @param[in] size The minimum size, in bytes, of the memory block.
     *  @param[in] user The user-defined pointer from the allocator.
     *  @return The address of the newly allocated memory block, or `NULL` if an
     *  error occurred.
     *
     *  @pointer_lifetime The returned memory block must be valid at least until it
     *  is deallocated.
     *
     *  @reentrancy This function should not call any GRWL function.
     *
     *  @thread_safety This function may be called from any thread that calls GRWL functions.
     *
     *  @sa @ref init_allocator
     *  @sa @ref GRWLallocator
     *
     *  @ingroup init
     */
    typedef void* (*GRWLallocatefun)(size_t size, void* user);

    /*! @brief The function pointer type for memory reallocation callbacks.
     *
     *  This is the function pointer type for memory reallocation callbacks.
     *  A memory reallocation callback function has the following signature:
     *  @code
     *  void* function_name(void* block, size_t size, void* user)
     *  @endcode
     *
     *  This function must return a memory block at least `size` bytes long, or
     *  `NULL` if allocation failed.  Note that not all parts of GRWL handle allocation
     *  failures gracefully yet.
     *
     *  This function may be called during @ref grwlInit but before the library is
     *  flagged as initialized, as well as during @ref grwlTerminate after the
     *  library is no longer flagged as initialized.
     *
     *  Any memory allocated by this function will be deallocated during library
     *  termination or earlier.
     *
     *  The block address will never be `NULL` and the size will always be greater than zero.
     *  Reallocations of a block to size zero are converted into deallocations.  Reallocations
     *  of `NULL` to a non-zero size are converted into regular allocations.
     *
     *  @param[in] block The address of the memory block to reallocate.
     *  @param[in] size The new minimum size, in bytes, of the memory block.
     *  @param[in] user The user-defined pointer from the allocator.
     *  @return The address of the newly allocated or resized memory block, or
     *  `NULL` if an error occurred.
     *
     *  @pointer_lifetime The returned memory block must be valid at least until it
     *  is deallocated.
     *
     *  @reentrancy This function should not call any GRWL function.
     *
     *  @thread_safety This function may be called from any thread that calls GRWL functions.
     *
     *  @sa @ref init_allocator
     *  @sa @ref GRWLallocator
     *
     *  @ingroup init
     */
    typedef void* (*GRWLreallocatefun)(void* block, size_t size, void* user);

    /*! @brief The function pointer type for memory deallocation callbacks.
     *
     *  This is the function pointer type for memory deallocation callbacks.
     *  A memory deallocation callback function has the following signature:
     *  @code
     *  void function_name(void* block, void* user)
     *  @endcode
     *
     *  This function may deallocate the specified memory block.  This memory block
     *  will have been allocated with the same allocator.
     *
     *  This function may be called during @ref grwlInit but before the library is
     *  flagged as initialized, as well as during @ref grwlTerminate after the
     *  library is no longer flagged as initialized.
     *
     *  The block address will never be `NULL`.  Deallocations of `NULL` are filtered out
     *  before reaching the custom allocator.
     *
     *  @param[in] block The address of the memory block to deallocate.
     *  @param[in] user The user-defined pointer from the allocator.
     *
     *  @pointer_lifetime The specified memory block will not be accessed by GRWL
     *  after this function is called.
     *
     *  @reentrancy This function should not call any GRWL function.
     *
     *  @thread_safety This function may be called from any thread that calls GRWL functions.
     *
     *  @sa @ref init_allocator
     *  @sa @ref GRWLallocator
     *
     *  @ingroup init
     */
    typedef void (*GRWLdeallocatefun)(void* block, void* user);

    /*! @brief The function pointer type for error callbacks.
     *
     *  This is the function pointer type for error callbacks.  An error callback
     *  function has the following signature:
     *  @code
     *  void callback_name(int error_code, const char* description)
     *  @endcode
     *
     *  @param[in] error_code An [error code](@ref errors).  Future releases may add
     *  more error codes.
     *  @param[in] description A UTF-8 encoded string describing the error.
     *
     *  @pointer_lifetime The error description string is valid until the callback
     *  function returns.
     *
     *  @sa @ref error_handling
     *  @sa @ref grwlSetErrorCallback
     *
     *  @ingroup init
     */
    typedef void (*GRWLerrorfun)(int error_code, const char* description);

    /*! @brief The function pointer type for keyboard layout callbacks.
     *
     *  This is the function pointer type for keyboard layout callbacks.  A keyboard
     *  layout callback function has the following signature:
     *  @code
     *  void callback_name();
     *  @endcode
     *
     *  @sa @ref keyboard_layout
     *  @sa @ref grwlSetKeyboardLayoutCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLkeyboardlayoutfun)();

    /*! @brief The function pointer type for window position callbacks.
     *
     *  This is the function pointer type for window position callbacks.  A window
     *  position callback function has the following signature:
     *  @code
     *  void callback_name(GRWLwindow* window, int xpos, int ypos)
     *  @endcode
     *
     *  @param[in] window The window that was moved.
     *  @param[in] xpos The new x-coordinate, in screen coordinates, of the
     *  upper-left corner of the content area of the window.
     *  @param[in] ypos The new y-coordinate, in screen coordinates, of the
     *  upper-left corner of the content area of the window.
     *
     *  @sa @ref window_pos
     *  @sa @ref grwlSetWindowPosCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowposfun)(GRWLwindow* window, int xpos, int ypos);

    /*! @brief The function pointer type for window size callbacks.
     *
     *  This is the function pointer type for window size callbacks.  A window size
     *  callback function has the following signature:
     *  @code
     *  void callback_name(GRWLwindow* window, int width, int height)
     *  @endcode
     *
     *  @param[in] window The window that was resized.
     *  @param[in] width The new width, in screen coordinates, of the window.
     *  @param[in] height The new height, in screen coordinates, of the window.
     *
     *  @sa @ref window_size
     *  @sa @ref grwlSetWindowSizeCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowsizefun)(GRWLwindow* window, int width, int height);

    /*! @brief The function pointer type for window close callbacks.
     *
     *  This is the function pointer type for window close callbacks.  A window
     *  close callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window)
     *  @endcode
     *
     *  @param[in] window The window that the user attempted to close.
     *
     *  @sa @ref window_close
     *  @sa @ref grwlSetWindowCloseCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowclosefun)(GRWLwindow* window);

    /*! @brief The function pointer type for window content refresh callbacks.
     *
     *  This is the function pointer type for window content refresh callbacks.
     *  A window content refresh callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window);
     *  @endcode
     *
     *  @param[in] window The window whose content needs to be refreshed.
     *
     *  @sa @ref window_refresh
     *  @sa @ref grwlSetWindowRefreshCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowrefreshfun)(GRWLwindow* window);

    /*! @brief The function pointer type for window focus callbacks.
     *
     *  This is the function pointer type for window focus callbacks.  A window
     *  focus callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int focused)
     *  @endcode
     *
     *  @param[in] window The window that gained or lost input focus.
     *  @param[in] focused `true` if the window was given input focus, or
     *  `false` if it lost it.
     *
     *  @sa @ref window_focus
     *  @sa @ref grwlSetWindowFocusCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowfocusfun)(GRWLwindow* window, int focused);

    /*! @brief The function pointer type for window iconify callbacks.
     *
     *  This is the function pointer type for window iconify callbacks.  A window
     *  iconify callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int iconified)
     *  @endcode
     *
     *  @param[in] window The window that was iconified or restored.
     *  @param[in] iconified `true` if the window was iconified, or
     *  `false` if it was restored.
     *
     *  @sa @ref window_iconify
     *  @sa @ref grwlSetWindowIconifyCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowiconifyfun)(GRWLwindow* window, int iconified);

    /*! @brief The function pointer type for window maximize callbacks.
     *
     *  This is the function pointer type for window maximize callbacks.  A window
     *  maximize callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int maximized)
     *  @endcode
     *
     *  @param[in] window The window that was maximized or restored.
     *  @param[in] maximized `true` if the window was maximized, or
     *  `false` if it was restored.
     *
     *  @sa @ref window_maximize
     *  @sa grwlSetWindowMaximizeCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowmaximizefun)(GRWLwindow* window, int maximized);

    /*! @brief The function pointer type for framebuffer size callbacks.
     *
     *  This is the function pointer type for framebuffer size callbacks.
     *  A framebuffer size callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int width, int height)
     *  @endcode
     *
     *  @param[in] window The window whose framebuffer was resized.
     *  @param[in] width The new width, in pixels, of the framebuffer.
     *  @param[in] height The new height, in pixels, of the framebuffer.
     *
     *  @sa @ref window_fbsize
     *  @sa @ref grwlSetFramebufferSizeCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLframebuffersizefun)(GRWLwindow* window, int width, int height);

    /*! @brief The function pointer type for window content scale callbacks.
     *
     *  This is the function pointer type for window content scale callbacks.
     *  A window content scale callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, float xscale, float yscale)
     *  @endcode
     *
     *  @param[in] window The window whose content scale changed.
     *  @param[in] xscale The new x-axis content scale of the window.
     *  @param[in] yscale The new y-axis content scale of the window.
     *
     *  @sa @ref window_scale
     *  @sa @ref grwlSetWindowContentScaleCallback
     *
     *  @ingroup window
     */
    typedef void (*GRWLwindowcontentscalefun)(GRWLwindow* window, float xscale, float yscale);

    /*! @brief The function pointer type for mouse button callbacks.
     *
     *  This is the function pointer type for mouse button callback functions.
     *  A mouse button callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int button, int action, int mods)
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] button The [mouse button](@ref buttons) that was pressed or
     *  released.
     *  @param[in] action One of `GRWL_PRESS` or `GRWL_RELEASE`.  Future releases
     *  may add more actions.
     *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
     *  held down.
     *
     *  @sa @ref input_mouse_button
     *  @sa @ref grwlSetMouseButtonCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLmousebuttonfun)(GRWLwindow* window, int button, int action, int mods);

    /*! @brief The function pointer type for cursor position callbacks.
     *
     *  This is the function pointer type for cursor position callbacks.  A cursor
     *  position callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, double xpos, double ypos);
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] xpos The new cursor x-coordinate, relative to the left edge of
     *  the content area.
     *  @param[in] ypos The new cursor y-coordinate, relative to the top edge of the
     *  content area.
     *
     *  @sa @ref cursor_pos
     *  @sa @ref grwlSetCursorPosCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLcursorposfun)(GRWLwindow* window, double xpos, double ypos);

    /*! @brief The function pointer type for cursor enter/leave callbacks.
     *
     *  This is the function pointer type for cursor enter/leave callbacks.
     *  A cursor enter/leave callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int entered)
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] entered `true` if the cursor entered the window's content
     *  area, or `false` if it left it.
     *
     *  @sa @ref cursor_enter
     *  @sa @ref grwlSetCursorEnterCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLcursorenterfun)(GRWLwindow* window, int entered);

    /*! @brief The function pointer type for scroll callbacks.
     *
     *  This is the function pointer type for scroll callbacks.  A scroll callback
     *  function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, double xoffset, double yoffset)
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] xoffset The scroll offset along the x-axis.
     *  @param[in] yoffset The scroll offset along the y-axis.
     *
     *  @sa @ref scrolling
     *  @sa @ref grwlSetScrollCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLscrollfun)(GRWLwindow* window, double xoffset, double yoffset);

    /*! @brief The function pointer type for keyboard key callbacks.
     *
     *  This is the function pointer type for keyboard key callbacks.  A keyboard
     *  key callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int key, int scancode, int action, int mods)
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] key The [keyboard key](@ref keys) that was pressed or released.
     *  @param[in] scancode The platform-specific scancode of the key.
     *  @param[in] action `GRWL_PRESS`, `GRWL_RELEASE` or `GRWL_REPEAT`.  Future
     *  releases may add more actions.
     *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
     *  held down.
     *
     *  @sa @ref input_key
     *  @sa @ref grwlSetKeyCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLkeyfun)(GRWLwindow* window, int key, int scancode, int action, int mods);

    /*! @brief The function pointer type for Unicode character callbacks.
     *
     *  This is the function pointer type for Unicode character callbacks.
     *  A Unicode character callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, unsigned int codepoint)
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] codepoint The Unicode code point of the character.
     *
     *  @sa @ref input_char
     *  @sa @ref grwlSetCharCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLcharfun)(GRWLwindow* window, unsigned int codepoint);

    /*! @brief The function pointer type for Unicode character with modifiers
     *  callbacks.
     *
     *  This is the function pointer type for Unicode character with modifiers
     *  callbacks.  It is called for each input character, regardless of what
     *  modifier keys are held down.  A Unicode character with modifiers callback
     *  function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, unsigned int codepoint, int mods)
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] codepoint The Unicode code point of the character.
     *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
     *  held down.
     *
     *  @sa @ref input_char
     *  @sa @ref grwlSetCharModsCallback
     *
     *  @deprecated Scheduled for removal in version 4.0.
     *
     *  @ingroup input
     */
    typedef void (*GRWLcharmodsfun)(GRWLwindow* window, unsigned int codepoint, int mods);

    /*! @brief The function pointer type for preedit callbacks.
     *
     *  This is the function pointer type for preedit callback functions.
     *
     *  @param[in] window The window that received the event.
     *  @param[in] preedit_count Preedit string count.
     *  @param[in] preedit_string Preedit string.
     *  @param[in] block_count Attributed block count.
     *  @param[in] block_sizes List of attributed block size.
     *  @param[in] focused_block Focused block index.
     *  @param[in] caret Caret position.
     *
     *  @sa @ref ime_support
     *  @sa grwlSetPreeditCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLpreeditfun)(GRWLwindow* window, int preedit_count, unsigned int* preedit_string, int block_count,
                                   int* block_sizes, int focused_block, int caret);

    /*! @brief The function pointer type for IME status change callbacks.
     *
     *  This is the function pointer type for IME status change callback functions.
     *
     *  @param[in] window The window that received the event.
     *
     *  @sa @ref ime_support
     *  @sa grwlSetIMEStatusCallback
     *
     *  @ingroup monitor
     */
    typedef void (*GRWLimestatusfun)(GRWLwindow* window);

    /*! @brief The function pointer type for preedit candidate callbacks.
     *
     *  This is the function pointer type for preedit candidate callback functions.
     *  Use @ref grwlGetPreeditCandidate to get the candidate text for a specific index.
     *
     *  @param[in] window The window that received the event.
     *  @param[in] candidates_count Candidates count.
     *  @param[in] selected_index.Index of selected candidate.
     *  @param[in] page_start Start index of candidate currently displayed.
     *  @param[in] page_size Count of candidates currently displayed.
     *
     *  @sa @ref ime_support
     *  @sa @ref grwlSetPreeditCandidateCallback
     *  @sa @ref grwlGetPreeditCandidate
     *
     *  @ingroup input
     */
    typedef void (*GRWLpreeditcandidatefun)(GRWLwindow* window, int candidates_count, int selected_index,
                                            int page_start, int page_size);

    /*! @brief The function pointer type for path drop callbacks.
     *
     *  This is the function pointer type for path drop callbacks.  A path drop
     *  callback function has the following signature:
     *  @code
     *  void function_name(GRWLwindow* window, int path_count, const char* paths[])
     *  @endcode
     *
     *  @param[in] window The window that received the event.
     *  @param[in] path_count The number of dropped paths.
     *  @param[in] paths The UTF-8 encoded file and/or directory path names.
     *
     *  @pointer_lifetime The path array and its strings are valid until the
     *  callback function returns.
     *
     *  @sa @ref path_drop
     *  @sa @ref grwlSetDropCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLdropfun)(GRWLwindow* window, int path_count, const char* paths[]);

    /*! @brief The function pointer type for monitor configuration callbacks.
     *
     *  This is the function pointer type for monitor configuration callbacks.
     *  A monitor callback function has the following signature:
     *  @code
     *  void function_name(GRWLmonitor* monitor, int event)
     *  @endcode
     *
     *  @param[in] monitor The monitor that was connected or disconnected.
     *  @param[in] event One of `GRWL_CONNECTED` or `GRWL_DISCONNECTED`.  Future
     *  releases may add more events.
     *
     *  @sa @ref monitor_event
     *  @sa @ref grwlSetMonitorCallback
     *
     *  @ingroup monitor
     */
    typedef void (*GRWLmonitorfun)(GRWLmonitor* monitor, int event);

    /*! @brief The function pointer type for joystick configuration callbacks.
     *
     *  This is the function pointer type for joystick configuration callbacks.
     *  A joystick configuration callback function has the following signature:
     *  @code
     *  void function_name(int jid, int event)
     *  @endcode
     *
     *  @param[in] jid The joystick that was connected or disconnected.
     *  @param[in] event One of `GRWL_CONNECTED` or `GRWL_DISCONNECTED`.  Future
     *  releases may add more events.
     *
     *  @sa @ref joystick_event
     *  @sa @ref grwlSetJoystickCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLjoystickfun)(int jid, int event);

    /*! @brief The function pointer type for joystick button callbacks.
     *
     *  This is the function pointer type for joystick button callbacks.  A joystick
     *  button callback function has the following signature:
     *  @code
     *  void function_name(int jid, int button, int action)
     *  @endcode
     *
     *  @param[in] jid The joystick that had a button pressed or released.
     *  @param[in] button The [joystick button](@ref buttons) that was pressed or released.
     *  @param[in] action `GRWL_PRESS` or `GRWL_RELEASE`.  Future
     *  releases may add more actions.
     *
     *  @sa @ref input_joystick_button
     *  @sa @ref grwlSetJoystickButonCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLjoystickbuttonfun)(int jid, int button, int action);

    /*! @brief The function pointer type for joystick axis movement callbacks.
     *
     *  This is the function pointer type for joystick axis movement callbacks.  A joystick
     *  axis movement callback function has the following signature:
     *  @code
     *  void function_name(int jid, int axis, float position)
     *  @endcode
     *
     *  @param[in] jid The joystick that had an axis moved.
     *  @param[in] axis The [joystick axis](@ref gamepad axes) that was moved.
     *  @param[in] position A value between -1.0 and 1.0 that indicates the position of the axis.
     *
     *  @sa @ref input_gamepad_axis
     *  @sa @ref grwlSetJoystickAxisCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLjoystickaxisfun)(int jid, int axis, float position);

    /*! @brief The function pointer type for joystick hat movement callbacks.
     *
     *  This is the function pointer type for joystick hat movement callbacks.  A joystick
     *  hat movement callback function has the following signature:
     *  @code
     *  void function_name(int jid, int hat, int position)
     *  @endcode
     *
     *  @param[in] jid The joystick that had an axis moved.
     *  @param[in] hat The [joystick hat](@ref joystick hats) that was moved.
     *  @param[in] position A value that indicates the position of the hat.
     *  The position parameter is one of the following values:
     *
     *  Name                  | Value
     *  ----                  | -----
     *  `GRWL_HAT_CENTERED`   | 0
     *  `GRWL_HAT_UP`         | 1
     *  `GRWL_HAT_RIGHT`      | 2
     *  `GRWL_HAT_DOWN`       | 4
     *  `GRWL_HAT_LEFT`       | 8
     *  `GRWL_HAT_RIGHT_UP`   | `GRWL_HAT_RIGHT` \| `GRWL_HAT_UP`
     *  `GRWL_HAT_RIGHT_DOWN` | `GRWL_HAT_RIGHT` \| `GRWL_HAT_DOWN`
     *  `GRWL_HAT_LEFT_UP`    | `GRWL_HAT_LEFT` \| `GRWL_HAT_UP`
     *  `GRWL_HAT_LEFT_DOWN`  | `GRWL_HAT_LEFT` \| `GRWL_HAT_DOWN`
     *
     *  The diagonal directions are bitwise combinations of the primary (up, right,
     *  down and left) directions and you can test for these individually by ANDing
     *  it with the corresponding direction.
     *
     *  @sa @ref input_joystick_hat
     *  @sa @ref grwlSetJoystickHatCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLjoystickhatfun)(int jid, int hat, int position);

    /*! @brief The function pointer type for game pad state changes.
     *
     *  This is the function pointer type for game pad state change callbacks.
     *  A game pad state change callback function has the following signature:
     *  @code
     *  void function_name(int jid, GRWLgamepadstate* state)
     *  @endcode
     *
     *  @param[in] jid The ID of the game pad that changed state.
     *  @param[in] buttons The states of each
     *             [gamepad button](@ref gamepad_buttons),
     *             `GRWL_PRESS` or `GRWL_RELEASE`.
     *  @param[in] axes The states of each [gamepad axis](@ref gamepad_axes),
     *                  in the range -1.0 to 1.0 inclusive.
     *
     *  @sa @ref input_gamepad
     *  @sa @ref grwlSetGamepadStateCallback
     *
     *  @ingroup input
     */
    typedef void (*GRWLgamepadstatefun)(int jid, unsigned char buttons[15], float axes[6]);

    /*! @brief Video mode type.
     *
     *  This describes a single video mode.
     *
     *  @sa @ref monitor_modes
     *  @sa @ref grwlGetVideoMode
     *  @sa @ref grwlGetVideoModes
     *
     *  @ingroup monitor
     */
    typedef struct GRWLvidmode
    {
        /*! The width, in screen coordinates, of the video mode.
         */
        int width;
        /*! The height, in screen coordinates, of the video mode.
         */
        int height;
        /*! The bit depth of the red channel of the video mode.
         */
        int redBits;
        /*! The bit depth of the green channel of the video mode.
         */
        int greenBits;
        /*! The bit depth of the blue channel of the video mode.
         */
        int blueBits;
        /*! The refresh rate, in Hz, of the video mode.
         */
        int refreshRate;
    } GRWLvidmode;

    /*! @brief Gamma ramp.
     *
     *  This describes the gamma ramp for a monitor.
     *
     *  @sa @ref monitor_gamma
     *  @sa @ref grwlGetGammaRamp
     *  @sa @ref grwlSetGammaRamp
     *
     *  @ingroup monitor
     */
    typedef struct GRWLgammaramp
    {
        /*! An array of value describing the response of the red channel.
         */
        unsigned short* red;
        /*! An array of value describing the response of the green channel.
         */
        unsigned short* green;
        /*! An array of value describing the response of the blue channel.
         */
        unsigned short* blue;
        /*! The number of elements in each array.
         */
        unsigned int size;
    } GRWLgammaramp;

    /*! @brief Image data.
     *
     *  This describes a single 2D image.  See the documentation for each related
     *  function what the expected pixel format is.
     *
     *  @sa @ref cursor_custom
     *  @sa @ref window_icon
     *
     *  @ingroup window
     */
    typedef struct GRWLimage
    {
        /*! The width, in pixels, of this image.
         */
        int width;
        /*! The height, in pixels, of this image.
         */
        int height;
        /*! The pixel data of this image, arranged left-to-right, top-to-bottom.
         */
        unsigned char* pixels;
    } GRWLimage;

    /*! @brief Gamepad input state
     *
     *  This describes the input state of a gamepad.
     *
     *  @sa @ref gamepad
     *  @sa @ref grwlGetGamepadState
     *  @sa @ref grwlSetGamepadStateCallback
     *
     *  @ingroup input
     */
    typedef struct GRWLgamepadstate
    {
        /*! The states of each [gamepad button](@ref gamepad_buttons), `GRWL_PRESS`
         *  or `GRWL_RELEASE`.
         */
        unsigned char buttons[15];
        /*! The states of each [gamepad axis](@ref gamepad_axes), in the range -1.0
         *  to 1.0 inclusive.
         */
        float axes[6];
    } GRWLgamepadstate;

    /*! @brief
     *
     *  @sa @ref init_allocator
     *  @sa @ref grwlInitAllocator
     *
     *  @ingroup init
     */
    typedef struct GRWLallocator
    {
        GRWLallocatefun allocate;
        GRWLreallocatefun reallocate;
        GRWLdeallocatefun deallocate;
        void* user;
    } GRWLallocator;

    /*************************************************************************
     * GRWL API functions
     *************************************************************************/

    /*! @brief Initializes the GRWL library.
     *
     *  This function initializes the GRWL library.  Before most GRWL functions can
     *  be used, GRWL must be initialized, and before an application terminates GRWL
     *  should be terminated in order to free any resources allocated during or
     *  after initialization.
     *
     *  If this function fails, it calls @ref grwlTerminate before returning.  If it
     *  succeeds, you should call @ref grwlTerminate before the application exits.
     *
     *  Additional calls to this function after successful initialization but before
     *  termination will return `true` immediately.
     *
     *  The @ref GRWL_PLATFORM init hint controls which platforms are considered during
     *  initialization.  This also depends on which platforms the library was compiled to
     *  support.
     *
     *  @return `true` if successful, or `false` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_PLATFORM_UNAVAILABLE and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @macos This function will change the current directory of the
     *  application to the `Contents/Resources` subdirectory of the application's
     *  bundle, if present.  This can be disabled with the @ref
     *  GRWL_COCOA_CHDIR_RESOURCES init hint.
     *
     *  @remark @macos This function will create the main menu and dock icon for the
     *  application.  If GRWL finds a `MainMenu.nib` it is loaded and assumed to
     *  contain a menu bar.  Otherwise a minimal menu bar is created manually with
     *  common commands like Hide, Quit and About.  The About entry opens a minimal
     *  about dialog with information from the application's bundle.  The menu bar
     *  and dock icon can be disabled entirely with the @ref GRWL_COCOA_MENUBAR init
     *  hint.
     *
     *  @remark @x11 This function will set the `LC_CTYPE` category of the
     *  application locale according to the current environment if that category is
     *  still "C".  This is because the "C" locale breaks Unicode text input.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref intro_init
     *  @sa @ref grwlInitHint
     *  @sa @ref grwlInitAllocator
     *  @sa @ref grwlTerminate
     *
     *  @ingroup init
     */
    GRWLAPI int grwlInit();

    /*! @brief Terminates the GRWL library.
     *
     *  This function destroys all remaining windows and cursors, restores any
     *  modified gamma ramps and frees any other allocated resources.  Once this
     *  function is called, you must again call @ref grwlInit successfully before
     *  you will be able to use most GRWL functions.
     *
     *  If GRWL has been successfully initialized, this function should be called
     *  before the application exits.  If initialization fails, there is no need to
     *  call this function, as it is called by @ref grwlInit before it returns
     *  failure.
     *
     *  This function has no effect if GRWL is not initialized.
     *
     *  @errors Possible errors include @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @warning The contexts of any remaining windows must not be current on any
     *  other thread when this function is called.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref intro_init
     *  @sa @ref grwlInit
     *
     *  @ingroup init
     */
    GRWLAPI void grwlTerminate();

    /*! @brief Sets the specified init hint to the desired value.
     *
     *  This function sets hints for the next initialization of GRWL.
     *
     *  The values you set hints to are never reset by GRWL, but they only take
     *  effect during initialization.  Once GRWL has been initialized, any values
     *  you set will be ignored until the library is terminated and initialized
     *  again.
     *
     *  Some hints are platform specific.  These may be set on any platform but they
     *  will only affect their specific platform.  Other platforms will ignore them.
     *  Setting these hints requires no platform specific headers or functions.
     *
     *  @param[in] hint The [init hint](@ref init_hints) to set.
     *  @param[in] value The new value of the init hint.
     *
     *  @errors Possible errors include @ref GRWL_INVALID_ENUM and @ref
     *  GRWL_INVALID_VALUE.
     *
     *  @remarks This function may be called before @ref grwlInit.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa init_hints
     *  @sa grwlInit
     *
     *  @ingroup init
     */
    GRWLAPI void grwlInitHint(int hint, int value);

    /*! @brief Sets the init allocator to the desired value.
     *
     *  To use the default allocator, call this function with a `NULL` argument.
     *
     *  If you specify an allocator struct, every member must be a valid function
     *  pointer.  If any member is `NULL`, this function emits @ref
     *  GRWL_INVALID_VALUE and the init allocator is unchanged.
     *
     *  @param[in] allocator The allocator to use at the next initialization, or
     *  `NULL` to use the default one.
     *
     *  @errors Possible errors include @ref GRWL_INVALID_VALUE.
     *
     *  @pointer_lifetime The specified allocator is copied before this function
     *  returns.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref init_allocator
     *  @sa @ref grwlInit
     *
     *  @ingroup init
     */
    GRWLAPI void grwlInitAllocator(const GRWLallocator* allocator);

#if defined(VK_VERSION_1_0)

    /*! @brief Sets the desired Vulkan `vkGetInstanceProcAddr` function.
     *
     *  This function sets the `vkGetInstanceProcAddr` function that GRWL will use for all
     *  Vulkan related entry point queries.
     *
     *  This feature is mostly useful on macOS, if your copy of the Vulkan loader is in
     *  a location where GRWL cannot find it through dynamic loading, or if you are still
     *  using the static library version of the loader.
     *
     *  If set to `NULL`, GRWL will try to load the Vulkan loader dynamically by its standard
     *  name and get this function from there.  This is the default behavior.
     *
     *  The standard name of the loader is `vulkan-1.dll` on Windows, `libvulkan.so.1` on
     *  Linux and other Unix-like systems and `libvulkan.1.dylib` on macOS.  If your code is
     *  also loading it via these names then you probably don't need to use this function.
     *
     *  The function address you set is never reset by GRWL, but it only takes effect during
     *  initialization.  Once GRWL has been initialized, any updates will be ignored until the
     *  library is terminated and initialized again.
     *
     *  @param[in] loader The address of the function to use, or `NULL`.
     *
     *  @par Loader function signature
     *  @code
     *  PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* name)
     *  @endcode
     *  For more information about this function, see the
     *  [Vulkan Registry](https://www.khronos.org/registry/vulkan/).
     *
     *  @errors None.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref vulkan_loader
     *  @sa @ref grwlInit
     *
     *  @ingroup init
     */
    GRWLAPI void grwlInitVulkanLoader(PFN_vkGetInstanceProcAddr loader);

#endif /*VK_VERSION_1_0*/

    /*! @brief Retrieves the version of the GRWL library.
     *
     *  This function retrieves the major, minor and revision numbers of the GRWL
     *  library.  It is intended for when you are using GRWL as a shared library and
     *  want to ensure that you are using the minimum required version.
     *
     *  Any or all of the version arguments may be `NULL`.
     *
     *  @param[out] major Where to store the major version number, or `NULL`.
     *  @param[out] minor Where to store the minor version number, or `NULL`.
     *  @param[out] rev Where to store the revision number, or `NULL`.
     *
     *  @errors None.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref intro_version
     *  @sa @ref grwlGetVersionString
     *
     *  @ingroup init
     */
    GRWLAPI void grwlGetVersion(int* major, int* minor, int* rev);

    /*! @brief Returns a string describing the compile-time configuration.
     *
     *  This function returns the compile-time generated
     *  [version string](@ref intro_version_string) of the GRWL library binary.  It describes
     *  the version, platforms, compiler and any platform or operating system specific
     *  compile-time options.  It should not be confused with the OpenGL or OpenGL ES version
     *  string, queried with `glGetString`.
     *
     *  __Do not use the version string__ to parse the GRWL library version.  The
     *  @ref grwlGetVersion function provides the version of the running library
     *  binary in numerical format.
     *
     *  __Do not use the version string__ to parse what platforms are supported.  The @ref
     *  grwlPlatformSupported function lets you query platform support.
     *
     *  @return The ASCII encoded GRWL version string.
     *
     *  @errors None.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @pointer_lifetime The returned string is static and compile-time generated.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref intro_version
     *  @sa @ref grwlGetVersion
     *
     *  @ingroup init
     */
    GRWLAPI const char* grwlGetVersionString();

    /*! @brief Returns and clears the last error for the calling thread.
     *
     *  This function returns and clears the [error code](@ref errors) of the last
     *  error that occurred on the calling thread, and optionally a UTF-8 encoded
     *  human-readable description of it.  If no error has occurred since the last
     *  call, it returns @ref GRWL_NO_ERROR (zero) and the description pointer is
     *  set to `NULL`.
     *
     *  @param[in] description Where to store the error description pointer, or `NULL`.
     *  @return The last error code for the calling thread, or @ref GRWL_NO_ERROR
     *  (zero).
     *
     *  @errors None.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is guaranteed to be valid only until the
     *  next error occurs or the library is terminated.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref error_handling
     *  @sa @ref grwlSetErrorCallback
     *
     *  @ingroup init
     */
    GRWLAPI int grwlGetError(const char** description);

    /*! @brief Returns the initialization state.
     *   This function returns whether the library has been initialized or not.
     *   @return `true` if initialized, or `false` otherwise.
     *
     *   @errors None.
     *
     *   @remark This function may be called before @ref grwlInit.
     *
     *   @thread_safety This function may be called from any thread.
     *
     *   @ingroup init
     */
    GRWLAPI int grwlIsInitialized();

    /*! @brief Sets the error callback.
     *
     *  This function sets the error callback, which is called with an error code
     *  and a human-readable description each time a GRWL error occurs.
     *
     *  The error code is set before the callback is called.  Calling @ref
     *  grwlGetError from the error callback will return the same value as the error
     *  code argument.
     *
     *  The error callback is called on the thread where the error occurred.  If you
     *  are using GRWL from multiple threads, your error callback needs to be
     *  written accordingly.
     *
     *  Because the description string may have been generated specifically for that
     *  error, it is not guaranteed to be valid after the callback has returned.  If
     *  you wish to use it after the callback returns, you need to make a copy.
     *
     *  Once set, the error callback remains set even after the library has been
     *  terminated.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set.
     *
     *  @callback_signature
     *  @code
     *  void callback_name(int error_code, const char* description)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [callback pointer type](@ref GRWLerrorfun).
     *
     *  @errors None.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref error_handling
     *  @sa @ref grwlGetError
     *
     *  @ingroup init
     */

    GRWLAPI GRWLerrorfun grwlSetErrorCallback(GRWLerrorfun callback);

    /*! @brief Returns the currently selected platform.
     *
     *  This function returns the platform that was selected during initialization.  The
     *  returned value will be one of `GRWL_PLATFORM_WIN32`, `GRWL_PLATFORM_COCOA`,
     *  `GRWL_PLATFORM_WAYLAND`, `GRWL_PLATFORM_X11` or `GRWL_PLATFORM_NULL`.
     *
     *  @return The currently selected platform, or zero if an error occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref platform
     *  @sa @ref grwlPlatformSupported
     *
     *  @ingroup init
     */
    GRWLAPI int grwlGetPlatform();

    /*! @brief Returns whether the library includes support for the specified platform.
     *
     *  This function returns whether the library was compiled with support for the specified
     *  platform.  The platform must be one of `GRWL_PLATFORM_WIN32`, `GRWL_PLATFORM_COCOA`,
     *  `GRWL_PLATFORM_WAYLAND`, `GRWL_PLATFORM_X11` or `GRWL_PLATFORM_NULL`.
     *
     *  @param[in] platform The platform to query.
     *  @return `true` if the platform is supported, or `false` otherwise.
     *
     *  @errors Possible errors include @ref GRWL_INVALID_ENUM.
     *
     *  @remark This function may be called before @ref grwlInit.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref platform
     *  @sa @ref grwlGetPlatform
     *
     *  @ingroup init
     */
    GRWLAPI int grwlPlatformSupported(int platform);

    /*! @brief Returns the currently connected monitors.
     *
     *  This function returns an array of handles for all currently connected
     *  monitors.  The primary monitor is always first in the returned array.  If no
     *  monitors were found, this function returns `NULL`.
     *
     *  @param[out] count Where to store the number of monitors in the returned
     *  array.  This is set to zero if an error occurred.
     *  @return An array of monitor handles, or `NULL` if no monitors were found or
     *  if an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is guaranteed to be valid only until the
     *  monitor configuration changes or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_monitors
     *  @sa @ref monitor_event
     *  @sa @ref grwlGetPrimaryMonitor
     *
     *  @ingroup monitor
     */
    GRWLAPI GRWLmonitor** grwlGetMonitors(int* count);

    /*! @brief Returns the primary monitor.
     *
     *  This function returns the primary monitor.  This is usually the monitor
     *  where elements like the task bar or global menu bar are located.
     *
     *  @return The primary monitor, or `NULL` if no monitors were found or if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @remark The primary monitor is always first in the array returned by @ref
     *  grwlGetMonitors.
     *
     *  @sa @ref monitor_monitors
     *  @sa @ref grwlGetMonitors
     *
     *  @ingroup monitor
     */
    GRWLAPI GRWLmonitor* grwlGetPrimaryMonitor();

    /*! @brief Returns the position of the monitor's viewport on the virtual screen.
     *
     *  This function returns the position, in screen coordinates, of the upper-left
     *  corner of the specified monitor.
     *
     *  Any or all of the position arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` position arguments will be set to zero.
     *
     *  @param[in] monitor The monitor to query.
     *  @param[out] xpos Where to store the monitor x-coordinate, or `NULL`.
     *  @param[out] ypos Where to store the monitor y-coordinate, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_properties
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlGetMonitorPos(GRWLmonitor* monitor, int* xpos, int* ypos);

    /*! @brief Retrieves the work area of the monitor.
     *
     *  This function returns the position, in screen coordinates, of the upper-left
     *  corner of the work area of the specified monitor along with the work area
     *  size in screen coordinates. The work area is defined as the area of the
     *  monitor not occluded by the window system task bar where present. If no
     *  task bar exists then the work area is the monitor resolution in screen
     *  coordinates.
     *
     *  Any or all of the position and size arguments may be `NULL`.  If an error
     *  occurs, all non-`NULL` position and size arguments will be set to zero.
     *
     *  @param[in] monitor The monitor to query.
     *  @param[out] xpos Where to store the monitor x-coordinate, or `NULL`.
     *  @param[out] ypos Where to store the monitor y-coordinate, or `NULL`.
     *  @param[out] width Where to store the monitor width, or `NULL`.
     *  @param[out] height Where to store the monitor height, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_workarea
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlGetMonitorWorkarea(GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height);

    /*! @brief Returns the physical size of the monitor.
     *
     *  This function returns the size, in millimetres, of the display area of the
     *  specified monitor.
     *
     *  Some platforms do not provide accurate monitor size information, either
     *  because the monitor
     *  [EDID](https://en.wikipedia.org/wiki/Extended_display_identification_data)
     *  data is incorrect or because the driver does not report it accurately.
     *
     *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` size arguments will be set to zero.
     *
     *  @param[in] monitor The monitor to query.
     *  @param[out] widthMM Where to store the width, in millimetres, of the
     *  monitor's display area, or `NULL`.
     *  @param[out] heightMM Where to store the height, in millimetres, of the
     *  monitor's display area, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @remark @win32 On Windows 8 and earlier the physical size is calculated from
     *  the current resolution and system DPI instead of querying the monitor EDID data.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_properties
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlGetMonitorPhysicalSize(GRWLmonitor* monitor, int* widthMM, int* heightMM);

    /*! @brief Retrieves the content scale for the specified monitor.
     *
     *  This function retrieves the content scale for the specified monitor.  The
     *  content scale is the ratio between the current DPI and the platform's
     *  default DPI.  This is especially important for text and any UI elements.  If
     *  the pixel dimensions of your UI scaled by this look appropriate on your
     *  machine then it should appear at a reasonable size on other machines
     *  regardless of their DPI and scaling settings.  This relies on the system DPI
     *  and scaling settings being somewhat correct.
     *
     *  The content scale may depend on both the monitor resolution and pixel
     *  density and on user settings.  It may be very different from the raw DPI
     *  calculated from the physical size and current resolution.
     *
     *  @param[in] monitor The monitor to query.
     *  @param[out] xscale Where to store the x-axis content scale, or `NULL`.
     *  @param[out] yscale Where to store the y-axis content scale, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_scale
     *  @sa @ref grwlGetWindowContentScale
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlGetMonitorContentScale(GRWLmonitor* monitor, float* xscale, float* yscale);

    /*! @brief Returns the name of the specified monitor.
     *
     *  This function returns a human-readable name, encoded as UTF-8, of the
     *  specified monitor.  The name typically reflects the make and model of the
     *  monitor and is not guaranteed to be unique among the connected monitors.
     *
     *  @param[in] monitor The monitor to query.
     *  @return The UTF-8 encoded name of the monitor, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified monitor is
     *  disconnected or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_properties
     *
     *  @ingroup monitor
     */
    GRWLAPI const char* grwlGetMonitorName(GRWLmonitor* monitor);

    /*! @brief Sets the user pointer of the specified monitor.
     *
     *  This function sets the user-defined pointer of the specified monitor.  The
     *  current value is retained until the monitor is disconnected.  The initial
     *  value is `NULL`.
     *
     *  This function may be called from the monitor callback, even for a monitor
     *  that is being disconnected.
     *
     *  @param[in] monitor The monitor whose pointer to set.
     *  @param[in] pointer The new value.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref monitor_userptr
     *  @sa @ref grwlGetMonitorUserPointer
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlSetMonitorUserPointer(GRWLmonitor* monitor, void* pointer);

    /*! @brief Returns the user pointer of the specified monitor.
     *
     *  This function returns the current value of the user-defined pointer of the
     *  specified monitor.  The initial value is `NULL`.
     *
     *  This function may be called from the monitor callback, even for a monitor
     *  that is being disconnected.
     *
     *  @param[in] monitor The monitor whose pointer to return.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref monitor_userptr
     *  @sa @ref grwlSetMonitorUserPointer
     *
     *  @ingroup monitor
     */
    GRWLAPI void* grwlGetMonitorUserPointer(GRWLmonitor* monitor);

    /*! @brief Sets the monitor configuration callback.
     *
     *  This function sets the monitor configuration callback, or removes the
     *  currently set callback.  This is called when a monitor is connected to or
     *  disconnected from the system.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLmonitor* monitor, int event)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLmonitorfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_event
     *
     *  @ingroup monitor
     */
    GRWLAPI GRWLmonitorfun grwlSetMonitorCallback(GRWLmonitorfun callback);

    /*! @brief Returns the available video modes for the specified monitor.
     *
     *  This function returns an array of all video modes supported by the specified
     *  monitor.  The returned array is sorted in ascending order, first by color
     *  bit depth (the sum of all channel depths), then by resolution area (the
     *  product of width and height), then resolution width and finally by refresh
     *  rate.
     *
     *  @param[in] monitor The monitor to query.
     *  @param[out] count Where to store the number of video modes in the returned
     *  array.  This is set to zero if an error occurred.
     *  @return An array of video modes, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified monitor is
     *  disconnected, this function is called again for that monitor or the library
     *  is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_modes
     *  @sa @ref grwlGetVideoMode
     *
     *  @ingroup monitor
     */
    GRWLAPI const GRWLvidmode* grwlGetVideoModes(GRWLmonitor* monitor, int* count);

    /*! @brief Returns the current mode of the specified monitor.
     *
     *  This function returns the current video mode of the specified monitor.  If
     *  you have created a full screen window for that monitor, the return value
     *  will depend on whether that window is iconified.
     *
     *  @param[in] monitor The monitor to query.
     *  @return The current mode of the monitor, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified monitor is
     *  disconnected or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_modes
     *  @sa @ref grwlGetVideoModes
     *
     *  @ingroup monitor
     */
    GRWLAPI const GRWLvidmode* grwlGetVideoMode(GRWLmonitor* monitor);

    /*! @brief Generates a gamma ramp and sets it for the specified monitor.
     *
     *  This function generates an appropriately sized gamma ramp from the specified
     *  exponent and then calls @ref grwlSetGammaRamp with it.  The value must be
     *  a finite number greater than zero.
     *
     *  The software controlled gamma ramp is applied _in addition_ to the hardware
     *  gamma correction, which today is usually an approximation of sRGB gamma.
     *  This means that setting a perfectly linear ramp, or gamma 1.0, will produce
     *  the default (usually sRGB-like) behavior.
     *
     *  For gamma correct rendering with OpenGL or OpenGL ES, see the @ref
     *  GRWL_SRGB_CAPABLE hint.
     *
     *  @param[in] monitor The monitor whose gamma ramp to set.
     *  @param[in] gamma The desired exponent.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref GRWL_INVALID_VALUE,
     *  @ref GRWL_PLATFORM_ERROR and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland Gamma handling is a privileged protocol, this function
     *  will thus never be implemented and emits @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_gamma
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlSetGamma(GRWLmonitor* monitor, float gamma);

    /*! @brief Returns the current gamma ramp for the specified monitor.
     *
     *  This function returns the current gamma ramp of the specified monitor.
     *
     *  @param[in] monitor The monitor to query.
     *  @return The current gamma ramp, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref GRWL_PLATFORM_ERROR
     *  and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland Gamma handling is a privileged protocol, this function
     *  will thus never be implemented and emits @ref GRWL_FEATURE_UNAVAILABLE while
     *  returning `NULL`.
     *
     *  @pointer_lifetime The returned structure and its arrays are allocated and
     *  freed by GRWL.  You should not free them yourself.  They are valid until the
     *  specified monitor is disconnected, this function is called again for that
     *  monitor or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_gamma
     *
     *  @ingroup monitor
     */
    GRWLAPI const GRWLgammaramp* grwlGetGammaRamp(GRWLmonitor* monitor);

    /*! @brief Sets the current gamma ramp for the specified monitor.
     *
     *  This function sets the current gamma ramp for the specified monitor.  The
     *  original gamma ramp for that monitor is saved by GRWL the first time this
     *  function is called and is restored by @ref grwlTerminate.
     *
     *  The software controlled gamma ramp is applied _in addition_ to the hardware
     *  gamma correction, which today is usually an approximation of sRGB gamma.
     *  This means that setting a perfectly linear ramp, or gamma 1.0, will produce
     *  the default (usually sRGB-like) behavior.
     *
     *  For gamma correct rendering with OpenGL or OpenGL ES, see the @ref
     *  GRWL_SRGB_CAPABLE hint.
     *
     *  @param[in] monitor The monitor whose gamma ramp to set.
     *  @param[in] ramp The gamma ramp to use.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref GRWL_PLATFORM_ERROR
     *  and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark The size of the specified gamma ramp should match the size of the
     *  current ramp for that monitor.
     *
     *  @remark @win32 The gamma ramp size must be 256.
     *
     *  @remark @wayland Gamma handling is a privileged protocol, this function
     *  will thus never be implemented and emits @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @pointer_lifetime The specified gamma ramp is copied before this function
     *  returns.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref monitor_gamma
     *
     *  @ingroup monitor
     */
    GRWLAPI void grwlSetGammaRamp(GRWLmonitor* monitor, const GRWLgammaramp* ramp);

    /*! @brief Resets all window hints to their default values.
     *
     *  This function resets all window hints to their
     *  [default values](@ref window_hints_values).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_hints
     *  @sa @ref grwlWindowHint
     *  @sa @ref grwlWindowHintString
     *
     *  @ingroup window
     */
    GRWLAPI void grwlDefaultWindowHints();

    /*! @brief Sets the specified window hint to the desired value.
     *
     *  This function sets hints for the next call to @ref grwlCreateWindow.  The
     *  hints, once set, retain their values until changed by a call to this
     *  function or @ref grwlDefaultWindowHints, or until the library is terminated.
     *
     *  Only integer value hints can be set with this function.  String value hints
     *  are set with @ref grwlWindowHintString.
     *
     *  This function does not check whether the specified hint values are valid.
     *  If you set hints to invalid values this will instead be reported by the next
     *  call to @ref grwlCreateWindow.
     *
     *  Some hints are platform specific.  These may be set on any platform but they
     *  will only affect their specific platform.  Other platforms will ignore them.
     *  Setting these hints requires no platform specific headers or functions.
     *
     *  @param[in] hint The [window hint](@ref window_hints) to set.
     *  @param[in] value The new value of the window hint.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_hints
     *  @sa @ref grwlWindowHintString
     *  @sa @ref grwlDefaultWindowHints
     *
     *  @ingroup window
     */
    GRWLAPI void grwlWindowHint(int hint, int value);

    /*! @brief Sets the specified window hint to the desired value.
     *
     *  This function sets hints for the next call to @ref grwlCreateWindow.  The
     *  hints, once set, retain their values until changed by a call to this
     *  function or @ref grwlDefaultWindowHints, or until the library is terminated.
     *
     *  Only string type hints can be set with this function.  Integer value hints
     *  are set with @ref grwlWindowHint.
     *
     *  This function does not check whether the specified hint values are valid.
     *  If you set hints to invalid values this will instead be reported by the next
     *  call to @ref grwlCreateWindow.
     *
     *  Some hints are platform specific.  These may be set on any platform but they
     *  will only affect their specific platform.  Other platforms will ignore them.
     *  Setting these hints requires no platform specific headers or functions.
     *
     *  @param[in] hint The [window hint](@ref window_hints) to set.
     *  @param[in] value The new value of the window hint.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @pointer_lifetime The specified string is copied before this function
     *  returns.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_hints
     *  @sa @ref grwlWindowHint
     *  @sa @ref grwlDefaultWindowHints
     *
     *  @ingroup window
     */
    GRWLAPI void grwlWindowHintString(int hint, const char* value);

    /*! @brief Creates a window and its associated context.
     *
     *  This function creates a window and its associated OpenGL or OpenGL ES
     *  context.  Most of the options controlling how the window and its context
     *  should be created are specified with [window hints](@ref window_hints).
     *
     *  Successful creation does not change which context is current.  Before you
     *  can use the newly created context, you need to
     *  [make it current](@ref context_current).  For information about the `share`
     *  parameter, see @ref context_sharing.
     *
     *  The created window, framebuffer and context may differ from what you
     *  requested, as not all parameters and hints are
     *  [hard constraints](@ref window_hints_hard).  This includes the size of the
     *  window, especially for full screen windows.  To query the actual attributes
     *  of the created window, framebuffer and context, see @ref
     *  grwlGetWindowAttrib, @ref grwlGetWindowSize and @ref grwlGetFramebufferSize.
     *
     *  To create a full screen window, you need to specify the monitor the window
     *  will cover.  If no monitor is specified, the window will be windowed mode.
     *  Unless you have a way for the user to choose a specific monitor, it is
     *  recommended that you pick the primary monitor.  For more information on how
     *  to query connected monitors, see @ref monitor_monitors.
     *
     *  For full screen windows, the specified size becomes the resolution of the
     *  window's _desired video mode_.  As long as a full screen window is not
     *  iconified, the supported video mode most closely matching the desired video
     *  mode is set for the specified monitor.  For more information about full
     *  screen windows, including the creation of so called _windowed full screen_
     *  or _borderless full screen_ windows, see @ref window_windowed_full_screen.
     *
     *  Once you have created the window, you can switch it between windowed and
     *  full screen mode with @ref grwlSetWindowMonitor.  This will not affect its
     *  OpenGL or OpenGL ES context.
     *
     *  By default, newly created windows use the placement recommended by the
     *  window system.  To create the window at a specific position, set the @ref
     *  GRWL_POSITION_X and @ref GRWL_POSITION_Y window hints before creation.  To
     *  restore the default behavior, set either or both hints back to
     *  `GRWL_ANY_POSITION`.
     *
     *  As long as at least one full screen window is not iconified, the screensaver
     *  is prohibited from starting.
     *
     *  Window systems put limits on window sizes.  Very large or very small window
     *  dimensions may be overridden by the window system on creation.  Check the
     *  actual [size](@ref window_size) after creation.
     *
     *  The [swap interval](@ref buffer_swap) is not set during window creation and
     *  the initial value may vary depending on driver settings and defaults.
     *
     *  @param[in] width The desired width, in screen coordinates, of the window.
     *  This must be greater than zero.
     *  @param[in] height The desired height, in screen coordinates, of the window.
     *  This must be greater than zero.
     *  @param[in] title The initial, UTF-8 encoded window title.
     *  @param[in] monitor The monitor to use for full screen mode, or `NULL` for
     *  windowed mode.
     *  @param[in] share The window whose context to share resources with, or `NULL`
     *  to not share resources.
     *  @return The handle of the created window, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM, @ref GRWL_INVALID_VALUE, @ref GRWL_API_UNAVAILABLE, @ref
     *  GRWL_VERSION_UNAVAILABLE, @ref GRWL_FORMAT_UNAVAILABLE and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @win32 Window creation will fail if the Microsoft GDI software
     *  OpenGL implementation is the only one available.
     *
     *  @remark @win32 If the executable has an icon resource named `GRWL_ICON,` it
     *  will be set as the initial icon for the window.  If no such icon is present,
     *  the `IDI_APPLICATION` icon will be used instead.  To set a different icon,
     *  see @ref grwlSetWindowIcon.
     *
     *  @remark @win32 The context to share resources with must not be current on
     *  any other thread.
     *
     *  @remark @macos The OS only supports core profile contexts for OpenGL
     *  versions 3.2 and later.  Before creating an OpenGL context of version 3.2 or
     *  later you must set the [GRWL_OPENGL_PROFILE](@ref GRWL_OPENGL_PROFILE_hint)
     *  hint accordingly.  OpenGL 3.0 and 3.1 contexts are not supported at all
     *  on macOS.
     *
     *  @remark @macos The GRWL window has no icon, as it is not a document
     *  window, but the dock icon will be the same as the application bundle's icon.
     *  For more information on bundles, see the
     *  [Bundle Programming
     * Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/) in the Mac
     * Developer Library.
     *
     *  @remark @macos On OS X 10.10 and later the window frame will not be rendered
     *  at full resolution on Retina displays unless the
     *  [GRWL_COCOA_RETINA_FRAMEBUFFER](@ref GRWL_COCOA_RETINA_FRAMEBUFFER_hint)
     *  hint is `true` and the `NSHighResolutionCapable` key is enabled in the
     *  application bundle's `Info.plist`.  For more information, see
     *  [High Resolution Guidelines for OS
     * X](https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Explained/Explained.html)
     *  in the Mac Developer Library.  The GRWL test and example programs use
     *  a custom `Info.plist` template for this, which can be found as
     *  `CMake/Info.plist.in` in the source tree.
     *
     *  @remark @macos When activating frame autosaving with
     *  [GRWL_COCOA_FRAME_NAME](@ref GRWL_COCOA_FRAME_NAME_hint), the specified
     *  window size and position may be overridden by previously saved values.
     *
     *  @remark @x11 Some window managers will not respect the placement of
     *  initially hidden windows.
     *
     *  @remark @x11 Due to the asynchronous nature of X11, it may take a moment for
     *  a window to reach its requested state.  This means you may not be able to
     *  query the final size, position or other attributes directly after window
     *  creation.
     *
     *  @remark @x11 The class part of the `WM_CLASS` window property will by
     *  default be set to the window title passed to this function.  The instance
     *  part will use the contents of the `RESOURCE_NAME` environment variable, if
     *  present and not empty, or fall back to the window title.  Set the
     *  [GRWL_X11_CLASS_NAME](@ref GRWL_X11_CLASS_NAME_hint) and
     *  [GRWL_X11_INSTANCE_NAME](@ref GRWL_X11_INSTANCE_NAME_hint) window hints to
     *  override this.
     *
     *  @remark @wayland Compositors should implement the xdg-decoration protocol
     *  for GRWL to decorate the window properly.  If this protocol isn't
     *  supported, or if the compositor prefers client-side decorations, a very
     *  simple fallback frame will be drawn using the wp_viewporter protocol.  A
     *  compositor can still emit close, maximize or fullscreen events, using for
     *  instance a keybind mechanism.  If neither of these protocols is supported,
     *  the window won't be decorated.
     *
     *  @remark @wayland A full screen window will not attempt to change the mode,
     *  no matter what the requested size or refresh rate.
     *
     *  @remark @wayland Screensaver inhibition requires the idle-inhibit protocol
     *  to be implemented in the user's compositor.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_creation
     *  @sa @ref grwlDestroyWindow
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindow* grwlCreateWindow(int width, int height, const char* title, GRWLmonitor* monitor,
                                         GRWLwindow* share);

    /*! @brief Destroys the specified window and its context.
     *
     *  This function destroys the specified window and its context.  On calling
     *  this function, no further callbacks will be called for that window.
     *
     *  If the context of the specified window is current on the main thread, it is
     *  detached before being destroyed.
     *
     *  @param[in] window The window to destroy.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @note The context of the specified window must not be current on any other
     *  thread when this function is called.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_creation
     *  @sa @ref grwlCreateWindow
     *
     *  @ingroup window
     */
    GRWLAPI void grwlDestroyWindow(GRWLwindow* window);

    /*! @brief Checks the close flag of the specified window.
     *
     *  This function returns the value of the close flag of the specified window.
     *
     *  @param[in] window The window to query.
     *  @return The value of the close flag.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref window_close
     *
     *  @ingroup window
     */
    GRWLAPI int grwlWindowShouldClose(GRWLwindow* window);

    /*! @brief Sets the close flag of the specified window.
     *
     *  This function sets the value of the close flag of the specified window.
     *  This can be used to override the user's attempt to close the window, or
     *  to signal that it should be closed.
     *
     *  @param[in] window The window whose flag to change.
     *  @param[in] value The new value.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref window_close
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowShouldClose(GRWLwindow* window, int value);

    /*! @brief Sets the title of the specified window.
     *
     *  This function sets the window title, encoded as UTF-8, of the specified
     *  window.
     *
     *  @param[in] window The window whose title to change.
     *  @param[in] title The UTF-8 encoded window title.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @macos The window title will not be updated until the next time you
     *  process events.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_title
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowTitle(GRWLwindow* window, const char* title);

    /*! @brief Sets the icon for the specified window.
     *
     *  This function sets the icon of the specified window.  If passed an array of
     *  candidate images, those of or closest to the sizes desired by the system are
     *  selected.  If no images are specified, the window reverts to its default
     *  icon.
     *
     *  The pixels are 32-bit, little-endian, non-premultiplied RGBA, i.e. eight
     *  bits per channel with the red channel first.  They are arranged canonically
     *  as packed sequential rows, starting from the top-left corner.
     *
     *  The desired image sizes varies depending on platform and system settings.
     *  The selected images will be rescaled as needed.  Good sizes include 16x16,
     *  32x32 and 48x48.
     *
     *  @param[in] window The window whose icon to set.
     *  @param[in] count The number of images in the specified array, or zero to
     *  revert to the default window icon.
     *  @param[in] images The images to create the icon from.  This is ignored if
     *  count is zero.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_VALUE, @ref GRWL_PLATFORM_ERROR and @ref
     *  GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @pointer_lifetime The specified image data is copied before this function
     *  returns.
     *
     *  @remark @macos Regular windows do not have icons on macOS.  This function
     *  will emit @ref GRWL_FEATURE_UNAVAILABLE.  The dock icon will be the same as
     *  the application bundle's icon.  For more information on bundles, see the
     *  [Bundle Programming
     * Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/) in the Mac
     * Developer Library.
     *
     *  @remark @wayland There is no existing protocol to change an icon, the
     *  window will thus inherit the one defined in the application's desktop file.
     *  This function will emit @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_icon
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowIcon(GRWLwindow* window, int count, const GRWLimage* images);

    /*! @brief Sets the dock or taskbar progress indicator for the specified window.
     *
     *  This function sets the dock or taskbar progress indicator of the specified window.
     *
     *  @param[in] window The window whose progress to set.
     *  @param[in] progressState The state of the progress to be displayed in the dock
     *  or taskbar. Valid values are: @ref GRWL_PROGRESS_INDICATOR_DISABLED,
     *  @ref GRWL_PROGRESS_INDICATOR_INDETERMINATE, @ref GRWL_PROGRESS_INDICATOR_NORMAL,
     *  @ref GRWL_PROGRESS_INDICATOR_ERROR and @ref GRWL_PROGRESS_INDICATOR_PAUSED.
     *  @param[in] value The amount of completed progress to set. Valid range is 0.0 to 1.0.
     *  This is ignored if progressState is set to @ref GRWL_PROGRESS_INDICATOR_DISABLED.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_VALUE, @ref GRWL_INVALID_ENUM, @ref GRWL_PLATFORM_ERROR,
     *  @ref GRWL_FEATURE_UNIMPLEMENTED and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @win32 On Windows Vista and earlier, this function will emit
     *  @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @remark @macos There exists only one Dock icon progress bar, and this
     *  displays the combined values of all the windows.
     *
     *  @remark @x11 @wayland Requires a valid application desktop file with the same name
     *  as the compiled executable. Due to limitations in the Unity Launcher API
     *  @ref GRWL_PROGRESS_INDICATOR_INDETERMINATE, @ref GRWL_PROGRESS_INDICATOR_ERROR
     *  and @ref GRWL_PROGRESS_INDICATOR_PAUSED have the same behaviour as
     *  @ref GRWL_PROGRESS_INDICATOR_NORMAL. The Unity Launcher API is only known
     *  to be supported on the Unity and KDE desktop environments; on other desktop
     *  environments this function may do nothing.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_progress_indicator
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowProgressIndicator(GRWLwindow* window, int progressState, double value);

    /*! @brief Sets the dock or taskbar badge for the specified window or the application.
     *
     *  This function sets the dock or taskbar badge for the specified window
     *  or the application as a whole.  Any badge set with this function
     *  overrides the string badge set with @ref grwlSetWindowBadgeString.
     *  If the platform does not support number badges, the string badge
     *  is not overridden.
     *
     *  @param[in] window The window whose badge to set.
     *  @param[in] count The number to set, or `0` to disable it.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR, and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @win32 On Windows Vista and earlier, this function will emit
     *  @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @remark @macos Only the Dock icon may contain a badge. Pass a `NULL`
     *  window handle to set it.  Emits @ref GRWL_FEATURE_UNAVAILABLE if a
     *  valid window handle is passed.
     *
     *  @remark @x11 @wayland @win32 Emits GRWL_FEATURE_UNAVAILABLE if a
     *  `NULL` window handle is passed.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowBadge(GRWLwindow* window, int count);

    /*! @brief Sets the dock or taskbar badge for the specified window or the application.
     *
     *  This function sets the dock or taskbar badge for the specified window
     *  or the application as a whole.  Any string badge set with this function
     *  overrides the number badge set with @ref grwlSetWindowBadge.
     *  If the platform does not support string badges, the number badge
     *  is not overridden.
     *
     *  @param[in] window The window whose badge to set.
     *  @param[in] string The text to set, or `NULL` to disable it.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR, and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @macos Only the Dock icon may contain a badge. Pass a `NULL`
     *  window handle to set it.  Emits @ref GRWL_FEATURE_UNAVAILABLE if a
     *  valid window handle is passed.
     *
     *  @remark @x11 @wayland @win32 Emits GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowBadgeString(GRWLwindow* window, const char* string);

    /*! @brief Retrieves the position of the content area of the specified window.
     *
     *  This function retrieves the position, in screen coordinates, of the
     *  upper-left corner of the content area of the specified window.
     *
     *  Any or all of the position arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` position arguments will be set to zero.
     *
     *  @param[in] window The window to query.
     *  @param[out] xpos Where to store the x-coordinate of the upper-left corner of
     *  the content area, or `NULL`.
     *  @param[out] ypos Where to store the y-coordinate of the upper-left corner of
     *  the content area, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland There is no way for an application to retrieve the global
     *  position of its windows.  This function will emit @ref
     *  GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_pos
     *  @sa @ref grwlSetWindowPos
     *
     *  @ingroup window
     */
    GRWLAPI void grwlGetWindowPos(GRWLwindow* window, int* xpos, int* ypos);

    /*! @brief Sets the position of the content area of the specified window.
     *
     *  This function sets the position, in screen coordinates, of the upper-left
     *  corner of the content area of the specified windowed mode window.  If the
     *  window is a full screen window, this function does nothing.
     *
     *  __Do not use this function__ to move an already visible window unless you
     *  have very good reasons for doing so, as it will confuse and annoy the user.
     *
     *  The window manager may put limits on what positions are allowed.  GRWL
     *  cannot and should not override these limits.
     *
     *  @param[in] window The window to query.
     *  @param[in] xpos The x-coordinate of the upper-left corner of the content area.
     *  @param[in] ypos The y-coordinate of the upper-left corner of the content area.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland There is no way for an application to set the global
     *  position of its windows.  This function will emit @ref
     *  GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_pos
     *  @sa @ref grwlGetWindowPos
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowPos(GRWLwindow* window, int xpos, int ypos);

    /*! @brief Retrieves the size of the content area of the specified window.
     *
     *  This function retrieves the size, in screen coordinates, of the content area
     *  of the specified window.  If you wish to retrieve the size of the
     *  framebuffer of the window in pixels, see @ref grwlGetFramebufferSize.
     *
     *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` size arguments will be set to zero.
     *
     *  @param[in] window The window whose size to retrieve.
     *  @param[out] width Where to store the width, in screen coordinates, of the
     *  content area, or `NULL`.
     *  @param[out] height Where to store the height, in screen coordinates, of the
     *  content area, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_size
     *  @sa @ref grwlSetWindowSize
     *
     *  @ingroup window
     */
    GRWLAPI void grwlGetWindowSize(GRWLwindow* window, int* width, int* height);

    /*! @brief Sets the size limits of the specified window.
     *
     *  This function sets the size limits of the content area of the specified
     *  window.  If the window is full screen, the size limits only take effect
     *  once it is made windowed.  If the window is not resizable, this function
     *  does nothing.
     *
     *  The size limits are applied immediately to a windowed mode window and may
     *  cause it to be resized.
     *
     *  The maximum dimensions must be greater than or equal to the minimum
     *  dimensions and all must be greater than or equal to zero.
     *
     *  @param[in] window The window to set limits for.
     *  @param[in] minwidth The minimum width, in screen coordinates, of the content
     *  area, or `GRWL_DONT_CARE`.
     *  @param[in] minheight The minimum height, in screen coordinates, of the
     *  content area, or `GRWL_DONT_CARE`.
     *  @param[in] maxwidth The maximum width, in screen coordinates, of the content
     *  area, or `GRWL_DONT_CARE`.
     *  @param[in] maxheight The maximum height, in screen coordinates, of the
     *  content area, or `GRWL_DONT_CARE`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_VALUE and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark If you set size limits and an aspect ratio that conflict, the
     *  results are undefined.
     *
     *  @remark @wayland The size limits will not be applied until the window is
     *  actually resized, either by the user or by the compositor.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_sizelimits
     *  @sa @ref grwlSetWindowAspectRatio
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowSizeLimits(GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);

    /*! @brief Sets the aspect ratio of the specified window.
     *
     *  This function sets the required aspect ratio of the content area of the
     *  specified window.  If the window is full screen, the aspect ratio only takes
     *  effect once it is made windowed.  If the window is not resizable, this
     *  function does nothing.
     *
     *  The aspect ratio is specified as a numerator and a denominator and both
     *  values must be greater than zero.  For example, the common 16:9 aspect ratio
     *  is specified as 16 and 9, respectively.
     *
     *  If the numerator and denominator is set to `GRWL_DONT_CARE` then the aspect
     *  ratio limit is disabled.
     *
     *  The aspect ratio is applied immediately to a windowed mode window and may
     *  cause it to be resized.
     *
     *  @param[in] window The window to set limits for.
     *  @param[in] numer The numerator of the desired aspect ratio, or
     *  `GRWL_DONT_CARE`.
     *  @param[in] denom The denominator of the desired aspect ratio, or
     *  `GRWL_DONT_CARE`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_VALUE and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark If you set size limits and an aspect ratio that conflict, the
     *  results are undefined.
     *
     *  @remark @wayland The aspect ratio will not be applied until the window is
     *  actually resized, either by the user or by the compositor.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_sizelimits
     *  @sa @ref grwlSetWindowSizeLimits
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowAspectRatio(GRWLwindow* window, int numer, int denom);

    /*! @brief Sets the size of the content area of the specified window.
     *
     *  This function sets the size, in screen coordinates, of the content area of
     *  the specified window.
     *
     *  For full screen windows, this function updates the resolution of its desired
     *  video mode and switches to the video mode closest to it, without affecting
     *  the window's context.  As the context is unaffected, the bit depths of the
     *  framebuffer remain unchanged.
     *
     *  If you wish to update the refresh rate of the desired video mode in addition
     *  to its resolution, see @ref grwlSetWindowMonitor.
     *
     *  The window manager may put limits on what sizes are allowed.  GRWL cannot
     *  and should not override these limits.
     *
     *  @param[in] window The window to resize.
     *  @param[in] width The desired width, in screen coordinates, of the window
     *  content area.
     *  @param[in] height The desired height, in screen coordinates, of the window
     *  content area.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @wayland A full screen window will not attempt to change the mode,
     *  no matter what the requested size.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_size
     *  @sa @ref grwlGetWindowSize
     *  @sa @ref grwlSetWindowMonitor
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowSize(GRWLwindow* window, int width, int height);

    /*! @brief Retrieves the size of the framebuffer of the specified window.
     *
     *  This function retrieves the size, in pixels, of the framebuffer of the
     *  specified window.  If you wish to retrieve the size of the window in screen
     *  coordinates, see @ref grwlGetWindowSize.
     *
     *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` size arguments will be set to zero.
     *
     *  @param[in] window The window whose framebuffer to query.
     *  @param[out] width Where to store the width, in pixels, of the framebuffer,
     *  or `NULL`.
     *  @param[out] height Where to store the height, in pixels, of the framebuffer,
     *  or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_fbsize
     *  @sa @ref grwlSetFramebufferSizeCallback
     *
     *  @ingroup window
     */
    GRWLAPI void grwlGetFramebufferSize(GRWLwindow* window, int* width, int* height);

    /*! @brief Retrieves the size of the frame of the window.
     *
     *  This function retrieves the size, in screen coordinates, of each edge of the
     *  frame of the specified window.  This size includes the title bar, if the
     *  window has one.  The size of the frame may vary depending on the
     *  [window-related hints](@ref window_hints_wnd) used to create it.
     *
     *  Because this function retrieves the size of each window frame edge and not
     *  the offset along a particular coordinate axis, the retrieved values will
     *  always be zero or positive.
     *
     *  Any or all of the size arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` size arguments will be set to zero.
     *
     *  @param[in] window The window whose frame size to query.
     *  @param[out] left Where to store the size, in screen coordinates, of the left
     *  edge of the window frame, or `NULL`.
     *  @param[out] top Where to store the size, in screen coordinates, of the top
     *  edge of the window frame, or `NULL`.
     *  @param[out] right Where to store the size, in screen coordinates, of the
     *  right edge of the window frame, or `NULL`.
     *  @param[out] bottom Where to store the size, in screen coordinates, of the
     *  bottom edge of the window frame, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_size
     *
     *  @ingroup window
     */
    GRWLAPI void grwlGetWindowFrameSize(GRWLwindow* window, int* left, int* top, int* right, int* bottom);

    /*! @brief Retrieves the content scale for the specified window.
     *
     *  This function retrieves the content scale for the specified window.  The
     *  content scale is the ratio between the current DPI and the platform's
     *  default DPI.  This is especially important for text and any UI elements.  If
     *  the pixel dimensions of your UI scaled by this look appropriate on your
     *  machine then it should appear at a reasonable size on other machines
     *  regardless of their DPI and scaling settings.  This relies on the system DPI
     *  and scaling settings being somewhat correct.
     *
     *  On platforms where each monitors can have its own content scale, the window
     *  content scale will depend on which monitor the system considers the window
     *  to be on.
     *
     *  @param[in] window The window to query.
     *  @param[out] xscale Where to store the x-axis content scale, or `NULL`.
     *  @param[out] yscale Where to store the y-axis content scale, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_scale
     *  @sa @ref grwlSetWindowContentScaleCallback
     *  @sa @ref grwlGetMonitorContentScale
     *
     *  @ingroup window
     */
    GRWLAPI void grwlGetWindowContentScale(GRWLwindow* window, float* xscale, float* yscale);

    /*! @brief Returns the opacity of the whole window.
     *
     *  This function returns the opacity of the window, including any decorations.
     *
     *  The opacity (or alpha) value is a positive finite number between zero and
     *  one, where zero is fully transparent and one is fully opaque.  If the system
     *  does not support whole window transparency, this function always returns one.
     *
     *  The initial opacity value for newly created windows is one.
     *
     *  @param[in] window The window to query.
     *  @return The opacity value of the specified window.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_transparency
     *  @sa @ref grwlSetWindowOpacity
     *
     *  @ingroup window
     */
    GRWLAPI float grwlGetWindowOpacity(GRWLwindow* window);

    /*! @brief Sets the opacity of the whole window.
     *
     *  This function sets the opacity of the window, including any decorations.
     *
     *  The opacity (or alpha) value is a positive finite number between zero and
     *  one, where zero is fully transparent and one is fully opaque.
     *
     *  The initial opacity value for newly created windows is one.
     *
     *  A window created with framebuffer transparency may not use whole window
     *  transparency.  The results of doing this are undefined.
     *
     *  @param[in] window The window to set the opacity for.
     *  @param[in] opacity The desired opacity of the specified window.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland There is no way to set an opacity factor for a window.
     *  This function will emit @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_transparency
     *  @sa @ref grwlGetWindowOpacity
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowOpacity(GRWLwindow* window, float opacity);

    /*! @brief Iconifies the specified window.
     *
     *  This function iconifies (minimizes) the specified window if it was
     *  previously restored.  If the window is already iconified, this function does
     *  nothing.
     *
     *  If the specified window is a full screen window, GRWL restores the original
     *  video mode of the monitor.  The window's desired video mode is set again
     *  when the window is restored.
     *
     *  @param[in] window The window to iconify.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @wayland Once a window is iconified, @ref grwlRestoreWindow won’t
     *  be able to restore it.  This is a design decision of the xdg-shell
     *  protocol.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_iconify
     *  @sa @ref grwlRestoreWindow
     *  @sa @ref grwlMaximizeWindow
     *
     *  @ingroup window
     */
    GRWLAPI void grwlIconifyWindow(GRWLwindow* window);

    /*! @brief Restores the specified window.
     *
     *  This function restores the specified window if it was previously iconified
     *  (minimized) or maximized.  If the window is already restored, this function
     *  does nothing.
     *
     *  If the specified window is an iconified full screen window, its desired
     *  video mode is set again for its monitor when the window is restored.
     *
     *  @param[in] window The window to restore.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_iconify
     *  @sa @ref grwlIconifyWindow
     *  @sa @ref grwlMaximizeWindow
     *
     *  @ingroup window
     */
    GRWLAPI void grwlRestoreWindow(GRWLwindow* window);

    /*! @brief Maximizes the specified window.
     *
     *  This function maximizes the specified window if it was previously not
     *  maximized.  If the window is already maximized, this function does nothing.
     *
     *  If the specified window is a full screen window, this function does nothing.
     *
     *  @param[in] window The window to maximize.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref window_iconify
     *  @sa @ref grwlIconifyWindow
     *  @sa @ref grwlRestoreWindow
     *
     *  @ingroup window
     */
    GRWLAPI void grwlMaximizeWindow(GRWLwindow* window);

    /*! @brief Makes the specified window visible.
     *
     *  This function makes the specified window visible if it was previously
     *  hidden.  If the window is already visible or is in full screen mode, this
     *  function does nothing.
     *
     *  By default, windowed mode windows are focused when shown
     *  Set the [GRWL_FOCUS_ON_SHOW](@ref GRWL_FOCUS_ON_SHOW_hint) window hint
     *  to change this behavior for all newly created windows, or change the
     *  behavior for an existing window with @ref grwlSetWindowAttrib.
     *
     *  @param[in] window The window to make visible.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @wayland Because Wayland wants every frame of the desktop to be
     *  complete, this function does not immediately make the window visible.
     *  Instead it will become visible the next time the window framebuffer is
     *  updated after this call.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_hide
     *  @sa @ref grwlHideWindow
     *
     *  @ingroup window
     */
    GRWLAPI void grwlShowWindow(GRWLwindow* window);

    /*! @brief Hides the specified window.
     *
     *  This function hides the specified window if it was previously visible.  If
     *  the window is already hidden or is in full screen mode, this function does
     *  nothing.
     *
     *  @param[in] window The window to hide.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_hide
     *  @sa @ref grwlShowWindow
     *
     *  @ingroup window
     */
    GRWLAPI void grwlHideWindow(GRWLwindow* window);

    /*! @brief Brings the specified window to front and sets input focus.
     *
     *  This function brings the specified window to front and sets input focus.
     *  The window should already be visible and not iconified.
     *
     *  By default, both windowed and full screen mode windows are focused when
     *  initially created.  Set the [GRWL_FOCUSED](@ref GRWL_FOCUSED_hint) to
     *  disable this behavior.
     *
     *  Also by default, windowed mode windows are focused when shown
     *  with @ref grwlShowWindow. Set the
     *  [GRWL_FOCUS_ON_SHOW](@ref GRWL_FOCUS_ON_SHOW_hint) to disable this behavior.
     *
     *  __Do not use this function__ to steal focus from other applications unless
     *  you are certain that is what the user wants.  Focus stealing can be
     *  extremely disruptive.
     *
     *  For a less disruptive way of getting the user's attention, see
     *  [attention requests](@ref window_attention).
     *
     *  @param[in] window The window to give input focus.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland It is not possible for an application to set the input
     *  focus.  This function will emit @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_focus
     *  @sa @ref window_attention
     *
     *  @ingroup window
     */
    GRWLAPI void grwlFocusWindow(GRWLwindow* window);

    /*! @brief Requests user attention to the specified window.
     *
     *  This function requests user attention to the specified window.  On
     *  platforms where this is not supported, attention is requested to the
     *  application as a whole.
     *
     *  Once the user has given attention, usually by focusing the window or
     *  application, the system will end the request automatically.
     *
     *  @param[in] window The window to request attention to.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark @macos Attention is requested to the application as a whole, not the
     *  specific window.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_attention
     *
     *  @ingroup window
     */
    GRWLAPI void grwlRequestWindowAttention(GRWLwindow* window);

    /*! @brief Returns the monitor that the window uses for full screen mode.
     *
     *  This function returns the handle of the monitor that the specified window is
     *  in full screen on.
     *
     *  @param[in] window The window to query.
     *  @return The monitor, or `NULL` if the window is in windowed mode or an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_monitor
     *  @sa @ref grwlSetWindowMonitor
     *
     *  @ingroup window
     */
    GRWLAPI GRWLmonitor* grwlGetWindowMonitor(GRWLwindow* window);

    /*! @brief Sets the mode, monitor, video mode and placement of a window.
     *
     *  This function sets the monitor that the window uses for full screen mode or,
     *  if the monitor is `NULL`, makes it windowed mode.
     *
     *  When setting a monitor, this function updates the width, height and refresh
     *  rate of the desired video mode and switches to the video mode closest to it.
     *  The window position is ignored when setting a monitor.
     *
     *  When the monitor is `NULL`, the position, width and height are used to
     *  place the window content area.  The refresh rate is ignored when no monitor
     *  is specified.
     *
     *  If you only wish to update the resolution of a full screen window or the
     *  size of a windowed mode window, see @ref grwlSetWindowSize.
     *
     *  When a window transitions from full screen to windowed mode, this function
     *  restores any previous window settings such as whether it is decorated,
     *  floating, resizable, has size or aspect ratio limits, etc.
     *
     *  @param[in] window The window whose monitor, size or video mode to set.
     *  @param[in] monitor The desired monitor, or `NULL` to set windowed mode.
     *  @param[in] xpos The desired x-coordinate of the upper-left corner of the
     *  content area.
     *  @param[in] ypos The desired y-coordinate of the upper-left corner of the
     *  content area.
     *  @param[in] width The desired with, in screen coordinates, of the content
     *  area or video mode.
     *  @param[in] height The desired height, in screen coordinates, of the content
     *  area or video mode.
     *  @param[in] refreshRate The desired refresh rate, in Hz, of the video mode,
     *  or `GRWL_DONT_CARE`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @remark The OpenGL or OpenGL ES context will not be destroyed or otherwise
     *  affected by any resizing or mode switching, although you may need to update
     *  your viewport if the framebuffer size has changed.
     *
     *  @remark @wayland The desired window position is ignored, as there is no way
     *  for an application to set this property.
     *
     *  @remark @wayland Setting the window to full screen will not attempt to
     *  change the mode, no matter what the requested size or refresh rate.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_monitor
     *  @sa @ref window_full_screen
     *  @sa @ref grwlGetWindowMonitor
     *  @sa @ref grwlSetWindowSize
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowMonitor(GRWLwindow* window, GRWLmonitor* monitor, int xpos, int ypos, int width,
                                      int height, int refreshRate);

    /*! @brief Returns an attribute of the specified window.
     *
     *  This function returns the value of an attribute of the specified window or
     *  its OpenGL or OpenGL ES context.
     *
     *  @param[in] window The window to query.
     *  @param[in] attrib The [window attribute](@ref window_attribs) whose value to
     *  return.
     *  @return The value of the attribute, or zero if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark Framebuffer related hints are not window attributes.  See @ref
     *  window_attribs_fb for more information.
     *
     *  @remark Zero is a valid value for many window and context related
     *  attributes so you cannot use a return value of zero as an indication of
     *  errors.  However, this function should not fail as long as it is passed
     *  valid arguments and the library has been [initialized](@ref intro_init).
     *
     *  @remark @wayland The Wayland protocol provides no way to check whether a
     *  window is iconfied, so @ref GRWL_ICONIFIED always returns `false`.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_attribs
     *  @sa @ref grwlSetWindowAttrib
     *
     *  `grwlGetGLVersion`.
     *
     *  @ingroup window
     */
    GRWLAPI int grwlGetWindowAttrib(GRWLwindow* window, int attrib);

    /*! @brief Sets an attribute of the specified window.
     *
     *  This function sets the value of an attribute of the specified window.
     *
     *  The supported attributes are [GRWL_DECORATED](@ref GRWL_DECORATED_attrib),
     *  [GRWL_RESIZABLE](@ref GRWL_RESIZABLE_attrib),
     *  [GRWL_FLOATING](@ref GRWL_FLOATING_attrib),
     *  [GRWL_AUTO_ICONIFY](@ref GRWL_AUTO_ICONIFY_attrib) and
     *  [GRWL_FOCUS_ON_SHOW](@ref GRWL_FOCUS_ON_SHOW_attrib).
     *  [GRWL_MOUSE_PASSTHROUGH](@ref GRWL_MOUSE_PASSTHROUGH_attrib)
     *
     *  Some of these attributes are ignored for full screen windows.  The new
     *  value will take effect if the window is later made windowed.
     *
     *  Some of these attributes are ignored for windowed mode windows.  The new
     *  value will take effect if the window is later made full screen.
     *
     *  @param[in] window The window to set the attribute for.
     *  @param[in] attrib A supported window attribute.
     *  @param[in] value `true` or `false`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM, @ref GRWL_INVALID_VALUE, @ref GRWL_PLATFORM_ERROR and @ref
     *  GRWL_FEATURE_UNAVAILABLE.
     *
     *  @remark Calling @ref grwlGetWindowAttrib will always return the latest
     *  value, even if that value is ignored by the current mode of the window.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_attribs
     *  @sa @ref grwlGetWindowAttrib
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowAttrib(GRWLwindow* window, int attrib, int value);

    /*! @brief Sets the user pointer of the specified window.
     *
     *  This function sets the user-defined pointer of the specified window.  The
     *  current value is retained until the window is destroyed.  The initial value
     *  is `NULL`.
     *
     *  @param[in] window The window whose pointer to set.
     *  @param[in] pointer The new value.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref window_userptr
     *  @sa @ref grwlGetWindowUserPointer
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSetWindowUserPointer(GRWLwindow* window, void* pointer);

    /*! @brief Returns the user pointer of the specified window.
     *
     *  This function returns the current value of the user-defined pointer of the
     *  specified window.  The initial value is `NULL`.
     *
     *  @param[in] window The window whose pointer to return.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref window_userptr
     *  @sa @ref grwlSetWindowUserPointer
     *
     *  @ingroup window
     */
    GRWLAPI void* grwlGetWindowUserPointer(GRWLwindow* window);

    /*! @brief Sets the position callback for the specified window.
     *
     *  This function sets the position callback of the specified window, which is
     *  called when the window is moved.  The callback is provided with the
     *  position, in screen coordinates, of the upper-left corner of the content
     *  area of the window.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int xpos, int ypos)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowposfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @remark @wayland This callback will never be called, as there is no way for
     *  an application to know its global position.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_pos
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowposfun grwlSetWindowPosCallback(GRWLwindow* window, GRWLwindowposfun callback);

    /*! @brief Sets the size callback for the specified window.
     *
     *  This function sets the size callback of the specified window, which is
     *  called when the window is resized.  The callback is provided with the size,
     *  in screen coordinates, of the content area of the window.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int width, int height)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowsizefun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_size
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowsizefun grwlSetWindowSizeCallback(GRWLwindow* window, GRWLwindowsizefun callback);

    /*! @brief Sets the close callback for the specified window.
     *
     *  This function sets the close callback of the specified window, which is
     *  called when the user attempts to close the window, for example by clicking
     *  the close widget in the title bar.
     *
     *  The close flag is set before this callback is called, but you can modify it
     *  at any time with @ref grwlSetWindowShouldClose.
     *
     *  The close callback is not triggered by @ref grwlDestroyWindow.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowclosefun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @remark @macos Selecting Quit from the application menu will trigger the
     *  close callback for all windows.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_close
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowclosefun grwlSetWindowCloseCallback(GRWLwindow* window, GRWLwindowclosefun callback);

    /*! @brief Sets the refresh callback for the specified window.
     *
     *  This function sets the refresh callback of the specified window, which is
     *  called when the content area of the window needs to be redrawn, for example
     *  if the window has been exposed after having been covered by another window.
     *
     *  On compositing window systems such as Aero, Compiz, Aqua or Wayland, where
     *  the window contents are saved off-screen, this callback may be called only
     *  very infrequently or never at all.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window);
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowrefreshfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_refresh
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowrefreshfun grwlSetWindowRefreshCallback(GRWLwindow* window, GRWLwindowrefreshfun callback);

    /*! @brief Sets the focus callback for the specified window.
     *
     *  This function sets the focus callback of the specified window, which is
     *  called when the window gains or loses input focus.
     *
     *  After the focus callback is called for a window that lost input focus,
     *  synthetic key and mouse button release events will be generated for all such
     *  that had been pressed.  For more information, see @ref grwlSetKeyCallback
     *  and @ref grwlSetMouseButtonCallback.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int focused)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowfocusfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_focus
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowfocusfun grwlSetWindowFocusCallback(GRWLwindow* window, GRWLwindowfocusfun callback);

    /*! @brief Sets the iconify callback for the specified window.
     *
     *  This function sets the iconification callback of the specified window, which
     *  is called when the window is iconified or restored.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int iconified)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowiconifyfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_iconify
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowiconifyfun grwlSetWindowIconifyCallback(GRWLwindow* window, GRWLwindowiconifyfun callback);

    /*! @brief Sets the maximize callback for the specified window.
     *
     *  This function sets the maximization callback of the specified window, which
     *  is called when the window is maximized or restored.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int maximized)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowmaximizefun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_maximize
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowmaximizefun grwlSetWindowMaximizeCallback(GRWLwindow* window, GRWLwindowmaximizefun callback);

    /*! @brief Sets the framebuffer resize callback for the specified window.
     *
     *  This function sets the framebuffer resize callback of the specified window,
     *  which is called when the framebuffer of the specified window is resized.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int width, int height)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLframebuffersizefun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_fbsize
     *
     *  @ingroup window
     */
    GRWLAPI GRWLframebuffersizefun grwlSetFramebufferSizeCallback(GRWLwindow* window, GRWLframebuffersizefun callback);

    /*! @brief Sets the window content scale callback for the specified window.
     *
     *  This function sets the window content scale callback of the specified window,
     *  which is called when the content scale of the specified window changes.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, float xscale, float yscale)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLwindowcontentscalefun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref window_scale
     *  @sa @ref grwlGetWindowContentScale
     *
     *  @ingroup window
     */
    GRWLAPI GRWLwindowcontentscalefun grwlSetWindowContentScaleCallback(GRWLwindow* window,
                                                                        GRWLwindowcontentscalefun callback);

    /*! @brief Processes all pending events.
     *
     *  This function processes only those events that are already in the event
     *  queue and then returns immediately.  Processing events will cause the window
     *  and input callbacks associated with those events to be called.
     *
     *  On some platforms, a window move, resize or menu operation will cause event
     *  processing to block.  This is due to how event processing is designed on
     *  those platforms.  You can use the
     *  [window refresh callback](@ref window_refresh) to redraw the contents of
     *  your window when necessary during such operations.
     *
     *  Do not assume that callbacks you set will _only_ be called in response to
     *  event processing functions like this one.  While it is necessary to poll for
     *  events, window systems that require GRWL to register callbacks of its own
     *  can pass events to GRWL in response to many window system function calls.
     *  GRWL will pass those events on to the application callbacks before
     *  returning.
     *
     *  Event processing is not required for joystick input to work.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref events
     *  @sa @ref grwlWaitEvents
     *  @sa @ref grwlWaitEventsTimeout
     *
     *  @ingroup window
     */
    GRWLAPI void grwlPollEvents();

    /*! @brief Waits until events are queued and processes them.
     *
     *  This function puts the calling thread to sleep until at least one event is
     *  available in the event queue.  Once one or more events are available,
     *  it behaves exactly like @ref grwlPollEvents, i.e. the events in the queue
     *  are processed and the function then returns immediately.  Processing events
     *  will cause the window and input callbacks associated with those events to be
     *  called.
     *
     *  Since not all events are associated with callbacks, this function may return
     *  without a callback having been called even if you are monitoring all
     *  callbacks.
     *
     *  On some platforms, a window move, resize or menu operation will cause event
     *  processing to block.  This is due to how event processing is designed on
     *  those platforms.  You can use the
     *  [window refresh callback](@ref window_refresh) to redraw the contents of
     *  your window when necessary during such operations.
     *
     *  Do not assume that callbacks you set will _only_ be called in response to
     *  event processing functions like this one.  While it is necessary to poll for
     *  events, window systems that require GRWL to register callbacks of its own
     *  can pass events to GRWL in response to many window system function calls.
     *  GRWL will pass those events on to the application callbacks before
     *  returning.
     *
     *  Event processing is not required for joystick input to work.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref events
     *  @sa @ref grwlPollEvents
     *  @sa @ref grwlWaitEventsTimeout
     *
     *  @ingroup window
     */
    GRWLAPI void grwlWaitEvents();

    /*! @brief Waits with timeout until events are queued and processes them.
     *
     *  This function puts the calling thread to sleep until at least one event is
     *  available in the event queue, or until the specified timeout is reached.  If
     *  one or more events are available, it behaves exactly like @ref
     *  grwlPollEvents, i.e. the events in the queue are processed and the function
     *  then returns immediately.  Processing events will cause the window and input
     *  callbacks associated with those events to be called.
     *
     *  The timeout value must be a positive finite number.
     *
     *  Since not all events are associated with callbacks, this function may return
     *  without a callback having been called even if you are monitoring all
     *  callbacks.
     *
     *  On some platforms, a window move, resize or menu operation will cause event
     *  processing to block.  This is due to how event processing is designed on
     *  those platforms.  You can use the
     *  [window refresh callback](@ref window_refresh) to redraw the contents of
     *  your window when necessary during such operations.
     *
     *  Do not assume that callbacks you set will _only_ be called in response to
     *  event processing functions like this one.  While it is necessary to poll for
     *  events, window systems that require GRWL to register callbacks of its own
     *  can pass events to GRWL in response to many window system function calls.
     *  GRWL will pass those events on to the application callbacks before
     *  returning.
     *
     *  Event processing is not required for joystick input to work.
     *
     *  @param[in] timeout The maximum amount of time, in seconds, to wait.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_VALUE and @ref GRWL_PLATFORM_ERROR.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref events
     *  @sa @ref grwlPollEvents
     *  @sa @ref grwlWaitEvents
     *
     *  @ingroup window
     */
    GRWLAPI void grwlWaitEventsTimeout(double timeout);

    /*! @brief Posts an empty event to the event queue.
     *
     *  This function posts an empty event from the current thread to the event
     *  queue, causing @ref grwlWaitEvents or @ref grwlWaitEventsTimeout to return.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref events
     *  @sa @ref grwlWaitEvents
     *  @sa @ref grwlWaitEventsTimeout
     *
     *  @ingroup window
     */
    GRWLAPI void grwlPostEmptyEvent();

    /*! @brief Returns the value of an input option for the specified window.
     *
     *  This function returns the value of an input option for the specified window.
     *  The mode must be one of @ref GRWL_CURSOR, @ref GRWL_STICKY_KEYS,
     *  @ref GRWL_STICKY_MOUSE_BUTTONS, @ref GRWL_LOCK_KEY_MODS,
     *  @ref GRWL_RAW_MOUSE_MOTION or @ref GRWL_IME.
     *
     *  @param[in] window The window to query.
     *  @param[in] mode One of `GRWL_CURSOR`, `GRWL_STICKY_KEYS`,
     *  `GRWL_STICKY_MOUSE_BUTTONS`, `GRWL_LOCK_KEY_MODS`, `GRWL_RAW_MOUSE_MOTION`,
     *  or `GRWL_IME`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref grwlSetInputMode
     *
     *  @ingroup input
     */
    GRWLAPI int grwlGetInputMode(GRWLwindow* window, int mode);

    /*! @brief Sets an input option for the specified window.
     *
     *  This function sets an input mode option for the specified window.  The mode
     *  must be one of @ref GRWL_CURSOR, @ref GRWL_STICKY_KEYS,
     *  @ref GRWL_STICKY_MOUSE_BUTTONS, @ref GRWL_LOCK_KEY_MODS,
     *  @ref GRWL_RAW_MOUSE_MOTION or @ref GRWL_IME.
     *
     *  If the mode is `GRWL_CURSOR`, the value must be one of the following cursor
     *  modes:
     *  - `GRWL_CURSOR_NORMAL` makes the cursor visible and behaving normally.
     *  - `GRWL_CURSOR_HIDDEN` makes the cursor invisible when it is over the
     *    content area of the window but does not restrict the cursor from leaving.
     *  - `GRWL_CURSOR_DISABLED` hides and grabs the cursor, providing virtual
     *    and unlimited cursor movement.  This is useful for implementing for
     *    example 3D camera controls.
     *  - `GRWL_CURSOR_CAPTURED` makes the cursor visible and confines it to the
     *    content area of the window.
     *
     *  If the mode is `GRWL_STICKY_KEYS`, the value must be either `true` to
     *  enable sticky keys, or `false` to disable it.  If sticky keys are
     *  enabled, a key press will ensure that @ref grwlGetKey returns `GRWL_PRESS`
     *  the next time it is called even if the key had been released before the
     *  call.  This is useful when you are only interested in whether keys have been
     *  pressed but not when or in which order.
     *
     *  If the mode is `GRWL_STICKY_MOUSE_BUTTONS`, the value must be either
     *  `true` to enable sticky mouse buttons, or `false` to disable it.
     *  If sticky mouse buttons are enabled, a mouse button press will ensure that
     *  @ref grwlGetMouseButton returns `GRWL_PRESS` the next time it is called even
     *  if the mouse button had been released before the call.  This is useful when
     *  you are only interested in whether mouse buttons have been pressed but not
     *  when or in which order.
     *
     *  If the mode is `GRWL_LOCK_KEY_MODS`, the value must be either `true` to
     *  enable lock key modifier bits, or `false` to disable them.  If enabled,
     *  callbacks that receive modifier bits will also have the @ref
     *  GRWL_MOD_CAPS_LOCK bit set when the event was generated with Caps Lock on,
     *  and the @ref GRWL_MOD_NUM_LOCK bit when Num Lock was on.
     *
     *  If the mode is `GRWL_RAW_MOUSE_MOTION`, the value must be either `true`
     *  to enable raw (unscaled and unaccelerated) mouse motion when the cursor is
     *  disabled, or `false` to disable it.  If raw motion is not supported,
     *  attempting to set this will emit @ref GRWL_FEATURE_UNAVAILABLE.  Call @ref
     *  grwlRawMouseMotionSupported to check for support.
     *
     *  If the mode is `GRWL_IME`, the value must be either `true` to turn on
     *  IME, or `false` to turn off it.
     *
     *  @param[in] window The window whose input mode to set.
     *  @param[in] mode One of `GRWL_CURSOR`, `GRWL_STICKY_KEYS`,
     *  `GRWL_STICKY_MOUSE_BUTTONS`, `GRWL_LOCK_KEY_MODS`,
     *  `GRWL_RAW_MOUSE_MOTION` or `GRWL_IME`.
     *  @param[in] value The new value of the specified input mode.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM, @ref GRWL_PLATFORM_ERROR and @ref
     *  GRWL_FEATURE_UNAVAILABLE (see above).
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref grwlGetInputMode
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetInputMode(GRWLwindow* window, int mode, int value);

    /*! @brief Returns whether raw mouse motion is supported.
     *
     *  This function returns whether raw mouse motion is supported on the current
     *  system.  This status does not change after GRWL has been initialized so you
     *  only need to check this once.  If you attempt to enable raw motion on
     *  a system that does not support it, @ref GRWL_PLATFORM_ERROR will be emitted.
     *
     *  Raw mouse motion is closer to the actual motion of the mouse across
     *  a surface.  It is not affected by the scaling and acceleration applied to
     *  the motion of the desktop cursor.  That processing is suitable for a cursor
     *  while raw motion is better for controlling for example a 3D camera.  Because
     *  of this, raw mouse motion is only provided when the cursor is disabled.
     *
     *  @return `true` if raw mouse motion is supported on the current machine,
     *  or `false` otherwise.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref raw_mouse_motion
     *  @sa @ref grwlSetInputMode
     *
     *  @ingroup input
     */
    GRWLAPI int grwlRawMouseMotionSupported();

    /*! @brief Returns the layout-specific name of the specified printable key.
     *
     *  This function returns the name of the specified printable key, encoded as
     *  UTF-8.  This is typically the character that key would produce without any
     *  modifier keys, intended for displaying key bindings to the user.  For dead
     *  keys, it is typically the diacritic it would add to a character.
     *
     *  __Do not use this function__ for [text input](@ref input_char).  You will
     *  break text input for many languages even if it happens to work for yours.
     *
     *  If the key is `GRWL_KEY_UNKNOWN`, the scancode is used to identify the key,
     *  otherwise the scancode is ignored.  If you specify a non-printable key, or
     *  `GRWL_KEY_UNKNOWN` and a scancode that maps to a non-printable key, this
     *  function returns `NULL` but does not emit an error.
     *
     *  This behavior allows you to always pass in the arguments in the
     *  [key callback](@ref input_key) without modification.
     *
     *  The printable keys are:
     *  - `GRWL_KEY_APOSTROPHE`
     *  - `GRWL_KEY_COMMA`
     *  - `GRWL_KEY_MINUS`
     *  - `GRWL_KEY_PERIOD`
     *  - `GRWL_KEY_SLASH`
     *  - `GRWL_KEY_SEMICOLON`
     *  - `GRWL_KEY_EQUAL`
     *  - `GRWL_KEY_LEFT_BRACKET`
     *  - `GRWL_KEY_RIGHT_BRACKET`
     *  - `GRWL_KEY_BACKSLASH`
     *  - `GRWL_KEY_WORLD_1`
     *  - `GRWL_KEY_WORLD_2`
     *  - `GRWL_KEY_0` to `GRWL_KEY_9`
     *  - `GRWL_KEY_A` to `GRWL_KEY_Z`
     *  - `GRWL_KEY_KP_0` to `GRWL_KEY_KP_9`
     *  - `GRWL_KEY_KP_DECIMAL`
     *  - `GRWL_KEY_KP_DIVIDE`
     *  - `GRWL_KEY_KP_MULTIPLY`
     *  - `GRWL_KEY_KP_SUBTRACT`
     *  - `GRWL_KEY_KP_ADD`
     *  - `GRWL_KEY_KP_EQUAL`
     *
     *  Names for printable keys depend on keyboard layout, while names for
     *  non-printable keys are the same across layouts but depend on the application
     *  language and should be localized along with other user interface text.
     *
     *  The contents of the returned string may change when a keyboard
     *  layout change event is received.  Set a
     *  [keyboard layout](@ref keyboard_layout) callback to be notified when the
     *  layout changes.
     *
     *  @param[in] key The key to query, or `GRWL_KEY_UNKNOWN`.
     *  @param[in] scancode The scancode of the key to query.
     *  @return The UTF-8 encoded, layout-specific name of the key, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_key_name
     *
     *  @ingroup input
     */
    GRWLAPI const char* grwlGetKeyName(int key, int scancode);

    /*! @brief Returns the platform-specific scancode of the specified key.
     *
     *  This function returns the platform-specific scancode of the specified key.
     *
     *  If the key is `GRWL_KEY_UNKNOWN` or does not exist on the keyboard this
     *  method will return `-1`.
     *
     *  @param[in] key Any [named key](@ref keys).
     *  @return The platform-specific scancode for the key, or `-1` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref input_key
     *
     *  @ingroup input
     */
    GRWLAPI int grwlGetKeyScancode(int key);

    /*! @brief Returns the human-readable name of the current keyboard layout.
     *
     *  This function returns the human-readable name, encoded as UTF-8, of the
     *  current keyboard layout.  On some platforms this may not be updated until
     *  one of the application's windows gets input focus.
     *
     *  The keyboard layout name is intended to be shown to the user during text
     *  input, especially in full screen applications.
     *
     *  The name may be localized into the current operating system UI language.  It
     *  is provided by the operating system and may not be identical for a given
     *  layout across platforms.
     *
     *  @return The UTF-8 encoded name of the current keyboard layout, or `NULL` if
     *  an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_PLATFORM_ERROR and @ref
     *  GRWL_NOT_INITIALIZED.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the next call to this
     *  function or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref keyboard_layout
     *  @sa @ref grwlSetKeyboardLayoutCallback
     *
     *  @ingroup input
     */
    GRWLAPI const char* grwlGetKeyboardLayoutName();

    /*! @brief Sets the keyboard layout callback.
     *
     *  This function sets the keyboard layout callback, which is called when the
     *  keyboard layout is changed.  The name of the current layout is returned by
     *  @ref grwlGetKeyboardLayoutName.
     *
     *  On some platforms the keyboard layout event may not arrive until one of the
     *  application's windows get input focus.  Layout changes may not be reported
     *  while other applications have input focus.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name()
     *  @endcode
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref keyboard_layout
     *  @sa @ref grwlGetKeyboardLayoutName
     *
     *  @ingroup input
     */
    GRWLAPI GRWLkeyboardlayoutfun grwlSetKeyboardLayoutCallback(GRWLkeyboardlayoutfun callback);

    /*! @brief Returns the last reported state of a keyboard key for the specified
     *  window.
     *
     *  This function returns the last state reported for the specified key to the
     *  specified window.  The returned state is one of `GRWL_PRESS` or
     *  `GRWL_RELEASE`.  The action `GRWL_REPEAT` is only reported to the key callback.
     *
     *  If the @ref GRWL_STICKY_KEYS input mode is enabled, this function returns
     *  `GRWL_PRESS` the first time you call it for a key that was pressed, even if
     *  that key has already been released.
     *
     *  The key functions deal with physical keys, with [key tokens](@ref keys)
     *  named after their use on the standard US keyboard layout.  If you want to
     *  input text, use the Unicode character callback instead.
     *
     *  The [modifier key bit masks](@ref mods) are not key tokens and cannot be
     *  used with this function.
     *
     *  __Do not use this function__ to implement [text input](@ref input_char).
     *
     *  @param[in] window The desired window.
     *  @param[in] key The desired [keyboard key](@ref keys).  `GRWL_KEY_UNKNOWN` is
     *  not a valid key for this function.
     *  @return One of `GRWL_PRESS` or `GRWL_RELEASE`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_key
     *
     *  @ingroup input
     */
    GRWLAPI int grwlGetKey(GRWLwindow* window, int key);

    /*! @brief Returns the last reported state of a mouse button for the specified
     *  window.
     *
     *  This function returns the last state reported for the specified mouse button
     *  to the specified window.  The returned state is one of `GRWL_PRESS` or
     *  `GRWL_RELEASE`.
     *
     *  If the @ref GRWL_STICKY_MOUSE_BUTTONS input mode is enabled, this function
     *  returns `GRWL_PRESS` the first time you call it for a mouse button that was
     *  pressed, even if that mouse button has already been released.
     *
     *  @param[in] window The desired window.
     *  @param[in] button The desired [mouse button](@ref buttons).
     *  @return One of `GRWL_PRESS` or `GRWL_RELEASE`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_mouse_button
     *
     *  @ingroup input
     */
    GRWLAPI int grwlGetMouseButton(GRWLwindow* window, int button);

    /*! @brief Retrieves the position of the cursor relative to the content area of
     *  the window.
     *
     *  This function returns the position of the cursor, in screen coordinates,
     *  relative to the upper-left corner of the content area of the specified
     *  window.
     *
     *  If the cursor is disabled (with `GRWL_CURSOR_DISABLED`) then the cursor
     *  position is unbounded and limited only by the minimum and maximum values of
     *  a `double`.
     *
     *  The coordinate can be converted to their integer equivalents with the
     *  `floor` function.  Casting directly to an integer type works for positive
     *  coordinates, but fails for negative ones.
     *
     *  Any or all of the position arguments may be `NULL`.  If an error occurs, all
     *  non-`NULL` position arguments will be set to zero.
     *
     *  @param[in] window The desired window.
     *  @param[out] xpos Where to store the cursor x-coordinate, relative to the
     *  left edge of the content area, or `NULL`.
     *  @param[out] ypos Where to store the cursor y-coordinate, relative to the to
     *  top edge of the content area, or `NULL`.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_pos
     *  @sa @ref grwlSetCursorPos
     *
     *  @ingroup input
     */
    GRWLAPI void grwlGetCursorPos(GRWLwindow* window, double* xpos, double* ypos);

    /*! @brief Sets the position of the cursor, relative to the content area of the
     *  window.
     *
     *  This function sets the position, in screen coordinates, of the cursor
     *  relative to the upper-left corner of the content area of the specified
     *  window.  The window must have input focus.  If the window does not have
     *  input focus when this function is called, it fails silently.
     *
     *  __Do not use this function__ to implement things like camera controls.  GRWL
     *  already provides the `GRWL_CURSOR_DISABLED` cursor mode that hides the
     *  cursor, transparently re-centers it and provides unconstrained cursor
     *  motion.  See @ref grwlSetInputMode for more information.
     *
     *  If the cursor mode is `GRWL_CURSOR_DISABLED` then the cursor position is
     *  unconstrained and limited only by the minimum and maximum values of
     *  a `double`.
     *
     *  @param[in] window The desired window.
     *  @param[in] xpos The desired x-coordinate, relative to the left edge of the
     *  content area.
     *  @param[in] ypos The desired y-coordinate, relative to the top edge of the
     *  content area.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_PLATFORM_ERROR and @ref GRWL_FEATURE_UNAVAILABLE (see remarks).
     *
     *  @remark @wayland This function will only work when the cursor mode is
     *  `GRWL_CURSOR_DISABLED`, otherwise it will emit @ref GRWL_FEATURE_UNAVAILABLE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_pos
     *  @sa @ref grwlGetCursorPos
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetCursorPos(GRWLwindow* window, double xpos, double ypos);

    /*! @brief Creates a custom cursor.
     *
     *  Creates a new custom cursor image that can be set for a window with @ref
     *  grwlSetCursor.  The cursor can be destroyed with @ref grwlDestroyCursor.
     *  Any remaining cursors are destroyed by @ref grwlTerminate.
     *
     *  The pixels are 32-bit, little-endian, non-premultiplied RGBA, i.e. eight
     *  bits per channel with the red channel first.  They are arranged canonically
     *  as packed sequential rows, starting from the top-left corner.
     *
     *  The cursor hotspot is specified in pixels, relative to the upper-left corner
     *  of the cursor image.  Like all other coordinate systems in GRWL, the X-axis
     *  points to the right and the Y-axis points down.
     *
     *  @param[in] image The desired cursor image.
     *  @param[in] xhot The desired x-coordinate, in pixels, of the cursor hotspot.
     *  @param[in] yhot The desired y-coordinate, in pixels, of the cursor hotspot.
     *  @return The handle of the created cursor, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_VALUE and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The specified image data is copied before this function
     *  returns.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_object
     *  @sa @ref grwlDestroyCursor
     *  @sa @ref grwlCreateStandardCursor
     *
     *  @ingroup input
     */
    GRWLAPI GRWLcursor* grwlCreateCursor(const GRWLimage* image, int xhot, int yhot);

    /*! @brief Creates a cursor with a standard shape.
     *
     *  Returns a cursor with a standard shape, that can be set for a window with
     *  @ref grwlSetCursor.  The images for these cursors come from the system
     *  cursor theme and their exact appearance will vary between platforms.
     *
     *  Most of these shapes are guaranteed to exist on every supported platform but
     *  a few may not be present.  See the table below for details.
     *
     *  Cursor shape                   | Windows | macOS | X11    | Wayland
     *  ------------------------------ | ------- | ----- | ------ | -------
     *  @ref GRWL_ARROW_CURSOR         | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_IBEAM_CURSOR         | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_CROSSHAIR_CURSOR     | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_POINTING_HAND_CURSOR | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_RESIZE_EW_CURSOR     | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_RESIZE_NS_CURSOR     | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_RESIZE_NWSE_CURSOR   | Yes     | Yes<sup>1</sup> | Maybe<sup>2</sup> | Maybe<sup>2</sup>
     *  @ref GRWL_RESIZE_NESW_CURSOR   | Yes     | Yes<sup>1</sup> | Maybe<sup>2</sup> | Maybe<sup>2</sup>
     *  @ref GRWL_RESIZE_ALL_CURSOR    | Yes     | Yes   | Yes    | Yes
     *  @ref GRWL_NOT_ALLOWED_CURSOR   | Yes     | Yes   | Maybe<sup>2</sup> | Maybe<sup>2</sup>
     *
     *  1) This uses a private system API and may fail in the future.
     *
     *  2) This uses a newer standard that not all cursor themes support.
     *
     *  If the requested shape is not available, this function emits a @ref
     *  GRWL_CURSOR_UNAVAILABLE error and returns `NULL`.
     *
     *  @param[in] shape One of the [standard shapes](@ref shapes).
     *  @return A new cursor ready to use or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM, @ref GRWL_CURSOR_UNAVAILABLE and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_standard
     *  @sa @ref grwlCreateCursor
     *
     *  @ingroup input
     */
    GRWLAPI GRWLcursor* grwlCreateStandardCursor(int shape);

    /*! @brief Destroys a cursor.
     *
     *  This function destroys a cursor previously created with @ref
     *  grwlCreateCursor.  Any remaining cursors will be destroyed by @ref
     *  grwlTerminate.
     *
     *  If the specified cursor is current for any window, that window will be
     *  reverted to the default cursor.  This does not affect the cursor mode.
     *
     *  @param[in] cursor The cursor object to destroy.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_object
     *  @sa @ref grwlCreateCursor
     *
     *  @ingroup input
     */
    GRWLAPI void grwlDestroyCursor(GRWLcursor* cursor);

    /*! @brief Sets the cursor for the window.
     *
     *  This function sets the cursor image to be used when the cursor is over the
     *  content area of the specified window.  The set cursor will only be visible
     *  when the [cursor mode](@ref cursor_mode) of the window is
     *  `GRWL_CURSOR_NORMAL`.
     *
     *  On some platforms, the set cursor may not be visible unless the window also
     *  has input focus.
     *
     *  @param[in] window The window to set the cursor for.
     *  @param[in] cursor The cursor to set, or `NULL` to switch back to the default
     *  arrow cursor.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_object
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetCursor(GRWLwindow* window, GRWLcursor* cursor);

    /*! @brief Retrieves the area of the preedit text cursor.
     *
     *  This area is used to decide the position of the candidate window.
     *  The cursor position is relative to the window.
     *
     *  @param[in] window The window to set the preedit text cursor for.
     *  @param[out] x The preedit text cursor x position (relative position from window coordinates).
     *  @param[out] y The preedit text cursor y position (relative position from window coordinates).
     *  @param[out] w The preedit text cursor width.
     *  @param[out] h The preedit text cursor height.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *
     *  @ingroup input
     */
    GRWLAPI void grwlGetPreeditCursorRectangle(GRWLwindow* window, int* x, int* y, int* w, int* h);

    /*! @brief Sets the area of the preedit text cursor.
     *
     *  This area is used to decide the position of the candidate window.
     *  The cursor position is relative to the window.
     *
     *  @param[in] window The window to set the text cursor for.
     *  @param[in] x The preedit text cursor x position (relative position from window coordinates).
     *  @param[in] y The preedit text cursor y position (relative position from window coordinates).
     *  @param[in] w The preedit text cursor width.
     *  @param[in] h The preedit text cursor height.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetPreeditCursorRectangle(GRWLwindow* window, int x, int y, int w, int h);

    /*! @brief Resets IME input status.
     *
     *  This function resets IME's preedit text.
     *
     *  @param[in] window The window.
     *
     *  @remark @x11 Since over-the-spot style is used by default, you don't need
     *  to use this function.
     *
     *  @remark @wayland This function is currently not supported.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *
     *  @ingroup input
     */
    GRWLAPI void grwlResetPreeditText(GRWLwindow* window);

    /*! @brief Returns the preedit candidate.
     *
     *  This function returns the text and the text-count of the preedit candidate.
     *
     *  By default, the IME manages the preedit candidates, so there is no need to
     *  use this function.  See @ref grwlSetPreeditCandidateCallback and
     *  [GRWL_MANAGE_PREEDIT_CANDIDATE](@ref GRWL_MANAGE_PREEDIT_CANDIDATE_hint) for details.
     *
     *  @param[in] window The window.
     *  @param[in] index The index of the candidate.
     *  @param[out] textCount The text-count of the candidate.
     *  @return The text of the candidate as Unicode code points.
     *
     *  @remark @macos @x11 @wayland Don't support this function.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *  @sa @ref grwlSetPreeditCandidateCallback
     *  @sa [GRWL_MANAGE_PREEDIT_CANDIDATE](@ref GRWL_MANAGE_PREEDIT_CANDIDATE_hint)
     *
     *  @ingroup input
     */
    GRWLAPI unsigned int* grwlGetPreeditCandidate(GRWLwindow* window, int index, int* textCount);

    /*! @brief Sets the key callback.
     *
     *  This function sets the key callback of the specified window, which is called
     *  when a key is pressed, repeated or released.
     *
     *  The key functions deal with physical keys, with layout independent
     *  [key tokens](@ref keys) named after their values in the standard US keyboard
     *  layout.  If you want to input text, use the
     *  [character callback](@ref grwlSetCharCallback) instead.
     *
     *  When a window loses input focus, it will generate synthetic key release
     *  events for all pressed keys.  You can tell these events from user-generated
     *  events by the fact that the synthetic ones are generated after the focus
     *  loss event has been processed, i.e. after the
     *  [window focus callback](@ref grwlSetWindowFocusCallback) has been called.
     *
     *  The scancode of a key is specific to that platform or sometimes even to that
     *  machine.  Scancodes are intended to allow users to bind keys that don't have
     *  a GRWL key token.  Such keys have `key` set to `GRWL_KEY_UNKNOWN`, their
     *  state is not saved and so it cannot be queried with @ref grwlGetKey.
     *
     *  Sometimes GRWL needs to generate synthetic key events, in which case the
     *  scancode may be zero.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new key callback, or `NULL` to remove the currently
     *  set callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int key, int scancode, int action, int mods)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLkeyfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_key
     *
     *  @ingroup input
     */
    GRWLAPI GRWLkeyfun grwlSetKeyCallback(GRWLwindow* window, GRWLkeyfun callback);

    /*! @brief Sets the Unicode character callback.
     *
     *  This function sets the character callback of the specified window, which is
     *  called when a Unicode character is input.
     *
     *  The character callback is intended for Unicode text input.  As it deals with
     *  characters, it is keyboard layout dependent, whereas the
     *  [key callback](@ref grwlSetKeyCallback) is not.  Characters do not map 1:1
     *  to physical keys, as a key may produce zero, one or more characters.  If you
     *  want to know whether a specific physical key was pressed or released, see
     *  the key callback instead.
     *
     *  The character callback behaves as system text input normally does and will
     *  not be called if modifier keys are held down that would prevent normal text
     *  input on that platform, for example a Super (Command) key on macOS or Alt key
     *  on Windows.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, unsigned int codepoint)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLcharfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_char
     *
     *  @ingroup input
     */
    GRWLAPI GRWLcharfun grwlSetCharCallback(GRWLwindow* window, GRWLcharfun callback);

    /*! @brief Sets the Unicode character with modifiers callback.
     *
     *  This function sets the character with modifiers callback of the specified
     *  window, which is called when a Unicode character is input regardless of what
     *  modifier keys are used.
     *
     *  The character with modifiers callback is intended for implementing custom
     *  Unicode character input.  For regular Unicode text input, see the
     *  [character callback](@ref grwlSetCharCallback).  Like the character
     *  callback, the character with modifiers callback deals with characters and is
     *  keyboard layout dependent.  Characters do not map 1:1 to physical keys, as
     *  a key may produce zero, one or more characters.  If you want to know whether
     *  a specific physical key was pressed or released, see the
     *  [key callback](@ref grwlSetKeyCallback) instead.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or an
     *  [error](@ref error_handling) occurred.
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, unsigned int codepoint, int mods)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLcharmodsfun).
     *
     *  @deprecated Scheduled for removal in version 4.0.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_char
     *
     *  @ingroup input
     */
    GRWLAPI GRWLcharmodsfun grwlSetCharModsCallback(GRWLwindow* window, GRWLcharmodsfun callback);

    /*! @brief Sets the preedit callback.
     *
     *  This function sets the preedit callback of the specified
     *  window, which is called when an IME is processing text before committed.
     *
     *  Callback receives relative position of input cursor inside preedit text and
     *  attributed text blocks.  This callback is used for on-the-spot text editing
     *  with IME.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or an
     *  error occurred.
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window,
                           int preedit_count,
                           unsigned int* preedit_string,
                           int block_count,
                           int* block_sizes,
                           int focused_block,
                           int caret)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLpreeditfun).
     *
     *  @remark @x11 Since over-the-spot style is used by default, you don't need
     *  to use this function.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *
     *  @ingroup input
     */
    GRWLAPI GRWLpreeditfun grwlSetPreeditCallback(GRWLwindow* window, GRWLpreeditfun cbfun);

    /*! @brief Sets the IME status change callback.
     *
     *  This function sets the IME status callback of the specified
     *  window, which is called when an IME is switched on and off.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or an
     *  error occurred.
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLimestatusfun).
     *
     *  @remark @x11 @wayland Don't support this function.  The callback is not called.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *
     *  @ingroup input
     */
    GRWLAPI GRWLimestatusfun grwlSetIMEStatusCallback(GRWLwindow* window, GRWLimestatusfun cbfun);

    /*! @brief Sets the preedit candidate change callback.
     *
     *  This function sets the preedit candidate callback of the specified
     *  window, which is called when the candidates are updated and can be used
     *  to display them by the application side.
     *
     *  By default, this callback is not called because the IME displays the
     *  candidates and there is nothing to do on the application side.  Only when
     *  the application side needs to use this to manage the displaying of
     *  IME candidates, you can set
     *  [GRWL_MANAGE_PREEDIT_CANDIDATE](@ref GRWL_MANAGE_PREEDIT_CANDIDATE_hint) init hint
     *  and stop the IME from managing it.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] cbfun The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or an
     *  error occurred.
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window,
                           int candidates_count,
                           int selected_index,
                           int page_start,
                           int page_size)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLpreeditcandidatefun).
     *
     *  @remark @macos @x11 @wayland Don't support this function.  The callback is
     *  not called.
     *
     *  @par Thread Safety
     *  This function may only be called from the main thread.
     *
     *  @sa @ref ime_support
     *  @sa [GRWL_MANAGE_PREEDIT_CANDIDATE](@ref GRWL_MANAGE_PREEDIT_CANDIDATE_hint)
     *
     *  @ingroup input
     */
    GRWLAPI GRWLpreeditcandidatefun grwlSetPreeditCandidateCallback(GRWLwindow* window, GRWLpreeditcandidatefun cbfun);

    /*! @brief Sets the mouse button callback.
     *
     *  This function sets the mouse button callback of the specified window, which
     *  is called when a mouse button is pressed or released.
     *
     *  When a window loses input focus, it will generate synthetic mouse button
     *  release events for all pressed mouse buttons.  You can tell these events
     *  from user-generated events by the fact that the synthetic ones are generated
     *  after the focus loss event has been processed, i.e. after the
     *  [window focus callback](@ref grwlSetWindowFocusCallback) has been called.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int button, int action, int mods)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLmousebuttonfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref input_mouse_button
     *
     *  @ingroup input
     */
    GRWLAPI GRWLmousebuttonfun grwlSetMouseButtonCallback(GRWLwindow* window, GRWLmousebuttonfun callback);

    /*! @brief Sets the cursor position callback.
     *
     *  This function sets the cursor position callback of the specified window,
     *  which is called when the cursor is moved.  The callback is provided with the
     *  position, in screen coordinates, relative to the upper-left corner of the
     *  content area of the window.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, double xpos, double ypos);
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLcursorposfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_pos
     *
     *  @ingroup input
     */
    GRWLAPI GRWLcursorposfun grwlSetCursorPosCallback(GRWLwindow* window, GRWLcursorposfun callback);

    /*! @brief Sets the cursor enter/leave callback.
     *
     *  This function sets the cursor boundary crossing callback of the specified
     *  window, which is called when the cursor enters or leaves the content area of
     *  the window.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int entered)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLcursorenterfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref cursor_enter
     *
     *  @ingroup input
     */
    GRWLAPI GRWLcursorenterfun grwlSetCursorEnterCallback(GRWLwindow* window, GRWLcursorenterfun callback);

    /*! @brief Sets the scroll callback.
     *
     *  This function sets the scroll callback of the specified window, which is
     *  called when a scrolling device is used, such as a mouse wheel or scrolling
     *  area of a touchpad.
     *
     *  The scroll callback receives all scrolling input, like that from a mouse
     *  wheel or a touchpad scrolling area.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new scroll callback, or `NULL` to remove the
     *  currently set callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, double xoffset, double yoffset)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLscrollfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref scrolling
     *
     *  @ingroup input
     */
    GRWLAPI GRWLscrollfun grwlSetScrollCallback(GRWLwindow* window, GRWLscrollfun callback);

    /*! @brief Sets the path drop callback.
     *
     *  This function sets the path drop callback of the specified window, which is
     *  called when one or more dragged paths are dropped on the window.
     *
     *  Because the path array and its strings may have been generated specifically
     *  for that event, they are not guaranteed to be valid after the callback has
     *  returned.  If you wish to use them after the callback returns, you need to
     *  make a deep copy.
     *
     *  @param[in] window The window whose callback to set.
     *  @param[in] callback The new file drop callback, or `NULL` to remove the
     *  currently set callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(GRWLwindow* window, int path_count, const char* paths[])
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLdropfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @remark @wayland File drop is currently unimplemented.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref path_drop
     *
     *  @ingroup input
     */
    GRWLAPI GRWLdropfun grwlSetDropCallback(GRWLwindow* window, GRWLdropfun callback);

    /*! @brief Returns whether the specified joystick is present.
     *
     *  This function returns whether the specified joystick is present.
     *
     *  There is no need to call this function before other functions that accept
     *  a joystick ID, as they all check for presence before performing any other
     *  work.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @return `true` if the joystick is present, or `false` otherwise.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick
     *
     *  @ingroup input
     */
    GRWLAPI int grwlJoystickPresent(int jid);

    /*! @brief Returns the values of all axes of the specified joystick.
     *
     *  This function returns the values of all axes of the specified joystick.
     *  Each element in the array is a value between -1.0 and 1.0.
     *
     *  If the specified joystick is not present this function will return `NULL`
     *  but will not generate an error.  This can be used instead of first calling
     *  @ref grwlJoystickPresent.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @param[out] count Where to store the number of axis values in the returned
     *  array.  This is set to zero if the joystick is not present or an error
     *  occurred.
     *  @return An array of axis values, or `NULL` if the joystick is not present or
     *  an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified joystick is
     *  disconnected or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_axis
     *
     *  @ingroup input
     */
    GRWLAPI const float* grwlGetJoystickAxes(int jid, int* count);

    /*! @brief Returns the state of all buttons of the specified joystick.
     *
     *  This function returns the state of all buttons of the specified joystick.
     *  Each element in the array is either `GRWL_PRESS` or `GRWL_RELEASE`.
     *
     *  For backward compatibility with earlier versions that did not have @ref
     *  grwlGetJoystickHats, the button array also includes all hats, each
     *  represented as four buttons.  The hats are in the same order as returned by
     *  __grwlGetJoystickHats__ and are in the order _up_, _right_, _down_ and
     *  _left_.  To disable these extra buttons, set the @ref
     *  GRWL_JOYSTICK_HAT_BUTTONS init hint before initialization.
     *
     *  If the specified joystick is not present this function will return `NULL`
     *  but will not generate an error.  This can be used instead of first calling
     *  @ref grwlJoystickPresent.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @param[out] count Where to store the number of button states in the returned
     *  array.  This is set to zero if the joystick is not present or an error
     *  occurred.
     *  @return An array of button states, or `NULL` if the joystick is not present
     *  or an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified joystick is
     *  disconnected or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_button
     *
     *  @ingroup input
     */
    GRWLAPI const unsigned char* grwlGetJoystickButtons(int jid, int* count);

    /*! @brief Returns the state of all hats of the specified joystick.
     *
     *  This function returns the state of all hats of the specified joystick.
     *  Each element in the array is one of the following values:
     *
     *  Name                  | Value
     *  ----                  | -----
     *  `GRWL_HAT_CENTERED`   | 0
     *  `GRWL_HAT_UP`         | 1
     *  `GRWL_HAT_RIGHT`      | 2
     *  `GRWL_HAT_DOWN`       | 4
     *  `GRWL_HAT_LEFT`       | 8
     *  `GRWL_HAT_RIGHT_UP`   | `GRWL_HAT_RIGHT` \| `GRWL_HAT_UP`
     *  `GRWL_HAT_RIGHT_DOWN` | `GRWL_HAT_RIGHT` \| `GRWL_HAT_DOWN`
     *  `GRWL_HAT_LEFT_UP`    | `GRWL_HAT_LEFT` \| `GRWL_HAT_UP`
     *  `GRWL_HAT_LEFT_DOWN`  | `GRWL_HAT_LEFT` \| `GRWL_HAT_DOWN`
     *
     *  The diagonal directions are bitwise combinations of the primary (up, right,
     *  down and left) directions and you can test for these individually by ANDing
     *  it with the corresponding direction.
     *
     *  @code
     *  if (hats[2] & GRWL_HAT_RIGHT)
     *  {
     *      // State of hat 2 could be right-up, right or right-down
     *  }
     *  @endcode
     *
     *  If the specified joystick is not present this function will return `NULL`
     *  but will not generate an error.  This can be used instead of first calling
     *  @ref grwlJoystickPresent.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @param[out] count Where to store the number of hat states in the returned
     *  array.  This is set to zero if the joystick is not present or an error
     *  occurred.
     *  @return An array of hat states, or `NULL` if the joystick is not present
     *  or an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified joystick is
     *  disconnected, this function is called again for that joystick or the library
     *  is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_hat
     *
     *  @ingroup input
     */
    GRWLAPI const unsigned char* grwlGetJoystickHats(int jid, int* count);

    /*! @brief Returns the name of the specified joystick.
     *
     *  This function returns the name, encoded as UTF-8, of the specified joystick.
     *  The returned string is allocated and freed by GRWL.  You should not free it
     *  yourself.
     *
     *  If the specified joystick is not present this function will return `NULL`
     *  but will not generate an error.  This can be used instead of first calling
     *  @ref grwlJoystickPresent.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @return The UTF-8 encoded name of the joystick, or `NULL` if the joystick
     *  is not present or an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified joystick is
     *  disconnected or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_name
     *
     *  @ingroup input
     */
    GRWLAPI const char* grwlGetJoystickName(int jid);

    /*! @brief Returns the SDL compatible GUID of the specified joystick.
     *
     *  This function returns the SDL compatible GUID, as a UTF-8 encoded
     *  hexadecimal string, of the specified joystick.  The returned string is
     *  allocated and freed by GRWL.  You should not free it yourself.
     *
     *  The GUID is what connects a joystick to a gamepad mapping.  A connected
     *  joystick will always have a GUID even if there is no gamepad mapping
     *  assigned to it.
     *
     *  If the specified joystick is not present this function will return `NULL`
     *  but will not generate an error.  This can be used instead of first calling
     *  @ref grwlJoystickPresent.
     *
     *  The GUID uses the format introduced in SDL 2.0.5.  This GUID tries to
     *  uniquely identify the make and model of a joystick but does not identify
     *  a specific unit, e.g. all wired Xbox 360 controllers will have the same
     *  GUID on that platform.  The GUID for a unit may vary between platforms
     *  depending on what hardware information the platform specific APIs provide.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @return The UTF-8 encoded GUID of the joystick, or `NULL` if the joystick
     *  is not present or an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_INVALID_ENUM and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified joystick is
     *  disconnected or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref gamepad
     *
     *  @ingroup input
     */
    GRWLAPI const char* grwlGetJoystickGUID(int jid);

    /*! @brief Sets the user pointer of the specified joystick.
     *
     *  This function sets the user-defined pointer of the specified joystick.  The
     *  current value is retained until the joystick is disconnected.  The initial
     *  value is `NULL`.
     *
     *  This function may be called from the joystick callback, even for a joystick
     *  that is being disconnected.
     *
     *  @param[in] jid The joystick whose pointer to set.
     *  @param[in] pointer The new value.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref joystick_userptr
     *  @sa @ref grwlGetJoystickUserPointer
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetJoystickUserPointer(int jid, void* pointer);

    /*! @brief Returns the user pointer of the specified joystick.
     *
     *  This function returns the current value of the user-defined pointer of the
     *  specified joystick.  The initial value is `NULL`.
     *
     *  This function may be called from the joystick callback, even for a joystick
     *  that is being disconnected.
     *
     *  @param[in] jid The joystick whose pointer to return.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Access is not
     *  synchronized.
     *
     *  @sa @ref joystick_userptr
     *  @sa @ref grwlSetJoystickUserPointer
     *
     *  @ingroup input
     */
    GRWLAPI void* grwlGetJoystickUserPointer(int jid);

    /*! @brief Returns whether the specified joystick has a gamepad mapping.
     *
     *  This function returns whether the specified joystick is both present and has
     *  a gamepad mapping.
     *
     *  If the specified joystick is present but does not have a gamepad mapping
     *  this function will return `false` but will not generate an error.  Call
     *  @ref grwlJoystickPresent to check if a joystick is present regardless of
     *  whether it has a mapping.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @return `true` if a joystick is both present and has a gamepad mapping,
     *  or `false` otherwise.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref gamepad
     *  @sa @ref grwlGetGamepadState
     *
     *  @ingroup input
     */
    GRWLAPI int grwlJoystickIsGamepad(int jid);

    /*! @brief Sets the joystick configuration callback.
     *
     *  This function sets the joystick configuration callback, or removes the
     *  currently set callback.  This is called when a joystick is connected to or
     *  disconnected from the system.
     *
     *  For joystick connection and disconnection events to be delivered on all
     *  platforms, you need to call one of the [event processing](@ref events)
     *  functions.  Joystick disconnection may also be detected and the callback
     *  called by joystick functions.  The function will then return whatever it
     *  returns if the joystick is not present.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(int jid, int event)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLjoystickfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_event
     *
     *  @ingroup input
     */
    GRWLAPI GRWLjoystickfun grwlSetJoystickCallback(GRWLjoystickfun callback);

    /*! @brief Sets the joystick button callback.
     *
     *  This function sets the joystick configuration callback, or removes the
     *  currently set callback.  This is called when a joystick button is pressed
     *  or released.
     *
     *  For joystick button events to be delivered on all platforms,
     *  you need to call one of the [event processing](@ref events)
     *  functions.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(int jid, int button, int state)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLjoystickbuttonfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_event
     *
     *  @ingroup input
     */
    GRWLAPI GRWLjoystickbuttonfun grwlSetJoystickButtonCallback(GRWLjoystickbuttonfun callback);

    /*! @brief Sets the joystick axis callback.
     *
     *  This function sets the joystick axis callback, or removes the
     *  currently set callback.  This is called when a joystick axis moved.
     *
     *  For joystick axis events to be delivered on all platforms,
     *  you need to call one of the [event processing](@ref events)
     *  functions.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(int jid, int axis, float state)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLjoystickaxisfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_event
     *
     *  @ingroup input
     */
    GRWLAPI GRWLjoystickaxisfun grwlSetJoystickAxisCallback(GRWLjoystickaxisfun callback);

    /*! @brief Sets the joystick hat callback.
     *
     *  This function sets the joystick hat callback, or removes the
     *  currently set callback.  This is called when a joystick hat moved.
     *
     *  For joystick hat events to be delivered on all platforms,
     *  you need to call one of the [event processing](@ref events)
     *  functions.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(int jid, int hat, int state)
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLjoystickhatfun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_event
     *
     *  @ingroup input
     */
    GRWLAPI GRWLjoystickhatfun grwlSetJoystickHatCallback(GRWLjoystickhatfun callback);

    /*! @brief Sets the game pad state callback.
     *
     *  This function sets the game pad state callback, or removes the
     *  currently set callback.  This is called when a game pad state changes.
     *
     *  For game pad events to be delivered on all platforms,
     *  you need to call one of the [event processing](@ref events)
     *  functions.
     *
     *  @param[in] callback The new callback, or `NULL` to remove the currently set
     *  callback.
     *  @return The previously set callback, or `NULL` if no callback was set or the
     *  library had not been [initialized](@ref intro_init).
     *
     *  @callback_signature
     *  @code
     *  void function_name(int jid, unsigned char buttons[15], float axes[6])
     *  @endcode
     *  For more information about the callback parameters, see the
     *  [function pointer type](@ref GRWLgamepadstatefun).
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref joystick_event
     *
     *  @ingroup input
     */
    GRWLAPI GRWLgamepadstatefun grwlSetGamepadStateCallback(GRWLgamepadstatefun callback);

    /*! @brief Adds the specified SDL_GameControllerDB gamepad mappings.
     *
     *  This function parses the specified ASCII encoded string and updates the
     *  internal list with any gamepad mappings it finds.  This string may
     *  contain either a single gamepad mapping or many mappings separated by
     *  newlines.  The parser supports the full format of the `gamecontrollerdb.txt`
     *  source file including empty lines and comments.
     *
     *  See @ref gamepad_mapping for a description of the format.
     *
     *  If there is already a gamepad mapping for a given GUID in the internal list,
     *  it will be replaced by the one passed to this function.  If the library is
     *  terminated and re-initialized the internal list will revert to the built-in
     *  default.
     *
     *  @param[in] string The string containing the gamepad mappings.
     *  @return `true` if successful, or `false` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_VALUE.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref gamepad
     *  @sa @ref grwlJoystickIsGamepad
     *  @sa @ref grwlGetGamepadName
     *
     *  @ingroup input
     */
    GRWLAPI int grwlUpdateGamepadMappings(const char* string);

    /*! @brief Returns the human-readable gamepad name for the specified joystick.
     *
     *  This function returns the human-readable name of the gamepad from the
     *  gamepad mapping assigned to the specified joystick.
     *
     *  If the specified joystick is not present or does not have a gamepad mapping
     *  this function will return `NULL` but will not generate an error.  Call
     *  @ref grwlJoystickPresent to check whether it is present regardless of
     *  whether it has a mapping.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @return The UTF-8 encoded name of the gamepad, or `NULL` if the
     *  joystick is not present, does not have a mapping or an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref GRWL_INVALID_ENUM.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the specified joystick is
     *  disconnected, the gamepad mappings are updated or the library is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref gamepad
     *  @sa @ref grwlJoystickIsGamepad
     *
     *  @ingroup input
     */
    GRWLAPI const char* grwlGetGamepadName(int jid);

    /*! @brief Retrieves the state of the specified joystick remapped as a gamepad.
     *
     *  This function retrieves the state of the specified joystick remapped to
     *  an Xbox-like gamepad.
     *
     *  If the specified joystick is not present or does not have a gamepad mapping
     *  this function will return `false` but will not generate an error.  Call
     *  @ref grwlJoystickPresent to check whether it is present regardless of
     *  whether it has a mapping.
     *
     *  The Guide button may not be available for input as it is often hooked by the
     *  system or the Steam client.
     *
     *  Not all devices have all the buttons or axes provided by @ref
     *  GRWLgamepadstate.  Unavailable buttons and axes will always report
     *  `GRWL_RELEASE` and 0.0 respectively.
     *
     *  @param[in] jid The [joystick](@ref joysticks) to query.
     *  @param[out] state The gamepad input state of the joystick.
     *  @return `true` if successful, or `false` if no joystick is
     *  connected, it has no gamepad mapping or an [error](@ref error_handling)
     *  occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_ENUM.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref gamepad
     *  @sa @ref grwlUpdateGamepadMappings
     *  @sa @ref grwlJoystickIsGamepad
     *
     *  @ingroup input
     */
    GRWLAPI int grwlGetGamepadState(int jid, GRWLgamepadstate* state);

    /*! @brief Sets the clipboard to the specified string.
     *
     *  This function sets the system clipboard to the specified, UTF-8 encoded
     *  string.
     *
     *  @param[in] window Deprecated.  Any valid window or `NULL`.
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
     *  @sa @ref grwlGetClipboardString
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetClipboardString(GRWLwindow* window, const char* string);

    /*! @brief Returns the contents of the clipboard as a string.
     *
     *  This function returns the contents of the system clipboard, if it contains
     *  or is convertible to a UTF-8 encoded string.  If the clipboard is empty or
     *  if its contents cannot be converted, `NULL` is returned and a @ref
     *  GRWL_FORMAT_UNAVAILABLE error is generated.
     *
     *  @param[in] window Deprecated.  Any valid window or `NULL`.
     *  @return The contents of the clipboard as a UTF-8 encoded string, or `NULL`
     *  if an [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_FORMAT_UNAVAILABLE and @ref GRWL_PLATFORM_ERROR.
     *
     *  @pointer_lifetime The returned string is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is valid until the next call to @ref
     *  grwlGetClipboardString or @ref grwlSetClipboardString, or until the library
     *  is terminated.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref clipboard
     *  @sa @ref grwlSetClipboardString
     *
     *  @ingroup input
     */
    GRWLAPI const char* grwlGetClipboardString(GRWLwindow* window);

    /*! @brief Returns the GRWL time.
     *
     *  This function returns the current GRWL time, in seconds.  Unless the time
     *  has been set using @ref grwlSetTime it measures time elapsed since GRWL was
     *  initialized.
     *
     *  This function and @ref grwlSetTime are helper functions on top of @ref
     *  grwlGetTimerFrequency and @ref grwlGetTimerValue.
     *
     *  The resolution of the timer is system dependent, but is usually on the order
     *  of a few micro- or nanoseconds.  It uses the highest-resolution monotonic
     *  time source on each operating system.
     *
     *  @return The current time, in seconds, or zero if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.  Reading and
     *  writing of the internal base time is not atomic, so it needs to be
     *  externally synchronized with calls to @ref grwlSetTime.
     *
     *  @sa @ref time
     *
     *  @ingroup input
     */
    GRWLAPI double grwlGetTime();

    /*! @brief Sets the GRWL time.
     *
     *  This function sets the current GRWL time, in seconds.  The value must be
     *  a positive finite number less than or equal to 18446744073.0, which is
     *  approximately 584.5 years.
     *
     *  This function and @ref grwlGetTime are helper functions on top of @ref
     *  grwlGetTimerFrequency and @ref grwlGetTimerValue.
     *
     *  @param[in] time The new value, in seconds.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_INVALID_VALUE.
     *
     *  @remark The upper limit of GRWL time is calculated as
     *  floor((2<sup>64</sup> - 1) / 10<sup>9</sup>) and is due to implementations
     *  storing nanoseconds in 64 bits.  The limit may be increased in the future.
     *
     *  @thread_safety This function may be called from any thread.  Reading and
     *  writing of the internal base time is not atomic, so it needs to be
     *  externally synchronized with calls to @ref grwlGetTime.
     *
     *  @sa @ref time
     *
     *  @ingroup input
     */
    GRWLAPI void grwlSetTime(double time);

    /*! @brief Returns the current value of the raw timer.
     *
     *  This function returns the current value of the raw timer, measured in
     *  1&nbsp;/&nbsp;frequency seconds.  To get the frequency, call @ref
     *  grwlGetTimerFrequency.
     *
     *  @return The value of the timer, or zero if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref time
     *  @sa @ref grwlGetTimerFrequency
     *
     *  @ingroup input
     */
    GRWLAPI uint64_t grwlGetTimerValue();

    /*! @brief Returns the frequency, in Hz, of the raw timer.
     *
     *  This function returns the frequency, in Hz, of the raw timer.
     *
     *  @return The frequency of the timer, in Hz, or zero if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref time
     *  @sa @ref grwlGetTimerValue
     *
     *  @ingroup input
     */
    GRWLAPI uint64_t grwlGetTimerFrequency();

    /*! @brief Makes the context of the specified window current for the calling
     *  thread.
     *
     *  This function makes the OpenGL or OpenGL ES context of the specified window
     *  current on the calling thread.  A context must only be made current on
     *  a single thread at a time and each thread can have only a single current
     *  context at a time.
     *
     *  Making a context of a window current on a given thread will detach
     *  any user context which is current on that thread and visa versa.
     *
     *  When moving a context between threads, you must make it non-current on the
     *  old thread before making it current on the new one.
     *
     *  By default, making a context non-current implicitly forces a pipeline flush.
     *  On machines that support `GL_KHR_context_flush_control`, you can control
     *  whether a context performs this flush by setting the
     *  [GRWL_CONTEXT_RELEASE_BEHAVIOR](@ref GRWL_CONTEXT_RELEASE_BEHAVIOR_hint)
     *  hint.
     *
     *  The specified window must have an OpenGL or OpenGL ES context.  Specifying
     *  a window without a context will generate a @ref GRWL_NO_WINDOW_CONTEXT
     *  error.
     *
     *  @param[in] window The window whose context to make current, or `NULL` to
     *  detach the current context.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_NO_WINDOW_CONTEXT and @ref GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref context_current
     *  @sa @ref grwlGetCurrentContext
     *  @sa @ref context_current_user
     *  @sa @ref grwlMakeUserContextCurrent
     *  @sa @ref grwlGetCurrentUserContext
     *
     *  @ingroup context
     */
    GRWLAPI void grwlMakeContextCurrent(GRWLwindow* window);

    /*! @brief Returns the window whose context is current on the calling thread.
     *
     *  This function returns the window whose OpenGL or OpenGL ES context is
     *  current on the calling thread.
     *
     *  @return The window whose context is current, or `NULL` if no window's
     *  context is current.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref context_current
     *  @sa @ref grwlMakeContextCurrent
     *
     *  @ingroup context
     */
    GRWLAPI GRWLwindow* grwlGetCurrentContext();

    /*! @brief Swaps the front and back buffers of the specified window.
     *
     *  This function swaps the front and back buffers of the specified window when
     *  rendering with OpenGL or OpenGL ES.  If the swap interval is greater than
     *  zero, the GPU driver waits the specified number of screen updates before
     *  swapping the buffers.
     *
     *  The specified window must have an OpenGL or OpenGL ES context.  Specifying
     *  a window without a context will generate a @ref GRWL_NO_WINDOW_CONTEXT
     *  error.
     *
     *  This function does not apply to Vulkan.  If you are rendering with Vulkan,
     *  see `vkQueuePresentKHR` instead.
     *
     *  @param[in] window The window whose buffers to swap.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_NO_WINDOW_CONTEXT and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark __EGL:__ The context of the specified window must be current on the
     *  calling thread.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref buffer_swap
     *  @sa @ref grwlSwapInterval
     *
     *  @ingroup window
     */
    GRWLAPI void grwlSwapBuffers(GRWLwindow* window);

    /*! @brief Sets the swap interval for the current context.
     *
     *  This function sets the swap interval for the current OpenGL or OpenGL ES
     *  context, i.e. the number of screen updates to wait from the time @ref
     *  grwlSwapBuffers was called before swapping the buffers and returning.  This
     *  is sometimes called _vertical synchronization_, _vertical retrace
     *  synchronization_ or just _vsync_.
     *
     *  A context that supports either of the `WGL_EXT_swap_control_tear` and
     *  `GLX_EXT_swap_control_tear` extensions also accepts _negative_ swap
     *  intervals, which allows the driver to swap immediately even if a frame
     *  arrives a little bit late.  You can check for these extensions with @ref
     *  grwlExtensionSupported.
     *
     *  A context must be current on the calling thread.  Calling this function
     *  without a current context will cause a @ref GRWL_NO_CURRENT_CONTEXT error.
     *
     *  This function does not apply to Vulkan.  If you are rendering with Vulkan,
     *  see the present mode of your swapchain instead.
     *
     *  @param[in] interval The minimum number of screen updates to wait for
     *  until the buffers are swapped by @ref grwlSwapBuffers.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_NO_CURRENT_CONTEXT and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark This function is not called during context creation, leaving the
     *  swap interval set to whatever is the default for that API.  This is done
     *  because some swap interval extensions used by GRWL do not allow the swap
     *  interval to be reset to zero once it has been set to a non-zero value.
     *
     *  @remark Some GPU drivers do not honor the requested swap interval, either
     *  because of a user setting that overrides the application's request or due to
     *  bugs in the driver.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref buffer_swap
     *  @sa @ref grwlSwapBuffers
     *
     *  @ingroup context
     */
    GRWLAPI void grwlSwapInterval(int interval);

    /*! @brief Returns whether the specified extension is available.
     *
     *  This function returns whether the specified
     *  [API extension](@ref context_glext) is supported by the current OpenGL or
     *  OpenGL ES context.  It searches both for client API extension and context
     *  creation API extensions.
     *
     *  A context must be current on the calling thread.  Calling this function
     *  without a current context will cause a @ref GRWL_NO_CURRENT_CONTEXT error.
     *
     *  As this functions retrieves and searches one or more extension strings each
     *  call, it is recommended that you cache its results if it is going to be used
     *  frequently.  The extension strings will not change during the lifetime of
     *  a context, so there is no danger in doing this.
     *
     *  This function does not apply to Vulkan.  If you are using Vulkan, see @ref
     *  grwlGetRequiredInstanceExtensions, `vkEnumerateInstanceExtensionProperties`
     *  and `vkEnumerateDeviceExtensionProperties` instead.
     *
     *  @param[in] extension The ASCII encoded name of the extension.
     *  @return `true` if the extension is available, or `false`
     *  otherwise.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_NO_CURRENT_CONTEXT, @ref GRWL_INVALID_VALUE and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref context_glext
     *  @sa @ref grwlGetProcAddress
     *
     *  @ingroup context
     */
    GRWLAPI int grwlExtensionSupported(const char* extension);

    /*! @brief Returns the address of the specified function for the current
     *  context.
     *
     *  This function returns the address of the specified OpenGL or OpenGL ES
     *  [core or extension function](@ref context_glext), if it is supported
     *  by the current context.
     *
     *  A context must be current on the calling thread.  Calling this function
     *  without a current context will cause a @ref GRWL_NO_CURRENT_CONTEXT error.
     *
     *  This function does not apply to Vulkan.  If you are rendering with Vulkan,
     *  see @ref grwlGetInstanceProcAddress, `vkGetInstanceProcAddr` and
     *  `vkGetDeviceProcAddr` instead.
     *
     *  @param[in] procname The ASCII encoded name of the function.
     *  @return The address of the function, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_NO_CURRENT_CONTEXT and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark The address of a given function is not guaranteed to be the same
     *  between contexts.
     *
     *  @remark This function may return a non-`NULL` address despite the
     *  associated version or extension not being available.  Always check the
     *  context version or extension string first.
     *
     *  @pointer_lifetime The returned function pointer is valid until the context
     *  is destroyed or the library is terminated.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref context_glext
     *  @sa @ref grwlExtensionSupported
     *
     *  @ingroup context
     */
    GRWLAPI GRWLglproc grwlGetProcAddress(const char* procname);

    /*! @brief Create a new OpenGL or OpenGL ES user context for a window
     *
     *  This function creates a new OpenGL or OpenGL ES user context for a
     *  window, which can be used to call OpenGL or OpenGL ES functions on
     *  another thread. For a valid user context the window must be created
     *  with a [GRWL_CLIENT_API](@ref GRWL_CLIENT_API_hint) other than
     *  `GRWL_NO_API`.
     *
     *  User context creation uses the window context and framebuffer related
     *  hints to ensure a valid context is created for that window, these hints
     *  should be the same at the time of user context creation as when the
     *  window was created.
     *
     *  Contexts share resources with the window context and with any other
     *  user context created for that window.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED,
     *  @ref GRWL_INVALID_VALUE the window parameter is `NULL`,
     *  @ref GRWL_NO_WINDOW_CONTEXT if the window has no OpenGL or
     *  OpenGL US context, and @ref GRWL_PLATFORM_ERROR.
     *
     *  @param[in] window The Window for which the user context is to be
     *  created.
     *  @return The handle of the user context created, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref context_user
     *  @sa @ref usercontext_creation
     *  @sa @ref grwlDestroyUserContext
     *  @sa @ref window_creation
     *  @sa @ref grwlCreateWindow
     *  @sa @ref grwlDestroyWindow
     *
     *  @ingroup context
     */
    GRWLAPI GRWLusercontext* grwlCreateUserContext(GRWLwindow* window);

    /*! @brief Destroys the specified user context
     *
     *  This function destroys the specified user context.
     *  User contexts should be destroyed before destroying the
     *  window they were made with.
     *
     *  If the user context is current on the main thread, it is
     *  detached before being destroyed.
     *
     *  @param[in] context The user context to destroy.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_PLATFORM_ERROR.
     *
     *  @note The user context must not be current on any other
     *  thread when this function is called.
     *
     *  @reentrancy This function must not be called from a callback.
     *
     *  @thread_safety This function must only be called from the main thread.
     *
     *  @sa @ref context_user
     *  @sa @ref usercontext_creation
     *  @sa @ref grwlCreateUserContext
     *  @sa @ref window_creation
     *  @sa @ref grwlCreateWindow
     *  @sa @ref grwlDestroyWindow
     *
     *  @ingroup context
     */
    GRWLAPI void grwlDestroyUserContext(GRWLusercontext* context);

    /*! @brief Makes the user context current for the calling thread.
     *
     *  This function makes the OpenGL or OpenGL ES context of the specified user
     *  context current on the calling thread.  A context must only be made current on
     *  a single thread at a time and each thread can have only a single current
     *  context at a time.
     *
     *  Making a user context current on a given thread will detach the context of
     *  any window which is current on that thread and visa versa.
     *
     *  When moving a context between threads, you must make it non-current on the
     *  old thread before making it current on the new one.
     *
     *  By default, making a context non-current implicitly forces a pipeline flush.
     *  On machines that support `GL_KHR_context_flush_control`, you can control
     *  whether a context performs this flush by setting the
     *  [GRWL_CONTEXT_RELEASE_BEHAVIOR](@ref GRWL_CONTEXT_RELEASE_BEHAVIOR_hint)
     *  hint.
     *
     *  @param[in] context The user context to make current, or `NULL` to
     *  detach the current context.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED,
     *  and @ref GRWL_PLATFORM_ERROR.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref context_user
     *  @sa @ref context_current_user
     *  @sa @ref grwlGetCurrentUserContext
     *  @sa @ref context_current
     *  @sa @ref grwlMakeContextCurrent
     *  @sa @ref grwlGetCurrentContext
     *
     *  @ingroup context
     */
    GRWLAPI void grwlMakeUserContextCurrent(GRWLusercontext* context);

    /*! @brief Returns the current OpenGL or OpenGL ES user context
     *
     *  This function returns the user context which is current
     *  on the calling thread.
     *
     *  @return The user context current, or `NULL` if no user context
     *  is current.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref context_user
     *  @sa @ref context_current_user
     *  @sa @ref grwlMakeUserContextCurrent
     *  @sa @ref context_current
     *  @sa @ref grwlMakeContextCurrent
     *  @sa @ref grwlGetCurrentContext
     *
     *  @ingroup context
     */
    GRWLAPI GRWLusercontext* grwlGetCurrentUserContext();

    /*! @brief Returns whether the Vulkan loader and an ICD have been found.
     *
     *  This function returns whether the Vulkan loader and any minimally functional
     *  ICD have been found.
     *
     *  The availability of a Vulkan loader and even an ICD does not by itself guarantee that
     *  surface creation or even instance creation is possible.  Call @ref
     *  grwlGetRequiredInstanceExtensions to check whether the extensions necessary for Vulkan
     *  surface creation are available and @ref grwlGetPhysicalDevicePresentationSupport to
     *  check whether a queue family of a physical device supports image presentation.
     *
     *  @return `true` if Vulkan is minimally available, or `false`
     *  otherwise.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref vulkan_support
     *
     *  @ingroup vulkan
     */
    GRWLAPI int grwlVulkanSupported();

    /*! @brief Returns the Vulkan instance extensions required by GRWL.
     *
     *  This function returns an array of names of Vulkan instance extensions required
     *  by GRWL for creating Vulkan surfaces for GRWL windows.  If successful, the
     *  list will always contain `VK_KHR_surface`, so if you don't require any
     *  additional extensions you can pass this list directly to the
     *  `VkInstanceCreateInfo` struct.
     *
     *  If Vulkan is not available on the machine, this function returns `NULL` and
     *  generates a @ref GRWL_API_UNAVAILABLE error.  Call @ref grwlVulkanSupported
     *  to check whether Vulkan is at least minimally available.
     *
     *  If Vulkan is available but no set of extensions allowing window surface
     *  creation was found, this function returns `NULL`.  You may still use Vulkan
     *  for off-screen rendering and compute work.
     *
     *  @param[out] count Where to store the number of extensions in the returned
     *  array.  This is set to zero if an error occurred.
     *  @return An array of ASCII encoded extension names, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_API_UNAVAILABLE.
     *
     *  @remark Additional extensions may be required by future versions of GRWL.
     *  You should check if any extensions you wish to enable are already in the
     *  returned array, as it is an error to specify an extension more than once in
     *  the `VkInstanceCreateInfo` struct.
     *
     *  @pointer_lifetime The returned array is allocated and freed by GRWL.  You
     *  should not free it yourself.  It is guaranteed to be valid only until the
     *  library is terminated.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref vulkan_ext
     *  @sa @ref grwlCreateWindowSurface
     *
     *  @ingroup vulkan
     */
    GRWLAPI const char** grwlGetRequiredInstanceExtensions(uint32_t* count);

#if defined(VK_VERSION_1_0)

    /*! @brief Returns the address of the specified Vulkan instance function.
     *
     *  This function returns the address of the specified Vulkan core or extension
     *  function for the specified instance.  If instance is set to `NULL` it can
     *  return any function exported from the Vulkan loader, including at least the
     *  following functions:
     *
     *  - `vkEnumerateInstanceExtensionProperties`
     *  - `vkEnumerateInstanceLayerProperties`
     *  - `vkCreateInstance`
     *  - `vkGetInstanceProcAddr`
     *
     *  If Vulkan is not available on the machine, this function returns `NULL` and
     *  generates a @ref GRWL_API_UNAVAILABLE error.  Call @ref grwlVulkanSupported
     *  to check whether Vulkan is at least minimally available.
     *
     *  This function is equivalent to calling `vkGetInstanceProcAddr` with
     *  a platform-specific query of the Vulkan loader as a fallback.
     *
     *  @param[in] instance The Vulkan instance to query, or `NULL` to retrieve
     *  functions related to instance creation.
     *  @param[in] procname The ASCII encoded name of the function.
     *  @return The address of the function, or `NULL` if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED and @ref
     *  GRWL_API_UNAVAILABLE.
     *
     *  @pointer_lifetime The returned function pointer is valid until the library
     *  is terminated.
     *
     *  @thread_safety This function may be called from any thread.
     *
     *  @sa @ref vulkan_proc
     *
     *  @ingroup vulkan
     */
    GRWLAPI GRWLvkproc grwlGetInstanceProcAddress(VkInstance instance, const char* procname);

    /*! @brief Returns whether the specified queue family can present images.
     *
     *  This function returns whether the specified queue family of the specified
     *  physical device supports presentation to the platform GRWL was built for.
     *
     *  If Vulkan or the required window surface creation instance extensions are
     *  not available on the machine, or if the specified instance was not created
     *  with the required extensions, this function returns `false` and
     *  generates a @ref GRWL_API_UNAVAILABLE error.  Call @ref grwlVulkanSupported
     *  to check whether Vulkan is at least minimally available and @ref
     *  grwlGetRequiredInstanceExtensions to check what instance extensions are
     *  required.
     *
     *  @param[in] instance The instance that the physical device belongs to.
     *  @param[in] device The physical device that the queue family belongs to.
     *  @param[in] queuefamily The index of the queue family to query.
     *  @return `true` if the queue family supports presentation, or
     *  `false` otherwise.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_API_UNAVAILABLE and @ref GRWL_PLATFORM_ERROR.
     *
     *  @remark @macos This function currently always returns `true`, as the
     *  `VK_MVK_macos_surface` and `VK_EXT_metal_surface` extensions do not provide
     *  a `vkGetPhysicalDevice*PresentationSupport` type function.
     *
     *  @thread_safety This function may be called from any thread.  For
     *  synchronization details of Vulkan objects, see the Vulkan specification.
     *
     *  @sa @ref vulkan_present
     *
     *  @ingroup vulkan
     */
    GRWLAPI int grwlGetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device,
                                                         uint32_t queuefamily);

    /*! @brief Creates a Vulkan surface for the specified window.
     *
     *  This function creates a Vulkan surface for the specified window.
     *
     *  If the Vulkan loader or at least one minimally functional ICD were not found,
     *  this function returns `VK_ERROR_INITIALIZATION_FAILED` and generates a @ref
     *  GRWL_API_UNAVAILABLE error.  Call @ref grwlVulkanSupported to check whether
     *  Vulkan is at least minimally available.
     *
     *  If the required window surface creation instance extensions are not
     *  available or if the specified instance was not created with these extensions
     *  enabled, this function returns `VK_ERROR_EXTENSION_NOT_PRESENT` and
     *  generates a @ref GRWL_API_UNAVAILABLE error.  Call @ref
     *  grwlGetRequiredInstanceExtensions to check what instance extensions are
     *  required.
     *
     *  The window surface cannot be shared with another API so the window must
     *  have been created with the [client api hint](@ref GRWL_CLIENT_API_attrib)
     *  set to `GRWL_NO_API` otherwise it generates a @ref GRWL_INVALID_VALUE error
     *  and returns `VK_ERROR_NATIVE_WINDOW_IN_USE_KHR`.
     *
     *  The window surface must be destroyed before the specified Vulkan instance.
     *  It is the responsibility of the caller to destroy the window surface.  GRWL
     *  does not destroy it for you.  Call `vkDestroySurfaceKHR` to destroy the
     *  surface.
     *
     *  @param[in] instance The Vulkan instance to create the surface in.
     *  @param[in] window The window to create the surface for.
     *  @param[in] allocator The allocator to use, or `NULL` to use the default
     *  allocator.
     *  @param[out] surface Where to store the handle of the surface.  This is set
     *  to `VK_NULL_HANDLE` if an error occurred.
     *  @return `VK_SUCCESS` if successful, or a Vulkan error code if an
     *  [error](@ref error_handling) occurred.
     *
     *  @errors Possible errors include @ref GRWL_NOT_INITIALIZED, @ref
     *  GRWL_API_UNAVAILABLE, @ref GRWL_PLATFORM_ERROR and @ref GRWL_INVALID_VALUE
     *
     *  @remark If an error occurs before the creation call is made, GRWL returns
     *  the Vulkan error code most appropriate for the error.  Appropriate use of
     *  @ref grwlVulkanSupported and @ref grwlGetRequiredInstanceExtensions should
     *  eliminate almost all occurrences of these errors.
     *
     *  @remark @macos GRWL prefers the `VK_EXT_metal_surface` extension, with the
     *  `VK_MVK_macos_surface` extension as a fallback.  The name of the selected
     *  extension, if any, is included in the array returned by @ref
     *  grwlGetRequiredInstanceExtensions.
     *
     *  @remark @macos This function creates and sets a `CAMetalLayer` instance for
     *  the window content view, which is required for MoltenVK to function.
     *
     *  @remark @x11 By default GRWL prefers the `VK_KHR_xcb_surface` extension,
     *  with the `VK_KHR_xlib_surface` extension as a fallback.  You can make
     *  `VK_KHR_xlib_surface` the preferred extension by setting the
     *  [GRWL_X11_XCB_VULKAN_SURFACE](@ref GRWL_X11_XCB_VULKAN_SURFACE_hint) init
     *  hint.  The name of the selected extension, if any, is included in the array
     *  returned by @ref grwlGetRequiredInstanceExtensions.
     *
     *  @thread_safety This function may be called from any thread.  For
     *  synchronization details of Vulkan objects, see the Vulkan specification.
     *
     *  @sa @ref vulkan_surface
     *  @sa @ref grwlGetRequiredInstanceExtensions
     *
     *  @ingroup vulkan
     */
    GRWLAPI VkResult grwlCreateWindowSurface(VkInstance instance, GRWLwindow* window,
                                             const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

#endif /*VK_VERSION_1_0*/

#if defined(WEBGPU_H_)

    /*! @brief Creates a WebGPU surface for the specified window.
     *
     *  This function creates a WGPUSurface object for the specified window.
     *
     *  If the surface cannot be created, this function returns `NULL`.
     *
     *  It is the responsibility of the caller to destroy the window surface. The
     *  window surface must be destroyed using `wgpuSurfaceDrop` (wgpu-native) or
     *  `wgpuSurfaceRelease` (Dawn/emscripten).
     *
     *  @param[in] instance The WebGPU instance to create the surface in.
     *  @param[in] window The window to create the surface for.
     *  @return The handle of the surface.  This is set to `NULL` if an error
     *  occurred.
     *
     *  @ingroup webgpu
     */
    GRWLAPI WGPUSurface grwlCreateWindowWGPUSurface(WGPUInstance instance, GRWLwindow* window);

#endif /* WEBGPU_H_ */

    /*************************************************************************
     * Global definition cleanup
     *************************************************************************/

    /* ------------------- BEGIN SYSTEM/COMPILER SPECIFIC -------------------- */

#ifdef GRWL_WINGDIAPI_DEFINED
    #undef WINGDIAPI
    #undef GRWL_WINGDIAPI_DEFINED
#endif

#ifdef GRWL_CALLBACK_DEFINED
    #undef CALLBACK
    #undef GRWL_CALLBACK_DEFINED
#endif

/* Some OpenGL related headers need GLAPIENTRY, but it is unconditionally
 * defined by some gl.h variants (OpenBSD) so define it after if needed.
 */
#ifndef GLAPIENTRY
    #define GLAPIENTRY APIENTRY
    #define GRWL_GLAPIENTRY_DEFINED
#endif

    /* -------------------- END SYSTEM/COMPILER SPECIFIC --------------------- */

#ifdef __cplusplus
}
#endif

#endif /* _grwl_h_ */
