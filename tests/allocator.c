//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================
// Custom heap allocator test

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CALL(x) (function_name = #x, x)
static const char* function_name = NULL;

struct allocator_stats
{
    size_t total;
    size_t current;
    size_t maximum;
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void* allocate(size_t size, void* user)
{
    struct allocator_stats* stats = user;
    assert(size > 0);

    stats->total += size;
    stats->current += size;
    if (stats->current > stats->maximum)
    {
        stats->maximum = stats->current;
    }

    printf("%s: allocate %zu bytes (current %zu maximum %zu total %zu)\n", function_name, size, stats->current,
           stats->maximum, stats->total);

    size_t* real_block = malloc(size + sizeof(size_t));
    assert(real_block != NULL);
    *real_block = size;
    return real_block + 1;
}

static void deallocate(void* block, void* user)
{
    struct allocator_stats* stats = user;
    assert(block != NULL);

    size_t* real_block = (size_t*)block - 1;
    stats->current -= *real_block;

    printf("%s: deallocate %zu bytes (current %zu maximum %zu total %zu)\n", function_name, *real_block, stats->current,
           stats->maximum, stats->total);

    free(real_block);
}

static void* reallocate(void* block, size_t size, void* user)
{
    struct allocator_stats* stats = user;
    assert(block != NULL);
    assert(size > 0);

    size_t* real_block = (size_t*)block - 1;
    stats->total += size;
    stats->current += size - *real_block;
    if (stats->current > stats->maximum)
    {
        stats->maximum = stats->current;
    }

    printf("%s: reallocate %zu bytes to %zu bytes (current %zu maximum %zu total %zu)\n", function_name, *real_block,
           size, stats->current, stats->maximum, stats->total);

    real_block = realloc(real_block, size + sizeof(size_t));
    assert(real_block != NULL);
    *real_block = size;
    return real_block + 1;
}

int main(void)
{
    struct allocator_stats stats = { 0 };
    const GLFWallocator allocator = {
        .allocate = allocate, .deallocate = deallocate, .reallocate = reallocate, .user = &stats
    };

    glfwSetErrorCallback(error_callback);
    glfwInitAllocator(&allocator);

    if (!CALL(glfwInit)())
    {
        exit(EXIT_FAILURE);
    }

    GLFWwindow* window = CALL(glfwCreateWindow)(400, 400, "Custom allocator test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    CALL(glfwMakeContextCurrent)(window);
    gladLoadGL(glfwGetProcAddress);
    CALL(glfwSwapInterval)(1);

    while (!CALL(glfwWindowShouldClose)(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        CALL(glfwSwapBuffers)(window);
        CALL(glfwWaitEvents)();
    }

    CALL(glfwTerminate)();
    exit(EXIT_SUCCESS);
}
