
add_library(grwl ${GRWL_LIBRARY_TYPE}
                 "${GRWL_SOURCE_DIR}/include/GRWL/grwl.h"
                 "${GRWL_SOURCE_DIR}/include/GRWL/grwlnative.h"
                 internal.hpp platform.hpp mappings.hpp
                 context.cpp init.cpp input.cpp monitor.cpp platform.cpp vulkan.cpp webgpu.cpp
                 window.cpp egl_context.cpp)

if (APPLE)
    target_sources(grwl PRIVATE
        mac/cocoa_time.hpp
        mac/cocoa_time.cpp
        posix_module.cpp
        posix_thread.cpp
        posix_thread.hpp)
elseif (WIN32)
    target_sources(grwl PRIVATE
        win32/win32_time.hpp
        win32/win32_thread.hpp
        win32/win32_module.cpp
        win32/win32_time.cpp
        win32/win32_thread.cpp)
else()
    target_sources(grwl PRIVATE
        linux/posix_time.hpp
        linux/posix_time.cpp
        linux/posix_dbus.hpp
        linux/posix_dbus.cpp
        posix_module.cpp
        posix_thread.cpp
        posix_thread.hpp)
endif()

add_custom_target(update_mappings
    COMMAND "${CMAKE_COMMAND}" -P "${GRWL_SOURCE_DIR}/CMake/GenerateMappings.cmake" mappings.hpp.in mappings.hpp
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Updating gamepad mappings from upstream repository"
    SOURCES mappings.hpp.in "${GRWL_SOURCE_DIR}/CMake/GenerateMappings.cmake"
    VERBATIM)

set_target_properties(update_mappings PROPERTIES FOLDER "GRWL")

if (GRWL_BUILD_COCOA)
    target_compile_definitions(grwl PRIVATE _GRWL_COCOA)
    target_sources(grwl PRIVATE
        mac/cocoa_platform.hpp
        mac/cocoa_joystick.hpp
        mac/cocoa_init.m
        mac/cocoa_joystick.m
        mac/cocoa_monitor.m
        mac/cocoa_window.m
        mac/nsgl_context.m)
endif()

if (GRWL_BUILD_WIN32)
    target_compile_definitions(grwl PRIVATE _GRWL_WIN32)
    target_sources(grwl PRIVATE
        win32/win32_platform.hpp
        win32/win32_joystick.hpp
        win32/win32_init.cpp
        win32/win32_joystick.cpp
        win32/win32_monitor.cpp
        win32/win32_window.cpp
        win32/wgl_context.cpp)
endif()

if (GRWL_BUILD_X11)
    target_compile_definitions(grwl PRIVATE _GRWL_X11)
    target_sources(grwl PRIVATE
        linux/x11_platform.hpp
        linux/xkb_unicode.hpp
        linux/x11_init.cpp
        linux/x11_monitor.cpp
        linux/x11_window.cpp
        linux/xkb_unicode.cpp
        linux/glx_context.cpp)
endif()

if (GRWL_BUILD_WAYLAND)
    target_compile_definitions(grwl PRIVATE _GRWL_WAYLAND)
    target_sources(grwl PRIVATE
        linux/wl_platform.hpp
        linux/xkb_unicode.hpp
        linux/wl_init.cpp
        linux/wl_monitor.cpp
        linux/wl_window.cpp
        linux/xkb_unicode.cpp)
endif()

if (GRWL_BUILD_X11 OR GRWL_BUILD_WAYLAND)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(grwl PRIVATE
            linux/linux_joystick.hpp
            linux/linux_joystick.cpp)
    endif()
    target_sources(grwl PRIVATE
        linux/posix_poll.hpp
        linux/posix_poll.cpp)
endif()

