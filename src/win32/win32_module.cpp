//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(GRWL_BUILD_WIN32_MODULE)

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void* _grwlPlatformLoadModule(const char* path)
{
    return LoadLibraryA(path);
}

void _grwlPlatformFreeModule(void* module)
{
    FreeLibrary((HMODULE)module);
}

GRWLproc _grwlPlatformGetModuleSymbol(void* module, const char* name)
{
    return (GRWLproc)GetProcAddress((HMODULE)module, name);
}

#endif // GRWL_BUILD_WIN32_MODULE
