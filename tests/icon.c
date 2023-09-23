//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Window icon test program

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// a simple grwl logo
const char* const logo[] = { "................", "................", "...0000..0......", "...0.....0......",
                             "...0.00..0......", "...0..0..0......", "...0000..0000...", "................",
                             "................", "...000..0...0...", "...0....0...0...", "...000..0.0.0...",
                             "...0....0.0.0...", "...0....00000...", "................", "................" };

const unsigned char icon_colors[5][4] = {
    { 0, 0, 0, 255 },      // black
    { 255, 0, 0, 255 },    // red
    { 0, 255, 0, 255 },    // green
    { 0, 0, 255, 255 },    // blue
    { 255, 255, 255, 255 } // white
};

static int cur_icon_color = 0;

static void set_icon(GRWLwindow* window, int icon_color)
{
    int x, y;
    unsigned char pixels[16 * 16 * 4];
    unsigned char* target = pixels;
    GRWLimage img = { 16, 16, pixels };

    for (y = 0; y < img.width; y++)
    {
        for (x = 0; x < img.height; x++)
        {
            if (logo[y][x] == '0')
            {
                memcpy(target, icon_colors[icon_color], 4);
            }
            else
            {
                memset(target, 0, 4);
            }

            target += 4;
        }
    }

    grwlSetWindowIcon(window, 1, &img);
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
        case GRWL_KEY_SPACE:
            cur_icon_color = (cur_icon_color + 1) % 5;
            set_icon(window, cur_icon_color);
            break;
        case GRWL_KEY_X:
            grwlSetWindowIcon(window, 0, NULL);
            break;
    }
}

int main(int argc, char** argv)
{
    GRWLwindow* window;

    if (!grwlInit())
    {
        fprintf(stderr, "Failed to initialize GRWL\n");
        exit(EXIT_FAILURE);
    }

    window = grwlCreateWindow(200, 200, "Window Icon", NULL, NULL);
    if (!window)
    {
        grwlTerminate();

        fprintf(stderr, "Failed to open GRWL window\n");
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);

    grwlSetKeyCallback(window, key_callback);
    set_icon(window, cur_icon_color);

    while (!grwlWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        grwlSwapBuffers(window);
        grwlWaitEvents();
    }

    grwlDestroyWindow(window);
    grwlTerminate();
    exit(EXIT_SUCCESS);
}