if (GRWL_BUILD_WAYLAND)
    include(CheckIncludeFiles)
    include(CheckFunctionExists)
    check_function_exists(memfd_create HAVE_MEMFD_CREATE)
    if (HAVE_MEMFD_CREATE)
        target_compile_definitions(grwl PRIVATE HAVE_MEMFD_CREATE)
    endif()

    find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

    include(FindPkgConfig)
    pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.15)
    pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)
    pkg_get_variable(WAYLAND_CLIENT_PKGDATADIR wayland-client pkgdatadir)

    macro(wayland_generate protocol_file output_file)
        add_custom_command(OUTPUT "${output_file}.h"
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_file}" "${output_file}.h"
            DEPENDS "${protocol_file}"
            VERBATIM)

        add_custom_command(OUTPUT "${output_file}-code.h"
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_file}" "${output_file}-code.h"
            DEPENDS "${protocol_file}"
            VERBATIM)

        target_sources(grwl PRIVATE "${output_file}.h" "${output_file}-code.h")
    endmacro()

    wayland_generate(
        "${WAYLAND_CLIENT_PKGDATADIR}/wayland.xml"
        "${GRWL_BINARY_DIR}/src/wayland-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/stable/xdg-shell/xdg-shell.xml"
        "${GRWL_BINARY_DIR}/src/wayland-xdg-shell-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
        "${GRWL_BINARY_DIR}/src/wayland-xdg-decoration-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/stable/viewporter/viewporter.xml"
        "${GRWL_BINARY_DIR}/src/wayland-viewporter-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
        "${GRWL_BINARY_DIR}/src/wayland-relative-pointer-unstable-v1-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
        "${GRWL_BINARY_DIR}/src/wayland-pointer-constraints-unstable-v1-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml"
        "${GRWL_BINARY_DIR}/src/wayland-idle-inhibit-unstable-v1-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/text-input/text-input-unstable-v1.xml"
        "${GRWL_BINARY_DIR}/src/wayland-text-input-unstable-v1-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/text-input/text-input-unstable-v3.xml"
        "${GRWL_BINARY_DIR}/src/wayland-text-input-unstable-v3-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/staging/xdg-activation/xdg-activation-v1.xml"
        "${GRWL_BINARY_DIR}/src/wayland-xdg-activation-v1-client-protocol")
endif()

if (WIN32 AND GRWL_BUILD_SHARED_LIBRARY)
    configure_file(grwl.rc.in grwl.rc @ONLY)
    target_sources(grwl PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/grwl.rc")
endif()

if (UNIX AND GRWL_BUILD_SHARED_LIBRARY)
    # On Unix-like systems, shared libraries can use the soname system.
    set(GRWL_LIB_NAME grwl)
else()
    set(GRWL_LIB_NAME grwl)
endif()

set_target_properties(grwl PROPERTIES
                      OUTPUT_NAME ${GRWL_LIB_NAME}
                      VERSION ${GRWL_VERSION_MAJOR}.${GRWL_VERSION_MINOR}
                      SOVERSION ${GRWL_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99
                      C_EXTENSIONS OFF
                      DEFINE_SYMBOL _GRWL_BUILD_DLL
                      FOLDER "GRWL")

target_include_directories(grwl PUBLIC
                           "$<BUILD_INTERFACE:${GRWL_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_include_directories(grwl PRIVATE
                           "${GRWL_SOURCE_DIR}/src"
                           "${GRWL_BINARY_DIR}/src")

if (GRWL_BUILD_WIN32)
    list(APPEND grwl_PKG_LIBS "-lgdi32")
endif()

if (GRWL_BUILD_COCOA)
    target_link_libraries(grwl PRIVATE "-framework Cocoa"
                                       "-framework IOKit"
                                       "-framework CoreFoundation")

    set(grwl_PKG_DEPS "")
    set(grwl_PKG_LIBS "-framework Cocoa -framework IOKit -framework CoreFoundation")
endif()

if (GRWL_BUILD_WAYLAND)
    pkg_check_modules(Wayland REQUIRED
        wayland-client>=0.2.7
        wayland-cursor>=0.2.7
        wayland-egl>=0.2.7
        xkbcommon>=0.5.0)

    target_include_directories(grwl PRIVATE ${Wayland_INCLUDE_DIRS})

    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(EpollShim)
        if (EPOLLSHIM_FOUND)
            target_include_directories(grwl PRIVATE ${EPOLLSHIM_INCLUDE_DIRS})
            target_link_libraries(grwl PRIVATE ${EPOLLSHIM_LIBRARIES})
        endif()
    endif()
endif()

if (GRWL_BUILD_X11)
    find_package(X11 REQUIRED)
    target_include_directories(grwl PRIVATE "${X11_X11_INCLUDE_PATH}")

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_INCLUDE_PATH)
        message(FATAL_ERROR "RandR headers not found; install libxrandr development package")
    endif()
    target_include_directories(grwl PRIVATE "${X11_Xrandr_INCLUDE_PATH}")

    # Check for Xinerama (legacy multi-monitor support)
    if (NOT X11_Xinerama_INCLUDE_PATH)
        message(FATAL_ERROR "Xinerama headers not found; install libxinerama development package")
    endif()
    target_include_directories(grwl PRIVATE "${X11_Xinerama_INCLUDE_PATH}")

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_INCLUDE_PATH)
        message(FATAL_ERROR "XKB headers not found; install X11 development package")
    endif()
    target_include_directories(grwl PRIVATE "${X11_Xkb_INCLUDE_PATH}")

    # Check for Xcursor (cursor creation from RGBA images)
    if (NOT X11_Xcursor_INCLUDE_PATH)
        message(FATAL_ERROR "Xcursor headers not found; install libxcursor development package")
    endif()
    target_include_directories(grwl PRIVATE "${X11_Xcursor_INCLUDE_PATH}")

    # Check for XInput (modern HID input)
    if (NOT X11_Xi_INCLUDE_PATH)
        message(FATAL_ERROR "XInput headers not found; install libxi development package")
    endif()
    target_include_directories(grwl PRIVATE "${X11_Xi_INCLUDE_PATH}")

    # Check for X Shape (custom window input shape)
    if (NOT X11_Xshape_INCLUDE_PATH)
        message(FATAL_ERROR "X Shape headers not found; install libxext development package")
    endif()
    target_include_directories(grwl PRIVATE "${X11_Xshape_INCLUDE_PATH}")
