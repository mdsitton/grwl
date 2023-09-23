//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// This test is intended to verify that posting of empty events works

#include "tinycthread.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static volatile int running = GRWL_TRUE;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static int thread_main(void* data)
{
    struct timespec time;

    while (running)
    {
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += 1;
        thrd_sleep(&time, NULL);

        grwlPostEmptyEvent();
    }

    return 0;
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
    int result;
    thrd_t thread;
    GRWLwindow* window;

    srand((unsigned int)time(NULL));

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    window = grwlCreateWindow(640, 480, "Empty Event Test", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSetKeyCallback(window, key_callback);

    if (thrd_create(&thread, thread_main, NULL) != thrd_success)
    {
        fprintf(stderr, "Failed to create secondary thread\n");

        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    while (running)
    {
        int width, height;
        float r = nrand(), g = nrand(), b = nrand();
        float l = (float)sqrt(r * r + g * g + b * b);

        grwlGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(r / l, g / l, b / l, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        grwlSwapBuffers(window);

        grwlWaitEvents();

        if (grwlWindowShouldClose(window))
        {
            running = GRWL_FALSE;
        }
    }

    grwlHideWindow(window);
    thrd_join(thread, &result);
    grwlDestroyWindow(window);

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
