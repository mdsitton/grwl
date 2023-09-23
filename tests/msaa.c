//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Multisample anti-aliasing test
//
// This test renders two high contrast, slowly rotating quads, one aliased
// and one (hopefully) anti-aliased, thus allowing for visual verification
// of whether MSAA is indeed enabled

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>

#if defined(_MSC_VER)
    // Make MS math.h define M_PI
    #define _USE_MATH_DEFINES
#endif

#include "linmath.h"

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static const vec2 vertices[4] = { { -0.6f, -0.6f }, { 0.6f, -0.6f }, { 0.6f, 0.6f }, { -0.6f, 0.6f } };

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
        case GRWL_KEY_SPACE:
            grwlSetTime(0.0);
            break;
        case GRWL_KEY_ESCAPE:
            grwlSetWindowShouldClose(window, GRWL_TRUE);
            break;
    }
}

static void usage(void)
{
    printf("Usage: msaa [-h] [-s SAMPLES]\n");
}

int main(int argc, char** argv)
{
    int ch, samples = 4;
    GRWLwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location;

    while ((ch = getopt(argc, argv, "hs:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 's':
                samples = atoi(optarg);
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

    if (samples)
    {
        printf("Requesting MSAA with %i samples\n", samples);
    }
    else
    {
        printf("Requesting that MSAA not be available\n");
    }

    grwlWindowHint(GRWL_SAMPLES, samples);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MAJOR, 2);
    grwlWindowHint(GRWL_CONTEXT_VERSION_MINOR, 0);

    window = grwlCreateWindow(800, 400, "Aliasing Detector", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        exit(EXIT_FAILURE);
    }

    grwlSetKeyCallback(window, key_callback);

    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);
    grwlSwapInterval(1);

    glGetIntegerv(GL_SAMPLES, &samples);
    if (samples)
    {
        printf("Context reports MSAA is available with %i samples\n", samples);
    }
    else
    {
        printf("Context reports MSAA is unavailable\n");
    }

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);

    while (!grwlWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
        const double angle = grwlGetTime() * M_PI / 180.0;

        grwlGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 0.f, 1.f);

        mat4x4_translate(m, -1.f, 0.f, 0.f);
        mat4x4_rotate_Z(m, m, (float)angle);
        mat4x4_mul(mvp, p, m);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glDisable(GL_MULTISAMPLE);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        mat4x4_translate(m, 1.f, 0.f, 0.f);
        mat4x4_rotate_Z(m, m, (float)angle);
        mat4x4_mul(mvp, p, m);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glEnable(GL_MULTISAMPLE);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        grwlSwapBuffers(window);
        grwlPollEvents();
    }

    grwlDestroyWindow(window);

    grwlTerminate();
    exit(EXIT_SUCCESS);
}
