//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// This test provides an interface to the cursor image and cursor mode
// parts of the API.
//
// Custom cursor image generation by urraka.

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#if defined(_MSC_VER)
    // Make MS math.h define M_PI
    #define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

#define CURSOR_FRAME_COUNT 60

static const char* vertex_shader_text = "#version 110\n"
                                        "uniform mat4 MVP;\n"
                                        "attribute vec2 vPos;\n"
                                        "void main()\n"
                                        "{\n"
                                        "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
                                        "}\n";

static const char* fragment_shader_text = "#version 110\n"
                                          "void main()\n"
                                          "{\n"
                                          "    gl_FragColor = vec4(1.0);\n"
                                          "}\n";

static double cursor_x;
static double cursor_y;
static int swap_interval = 1;
static int wait_events = GRWL_TRUE;
static int animate_cursor = GRWL_FALSE;
static int track_cursor = GRWL_FALSE;
static GRWLcursor* standard_cursors[10];
static GRWLcursor* tracking_cursor = NULL;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static float star(int x, int y, float t)
{
    const float c = 64 / 2.f;

    const float i = (0.25f * (float)sin(2.f * M_PI * t) + 0.75f);
    const float k = 64 * 0.046875f * i;

    const float dist = (float)sqrt((x - c) * (x - c) + (y - c) * (y - c));

    const float salpha = 1.f - dist / c;
    const float xalpha = (float)x == c ? c : k / (float)fabs(x - c);
    const float yalpha = (float)y == c ? c : k / (float)fabs(y - c);

    return (float)fmax(0.f, fmin(1.f, i * salpha * 0.2f + salpha * xalpha * yalpha));
}

static GRWLcursor* create_cursor_frame(float t)
{
    int i = 0, x, y;
    unsigned char buffer[64 * 64 * 4];
    const GRWLimage image = { 64, 64, buffer };

    for (y = 0; y < image.width; y++)
    {
        for (x = 0; x < image.height; x++)
        {
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = (unsigned char)(255 * star(x, y, t));
        }
    }

    return grwlCreateCursor(&image, image.width / 2, image.height / 2);
}

static GRWLcursor* create_tracking_cursor(void)
{
    int i = 0, x, y;
    unsigned char buffer[32 * 32 * 4];
    const GRWLimage image = { 32, 32, buffer };

    for (y = 0; y < image.width; y++)
    {
        for (x = 0; x < image.height; x++)
        {
            if (x == 7 || y == 7)
            {
                buffer[i++] = 255;
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 255;
            }
            else
            {
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 0;
            }
        }
    }

    return grwlCreateCursor(&image, 7, 7);
}

