//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwInitJoysticksNull(void)
{
    return GLFW_TRUE;
}

void _glfwTerminateJoysticksNull(void)
{
}

GLFWbool _glfwPollJoystickNull(_GLFWjoystick* js, int mode)
{
    return GLFW_FALSE;
}

const char* _glfwGetMappingNameNull(void)
{
    return "";
}

void _glfwUpdateGamepadGUIDNull(char* guid)
{
}
