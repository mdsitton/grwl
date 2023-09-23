//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(GLFW_BUILD_WIN32_MODULE)

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void* _glfwPlatformLoadModule(const char* path)
{
    return LoadLibraryA(path);
}

void _glfwPlatformFreeModule(void* module)
{
    FreeLibrary((HMODULE)module);
}

GLFWproc _glfwPlatformGetModuleSymbol(void* module, const char* name)
{
    return (GLFWproc)GetProcAddress((HMODULE)module, name);
}

#endif // GLFW_BUILD_WIN32_MODULE