static void cursor_position_callback(GRWLwindow* window, double x, double y)
{
    printf("%0.3f: Cursor position: %f %f (%+f %+f)\n", grwlGetTime(), x, y, x - cursor_x, y - cursor_y);

    cursor_x = x;
    cursor_y = y;
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GRWL_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GRWL_KEY_A:
        {
            animate_cursor = !animate_cursor;
            if (!animate_cursor)
            {
                grwlSetCursor(window, NULL);
            }

            break;
        }

        case GRWL_KEY_ESCAPE:
        {
            const int mode = grwlGetInputMode(window, GRWL_CURSOR);
            if (mode != GRWL_CURSOR_DISABLED && mode != GRWL_CURSOR_CAPTURED)
            {
                grwlSetWindowShouldClose(window, GRWL_TRUE);
                break;
            }

            /* FALLTHROUGH */
        }

        case GRWL_KEY_N:
            grwlSetInputMode(window, GRWL_CURSOR, GRWL_CURSOR_NORMAL);
            grwlGetCursorPos(window, &cursor_x, &cursor_y);
            printf("(( cursor is normal ))\n");
            break;

        case GRWL_KEY_D:
            grwlSetInputMode(window, GRWL_CURSOR, GRWL_CURSOR_DISABLED);
            printf("(( cursor is disabled ))\n");
            break;

        case GRWL_KEY_H:
            grwlSetInputMode(window, GRWL_CURSOR, GRWL_CURSOR_HIDDEN);
            printf("(( cursor is hidden ))\n");
            break;

        case GRWL_KEY_C:
            grwlSetInputMode(window, GRWL_CURSOR, GRWL_CURSOR_CAPTURED);
            printf("(( cursor is captured ))\n");
            break;

        case GRWL_KEY_R:
            if (!grwlRawMouseMotionSupported())
            {
                break;
            }

            if (grwlGetInputMode(window, GRWL_RAW_MOUSE_MOTION))
            {
                grwlSetInputMode(window, GRWL_RAW_MOUSE_MOTION, GRWL_FALSE);
                printf("(( raw input is disabled ))\n");
            }
            else
            {
                grwlSetInputMode(window, GRWL_RAW_MOUSE_MOTION, GRWL_TRUE);
                printf("(( raw input is enabled ))\n");
            }
            break;

        case GRWL_KEY_SPACE:
            swap_interval = 1 - swap_interval;
            printf("(( swap interval: %i ))\n", swap_interval);
            grwlSwapInterval(swap_interval);
            break;

        case GRWL_KEY_W:
            wait_events = !wait_events;
            printf("(( %sing for events ))\n", wait_events ? "wait" : "poll");
            break;

        case GRWL_KEY_T:
            track_cursor = !track_cursor;
            if (track_cursor)
            {
                grwlSetCursor(window, tracking_cursor);
            }
            else
            {
                grwlSetCursor(window, NULL);
            }

            break;

        case GRWL_KEY_P:
        {
            double x, y;
            grwlGetCursorPos(window, &x, &y);

            printf("Query before set: %f %f (%+f %+f)\n", x, y, x - cursor_x, y - cursor_y);
            cursor_x = x;
            cursor_y = y;

            grwlSetCursorPos(window, cursor_x, cursor_y);
            grwlGetCursorPos(window, &x, &y);

            printf("Query after set: %f %f (%+f %+f)\n", x, y, x - cursor_x, y - cursor_y);
            cursor_x = x;
            cursor_y = y;
            break;
        }

        case GRWL_KEY_UP:
            grwlSetCursorPos(window, 0, 0);
            grwlGetCursorPos(window, &cursor_x, &cursor_y);
            break;

        case GRWL_KEY_DOWN:
        {
            int width, height;
            grwlGetWindowSize(window, &width, &height);
            grwlSetCursorPos(window, width - 1, height - 1);
            grwlGetCursorPos(window, &cursor_x, &cursor_y);
            break;
        }

        case GRWL_KEY_0:
            grwlSetCursor(window, NULL);
            break;

        case GRWL_KEY_1:
        case GRWL_KEY_2:
        case GRWL_KEY_3:
        case GRWL_KEY_4:
        case GRWL_KEY_5:
        case GRWL_KEY_6:
        case GRWL_KEY_7:
        case GRWL_KEY_8:
        case GRWL_KEY_9:
        {
            int index = key - GRWL_KEY_1;
            if (mods & GRWL_MOD_SHIFT)
            {
                index += 9;
            }

            if (index < sizeof(standard_cursors) / sizeof(standard_cursors[0]))
            {
                grwlSetCursor(window, standard_cursors[index]);
            }

            break;
        }

        case GRWL_KEY_F11:
        case GRWL_KEY_ENTER:
        {
            static int x, y, width, height;

            if (mods != GRWL_MOD_ALT)
            {
                return;
            }

            if (grwlGetWindowMonitor(window))
            {
                grwlSetWindowMonitor(window, NULL, x, y, width, height, 0);
            }
            else
            {
                GRWLmonitor* monitor = grwlGetPrimaryMonitor();
                const GRWLvidmode* mode = grwlGetVideoMode(monitor);
                grwlGetWindowPos(window, &x, &y);
                grwlGetWindowSize(window, &width, &height);
                grwlSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }

            grwlGetCursorPos(window, &cursor_x, &cursor_y);
            break;
        }
    }
}

