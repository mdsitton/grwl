//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Multi-threading test
//
// This test is intended to verify whether the OpenGL context part of
// the GRWL API is able to be used from multiple threads

#include "tinycthread.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    GRWLwindow* window;
    const char* title;
    float r, g, b;
    thrd_t id;
} Thread;

static volatile int running = GRWL_TRUE;

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

static int thread_main(void* data)
{
    const Thread* thread = data;

    grwlMakeContextCurrent(thread->window);
    grwlSwapInterval(1);

    while (running)
    {
        const float v = (float)fabs(sin(grwlGetTime() * 2.f));
        glClearColor(thread->r * v, thread->g * v, thread->b * v, 0.f);

        glClear(GL_COLOR_BUFFER_BIT);
        grwlSwapBuffers(thread->window);
    }

    grwlMakeContextCurrent(NULL);
    return 0;
}

int main(void)
{
    int i, result;
    Thread threads[] = { { NULL, "Red", 1.f, 0.f, 0.f, 0 },
                         { NULL, "Green", 0.f, 1.f, 0.f, 0 },
                         { NULL, "Blue", 0.f, 0.f, 1.f, 0 } };
    const int count = sizeof(threads) / sizeof(Thread);

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < count; i++)
    {
        grwlWindowHint(GRWL_POSITION_X, 200 + 250 * i);
        grwlWindowHint(GRWL_POSITION_Y, 200);

        threads[i].window = grwlCreateWindow(200, 200, threads[i].title, NULL, NULL);
        if (!threads[i].window)
        {
            grwlTerminate();
            exit(EXIT_FAILURE);
        }

        grwlSetKeyCallback(threads[i].window, key_callback);
    }

    grwlMakeContextCurrent(threads[0].window);
    gladLoadGL(grwlGetProcAddress);
    grwlMakeContextCurrent(NULL);

    for (i = 0; i < count; i++)
    {
        if (thrd_create(&threads[i].id, thread_main, threads + i) != thrd_success)
        {
            fprintf(stderr, "Failed to create secondary thread\n");

            grwlTerminate();
            exit(EXIT_FAILURE);
        }
    }

    while (running)
    {
        grwlWaitEvents();

        for (i = 0; i < count; i++)
        {
            if (grwlWindowShouldClose(threads[i].window))
            {
                running = GRWL_FALSE;
            }
        }
    }

    for (i = 0; i < count; i++)
    {
        grwlHideWindow(threads[i].window);
    }

    for (i = 0; i < count; i++)
    {
        thrd_join(threads[i].id, &result);
    }

    exit(EXIT_SUCCESS);
}
