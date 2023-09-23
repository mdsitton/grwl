//===========================================================================
// This file is part of GRWL(a fork of GRWL) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Test user context creation

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GRWL_INCLUDE_NONE
#include <GRWL/grwl.h>
#include <stdio.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(void)
{
    GRWLwindow* window;
    GRWLusercontext* usercontext;

    grwlSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!grwlInit())
    {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = grwlCreateWindow(640, 480, "User Context", NULL, NULL);
    if (!window)
    {
        grwlTerminate();
        return -1;
    }

    /* Make the window's context current */
    grwlMakeContextCurrent(window);
    gladLoadGL(grwlGetProcAddress);

    /* make a new context */
    usercontext = grwlCreateUserContext(window);
    if (!usercontext)
    {
        fprintf(stderr, "Failed to create user context\n");
        grwlTerminate();
        return -1;
    }

    /* set the user context current */
    grwlMakeUserContextCurrent(usercontext);

    if (grwlGetCurrentContext() != NULL)
    {
        fprintf(stderr, "Current grwl window context not NULL after grwlMakeUserContextCurrent\n");
        grwlTerminate();
        return -1;
    }
    if (grwlGetCurrentUserContext() != usercontext)
    {
        fprintf(stderr, "Current user context not correct after grwlMakeUserContextCurrent\n");
        grwlTerminate();
        return -1;
    }

    /* set the window context current */
    grwlMakeContextCurrent(window);

    if (grwlGetCurrentUserContext() != NULL)
    {
        fprintf(stderr, "Current user context not NULL after grwlMakeContextCurrent\n");
        grwlTerminate();
        return -1;
    }
    if (grwlGetCurrentContext() != window)
    {
        fprintf(stderr, "Current grwl window context not correct after grwlMakeContextCurrent\n");
        grwlTerminate();
        return -1;
    }

    glClearColor(0.4f, 0.3f, 0.4f, 1.0f);

    /* Loop until the user closes the window */
    while (!grwlWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        grwlSwapBuffers(window);

        /* Poll for and process events */
        grwlPollEvents();
    }

    grwlDestroyUserContext(usercontext);
    grwlTerminate();
    return 0;
}
