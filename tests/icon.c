//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Window icon test program

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// a simple glfw logo
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

static void set_icon(GLFWwindow* window, int icon_color)
{
    int x, y;
    unsigned char pixels[16 * 16 * 4];
    unsigned char* target = pixels;
    GLFWimage img = { 16, 16, pixels };

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

    glfwSetWindowIcon(window, 1, &img);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
    {
        return;
    }

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            cur_icon_color = (cur_icon_color + 1) % 5;
            set_icon(window, cur_icon_color);
            break;
        case GLFW_KEY_X:
            glfwSetWindowIcon(window, 0, NULL);
            break;
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(200, 200, "Window Icon", NULL, NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glfwSetKeyCallback(window, key_callback);
    set_icon(window, cur_icon_color);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
