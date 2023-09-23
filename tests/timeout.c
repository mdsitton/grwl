//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Event wait timeout test
//
// This test is intended to verify that waiting for events with timeout works

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GRWL_KEY_ESCAPE && action == GRWL_PRESS)
    {
        grwlSetWindowShouldClose(window, GRWL_TRUE);
    }
}

static float nrand(void)
{
    return (float)rand() / (float)RAND_MAX;
}

int main(void)
{
    GRWLwindow* window;

    srand((unsigned int)time(NULL));

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    window = grwlCreateWindow(640, 480, "Event Wait Timeout Test", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSetKeyCallback(window, key_callback);

    while (!grwlWindowShouldClose(window))
    {
        int width, height;
        float r = nrand(), g = nrand(), b = nrand();
        float l = (float)sqrt(r * r + g * g + b * b);

        grwlGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(r / l, g / l, b / l, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        grwlSwapBuffers(window);

        grwlWaitEventsTimeout(1.0);
    }

    grwlDestroyWindow(window);

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
