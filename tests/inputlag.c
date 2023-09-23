//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Input lag test
//
// This test renders a marker at the cursor position reported by GRWL to
// check how much it lags behind the hardware mouse cursor

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
#include <nuklear.h>

#define NK_GRWL_GL2_IMPLEMENTATION
#include <nuklear_grwl_gl2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

void usage(void)
{
    printf("Usage: inputlag [-h] [-f]\n");
    printf("Options:\n");
    printf("  -f create full screen window\n");
    printf("  -h show this help\n");
}

struct nk_vec2 cursor_new, cursor_pos, cursor_vel;

enum
{
    cursor_sync_query,
    cursor_input_message
} cursor_method = cursor_sync_query;

void sample_input(GRWLwindow* window)
{
    float a = .25; // exponential smoothing factor

    if (cursor_method == cursor_sync_query)
    {
        double x, y;
        grwlGetCursorPos(window, &x, &y);
        cursor_new.x = (float)x;
        cursor_new.y = (float)y;
    }

    cursor_vel.x = (cursor_new.x - cursor_pos.x) * a + cursor_vel.x * (1 - a);
    cursor_vel.y = (cursor_new.y - cursor_pos.y) * a + cursor_vel.y * (1 - a);
    cursor_pos = cursor_new;
}

void cursor_pos_callback(GRWLwindow* window, double xpos, double ypos)
{
    cursor_new.x = (float)xpos;
    cursor_new.y = (float)ypos;
}

int enable_vsync = nk_true;

void update_vsync()
{
    grwlSwapInterval(enable_vsync == nk_true ? 1 : 0);
}

int swap_clear = nk_false;
int swap_finish = nk_true;
int swap_occlusion_query = nk_false;
int swap_read_pixels = nk_false;
GLuint occlusion_query;

void swap_buffers(GRWLwindow* window)
{
    grwlSwapBuffers(window);

    if (swap_clear)
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    if (swap_finish)
    {
        glFinish();
    }

    if (swap_occlusion_query)
    {
        GLint occlusion_result;
        if (!occlusion_query)
        {
            glGenQueries(1, &occlusion_query);
        }
        glBeginQuery(GL_SAMPLES_PASSED, occlusion_query);
        glBegin(GL_POINTS);
        glVertex2f(0, 0);
        glEnd();
        glEndQuery(GL_SAMPLES_PASSED);
        glGetQueryObjectiv(occlusion_query, GL_QUERY_RESULT, &occlusion_result);
    }

    if (swap_read_pixels)
    {
        unsigned char rgba[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    }
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GRWL_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GRWL_KEY_ESCAPE:
            grwlSetWindowShouldClose(window, 1);
            break;
    }
}

void draw_marker(struct nk_command_buffer* canvas, int lead, struct nk_vec2 pos)
{
    struct nk_color colors[4] = { nk_rgb(255, 0, 0), nk_rgb(255, 255, 0), nk_rgb(0, 255, 0), nk_rgb(0, 96, 255) };
    struct nk_rect rect = { -5 + pos.x, -5 + pos.y, 10, 10 };
    nk_fill_circle(canvas, rect, colors[lead]);
}

int main(int argc, char** argv)
{
    int ch, width, height;
    unsigned long frame_count = 0;
    double last_time, current_time;
    double frame_rate = 0;
    int fullscreen = GRWL_FALSE;
    GRWLmonitor* monitor = NULL;
    GRWLwindow* window;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    int show_forecasts = nk_true;

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = GRWL_TRUE;
                break;
        }
    }

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    if (fullscreen)
    {
        const GRWLvidmode* mode;

        monitor = grwlGetPrimaryMonitor();
        mode = grwlGetVideoMode(monitor);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = 640;
        height = 480;
    }

    grwlWindowHint(GRWL_CONTEXT_VERSION_MAJOR, 2);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MINOR, 0);

    grwlWindowHint(GRWL_SCALE_TO_MONITOR, GRWL_TRUE);
    grwlWindowHint(GRWL_WIN32_KEYBOARD_MENU, GRWL_TRUE);

    window = grwlCreateWindow(width, height, "Input lag test", monitor, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    update_vsync();

    last_time = grwlGetTime();

    nk = nk_grwl_init(window, NK_GRWL_INSTALL_CALLBACKS);
    nk_grwl_font_stash_begin(&atlas);
    nk_grwl_font_stash_end();

    grwlSetKeyCallback(window, key_callback);
    grwlSetCursorPosCallback(window, cursor_pos_callback);

    while (!grwlWindowShouldClose(window))
    {
        int width, height;
        struct nk_rect area;

        grwlPollEvents();
        sample_input(window);

        grwlGetWindowSize(window, &width, &height);
        area = nk_rect(0.f, 0.f, (float)width, (float)height);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_grwl_new_frame();
        if (nk_begin(nk, "", area, 0))
        {
            nk_flags align_left = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;
            struct nk_command_buffer* canvas = nk_window_get_canvas(nk);
            int lead;

            for (lead = show_forecasts ? 3 : 0; lead >= 0; lead--)
            {
                draw_marker(canvas, lead,
                            nk_vec2(cursor_pos.x + cursor_vel.x * lead, cursor_pos.y + cursor_vel.y * lead));
            }

            // print instructions
            nk_layout_row_dynamic(nk, 20, 1);
            nk_label(nk, "Move mouse uniformly and check marker under cursor:", align_left);
            for (lead = 0; lead <= 3; lead++)
            {
                nk_layout_row_begin(nk, NK_STATIC, 12, 2);
                nk_layout_row_push(nk, 25);
                draw_marker(canvas, lead, nk_layout_space_to_screen(nk, nk_vec2(20, 5)));
                nk_label(nk, "", 0);
                nk_layout_row_push(nk, 500);
                if (lead == 0)
                {
                    nk_label(nk, "- current cursor position (no input lag)", align_left);
                }
                else
                {
                    nk_labelf(nk, align_left, "- %d-frame forecast (input lag is %d frame)", lead, lead);
                }
                nk_layout_row_end(nk);
            }

            nk_layout_row_dynamic(nk, 20, 1);

            nk_checkbox_label(nk, "Show forecasts", &show_forecasts);
            nk_label(nk, "Input method:", align_left);
            if (nk_option_label(nk, "grwlGetCursorPos (sync query)", cursor_method == cursor_sync_query))
            {
                cursor_method = cursor_sync_query;
            }
            if (nk_option_label(nk, "grwlSetCursorPosCallback (latest input message)",
                                cursor_method == cursor_input_message))
            {
                cursor_method = cursor_input_message;
            }

            nk_label(nk, "", 0); // separator

            nk_value_float(nk, "FPS", (float)frame_rate);
            if (nk_checkbox_label(nk, "Enable vsync", &enable_vsync))
            {
                update_vsync();
            }

            nk_label(nk, "", 0); // separator

            nk_label(nk, "After swap:", align_left);
            nk_checkbox_label(nk, "glClear", &swap_clear);
            nk_checkbox_label(nk, "glFinish", &swap_finish);
            nk_checkbox_label(nk, "draw with occlusion query", &swap_occlusion_query);
            nk_checkbox_label(nk, "glReadPixels", &swap_read_pixels);
        }

        nk_end(nk);
        nk_grwl_render(NK_ANTI_ALIASING_ON);

        swap_buffers(window);

        frame_count++;

        current_time = grwlGetTime();
        if (current_time - last_time > 1.0)
        {
            frame_rate = frame_count / (current_time - last_time);
            frame_count = 0;
            last_time = current_time;
        }
    }

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
