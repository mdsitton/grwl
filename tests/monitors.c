//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Monitor information tool
//
// This test prints monitor and video mode information or verifies video
// modes

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "getopt.h"

enum Mode
{
    LIST_MODE,
    TEST_MODE
};

static void usage(void)
{
    printf("Usage: monitors [-t]\n");
    printf("       monitors -h\n");
}

static int euclid(int a, int b)
{
    return b ? euclid(b, a % b) : a;
}

static const char* format_mode(const GRWLvidmode* mode)
{
    static char buffer[512];
    const int gcd = euclid(mode->width, mode->height);

    snprintf(buffer, sizeof(buffer), "%i x %i x %i (%i:%i) (%i %i %i) %i Hz", mode->width, mode->height,
             mode->redBits + mode->greenBits + mode->blueBits, mode->width / gcd, mode->height / gcd, mode->redBits,
             mode->greenBits, mode->blueBits, mode->refreshRate);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GRWLwindow* window, int width, int height)
{
    printf("Framebuffer resized to %ix%i\n", width, height);

    glViewport(0, 0, width, height);
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GRWL_KEY_ESCAPE)
    {
        grwlSetWindowShouldClose(window, GRWL_TRUE);
    }
}

static void list_modes(GRWLmonitor* monitor)
{
    int count, x, y, width_mm, height_mm, i;
    int workarea_x, workarea_y, workarea_width, workarea_height;
    float xscale, yscale;

    const GRWLvidmode* mode = grwlGetVideoMode(monitor);
    const GRWLvidmode* modes = grwlGetVideoModes(monitor, &count);

    grwlGetMonitorPos(monitor, &x, &y);
    grwlGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
    grwlGetMonitorContentScale(monitor, &xscale, &yscale);
    grwlGetMonitorWorkarea(monitor, &workarea_x, &workarea_y, &workarea_width, &workarea_height);

    printf("Name: %s (%s)\n", grwlGetMonitorName(monitor),
           grwlGetPrimaryMonitor() == monitor ? "primary" : "secondary");
    printf("Current mode: %s\n", format_mode(mode));
    printf("Virtual position: %i, %i\n", x, y);
    printf("Content scale: %f x %f\n", xscale, yscale);

    printf("Physical size: %i x %i mm (%0.2f dpi at %i x %i)\n", width_mm, height_mm, mode->width * 25.4f / width_mm,
           mode->width, mode->height);
    printf("Monitor work area: %i x %i starting at %i, %i\n", workarea_width, workarea_height, workarea_x, workarea_y);

    printf("Modes:\n");

    for (i = 0; i < count; i++)
    {
        printf("%3u: %s", (unsigned int)i, format_mode(modes + i));

        if (memcmp(mode, modes + i, sizeof(GRWLvidmode)) == 0)
        {
            printf(" (current mode)");
        }

        putchar('\n');
    }
}

static void test_modes(GRWLmonitor* monitor)
{
    int i, count;
    GRWLwindow* window;
    const GRWLvidmode* modes = grwlGetVideoModes(monitor, &count);

    for (i = 0; i < count; i++)
    {
        const GRWLvidmode* mode = modes + i;
        GRWLvidmode current;

        grwlWindowHint(GRWL_RED_BITS, mode->redBits);
        grwlWindowHint(GRWL_GREEN_BITS, mode->greenBits);
        grwlWindowHint(GRWL_BLUE_BITS, mode->blueBits);
        grwlWindowHint(GRWL_REFRESH_RATE, mode->refreshRate);

        printf("Testing mode %u on monitor %s: %s\n", (unsigned int)i, grwlGetMonitorName(monitor), format_mode(mode));

        window = grwlCreateWindow(mode->width, mode->height, "Video Mode Test", grwlGetPrimaryMonitor(), NULL);
        if (!window)
        {
            printf("Failed to enter mode %u: %s\n", (unsigned int)i, format_mode(mode));
            continue;
        }

        grwlSetFramebufferSizeCallback(window, framebuffer_size_callback);
        grwlSetKeyCallback(window, key_callback);

        grwlMakeContextCurrent(window);
        gladLoadGL(grwlGetProcAddress);
        grwlSwapInterval(1);

        grwlSetTime(0.0);

        while (grwlGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            grwlSwapBuffers(window);
            grwlPollEvents();

            if (grwlWindowShouldClose(window))
            {
                printf("User terminated program\n");

                grwlTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        glGetIntegerv(GL_RED_BITS, &current.redBits);
        glGetIntegerv(GL_GREEN_BITS, &current.greenBits);
        glGetIntegerv(GL_BLUE_BITS, &current.blueBits);

        grwlGetWindowSize(window, &current.width, &current.height);

        if (current.redBits != mode->redBits || current.greenBits != mode->greenBits ||
            current.blueBits != mode->blueBits)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n", current.redBits, current.greenBits,
                   current.blueBits, mode->redBits, mode->greenBits, mode->blueBits);
        }

        if (current.width != mode->width || current.height != mode->height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n", current.width, current.height, mode->width,
                   mode->height);
        }

        printf("Closing window\n");

        grwlDestroyWindow(window);
        window = NULL;

        grwlPollEvents();
    }
}

int main(int argc, char** argv)
{
    int ch, i, count, mode = LIST_MODE;
    GRWLmonitor** monitors;

    while ((ch = getopt(argc, argv, "th")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 't':
                mode = TEST_MODE;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    grwlSetErrorCallback(error_callback);

    grwlInitHint(GRWL_COCOA_MENUBAR, GRWL_FALSE);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    monitors = grwlGetMonitors(&count);

    for (i = 0; i < count; i++)
    {
        if (mode == LIST_MODE)
        {
            list_modes(monitors[i]);
        }
        else if (mode == TEST_MODE)
        {
            test_modes(monitors[i]);
        }
    }

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
