//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Multi-threading test
//
// This test is intended to verify whether the OpenGL context part of
// the GLFW API is able to be used from multiple threads

#include "tinycthread.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    GLFWwindow* window;
    const char* title;
    float r, g, b;
    thrd_t id;
} Thread;

static volatile int running = GLFW_TRUE;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static int thread_main(void* data)
{
    const Thread* thread = data;

    glfwMakeContextCurrent(thread->window);
    glfwSwapInterval(1);

    while (running)
    {
        const float v = (float)fabs(sin(glfwGetTime() * 2.f));
        glClearColor(thread->r * v, thread->g * v, thread->b * v, 0.f);

        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(thread->window);
    }

    glfwMakeContextCurrent(NULL);
    return 0;
}

int main(void)
{
    int i, result;
    Thread threads[] = { { NULL, "Red", 1.f, 0.f, 0.f, 0 },
                         { NULL, "Green", 0.f, 1.f, 0.f, 0 },
                         { NULL, "Blue", 0.f, 0.f, 1.f, 0 } };
    const int count = sizeof(threads) / sizeof(Thread);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < count; i++)
    {
        glfwWindowHint(GLFW_POSITION_X, 200 + 250 * i);
        glfwWindowHint(GLFW_POSITION_Y, 200);

        threads[i].window = glfwCreateWindow(200, 200, threads[i].title, NULL, NULL);
        if (!threads[i].window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetKeyCallback(threads[i].window, key_callback);
    }

    glfwMakeContextCurrent(threads[0].window);
    gladLoadGL(glfwGetProcAddress);
    glfwMakeContextCurrent(NULL);

    for (i = 0; i < count; i++)
    {
        if (thrd_create(&threads[i].id, thread_main, threads + i) != thrd_success)
        {
            fprintf(stderr, "Failed to create secondary thread\n");

            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    while (running)
    {
        glfwWaitEvents();

        for (i = 0; i < count; i++)
        {
            if (glfwWindowShouldClose(threads[i].window))
            {
                running = GLFW_FALSE;
            }
        }
    }

    for (i = 0; i < count; i++)
    {
        glfwHideWindow(threads[i].window);
    }

    for (i = 0; i < count; i++)
    {
        thrd_join(threads[i].id, &result);
    }

    exit(EXIT_SUCCESS);
}
