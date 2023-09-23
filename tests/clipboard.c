//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// This program is used to test the clipboard functionality.

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#if defined(__APPLE__)
    #define MODIFIER GRWL_MOD_SUPER
#else
    #define MODIFIER GRWL_MOD_CONTROL
#endif

static void usage(void)
{
    printf("Usage: clipboard [-h]\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GRWL_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GRWL_KEY_ESCAPE:
            grwlSetWindowShouldClose(window, GRWL_TRUE);
            break;

        case GRWL_KEY_V:
            if (mods == MODIFIER)
            {
                const char* string;

                string = grwlGetClipboardString(NULL);
                if (string)
                {
                    printf("Clipboard contains \"%s\"\n", string);
                }
                else
                {
                    printf("Clipboard does not contain a string\n");
                }
            }
            break;

        case GRWL_KEY_C:
            if (mods == MODIFIER)
            {
                const char* string = "Hello GRWL World!";
                grwlSetClipboardString(NULL, string);
                printf("Setting clipboard to \"%s\"\n", string);
            }
            break;
    }
}

int main(int argc, char** argv)
{
    int ch;
    GRWLwindow* window;

    while ((ch = getopt(argc, argv, "h")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        fprintf(stderr, "Failed to initialize GRWL\n");
        exit(EXIT_FAILURE);
    }

    window = grwlCreateWindow(200, 200, "Clipboard Test", NULL, NULL);
    if (!window)
    {
        grwlTerminate();

        fprintf(stderr, "Failed to open GRWL window\n");
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSwapInterval(1);

    grwlSetKeyCallback(window, key_callback);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    while (!grwlWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        grwlSwapBuffers(window);
        grwlWaitEvents();
    }

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
