//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// This program is used to test the gamma correction functionality for
// both full screen and windowed mode windows

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GRWL_GL2_IMPLEMENTATION
#include <nuklear_grwl_gl2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GRWL_PRESS && key == GRWL_KEY_ESCAPE)
    {
        grwlSetWindowShouldClose(window, GRWL_TRUE);
    }
}

static void chart_ramp_array(struct nk_context* nk, struct nk_color color, int count, unsigned short int* values)
{
    if (nk_chart_begin_colored(nk, NK_CHART_LINES, color, nk_rgb(255, 255, 255), count, 0, 65535))
    {
        int i;
        for (i = 0; i < count; i++)
        {
            char buffer[1024];
            if (nk_chart_push(nk, values[i]))
            {
                snprintf(buffer, sizeof(buffer), "#%u: %u (%0.5f) ", i, values[i], values[i] / 65535.f);
                nk_tooltip(nk, buffer);
            }
        }

        nk_chart_end(nk);
    }
}

int main(int argc, char** argv)
{
    GRWLmonitor* monitor = NULL;
    GRWLwindow* window;
    GRWLgammaramp orig_ramp;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;
    float gamma_value = 1.f;

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    monitor = grwlGetPrimaryMonitor();

    grwlWindowHint(GRWL_SCALE_TO_MONITOR, GRWL_TRUE);
    grwlWindowHint(GRWL_WIN32_KEYBOARD_MENU, GRWL_TRUE);

    window = grwlCreateWindow(800, 400, "Gamma Test", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    {
        const GRWLgammaramp* ramp = grwlGetGammaRamp(monitor);
        if (!ramp)
        {
            grwlTerminate();
            exit(EXIT_FAILURE);
        }

        const size_t array_size = ramp->size * sizeof(short);
        orig_ramp.size = ramp->size;
        orig_ramp.red = malloc(array_size);
        orig_ramp.green = malloc(array_size);
        orig_ramp.blue = malloc(array_size);
        memcpy(orig_ramp.red, ramp->red, array_size);
        memcpy(orig_ramp.green, ramp->green, array_size);
        memcpy(orig_ramp.blue, ramp->blue, array_size);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSwapInterval(1);

    nk = nk_grwl_init(window, NK_GRWL_INSTALL_CALLBACKS);
    nk_grwl_font_stash_begin(&atlas);
    nk_grwl_font_stash_end();

    grwlSetKeyCallback(window, key_callback);

    while (!grwlWindowShouldClose(window))
    {
        int width, height;
        struct nk_rect area;

        grwlGetWindowSize(window, &width, &height);
        area = nk_rect(0.f, 0.f, (float)width, (float)height);
        nk_window_set_bounds(nk, "", area);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_grwl_new_frame();
        if (nk_begin(nk, "", area, 0))
        {
            const GRWLgammaramp* ramp;

            nk_layout_row_dynamic(nk, 30, 3);
            if (nk_slider_float(nk, 0.1f, &gamma_value, 5.f, 0.1f))
            {
                grwlSetGamma(monitor, gamma_value);
            }
            nk_labelf(nk, NK_TEXT_LEFT, "%0.1f", gamma_value);
            if (nk_button_label(nk, "Revert"))
            {
                grwlSetGammaRamp(monitor, &orig_ramp);
            }

            ramp = grwlGetGammaRamp(monitor);

            nk_layout_row_dynamic(nk, height - 60.f, 3);
            chart_ramp_array(nk, nk_rgb(255, 0, 0), ramp->size, ramp->red);
            chart_ramp_array(nk, nk_rgb(0, 255, 0), ramp->size, ramp->green);
            chart_ramp_array(nk, nk_rgb(0, 0, 255), ramp->size, ramp->blue);
        }

        nk_end(nk);
        nk_grwl_render(NK_ANTI_ALIASING_ON);

        grwlSwapBuffers(window);
        grwlWaitEventsTimeout(1.0);
    }

    free(orig_ramp.red);
    free(orig_ramp.green);
    free(orig_ramp.blue);

    nk_grwl_shutdown();
    grwlTerminate();
    exit(EXIT_SUCCESS);
}
