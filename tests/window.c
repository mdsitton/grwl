//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Window properties test

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <stdarg.h>

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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char** argv)
{
    int windowed_x, windowed_y, windowed_width, windowed_height;
    int last_xpos = INT_MIN, last_ypos = INT_MIN;
    int last_width = INT_MIN, last_height = INT_MIN;
    int limit_aspect_ratio = false, aspect_numer = 1, aspect_denom = 1;
    int limit_min_size = false, min_width = 400, min_height = 400;
    int limit_max_size = false, max_width = 400, max_height = 400;
    char width_buffer[12] = "", height_buffer[12] = "";
    char xpos_buffer[12] = "", ypos_buffer[12] = "";
    char numer_buffer[12] = "", denom_buffer[12] = "";
    char min_width_buffer[12] = "", min_height_buffer[12] = "";
    char max_width_buffer[12] = "", max_height_buffer[12] = "";
    int may_close = true;

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    grwlWindowHint(GRWL_SCALE_TO_MONITOR, GRWL_TRUE);
    grwlWindowHint(GRWL_WIN32_KEYBOARD_MENU, GRWL_TRUE);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MAJOR, 2);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MINOR, 1);

    GRWLwindow* window = grwlCreateWindow(600, 800, "Window Features", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSwapInterval(0);

    bool position_supported = true;

    grwlGetError(NULL);
    grwlGetWindowPos(window, &last_xpos, &last_ypos);
    sprintf(xpos_buffer, "%i", last_xpos);
    sprintf(ypos_buffer, "%i", last_ypos);
    if (grwlGetError(NULL) == GRWL_FEATURE_UNAVAILABLE)
    {
        position_supported = false;
    }

    grwlGetWindowSize(window, &last_width, &last_height);
    sprintf(width_buffer, "%i", last_width);
    sprintf(height_buffer, "%i", last_height);

    sprintf(numer_buffer, "%i", aspect_numer);
    sprintf(denom_buffer, "%i", aspect_denom);

    sprintf(min_width_buffer, "%i", min_width);
    sprintf(min_height_buffer, "%i", min_height);
    sprintf(max_width_buffer, "%i", max_width);
    sprintf(max_height_buffer, "%i", max_height);

    struct nk_context* nk = nk_grwl_init(window, NK_GRWL_INSTALL_CALLBACKS);

    struct nk_font_atlas* atlas;
    nk_grwl_font_stash_begin(&atlas);
    nk_grwl_font_stash_end();

    while (!(may_close && grwlWindowShouldClose(window)))
    {
        int width, height;

        grwlGetWindowSize(window, &width, &height);

        struct nk_rect area = nk_rect(0.f, 0.f, (float)width, (float)height);
        nk_window_set_bounds(nk, "main", area);

        nk_grwl_new_frame();
        if (nk_begin(nk, "main", area, 0))
        {
            nk_layout_row_dynamic(nk, 30, 5);

            if (nk_button_label(nk, "Toggle Fullscreen"))
            {
                if (grwlGetWindowMonitor(window))
                {
                    grwlSetWindowMonitor(window, NULL, windowed_x, windowed_y, windowed_width, windowed_height, 0);
                }
                else
                {
                    GRWLmonitor* monitor = grwlGetPrimaryMonitor();
                    const GRWLvidmode* mode = grwlGetVideoMode(monitor);
                    grwlGetWindowPos(window, &windowed_x, &windowed_y);
                    grwlGetWindowSize(window, &windowed_width, &windowed_height);
                    grwlSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                }
            }

            if (nk_button_label(nk, "Maximize"))
            {
                grwlMaximizeWindow(window);
            }
            if (nk_button_label(nk, "Iconify"))
            {
                grwlIconifyWindow(window);
            }
            if (nk_button_label(nk, "Restore"))
            {
                grwlRestoreWindow(window);
            }
            if (nk_button_label(nk, "Hide (briefly)"))
            {
                grwlHideWindow(window);

                const double time = grwlGetTime() + 3.0;
                while (grwlGetTime() < time)
                {
                    grwlWaitEventsTimeout(1.0);
                }

                grwlShowWindow(window);
            }

            nk_layout_row_dynamic(nk, 30, 1);

            if (grwlGetWindowAttrib(window, GRWL_MOUSE_PASSTHROUGH))
            {
                nk_label(nk, "Press H to disable mouse passthrough", NK_TEXT_CENTERED);

                if (grwlGetKey(window, GRWL_KEY_H))
                {
                    grwlSetWindowAttrib(window, GRWL_MOUSE_PASSTHROUGH, false);
                }
            }

            nk_label(nk, "Press Enter in a text field to set value", NK_TEXT_CENTERED);

            nk_flags events;
            const nk_flags flags = NK_EDIT_FIELD | NK_EDIT_SIG_ENTER | NK_EDIT_GOTO_END_ON_ACTIVATE;

            if (position_supported)
            {
                int xpos, ypos;
                grwlGetWindowPos(window, &xpos, &ypos);

                nk_layout_row_dynamic(nk, 30, 3);
                nk_label(nk, "Position", NK_TEXT_LEFT);

                events = nk_edit_string_zero_terminated(nk, flags, xpos_buffer, sizeof(xpos_buffer), nk_filter_decimal);
                if (events & NK_EDIT_COMMITED)
                {
                    xpos = atoi(xpos_buffer);
                    grwlSetWindowPos(window, xpos, ypos);
                }
                else if (xpos != last_xpos || (events & NK_EDIT_DEACTIVATED))
                {
                    sprintf(xpos_buffer, "%i", xpos);
                }

                events = nk_edit_string_zero_terminated(nk, flags, ypos_buffer, sizeof(ypos_buffer), nk_filter_decimal);
                if (events & NK_EDIT_COMMITED)
                {
                    ypos = atoi(ypos_buffer);
                    grwlSetWindowPos(window, xpos, ypos);
                }
                else if (ypos != last_ypos || (events & NK_EDIT_DEACTIVATED))
                {
                    sprintf(ypos_buffer, "%i", ypos);
                }

                last_xpos = xpos;
                last_ypos = ypos;
            }
            else
            {
                nk_label(nk, "Position not supported", NK_TEXT_LEFT);
            }

            nk_layout_row_dynamic(nk, 30, 3);
            nk_label(nk, "Size", NK_TEXT_LEFT);

            events = nk_edit_string_zero_terminated(nk, flags, width_buffer, sizeof(width_buffer), nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                width = atoi(width_buffer);
                grwlSetWindowSize(window, width, height);
            }
            else if (width != last_width || (events & NK_EDIT_DEACTIVATED))
            {
                sprintf(width_buffer, "%i", width);
            }

            events = nk_edit_string_zero_terminated(nk, flags, height_buffer, sizeof(height_buffer), nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                height = atoi(height_buffer);
                grwlSetWindowSize(window, width, height);
            }
            else if (height != last_height || (events & NK_EDIT_DEACTIVATED))
            {
                sprintf(height_buffer, "%i", height);
            }

            last_width = width;
            last_height = height;

            bool update_ratio_limit = false;
            if (nk_checkbox_label(nk, "Aspect Ratio", &limit_aspect_ratio))
            {
                update_ratio_limit = true;
            }

            events = nk_edit_string_zero_terminated(nk, flags, numer_buffer, sizeof(numer_buffer), nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                aspect_numer = abs(atoi(numer_buffer));
                update_ratio_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
            {
                sprintf(numer_buffer, "%i", aspect_numer);
            }

            events = nk_edit_string_zero_terminated(nk, flags, denom_buffer, sizeof(denom_buffer), nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                aspect_denom = abs(atoi(denom_buffer));
                update_ratio_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
            {
                sprintf(denom_buffer, "%i", aspect_denom);
            }

            if (update_ratio_limit)
            {
                if (limit_aspect_ratio)
                {
                    grwlSetWindowAspectRatio(window, aspect_numer, aspect_denom);
                }
                else
                {
                    grwlSetWindowAspectRatio(window, GRWL_DONT_CARE, GRWL_DONT_CARE);
                }
            }

            bool update_size_limit = false;

            if (nk_checkbox_label(nk, "Minimum Size", &limit_min_size))
            {
                update_size_limit = true;
            }

            events = nk_edit_string_zero_terminated(nk, flags, min_width_buffer, sizeof(min_width_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                min_width = abs(atoi(min_width_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
            {
                sprintf(min_width_buffer, "%i", min_width);
            }

            events = nk_edit_string_zero_terminated(nk, flags, min_height_buffer, sizeof(min_height_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                min_height = abs(atoi(min_height_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
            {
                sprintf(min_height_buffer, "%i", min_height);
            }

            if (nk_checkbox_label(nk, "Maximum Size", &limit_max_size))
            {
                update_size_limit = true;
            }

            events = nk_edit_string_zero_terminated(nk, flags, max_width_buffer, sizeof(max_width_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                max_width = abs(atoi(max_width_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
            {
                sprintf(max_width_buffer, "%i", max_width);
            }

            events = nk_edit_string_zero_terminated(nk, flags, max_height_buffer, sizeof(max_height_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                max_height = abs(atoi(max_height_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
            {
                sprintf(max_height_buffer, "%i", max_height);
            }

            if (update_size_limit)
            {
                grwlSetWindowSizeLimits(
                    window, limit_min_size ? min_width : GRWL_DONT_CARE, limit_min_size ? min_height : GRWL_DONT_CARE,
                    limit_max_size ? max_width : GRWL_DONT_CARE, limit_max_size ? max_height : GRWL_DONT_CARE);
            }

            int fb_width, fb_height;
            grwlGetFramebufferSize(window, &fb_width, &fb_height);
            nk_label(nk, "Framebuffer Size", NK_TEXT_LEFT);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", fb_width);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", fb_height);

            float xscale, yscale;
            grwlGetWindowContentScale(window, &xscale, &yscale);
            nk_label(nk, "Content Scale", NK_TEXT_LEFT);
            nk_labelf(nk, NK_TEXT_LEFT, "%f", xscale);
            nk_labelf(nk, NK_TEXT_LEFT, "%f", yscale);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 5);
            int frame_left, frame_top, frame_right, frame_bottom;
            grwlGetWindowFrameSize(window, &frame_left, &frame_top, &frame_right, &frame_bottom);
            nk_layout_row_push(nk, 1.f / 3.f);
            nk_label(nk, "Frame Size:", NK_TEXT_LEFT);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_left);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_top);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_right);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_bottom);
            nk_layout_row_end(nk);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 2);
            float opacity = grwlGetWindowOpacity(window);
            nk_layout_row_push(nk, 1.f / 3.f);
            nk_labelf(nk, NK_TEXT_LEFT, "Opacity: %0.3f", opacity);
            nk_layout_row_push(nk, 2.f / 3.f);
            if (nk_slider_float(nk, 0.f, &opacity, 1.f, 0.001f))
            {
                grwlSetWindowOpacity(window, opacity);
            }
            nk_layout_row_end(nk);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 2);
            int should_close = grwlWindowShouldClose(window);
            nk_layout_row_push(nk, 1.f / 3.f);
            if (nk_checkbox_label(nk, "Should Close", &should_close))
            {
                grwlSetWindowShouldClose(window, should_close);
            }
            nk_layout_row_push(nk, 2.f / 3.f);
            nk_checkbox_label(nk, "May Close", &may_close);
            nk_layout_row_end(nk);

            nk_layout_row_dynamic(nk, 30, 1);
            nk_label(nk, "Attributes", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(nk, 30, width > 200 ? width / 200 : 1);

            int decorated = grwlGetWindowAttrib(window, GRWL_DECORATED);
            if (nk_checkbox_label(nk, "Decorated", &decorated))
            {
                grwlSetWindowAttrib(window, GRWL_DECORATED, decorated);
            }

            int resizable = grwlGetWindowAttrib(window, GRWL_RESIZABLE);
            if (nk_checkbox_label(nk, "Resizable", &resizable))
            {
                grwlSetWindowAttrib(window, GRWL_RESIZABLE, resizable);
            }

            int floating = grwlGetWindowAttrib(window, GRWL_FLOATING);
            if (nk_checkbox_label(nk, "Floating", &floating))
            {
                grwlSetWindowAttrib(window, GRWL_FLOATING, floating);
            }

            int passthrough = grwlGetWindowAttrib(window, GRWL_MOUSE_PASSTHROUGH);
            if (nk_checkbox_label(nk, "Mouse Passthrough", &passthrough))
            {
                grwlSetWindowAttrib(window, GRWL_MOUSE_PASSTHROUGH, passthrough);
            }

            int auto_iconify = grwlGetWindowAttrib(window, GRWL_AUTO_ICONIFY);
            if (nk_checkbox_label(nk, "Auto Iconify", &auto_iconify))
            {
                grwlSetWindowAttrib(window, GRWL_AUTO_ICONIFY, auto_iconify);
            }

            nk_value_bool(nk, "Focused", grwlGetWindowAttrib(window, GRWL_FOCUSED));
            nk_value_bool(nk, "Hovered", grwlGetWindowAttrib(window, GRWL_HOVERED));
            nk_value_bool(nk, "Visible", grwlGetWindowAttrib(window, GRWL_VISIBLE));
            nk_value_bool(nk, "Iconified", grwlGetWindowAttrib(window, GRWL_ICONIFIED));
            nk_value_bool(nk, "Maximized", grwlGetWindowAttrib(window, GRWL_MAXIMIZED));

            nk_layout_row_dynamic(nk, 30, 1);

            nk_label(nk, "Window Progress indicator", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(nk, 30, 5);

            static int state = GRWL_PROGRESS_INDICATOR_DISABLED;
            static float progress = 0;
            if (nk_button_label(nk, "No progress"))
            {
                grwlSetWindowProgressIndicator(window, state = GRWL_PROGRESS_INDICATOR_DISABLED, (double)progress);
            }
            if (nk_button_label(nk, "Indeterminate"))
            {
                grwlSetWindowProgressIndicator(window, state = GRWL_PROGRESS_INDICATOR_INDETERMINATE, (double)progress);
            }
            if (nk_button_label(nk, "Normal"))
            {
                grwlSetWindowProgressIndicator(window, state = GRWL_PROGRESS_INDICATOR_NORMAL, (double)progress);
            }
            if (nk_button_label(nk, "Error"))
            {
                grwlSetWindowProgressIndicator(window, state = GRWL_PROGRESS_INDICATOR_ERROR, (double)progress);
            }
            if (nk_button_label(nk, "Paused"))
            {
                grwlSetWindowProgressIndicator(window, state = GRWL_PROGRESS_INDICATOR_PAUSED, (double)progress);
            }

            nk_label(nk, "Progress: ", NK_TEXT_ALIGN_LEFT);
            if (nk_slider_float(nk, 0.0f, &progress, 1.0f, 0.05f))
            {
                grwlSetWindowProgressIndicator(window, state, (double)progress);
            }

            nk_layout_row_dynamic(nk, 30, 1);

            nk_label(nk, "Badge", NK_TEXT_CENTERED);

            static int badgeCount = 0;
            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 3);
            nk_layout_row_push(nk, 1.0f / 3.f);
            nk_labelf(nk, NK_TEXT_LEFT, "Badge count: %d", badgeCount);
            nk_layout_row_push(nk, 2.f / 3.f);
            if (nk_slider_int(nk, 0, &badgeCount, 10000, 1))
            {
                grwlSetWindowBadge(window, badgeCount);
                grwlSetWindowBadge(NULL, badgeCount);
            }
            nk_layout_row_end(nk);
        }
        nk_end(nk);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_grwl_render(NK_ANTI_ALIASING_ON);
        grwlSwapBuffers(window);

        grwlWaitEvents();
    }

    nk_grwl_shutdown();
    grwlTerminate();
    exit(EXIT_SUCCESS);
}