int main(void)
{
    int i;
    GRWLwindow* window;
    GRWLcursor* star_cursors[CURSOR_FRAME_COUNT];
    GRWLcursor* current_frame = NULL;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location;

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    tracking_cursor = create_tracking_cursor();
    if (!tracking_cursor)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < CURSOR_FRAME_COUNT; i++)
    {
        star_cursors[i] = create_cursor_frame(i / (float)CURSOR_FRAME_COUNT);
        if (!star_cursors[i])
        {
            grwlTerminate();
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < sizeof(standard_cursors) / sizeof(standard_cursors[0]); i++)
    {
        const int shapes[] = { GRWL_ARROW_CURSOR,         GRWL_IBEAM_CURSOR,       GRWL_CROSSHAIR_CURSOR,
                               GRWL_POINTING_HAND_CURSOR, GRWL_RESIZE_EW_CURSOR,   GRWL_RESIZE_NS_CURSOR,
                               GRWL_RESIZE_NWSE_CURSOR,   GRWL_RESIZE_NESW_CURSOR, GRWL_RESIZE_ALL_CURSOR,
                               GRWL_NOT_ALLOWED_CURSOR };

        standard_cursors[i] = grwlCreateStandardCursor(shapes[i]);
    }

    grwlWindowHint(GRWL_CONTEXT_VERSION_MAJOR, 2);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MINOR, 0);

    window = grwlCreateWindow(640, 480, "Cursor Test", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    glUseProgram(program);

    grwlGetCursorPos(window, &cursor_x, &cursor_y);
    printf("Cursor position: %f %f\n", cursor_x, cursor_y);

    grwlSetCursorPosCallback(window, cursor_position_callback);
    grwlSetKeyCallback(window, key_callback);

    while (!grwlWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if (track_cursor)
        {
            int wnd_width, wnd_height, fb_width, fb_height;
            float scale;
            vec2 vertices[4];
            mat4x4 mvp;

            grwlGetWindowSize(window, &wnd_width, &wnd_height);
            grwlGetFramebufferSize(window, &fb_width, &fb_height);

            glViewport(0, 0, fb_width, fb_height);

            scale = (float)fb_width / (float)wnd_width;
            vertices[0][0] = 0.5f;
            vertices[0][1] = (float)(fb_height - floor(cursor_y * scale) - 1.f + 0.5f);
            vertices[1][0] = (float)fb_width + 0.5f;
            vertices[1][1] = (float)(fb_height - floor(cursor_y * scale) - 1.f + 0.5f);
            vertices[2][0] = (float)floor(cursor_x * scale) + 0.5f;
            vertices[2][1] = 0.5f;
            vertices[3][0] = (float)floor(cursor_x * scale) + 0.5f;
            vertices[3][1] = (float)fb_height + 0.5f;

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

            mat4x4_ortho(mvp, 0.f, (float)fb_width, 0.f, (float)fb_height, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

            glDrawArrays(GL_LINES, 0, 4);
        }

        grwlSwapBuffers(window);

        if (animate_cursor)
        {
            const int i = (int)(grwlGetTime() * 30.0) % CURSOR_FRAME_COUNT;
            if (current_frame != star_cursors[i])
            {
                grwlSetCursor(window, star_cursors[i]);
                current_frame = star_cursors[i];
            }
        }
        else
        {
            current_frame = NULL;
        }

        if (wait_events)
        {
            if (animate_cursor)
            {
                grwlWaitEventsTimeout(1.0 / 30.0);
            }
            else
            {
                grwlWaitEvents();
            }
        }
        else
        {
            grwlPollEvents();
        }

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    grwlDestroyWindow(window);

    for (i = 0; i < CURSOR_FRAME_COUNT; i++)
    {
        grwlDestroyCursor(star_cursors[i]);
    }

    for (i = 0; i < sizeof(standard_cursors) / sizeof(standard_cursors[0]); i++)
    {
        grwlDestroyCursor(standard_cursors[i]);
    }

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
