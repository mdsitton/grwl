//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// This test is intended to verify that posting of empty events works

#include "tinycthread.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static volatile int running = GLFW_TRUE;

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

        glfwPostEmptyEvent();
    }

    return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
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
    GLFWwindow* window;

    srand((unsigned int)time(NULL));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(640, 480, "Empty Event Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSetKeyCallback(window, key_callback);

    if (thrd_create(&thread, thread_main, NULL) != thrd_success)
    {
        fprintf(stderr, "Failed to create secondary thread\n");

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (running)
    {
        int width, height;
        float r = nrand(), g = nrand(), b = nrand();
        float l = (float)sqrt(r * r + g * g + b * b);

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(r / l, g / l, b / l, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        glfwWaitEvents();

        if (glfwWindowShouldClose(window))
        {
            running = GLFW_FALSE;
        }
    }

    glfwHideWindow(window);
    thrd_join(thread, &result);
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
