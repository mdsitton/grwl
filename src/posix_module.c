//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#if defined(GRWL_BUILD_POSIX_MODULE)

    #include <dlfcn.h>

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

void* _grwlPlatformLoadModule(const char* path)
{
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
}

void _grwlPlatformFreeModule(void* module)
{
    dlclose(module);
}

GRWLproc _grwlPlatformGetModuleSymbol(void* module, const char* name)
{
    return dlsym(module, name);
}

#endif // GRWL_BUILD_POSIX_MODULE
