//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Window re-opener (open/close stress test)
//
//
// This test came about as the result of bug #1262773
//
// It closes and re-opens the GRWL window every five seconds, alternating
// between windowed and full screen mode
//
// It also times and logs opening and closing actions and attempts to separate
// user initiated window closing from its own

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

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

static const vec2 vertices[4] = { { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } };

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_close_callback(GRWLwindow* window)
{
    printf("Close callback triggered\n");
}

static void key_callback(GRWLwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GRWL_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GRWL_KEY_Q:
        case GRWL_KEY_ESCAPE:
            grwlSetWindowShouldClose(window, GRWL_TRUE);
            break;
    }
}

static void close_window(GRWLwindow* window)
{
    double base = grwlGetTime();
    grwlDestroyWindow(window);
    printf("Closing window took %0.3f seconds\n", grwlGetTime() - base);
}

int main(int argc, char** argv)
{
    int count = 0;
    double base;
    GRWLwindow* window;

    srand((unsigned int)time(NULL));

    grwlSetErrorCallback(error_callback);

    if (!grwlInit())
    {
        exit(EXIT_FAILURE);
    }

    grwlWindowHint(GRWL_CONTEXT_VERSION_MAJOR, 2);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MINOR, 0);

    for (;;)
    {
        int width, height;
        GRWLmonitor* monitor = NULL;
        GLuint vertex_shader, fragment_shader, program, vertex_buffer;
        GLint mvp_location, vpos_location;

        if (count & 1)
        {
            int monitorCount;
            GRWLmonitor** monitors = grwlGetMonitors(&monitorCount);
            monitor = monitors[rand() % monitorCount];
        }

        if (monitor)
        {
            const GRWLvidmode* mode = grwlGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        else
        {
            width = 640;
            height = 480;
        }

        base = grwlGetTime();

        window = grwlCreateWindow(width, height, "Window Re-opener", monitor, NULL);
        if (!window)
        {
            grwlTerminate();
            exit(EXIT_FAILURE);
        }

        if (monitor)
        {
            printf("Opening full screen window on monitor %s took %0.3f seconds\n", grwlGetMonitorName(monitor),
                   grwlGetTime() - base);
        }
        else
        {
            printf("Opening regular window took %0.3f seconds\n", grwlGetTime() - base);
        }

        grwlSetWindowCloseCallback(window, window_close_callback);
        grwlSetKeyCallback(window, key_callback);

        grwlMakeContextCurrent(window);
        gladLoadGL(grwlGetProcAddress);
        grwlSwapInterval(1);

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

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);

        grwlSetTime(0.0);

        while (grwlGetTime() < 5.0)
        {
            float ratio;
            int width, height;
            mat4x4 m, p, mvp;

            grwlGetFramebufferSize(window, &width, &height);
            ratio = width / (float)height;

            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);

            mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 0.f, 1.f);

            mat4x4_identity(m);
            mat4x4_rotate_Z(m, m, (float)grwlGetTime());
            mat4x4_mul(mvp, p, m);

            glUseProgram(program);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            grwlSwapBuffers(window);
            grwlPollEvents();

            if (grwlWindowShouldClose(window))
            {
                close_window(window);
                printf("User closed window\n");

                grwlTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        printf("Closing window\n");
        close_window(window);

        count++;
    }

    grwlTerminate();
}
