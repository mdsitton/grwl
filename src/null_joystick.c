//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

GRWLbool _grwlInitJoysticksNull(void)
{
    return GRWL_TRUE;
}

void _grwlTerminateJoysticksNull(void)
{
}

GRWLbool _grwlPollJoystickNull(_GRWLjoystick* js, int mode)
{
    return GRWL_FALSE;
}

const char* _grwlGetMappingNameNull(void)
{
    return "";
}

void _grwlUpdateGamepadGUIDNull(char* guid)
{
}
