cmake_minimum_required(VERSION 3.16...3.20 FATAL_ERROR)

project(GRWL VERSION 3.4.0 LANGUAGES C CXX)

if (POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" GRWL_STANDALONE)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GRWL_BUILD_EXAMPLES "Build the GRWL example programs" ${GRWL_STANDALONE})
option(GRWL_BUILD_TESTS "Build the GRWL test programs" ${GRWL_STANDALONE})
option(GRWL_BUILD_DOCS "Build the GRWL documentation" ON)
option(GRWL_INSTALL "Generate installation target" ON)

include(GNUInstallDirs)
include(CMakeDependentOption)

cmake_dependent_option(GRWL_BUILD_WIN32 "Build support for Win32" ON "WIN32" OFF)
cmake_dependent_option(GRWL_BUILD_COCOA "Build support for Cocoa" ON "APPLE" OFF)
cmake_dependent_option(GRWL_BUILD_X11 "Build support for X11" ON "UNIX;NOT APPLE" OFF)
cmake_dependent_option(GRWL_BUILD_WAYLAND "Build support for Wayland"
    "${GRWL_USE_WAYLAND}" "UNIX;NOT APPLE" OFF)

cmake_dependent_option(GRWL_USE_HYBRID_HPG "Force use of high-performance GPU on hybrid systems" OFF
                       "WIN32" OFF)
cmake_dependent_option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON
                       "MSVC" OFF)
option(GRWL_BUILD_WEBGPU "Build support for WebGPU" OFF)

set(GRWL_LIBRARY_TYPE "${GRWL_LIBRARY_TYPE}" CACHE STRING
    "Library type override for GRWL (SHARED, STATIC, OBJECT, or empty to follow BUILD_SHARED_LIBS)")

if (GRWL_LIBRARY_TYPE)
    if (GRWL_LIBRARY_TYPE STREQUAL "SHARED")
        set(GRWL_BUILD_SHARED_LIBRARY TRUE)
    else()
        set(GRWL_BUILD_SHARED_LIBRARY FALSE)
    endif()
else()
    set(GRWL_BUILD_SHARED_LIBRARY ${BUILD_SHARED_LIBS})
endif()

list(APPEND CMAKE_MODULE_PATH "${GRWL_SOURCE_DIR}/CMake/modules")

if (GRWL_BUILD_DOCS)
    set(DOXYGEN_SKIP_DOT TRUE)
    find_package(Doxygen)
endif()

#--------------------------------------------------------------------
# Report backend selection
#--------------------------------------------------------------------
if (GRWL_BUILD_WIN32)
    message(STATUS "Including Win32 support")
endif()
if (GRWL_BUILD_COCOA)
    message(STATUS "Including Cocoa support")
endif()
if (GRWL_BUILD_WAYLAND)
    message(STATUS "Including Wayland support")
endif()
if (GRWL_BUILD_X11)
    message(STATUS "Including X11 support")
endif()
if (GRWL_BUILD_WEBGPU)
    message(STATUS "Including WebGPU support")
endif()


#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------
include(CMakePackageConfigHelpers)

set(GRWL_CONFIG_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/grwl")

configure_package_config_file(CMake/grwlConfig.cmake.in
                              src/grwlConfig.cmake
                              INSTALL_DESTINATION "${GRWL_CONFIG_PATH}"
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(src/grwlConfigVersion.cmake
                                 VERSION ${GRWL_VERSION}
                                 COMPATIBILITY SameMajorVersion)

#--------------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------------
add_subdirectory(src)

if (DOXYGEN_FOUND AND GRWL_BUILD_DOCS)
    add_subdirectory(docs)
endif()

#--------------------------------------------------------------------
# Install files other than the library
# The library is installed by src/CMakeLists.txt
#--------------------------------------------------------------------
if (GRWL_INSTALL)
    install(DIRECTORY include/GRWL DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN grwl.h PATTERN grwlnative.h)

    install(FILES "${GRWL_BINARY_DIR}/src/grwlConfig.cmake"
                  "${GRWL_BINARY_DIR}/src/grwlConfigVersion.cmake"
            DESTINATION "${GRWL_CONFIG_PATH}")

    install(EXPORT grwlTargets FILE grwlTargets.cmake
            EXPORT_LINK_INTERFACE_LIBRARIES
            DESTINATION "${GRWL_CONFIG_PATH}")
    install(FILES "${GRWL_BINARY_DIR}/src/grwl.pc"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

    if (DOXYGEN_FOUND AND GRWL_BUILD_DOCS)
        install(DIRECTORY "${GRWL_BINARY_DIR}/docs/html"
                DESTINATION "${CMAKE_INSTALL_DOCDIR}")
    endif()
endif()

