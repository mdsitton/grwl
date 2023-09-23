//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Iconify/restore test program

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static int windowed_xpos, windowed_ypos, windowed_width = 640, windowed_height = 480;

static void usage(void)
{
    printf("Usage: iconify [-h] [-f [-a] [-n]]\n");
    printf("Options:\n");
    printf("  -a create windows for all monitors\n");
    printf("  -f create full screen window(s)\n");
    printf("  -h show this help\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    printf("%0.2f Key %s\n", grwlGetTime(), action == GRWL_PRESS ? "pressed" : "released");

    if (action != GRWL_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GRWL_KEY_I:
            grwlIconifyWindow(window);
            break;
        case GRWL_KEY_M:
            grwlMaximizeWindow(window);
            break;
        case GRWL_KEY_R:
            grwlRestoreWindow(window);
            break;
        case GRWL_KEY_ESCAPE:
            grwlSetWindowShouldClose(window, GRWL_TRUE);
            break;
        case GRWL_KEY_A:
            grwlSetWindowAttrib(window, GRWL_AUTO_ICONIFY, !grwlGetWindowAttrib(window, GRWL_AUTO_ICONIFY));
            break;
        case GRWL_KEY_B:
            grwlSetWindowAttrib(window, GRWL_RESIZABLE, !grwlGetWindowAttrib(window, GRWL_RESIZABLE));
            break;
        case GRWL_KEY_D:
            grwlSetWindowAttrib(window, GRWL_DECORATED, !grwlGetWindowAttrib(window, GRWL_DECORATED));
            break;
        case GRWL_KEY_F:
            grwlSetWindowAttrib(window, GRWL_FLOATING, !grwlGetWindowAttrib(window, GRWL_FLOATING));
            break;
        case GRWL_KEY_F11:
        case GRWL_KEY_ENTER:
        {
            if (mods != GRWL_MOD_ALT)
            {
                return;
            }

            if (grwlGetWindowMonitor(window))
            {
                grwlSetWindowMonitor(window, NULL, windowed_xpos, windowed_ypos, windowed_width, windowed_height, 0);
            }
            else
            {
                GRWLmonitor* monitor = grwlGetPrimaryMonitor();
                if (monitor)
                {
                    const GRWLvidmode* mode = grwlGetVideoMode(monitor);
                    grwlGetWindowPos(window, &windowed_xpos, &windowed_ypos);
                    grwlGetWindowSize(window, &windowed_width, &windowed_height);
                    grwlSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                }
            }

            break;
        }
    }
}

static void window_size_callback(GRWLwindow* window, int width, int height)
{
    printf("%0.2f Window resized to %ix%i\n", grwlGetTime(), width, height);
}

static void framebuffer_size_callback(GRWLwindow* window, int width, int height)
{
    printf("%0.2f Framebuffer resized to %ix%i\n", grwlGetTime(), width, height);
}

static void window_focus_callback(GRWLwindow* window, int focused)
{
    printf("%0.2f Window %s\n", grwlGetTime(), focused ? "focused" : "defocused");
}

static void window_iconify_callback(GRWLwindow* window, int iconified)
{
    printf("%0.2f Window %s\n", grwlGetTime(), iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(GRWLwindow* window, int maximized)
{
    printf("%0.2f Window %s\n", grwlGetTime(), maximized ? "maximized" : "unmaximized");
}

static void window_refresh_callback(GRWLwindow* window)
{
    printf("%0.2f Window refresh\n", grwlGetTime());

    grwlMakeContextCurrent(window);

    glClear(GL_COLOR_BUFFER_BIT);
    grwlSwapBuffers(window);
}

static GRWLwindow* create_window(GRWLmonitor* monitor)
{
    int width, height;
    GRWLwindow* window;

    if (monitor)
    {
        const GRWLvidmode* mode = grwlGetVideoMode(monitor);

        grwlWindowHint(GRWL_REFRESH_RATE, mode->refreshRate);
        grwlWindowHint(GRWL_RED_BITS, mode->redBits);
        grwlWindowHint(GRWL_GREEN_BITS, mode->greenBits);
        grwlWindowHint(GRWL_BLUE_BITS, mode->blueBits);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = windowed_width;
        height = windowed_height;
    }

    window = grwlCreateWindow(width, height, "Iconify", monitor, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);

    return window;
}

int main(int argc, char** argv)
{
    int ch, i, window_count;
    int fullscreen = GRWL_FALSE, all_monitors = GRWL_FALSE;
    GRWLwindow** windows;

    while ((ch = getopt(argc, argv, "afhn")) != -1)
    {
        switch (ch)
        {
            case 'a':
                all_monitors = GRWL_TRUE;
                break;

            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = GRWL_TRUE;
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    if (fullscreen && all_monitors)
    {
        int monitor_count;
        GRWLmonitor** monitors = grwlGetMonitors(&monitor_count);

        window_count = monitor_count;
        windows = calloc(window_count, sizeof(GRWLwindow*));

        for (i = 0; i < monitor_count; i++)
        {
            windows[i] = create_window(monitors[i]);
            if (!windows[i])
            {
                break;
            }
        }
    }
    else
    {
        GRWLmonitor* monitor = NULL;

        if (fullscreen)
        {
            monitor = grwlGetPrimaryMonitor();
        }

        window_count = 1;
        windows = calloc(window_count, sizeof(GRWLwindow*));
        windows[0] = create_window(monitor);
    }

    for (i = 0; i < window_count; i++)
    {
        grwlSetKeyCallback(windows[i], key_callback);
        grwlSetFramebufferSizeCallback(windows[i], framebuffer_size_callback);
        grwlSetWindowSizeCallback(windows[i], window_size_callback);
        grwlSetWindowFocusCallback(windows[i], window_focus_callback);
        grwlSetWindowIconifyCallback(windows[i], window_iconify_callback);
        grwlSetWindowMaximizeCallback(windows[i], window_maximize_callback);
        grwlSetWindowRefreshCallback(windows[i], window_refresh_callback);

        window_refresh_callback(windows[i]);

        printf("Window is %s and %s\n", grwlGetWindowAttrib(windows[i], GRWL_ICONIFIED) ? "iconified" : "restored",
               grwlGetWindowAttrib(windows[i], GRWL_FOCUSED) ? "focused" : "defocused");
    }

    for (;;)
    {
        grwlWaitEvents();

        for (i = 0; i < window_count; i++)
        {
            if (grwlWindowShouldClose(windows[i]))
            {
                break;
            }
        }

        if (i < window_count)
        {
            break;
        }

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