endif()

if (UNIX AND NOT APPLE)
    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        target_link_libraries(grwl PRIVATE "${RT_LIBRARY}")
        list(APPEND grwl_PKG_LIBS "-lrt")
    endif()

    if (CMAKE_DL_LIBS)
        target_link_libraries(grwl PRIVATE "${CMAKE_DL_LIBS}")
        list(APPEND grwl_PKG_LIBS "-l${CMAKE_DL_LIBS}")
    endif()
endif()

if (WIN32)
    if (GRWL_USE_HYBRID_HPG)
        target_compile_definitions(grwl PRIVATE _GRWL_USE_HYBRID_HPG)
    endif()
endif()

# Enable a reasonable set of warnings
# NOTE: The order matters here, Clang-CL matches both MSVC and Clang
if (MSVC)
    target_compile_options(grwl PRIVATE "/W3")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
        CMAKE_C_COMPILER_ID STREQUAL "AppleClang")

    target_compile_options(grwl PRIVATE "-Wall")
endif()

if (GRWL_BUILD_WIN32)
    target_compile_definitions(grwl PRIVATE UNICODE _UNICODE)
endif()

# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    target_compile_definitions(grwl PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Workaround for -std=c99 on Linux disabling _DEFAULT_SOURCE (POSIX 2008 and more)
if (GRWL_BUILD_X11 OR GRWL_BUILD_WAYLAND)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(grwl PRIVATE _DEFAULT_SOURCE)
    endif()
endif()

if (GRWL_BUILD_WEBGPU)
    target_compile_definitions(grwl PRIVATE _GRWL_BUILD_WEBGPU)
    if (APPLE)
      target_compile_options(grwl PRIVATE -x objective-c)
      target_link_libraries(grwl PRIVATE "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")
    endif ()
endif ()

if (GRWL_BUILD_SHARED_LIBRARY)
    if (WIN32)
        if (MINGW)
            set_target_properties(grwl PROPERTIES
                PREFIX ""              # Remove the lib prefix on the DLL (but not the import library)
                IMPORT_SUFFIX "dll.a") # Add a suffix to the import library to avoid naming conflicts
        else()
            # Add a suffix to the import library to avoid naming conflicts
            set_target_properties(grwl PROPERTIES IMPORT_SUFFIX "dll.lib")
        endif()

        target_compile_definitions(grwl INTERFACE GRWL_DLL)
    endif()

    # set default visibility
    set_target_properties(grwl PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1
    )

endif()

foreach(arg ${grwl_PKG_DEPS})
    string(APPEND deps " ${arg}")
endforeach()
foreach(arg ${grwl_PKG_LIBS})
    string(APPEND libs " ${arg}")
endforeach()

set(GRWL_PKG_CONFIG_REQUIRES_PRIVATE "${deps}" CACHE INTERNAL
    "GRWL pkg-config Requires.private")
set(GRWL_PKG_CONFIG_LIBS_PRIVATE "${libs}" CACHE INTERNAL
    "GRWL pkg-config Libs.private")

configure_file("${GRWL_SOURCE_DIR}/CMake/grwl.pc.in" grwl.pc @ONLY)

if (GRWL_INSTALL)
    install(TARGETS grwl
            EXPORT grwlTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

