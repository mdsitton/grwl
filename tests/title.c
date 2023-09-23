//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// UTF-8 window title test
//
// This test sets a UTF-8 window title

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(void)
{
    GRWLwindow* window;

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    window = grwlCreateWindow(400, 400, "English 日本語 русский язык 官話", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSwapInterval(1);

    while (!grwlWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        grwlSwapBuffers(window);
        grwlWaitEvents();
    }

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
