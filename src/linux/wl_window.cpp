//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_WAYLAND)

    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <assert.h>
    #include <unistd.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/timerfd.h>
    #include <poll.h>

    #include "wayland-client-protocol.h"
    #include "wayland-xdg-shell-client-protocol.h"
    #include "wayland-xdg-decoration-client-protocol.h"
    #include "wayland-viewporter-client-protocol.h"
    #include "wayland-relative-pointer-unstable-v1-client-protocol.h"
    #include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
    #include "wayland-idle-inhibit-unstable-v1-client-protocol.h"
    #include "wayland-text-input-unstable-v1-client-protocol.h"
    #include "wayland-text-input-unstable-v3-client-protocol.h"
    #include "wayland-xdg-activation-v1-client-protocol.h"

    #define GRWL_BORDER_SIZE 4
    #define GRWL_CAPTION_HEIGHT 24

static int createTmpfileCloexec(char* tmpname)
{
    int fd;

    fd = mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0)
    {
        unlink(tmpname);
    }

    return fd;
}

/*
 * Create a new, unique, anonymous file of the given size, and
 * return the file descriptor for it. The file descriptor is set
 * CLOEXEC. The file is immediately suitable for mmap()'ing
 * the given size at offset zero.
 *
 * The file should not have a permanent backing store like a disk,
 * but may have if XDG_RUNTIME_DIR is not properly implemented in OS.
 *
 * The file name is deleted from the file system.
 *
 * The file is suitable for buffer sharing between processes by
 * transmitting the file descriptor over Unix sockets using the
 * SCM_RIGHTS methods.
 *
 * posix_fallocate() is used to guarantee that disk space is available
 * for the file at the given size. If disk space is insufficient, errno
 * is set to ENOSPC. If posix_fallocate() is not supported, program may
 * receive SIGBUS on accessing mmap()'ed file contents instead.
 */
static int createAnonymousFile(off_t size)
{
    static const char template[] = "/grwl-shared-XXXXXX";
    const char* path;
    char* name;
    int fd;
    int ret;

    #ifdef HAVE_MEMFD_CREATE
    fd = memfd_create("grwl-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd >= 0)
    {
        // We can add this seal before calling posix_fallocate(), as the file
        // is currently zero-sized anyway.
        //
        // There is also no need to check for the return value, we couldn’t do
        // anything with it anyway.
        fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    }
    else
    #elif defined(SHM_ANON)
    fd = shm_open(SHM_ANON, O_RDWR | O_CLOEXEC, 0600);
    if (fd < 0)
    #endif
    {
        path = getenv("XDG_RUNTIME_DIR");
        if (!path)
        {
            errno = ENOENT;
            return -1;
        }

        name = _grwl_calloc(strlen(path) + sizeof(template), 1);
        strcpy(name, path);
        strcat(name, template);

        fd = createTmpfileCloexec(name);
        _grwl_free(name);
        if (fd < 0)
        {
            return -1;
        }
    }

    #if defined(SHM_ANON)
    // posix_fallocate does not work on SHM descriptors
    ret = ftruncate(fd, size);
    #else
    ret = posix_fallocate(fd, 0, size);
    #endif
    if (ret != 0)
    {
        close(fd);
        errno = ret;
        return -1;
    }
    return fd;
}

static struct wl_buffer* createShmBuffer(const GRWLimage* image)
{
    const int stride = image->width * 4;
    const int length = image->width * image->height * 4;

    const int fd = createAnonymousFile(length);
    if (fd < 0)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create buffer file of size %d: %s", length,
                        strerror(errno));
        return nullptr;
    }

    void* data = mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to map file: %s", strerror(errno));
        close(fd);
        return nullptr;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(_grwl.wl.shm, fd, length);

    close(fd);

    unsigned char* source = (unsigned char*)image->pixels;
    unsigned char* target = data;
    for (int i = 0; i < image->width * image->height; i++, source += 4)
    {
        unsigned int alpha = source[3];

        *target++ = (unsigned char)((source[2] * alpha) / 255);
        *target++ = (unsigned char)((source[1] * alpha) / 255);
        *target++ = (unsigned char)((source[0] * alpha) / 255);
        *target++ = (unsigned char)alpha;
    }

    struct wl_buffer* buffer =
        wl_shm_pool_create_buffer(pool, 0, image->width, image->height, stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

static void createFallbackDecoration(_GRWLwindow* window, _GRWLdecorationWayland* decoration, struct wl_surface* parent,
                                     struct wl_buffer* buffer, int x, int y, int width, int height)
{
    decoration->surface = wl_compositor_create_surface(_grwl.wl.compositor);
    wl_surface_set_user_data(decoration->surface, window);
    wl_proxy_set_tag((struct wl_proxy*)decoration->surface, &_grwl.wl.tag);
    decoration->subsurface = wl_subcompositor_get_subsurface(_grwl.wl.subcompositor, decoration->surface, parent);
    wl_subsurface_set_position(decoration->subsurface, x, y);
    decoration->viewport = wp_viewporter_get_viewport(_grwl.wl.viewporter, decoration->surface);
    wp_viewport_set_destination(decoration->viewport, width, height);
    wl_surface_attach(decoration->surface, buffer, 0, 0);

    struct wl_region* region = wl_compositor_create_region(_grwl.wl.compositor);
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(decoration->surface, region);
    wl_surface_commit(decoration->surface);
    wl_region_destroy(region);
}

static void createFallbackDecorations(_GRWLwindow* window)
{
    unsigned char data[] = { 224, 224, 224, 255 };
    const GRWLimage image = { 1, 1, data };

    if (!_grwl.wl.viewporter)
    {
        return;
    }

    if (!window->wl.decorations.buffer)
    {
        window->wl.decorations.buffer = createShmBuffer(&image);
    }
    if (!window->wl.decorations.buffer)
    {
        return;
    }

    createFallbackDecoration(window, &window->wl.decorations.top, window->wl.surface, window->wl.decorations.buffer, 0,
                             -GRWL_CAPTION_HEIGHT, window->wl.width, GRWL_CAPTION_HEIGHT);
    createFallbackDecoration(window, &window->wl.decorations.left, window->wl.surface, window->wl.decorations.buffer,
                             -GRWL_BORDER_SIZE, -GRWL_CAPTION_HEIGHT, GRWL_BORDER_SIZE,
                             window->wl.height + GRWL_CAPTION_HEIGHT);
    createFallbackDecoration(window, &window->wl.decorations.right, window->wl.surface, window->wl.decorations.buffer,
                             window->wl.width, -GRWL_CAPTION_HEIGHT, GRWL_BORDER_SIZE,
                             window->wl.height + GRWL_CAPTION_HEIGHT);
    createFallbackDecoration(window, &window->wl.decorations.bottom, window->wl.surface, window->wl.decorations.buffer,
                             -GRWL_BORDER_SIZE, window->wl.height, window->wl.width + GRWL_BORDER_SIZE * 2,
                             GRWL_BORDER_SIZE);
}

static void destroyFallbackDecoration(_GRWLdecorationWayland* decoration)
{
    if (decoration->subsurface)
    {
        wl_subsurface_destroy(decoration->subsurface);
    }
    if (decoration->surface)
    {
        wl_surface_destroy(decoration->surface);
    }
    if (decoration->viewport)
    {
        wp_viewport_destroy(decoration->viewport);
    }
    decoration->surface = nullptr;
    decoration->subsurface = nullptr;
    decoration->viewport = nullptr;
}

static void destroyFallbackDecorations(_GRWLwindow* window)
{
    destroyFallbackDecoration(&window->wl.decorations.top);
    destroyFallbackDecoration(&window->wl.decorations.left);
    destroyFallbackDecoration(&window->wl.decorations.right);
    destroyFallbackDecoration(&window->wl.decorations.bottom);
}

static void xdgDecorationHandleConfigure(void* userData, struct zxdg_toplevel_decoration_v1* decoration, uint32_t mode)
{
    _GRWLwindow* window = userData;

    window->wl.xdg.decorationMode = mode;

    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE)
    {
        if (window->decorated && !window->monitor)
        {
            createFallbackDecorations(window);
        }
    }
    else
    {
        destroyFallbackDecorations(window);
    }
}

static const struct zxdg_toplevel_decoration_v1_listener xdgDecorationListener = {
    xdgDecorationHandleConfigure,
};

// Makes the surface considered as XRGB instead of ARGB.
static void setContentAreaOpaque(_GRWLwindow* window)
{
    struct wl_region* region;

    region = wl_compositor_create_region(_grwl.wl.compositor);
    if (!region)
    {
        return;
    }

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
}

static void resizeWindow(_GRWLwindow* window)
{
    int scale = window->wl.contentScale;
    int scaledWidth = window->wl.width * scale;
    int scaledHeight = window->wl.height * scale;

    if (window->wl.egl.window)
    {
        wl_egl_window_resize(window->wl.egl.window, scaledWidth, scaledHeight, 0, 0);
    }
    if (!window->wl.transparent)
    {
        setContentAreaOpaque(window);
    }
    _grwlInputFramebufferSize(window, scaledWidth, scaledHeight);

    if (!window->wl.decorations.top.surface)
    {
        return;
    }

    wp_viewport_set_destination(window->wl.decorations.top.viewport, window->wl.width, GRWL_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.top.surface);

    wp_viewport_set_destination(window->wl.decorations.left.viewport, GRWL_BORDER_SIZE,
                                window->wl.height + GRWL_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.left.surface);

    wl_subsurface_set_position(window->wl.decorations.right.subsurface, window->wl.width, -GRWL_CAPTION_HEIGHT);
    wp_viewport_set_destination(window->wl.decorations.right.viewport, GRWL_BORDER_SIZE,
                                window->wl.height + GRWL_CAPTION_HEIGHT);
    wl_surface_commit(window->wl.decorations.right.surface);

    wl_subsurface_set_position(window->wl.decorations.bottom.subsurface, -GRWL_BORDER_SIZE, window->wl.height);
    wp_viewport_set_destination(window->wl.decorations.bottom.viewport, window->wl.width + GRWL_BORDER_SIZE * 2,
                                GRWL_BORDER_SIZE);
    wl_surface_commit(window->wl.decorations.bottom.surface);
}

void _grwlUpdateContentScaleWayland(_GRWLwindow* window)
{
    if (wl_compositor_get_version(_grwl.wl.compositor) < WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION)
    {
        return;
    }

    // Get the scale factor from the highest scale monitor.
    int maxScale = 1;

    for (int i = 0; i < window->wl.scaleCount; i++)
    {
        maxScale = _grwl_max(window->wl.scales[i].factor, maxScale);
    }

    // Only change the framebuffer size if the scale changed.
    if (window->wl.contentScale != maxScale)
    {
        window->wl.contentScale = maxScale;
        wl_surface_set_buffer_scale(window->wl.surface, maxScale);
        _grwlInputWindowContentScale(window, maxScale, maxScale);
        resizeWindow(window);

        if (window->wl.visible)
        {
            _grwlInputWindowDamage(window);
        }
    }
}

static void surfaceHandleEnter(void* userData, struct wl_surface* surface, struct wl_output* output)
{
    if (wl_proxy_get_tag((struct wl_proxy*)output) != &_grwl.wl.tag)
    {
        return;
    }

    _GRWLwindow* window = userData;
    _GRWLmonitor* monitor = wl_output_get_user_data(output);
    if (!window || !monitor)
    {
        return;
    }

    if (window->wl.scaleCount + 1 > window->wl.scaleSize)
    {
        window->wl.scaleSize++;
        window->wl.scales = _grwl_realloc(window->wl.scales, window->wl.scaleSize * sizeof(_GRWLscaleWayland));
    }

    window->wl.scaleCount++;
    window->wl.scales[window->wl.scaleCount - 1].factor = monitor->wl.contentScale;
    window->wl.scales[window->wl.scaleCount - 1].output = output;

    _grwlUpdateContentScaleWayland(window);
}

static void surfaceHandleLeave(void* userData, struct wl_surface* surface, struct wl_output* output)
{
    if (wl_proxy_get_tag((struct wl_proxy*)output) != &_grwl.wl.tag)
    {
        return;
    }

    _GRWLwindow* window = userData;

    for (int i = 0; i < window->wl.scaleCount; i++)
    {
        if (window->wl.scales[i].output == output)
        {
            window->wl.scales[i] = window->wl.scales[window->wl.scaleCount - 1];
            window->wl.scaleCount--;
            break;
        }
    }

    _grwlUpdateContentScaleWayland(window);
}

static const struct wl_surface_listener surfaceListener = { surfaceHandleEnter, surfaceHandleLeave };

static void setIdleInhibitor(_GRWLwindow* window, bool enable)
{
    if (enable && !window->wl.idleInhibitor && _grwl.wl.idleInhibitManager)
    {
        window->wl.idleInhibitor =
            zwp_idle_inhibit_manager_v1_create_inhibitor(_grwl.wl.idleInhibitManager, window->wl.surface);
        if (!window->wl.idleInhibitor)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create idle inhibitor");
        }
    }
    else if (!enable && window->wl.idleInhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);
        window->wl.idleInhibitor = nullptr;
    }
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_GRWLwindow* window)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_fullscreen(window->wl.libdecor.frame, window->monitor->wl.output);
    }
    else if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->monitor->wl.output);
    }

    setIdleInhibitor(window, true);

    if (window->wl.decorations.top.surface)
    {
        destroyFallbackDecorations(window);
    }
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_GRWLwindow* window)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_unset_fullscreen(window->wl.libdecor.frame);
    }
    else if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);
    }

    setIdleInhibitor(window, false);

    if (!window->wl.libdecor.frame && window->wl.xdg.decorationMode != ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE)
    {
        if (window->decorated)
        {
            createFallbackDecorations(window);
        }
    }
}

static void activateTextInputV1(_GRWLwindow* window)
{
    if (!window->wl.textInputV1)
    {
        return;
    }
    zwp_text_input_v1_show_input_panel(window->wl.textInputV1);
    zwp_text_input_v1_activate(window->wl.textInputV1, _grwl.wl.seat, window->wl.surface);
}

static void deactivateTextInputV1(_GRWLwindow* window)
{
    if (!window->wl.textInputV1)
    {
        return;
    }
    zwp_text_input_v1_hide_input_panel(window->wl.textInputV1);
    zwp_text_input_v1_deactivate(window->wl.textInputV1, _grwl.wl.seat);
}

static void xdgToplevelHandleConfigure(void* userData, struct xdg_toplevel* toplevel, int32_t width, int32_t height,
                                       struct wl_array* states)
{
    _GRWLwindow* window = userData;
    uint32_t* state;

    window->wl.pending.activated = false;
    window->wl.pending.maximized = false;
    window->wl.pending.fullscreen = false;

    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                window->wl.pending.maximized = true;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                window->wl.pending.fullscreen = true;
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                window->wl.pending.activated = true;
                activateTextInputV1(window);
                break;
        }
    }

    if (width && height)
    {
        if (window->wl.decorations.top.surface)
        {
            window->wl.pending.width = _grwl_max(0, width - GRWL_BORDER_SIZE * 2);
            window->wl.pending.height = _grwl_max(0, height - GRWL_BORDER_SIZE - GRWL_CAPTION_HEIGHT);
        }
        else
        {
            window->wl.pending.width = width;
            window->wl.pending.height = height;
        }
    }
    else
    {
        window->wl.pending.width = window->wl.width;
        window->wl.pending.height = window->wl.height;
    }
}

static void xdgToplevelHandleClose(void* userData, struct xdg_toplevel* toplevel)
{
    _GRWLwindow* window = userData;
    _grwlInputWindowCloseRequest(window);
}

static const struct xdg_toplevel_listener xdgToplevelListener = { xdgToplevelHandleConfigure, xdgToplevelHandleClose };

static void xdgSurfaceHandleConfigure(void* userData, struct xdg_surface* surface, uint32_t serial)
{
    _GRWLwindow* window = userData;

    xdg_surface_ack_configure(surface, serial);

    if (window->wl.activated != window->wl.pending.activated)
    {
        window->wl.activated = window->wl.pending.activated;
        if (!window->wl.activated)
        {
            if (window->monitor && window->autoIconify)
            {
                xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
            }
        }
    }

    if (window->wl.maximized != window->wl.pending.maximized)
    {
        window->wl.maximized = window->wl.pending.maximized;
        _grwlInputWindowMaximize(window, window->wl.maximized);
    }

    window->wl.fullscreen = window->wl.pending.fullscreen;

    int width = window->wl.pending.width;
    int height = window->wl.pending.height;

    if (!window->wl.maximized && !window->wl.fullscreen)
    {
        if (window->numer != GRWL_DONT_CARE && window->denom != GRWL_DONT_CARE)
        {
            const float aspectRatio = (float)width / (float)height;
            const float targetRatio = (float)window->numer / (float)window->denom;
            if (aspectRatio < targetRatio)
            {
                height = width / targetRatio;
            }
            else if (aspectRatio > targetRatio)
            {
                width = height * targetRatio;
            }
        }
    }

    if (width != window->wl.width || height != window->wl.height)
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeWindow(window);

        _grwlInputWindowSize(window, width, height);

        if (window->wl.visible)
        {
            _grwlInputWindowDamage(window);
        }
    }

    if (!window->wl.visible)
    {
        // Allow the window to be mapped only if it either has no XDG
        // decorations or they have already received a configure event
        if (!window->wl.xdg.decoration || window->wl.xdg.decorationMode)
        {
            window->wl.visible = true;
            _grwlInputWindowDamage(window);
        }
    }
}

static const struct xdg_surface_listener xdgSurfaceListener = { xdgSurfaceHandleConfigure };

void libdecorFrameHandleConfigure(struct libdecor_frame* frame, struct libdecor_configuration* config, void* userData)
{
    _GRWLwindow* window = userData;
    int width, height;

    enum libdecor_window_state windowState;
    bool fullscreen, activated, maximized;

    if (libdecor_configuration_get_window_state(config, &windowState))
    {
        fullscreen = (windowState & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
        activated = (windowState & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
        maximized = (windowState & LIBDECOR_WINDOW_STATE_MAXIMIZED) != 0;
    }
    else
    {
        fullscreen = window->wl.fullscreen;
        activated = window->wl.activated;
        maximized = window->wl.maximized;
    }

    if (!libdecor_configuration_get_content_size(config, frame, &width, &height))
    {
        width = window->wl.width;
        height = window->wl.height;
    }

    if (!maximized && !fullscreen)
    {
        if (window->numer != GRWL_DONT_CARE && window->denom != GRWL_DONT_CARE)
        {
            const float aspectRatio = (float)width / (float)height;
            const float targetRatio = (float)window->numer / (float)window->denom;
            if (aspectRatio < targetRatio)
            {
                height = width / targetRatio;
            }
            else if (aspectRatio > targetRatio)
            {
                width = height * targetRatio;
            }
        }
    }

    struct libdecor_state* frameState = libdecor_state_new(width, height);
    libdecor_frame_commit(frame, frameState, config);
    libdecor_state_free(frameState);

    if (window->wl.activated != activated)
    {
        window->wl.activated = activated;
        if (!window->wl.activated)
        {
            if (window->monitor && window->autoIconify)
            {
                libdecor_frame_set_minimized(window->wl.libdecor.frame);
            }
        }
    }

    if (window->wl.maximized != maximized)
    {
        window->wl.maximized = maximized;
        _grwlInputWindowMaximize(window, window->wl.maximized);
    }

    window->wl.fullscreen = fullscreen;

    bool damaged = false;

    if (!window->wl.visible)
    {
        window->wl.visible = true;
        damaged = true;
    }

    if (width != window->wl.width || height != window->wl.height)
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeWindow(window);

        _grwlInputWindowSize(window, width, height);
        damaged = true;
    }

    if (damaged)
    {
        _grwlInputWindowDamage(window);
    }
    else
    {
        wl_surface_commit(window->wl.surface);
    }
}

void libdecorFrameHandleClose(struct libdecor_frame* frame, void* userData)
{
    _GRWLwindow* window = userData;
    _grwlInputWindowCloseRequest(window);
}

void libdecorFrameHandleCommit(struct libdecor_frame* frame, void* userData)
{
    _GRWLwindow* window = userData;
    wl_surface_commit(window->wl.surface);
}

void libdecorFrameHandleDismissPopup(struct libdecor_frame* frame, const char* seatName, void* userData)
{
}

static const struct libdecor_frame_interface libdecorFrameInterface = {
    libdecorFrameHandleConfigure, libdecorFrameHandleClose, libdecorFrameHandleCommit, libdecorFrameHandleDismissPopup
};

static bool createLibdecorFrame(_GRWLwindow* window)
{
    window->wl.libdecor.frame =
        libdecor_decorate(_grwl.wl.libdecor.context, window->wl.surface, &libdecorFrameInterface, window);
    if (!window->wl.libdecor.frame)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create libdecor frame");
        return false;
    }

    if (strlen(window->wl.appId))
    {
        libdecor_frame_set_app_id(window->wl.libdecor.frame, window->wl.appId);
    }

    if (strlen(window->wl.title))
    {
        libdecor_frame_set_title(window->wl.libdecor.frame, window->wl.title);
    }

    if (window->minwidth != GRWL_DONT_CARE && window->minheight != GRWL_DONT_CARE)
    {
        libdecor_frame_set_min_content_size(window->wl.libdecor.frame, window->minwidth, window->minheight);
    }

    if (window->maxwidth != GRWL_DONT_CARE && window->maxheight != GRWL_DONT_CARE)
    {
        libdecor_frame_set_max_content_size(window->wl.libdecor.frame, window->maxwidth, window->maxheight);
    }

    if (!window->resizable)
    {
        libdecor_frame_unset_capabilities(window->wl.libdecor.frame, LIBDECOR_ACTION_RESIZE);
    }

    if (window->monitor)
    {
        // HACK: Allow libdecor to finish initialization of itself and its
        //       plugin so it will create the xdg_toplevel for the frame
        //       This needs to exist when setting the frame to fullscreen
        while (!libdecor_frame_get_xdg_toplevel(window->wl.libdecor.frame))
        {
            _grwlWaitEventsWayland();
        }

        libdecor_frame_set_fullscreen(window->wl.libdecor.frame, window->monitor->wl.output);
        setIdleInhibitor(window, true);
    }
    else
    {
        if (window->wl.maximized)
        {
            libdecor_frame_set_maximized(window->wl.libdecor.frame);
        }

        if (!window->decorated)
        {
            libdecor_frame_set_visibility(window->wl.libdecor.frame, false);
        }

        setIdleInhibitor(window, false);
    }

    libdecor_frame_map(window->wl.libdecor.frame);
    wl_display_roundtrip(_grwl.wl.display);
    return true;
}

static bool createXdgShellObjects(_GRWLwindow* window)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_grwl.wl.wmBase, window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create xdg-surface for window");
        return false;
    }

    xdg_surface_add_listener(window->wl.xdg.surface, &xdgSurfaceListener, window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create xdg-toplevel for window");
        return false;
    }

    xdg_toplevel_add_listener(window->wl.xdg.toplevel, &xdgToplevelListener, window);

    if (window->wl.appId)
    {
        xdg_toplevel_set_app_id(window->wl.xdg.toplevel, window->wl.appId);
    }

    if (window->wl.title)
    {
        xdg_toplevel_set_title(window->wl.xdg.toplevel, window->wl.title);
    }

    if (window->monitor)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->monitor->wl.output);
        setIdleInhibitor(window, true);
    }
    else
    {
        if (window->wl.maximized)
        {
            xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
        }

        setIdleInhibitor(window, false);
    }

    if (_grwl.wl.decorationManager)
    {
        window->wl.xdg.decoration =
            zxdg_decoration_manager_v1_get_toplevel_decoration(_grwl.wl.decorationManager, window->wl.xdg.toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->wl.xdg.decoration, &xdgDecorationListener, window);

        uint32_t mode;

        if (window->decorated)
        {
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        }
        else
        {
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
        }

        zxdg_toplevel_decoration_v1_set_mode(window->wl.xdg.decoration, mode);
    }
    else
    {
        if (window->decorated && !window->monitor)
        {
            createFallbackDecorations(window);
        }
    }

    if (window->minwidth != GRWL_DONT_CARE && window->minheight != GRWL_DONT_CARE)
    {
        int minwidth = window->minwidth;
        int minheight = window->minheight;

        if (window->wl.decorations.top.surface)
        {
            minwidth += GRWL_BORDER_SIZE * 2;
            minheight += GRWL_CAPTION_HEIGHT + GRWL_BORDER_SIZE;
        }

        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
    }

    if (window->maxwidth != GRWL_DONT_CARE && window->maxheight != GRWL_DONT_CARE)
    {
        int maxwidth = window->maxwidth;
        int maxheight = window->maxheight;

        if (window->wl.decorations.top.surface)
        {
            maxwidth += GRWL_BORDER_SIZE * 2;
            maxheight += GRWL_CAPTION_HEIGHT + GRWL_BORDER_SIZE;
        }

        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
    }

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_grwl.wl.display);
    return true;
}

static bool createShellObjects(_GRWLwindow* window)
{
    if (_grwl.wl.libdecor.context)
    {
        if (createLibdecorFrame(window))
        {
            return true;
        }
    }

    return createXdgShellObjects(window);
}

static void destroyShellObjects(_GRWLwindow* window)
{
    destroyFallbackDecorations(window);

    if (window->wl.libdecor.frame)
    {
        libdecor_frame_unref(window->wl.libdecor.frame);
    }

    if (window->wl.xdg.decoration)
    {
        zxdg_toplevel_decoration_v1_destroy(window->wl.xdg.decoration);
    }

    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_destroy(window->wl.xdg.toplevel);
    }

    if (window->wl.xdg.surface)
    {
        xdg_surface_destroy(window->wl.xdg.surface);
    }

    window->wl.libdecor.frame = nullptr;
    window->wl.xdg.decoration = nullptr;
    window->wl.xdg.decorationMode = 0;
    window->wl.xdg.toplevel = nullptr;
    window->wl.xdg.surface = nullptr;
}

static bool createNativeSurface(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLfbconfig* fbconfig)
{
    window->wl.surface = wl_compositor_create_surface(_grwl.wl.compositor);
    if (!window->wl.surface)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create window surface");
        return false;
    }

    wl_proxy_set_tag((struct wl_proxy*)window->wl.surface, &_grwl.wl.tag);
    wl_surface_add_listener(window->wl.surface, &surfaceListener, window);

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;
    window->wl.contentScale = 1;
    window->wl.title = _grwl_strdup(wndconfig->title);
    window->wl.appId = _grwl_strdup(wndconfig->wl.appId);

    window->wl.maximized = wndconfig->maximized;

    window->wl.transparent = fbconfig->transparent;
    if (!window->wl.transparent)
    {
        setContentAreaOpaque(window);
    }

    return true;
}

static void setCursorImage(_GRWLwindow* window, _GRWLcursorWayland* cursorWayland)
{
    struct itimerspec timer = { 0 };
    struct wl_cursor* wlCursor = cursorWayland->cursor;
    struct wl_cursor_image* image;
    struct wl_buffer* buffer;
    struct wl_surface* surface = _grwl.wl.cursorSurface;
    int scale = 1;

    if (!wlCursor)
    {
        buffer = cursorWayland->buffer;
    }
    else
    {
        if (window->wl.contentScale > 1 && cursorWayland->cursorHiDPI)
        {
            wlCursor = cursorWayland->cursorHiDPI;
            scale = 2;
        }

        image = wlCursor->images[cursorWayland->currentImage];
        buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
        {
            return;
        }

        timer.it_value.tv_sec = image->delay / 1000;
        timer.it_value.tv_nsec = (image->delay % 1000) * 1000000;
        timerfd_settime(_grwl.wl.cursorTimerfd, 0, &timer, nullptr);

        cursorWayland->width = image->width;
        cursorWayland->height = image->height;
        cursorWayland->xhot = image->hotspot_x;
        cursorWayland->yhot = image->hotspot_y;
    }

    wl_pointer_set_cursor(_grwl.wl.pointer, _grwl.wl.pointerEnterSerial, surface, cursorWayland->xhot / scale,
                          cursorWayland->yhot / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, cursorWayland->width, cursorWayland->height);
    wl_surface_commit(surface);
}

static void incrementCursorImage(_GRWLwindow* window)
{
    _GRWLcursor* cursor;

    if (!window || window->wl.decorations.focus != GRWL_MAIN_WINDOW)
    {
        return;
    }

    cursor = window->wl.currentCursor;
    if (cursor && cursor->wl.cursor)
    {
        cursor->wl.currentImage += 1;
        cursor->wl.currentImage %= cursor->wl.cursor->image_count;
        setCursorImage(window, &cursor->wl);
    }
}

static bool flushDisplay()
{
    while (wl_display_flush(_grwl.wl.display) == -1)
    {
        if (errno != EAGAIN)
        {
            return false;
        }

        struct pollfd fd = { wl_display_get_fd(_grwl.wl.display), POLLOUT };

        while (poll(&fd, 1, -1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                return false;
            }
        }
    }

    return true;
}

static int translateKey(uint32_t scancode)
{
    if (scancode < sizeof(_grwl.wl.keycodes) / sizeof(_grwl.wl.keycodes[0]))
    {
        return _grwl.wl.keycodes[scancode];
    }

    return GRWL_KEY_UNKNOWN;
}

static xkb_keysym_t composeSymbol(xkb_keysym_t sym)
{
    if (sym == XKB_KEY_NoSymbol || !_grwl.wl.xkb.composeState)
    {
        return sym;
    }
    if (xkb_compose_state_feed(_grwl.wl.xkb.composeState, sym) != XKB_COMPOSE_FEED_ACCEPTED)
    {
        return sym;
    }
    switch (xkb_compose_state_get_status(_grwl.wl.xkb.composeState))
    {
        case XKB_COMPOSE_COMPOSED:
            return xkb_compose_state_get_one_sym(_grwl.wl.xkb.composeState);
        case XKB_COMPOSE_COMPOSING:
        case XKB_COMPOSE_CANCELLED:
            return XKB_KEY_NoSymbol;
        case XKB_COMPOSE_NOTHING:
        default:
            return sym;
    }
}

static void inputText(_GRWLwindow* window, uint32_t scancode)
{
    const xkb_keysym_t* keysyms;
    const xkb_keycode_t keycode = scancode + 8;

    if (xkb_state_key_get_syms(_grwl.wl.xkb.state, keycode, &keysyms) == 1)
    {
        const xkb_keysym_t keysym = composeSymbol(keysyms[0]);
        const uint32_t codepoint = _grwlKeySym2Unicode(keysym);
        if (codepoint != GRWL_INVALID_CODEPOINT)
        {
            const int mods = _grwl.wl.xkb.modifiers;
            const int plain = !(mods & (GRWL_MOD_CONTROL | GRWL_MOD_ALT));
            _grwlInputChar(window, codepoint, mods, plain);
        }
    }
}

static void handleEvents(double* timeout)
{
    #if defined(GRWL_BUILD_LINUX_JOYSTICK)
    if (_grwl.joysticksInitialized)
    {
        _grwlDetectJoystickConnectionLinux();
    }
    #endif

    bool event = false;
    struct pollfd fds[4] = { { wl_display_get_fd(_grwl.wl.display), POLLIN },
                             { _grwl.wl.keyRepeatTimerfd, POLLIN },
                             { _grwl.wl.cursorTimerfd, POLLIN },
                             { -1, POLLIN } };

    if (_grwl.wl.libdecor.context)
    {
        fds[3].fd = libdecor_get_fd(_grwl.wl.libdecor.context);
    }

    while (!event)
    {
        while (wl_display_prepare_read(_grwl.wl.display) != 0)
        {
            wl_display_dispatch_pending(_grwl.wl.display);
        }

        // If an error other than EAGAIN happens, we have likely been disconnected
        // from the Wayland session; try to handle that the best we can.
        if (!flushDisplay())
        {
            wl_display_cancel_read(_grwl.wl.display);

            _GRWLwindow* window = _grwl.windowListHead;
            while (window)
            {
                _grwlInputWindowCloseRequest(window);
                window = window->next;
            }

            return;
        }

        if (!_grwlPollPOSIX(fds, sizeof(fds) / sizeof(fds[0]), timeout))
        {
            wl_display_cancel_read(_grwl.wl.display);
            return;
        }

        if (fds[0].revents & POLLIN)
        {
            wl_display_read_events(_grwl.wl.display);
            if (wl_display_dispatch_pending(_grwl.wl.display) > 0)
            {
                event = true;
            }
        }
        else
        {
            wl_display_cancel_read(_grwl.wl.display);
        }

        if (fds[1].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_grwl.wl.keyRepeatTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    _grwlInputKey(_grwl.wl.keyboardFocus, translateKey(_grwl.wl.keyRepeatScancode),
                                  _grwl.wl.keyRepeatScancode, GRWL_PRESS, _grwl.wl.xkb.modifiers);
                    inputText(_grwl.wl.keyboardFocus, _grwl.wl.keyRepeatScancode);
                }

                event = true;
            }
        }

        if (fds[2].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_grwl.wl.cursorTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                incrementCursorImage(_grwl.wl.pointerFocus);
                event = true;
            }
        }

        if (fds[3].revents & POLLIN)
        {
            libdecor_dispatch(_grwl.wl.libdecor.context, 0);
        }
    }
}

// Reads the specified data offer as the specified MIME type
//
static char* readDataOfferAsString(struct wl_data_offer* offer, const char* mimeType)
{
    int fds[2];

    if (pipe2(fds, O_CLOEXEC) == -1)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create pipe for data offer: %s", strerror(errno));
        return nullptr;
    }

    wl_data_offer_receive(offer, mimeType, fds[1]);
    flushDisplay();
    close(fds[1]);

    char* string = nullptr;
    size_t size = 0;
    size_t length = 0;

    for (;;)
    {
        const size_t readSize = 4096;
        const size_t requiredSize = length + readSize + 1;
        if (requiredSize > size)
        {
            char* longer = _grwl_realloc(string, requiredSize);
            if (!longer)
            {
                _grwlInputError(GRWL_OUT_OF_MEMORY, nullptr);
                close(fds[0]);
                return nullptr;
            }

            string = longer;
            size = requiredSize;
        }

        const ssize_t result = read(fds[0], string + length, readSize);
        if (result == 0)
        {
            break;
        }
        else if (result == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to read from data offer pipe: %s", strerror(errno));
            close(fds[0]);
            return nullptr;
        }

        length += result;
    }

    close(fds[0]);

    string[length] = '\0';
    return string;
}

static void pointerHandleEnter(void* userData, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface,
                               wl_fixed_t sx, wl_fixed_t sy)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
    {
        return;
    }

    if (wl_proxy_get_tag((struct wl_proxy*)surface) != &_grwl.wl.tag)
    {
        return;
    }

    _GRWLwindow* window = wl_surface_get_user_data(surface);

    if (surface == window->wl.decorations.top.surface)
    {
        window->wl.decorations.focus = GRWL_TOP_DECORATION;
    }
    else if (surface == window->wl.decorations.left.surface)
    {
        window->wl.decorations.focus = GRWL_LEFT_DECORATION;
    }
    else if (surface == window->wl.decorations.right.surface)
    {
        window->wl.decorations.focus = GRWL_RIGHT_DECORATION;
    }
    else if (surface == window->wl.decorations.bottom.surface)
    {
        window->wl.decorations.focus = GRWL_BOTTOM_DECORATION;
    }
    else
    {
        window->wl.decorations.focus = GRWL_MAIN_WINDOW;
    }

    _grwl.wl.serial = serial;
    _grwl.wl.pointerEnterSerial = serial;
    _grwl.wl.pointerFocus = window;

    window->wl.hovered = true;

    _grwlSetCursorWayland(window, window->wl.currentCursor);
    _grwlInputCursorEnter(window, true);
}

static void pointerHandleLeave(void* userData, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface)
{
    if (!surface)
    {
        return;
    }

    if (wl_proxy_get_tag((struct wl_proxy*)surface) != &_grwl.wl.tag)
    {
        return;
    }

    _GRWLwindow* window = _grwl.wl.pointerFocus;
    if (!window)
    {
        return;
    }

    window->wl.hovered = false;

    _grwl.wl.serial = serial;
    _grwl.wl.pointerFocus = nullptr;
    _grwl.wl.cursorPreviousName = nullptr;
    _grwlInputCursorEnter(window, false);
}

static void pointerHandleMotion(void* userData, struct wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    _GRWLwindow* window = _grwl.wl.pointerFocus;
    if (!window)
    {
        return;
    }

    if (window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        return;
    }

    const double xpos = wl_fixed_to_double(sx);
    const double ypos = wl_fixed_to_double(sy);
    window->wl.cursorPosX = xpos;
    window->wl.cursorPosY = ypos;

    const char* cursorName = nullptr;

    switch (window->wl.decorations.focus)
    {
        case GRWL_MAIN_WINDOW:
            _grwl.wl.cursorPreviousName = nullptr;
            _grwlInputCursorPos(window, xpos, ypos);
            return;
        case GRWL_TOP_DECORATION:
            if (ypos < GRWL_BORDER_SIZE)
            {
                cursorName = "n-resize";
            }
            else
            {
                cursorName = "left_ptr";
            }
            break;
        case GRWL_LEFT_DECORATION:
            if (ypos < GRWL_BORDER_SIZE)
            {
                cursorName = "nw-resize";
            }
            else
            {
                cursorName = "w-resize";
            }
            break;
        case GRWL_RIGHT_DECORATION:
            if (ypos < GRWL_BORDER_SIZE)
            {
                cursorName = "ne-resize";
            }
            else
            {
                cursorName = "e-resize";
            }
            break;
        case GRWL_BOTTOM_DECORATION:
            if (xpos < GRWL_BORDER_SIZE)
            {
                cursorName = "sw-resize";
            }
            else if (xpos > window->wl.width + GRWL_BORDER_SIZE)
            {
                cursorName = "se-resize";
            }
            else
            {
                cursorName = "s-resize";
            }
            break;
        default:
            assert(0);
    }

    if (_grwl.wl.cursorPreviousName != cursorName)
    {
        struct wl_surface* surface = _grwl.wl.cursorSurface;
        struct wl_cursor_theme* theme = _grwl.wl.cursorTheme;
        int scale = 1;

        if (window->wl.contentScale > 1 && _grwl.wl.cursorThemeHiDPI)
        {
            // We only support up to scale=2 for now, since libwayland-cursor
            // requires us to load a different theme for each size.
            scale = 2;
            theme = _grwl.wl.cursorThemeHiDPI;
        }

        struct wl_cursor* cursor = wl_cursor_theme_get_cursor(theme, cursorName);
        if (!cursor)
        {
            return;
        }

        // TODO: handle animated cursors too.
        struct wl_cursor_image* image = cursor->images[0];
        if (!image)
        {
            return;
        }

        struct wl_buffer* buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
        {
            return;
        }

        wl_pointer_set_cursor(_grwl.wl.pointer, _grwl.wl.pointerEnterSerial, surface, image->hotspot_x / scale,
                              image->hotspot_y / scale);
        wl_surface_set_buffer_scale(surface, scale);
        wl_surface_attach(surface, buffer, 0, 0);
        wl_surface_damage(surface, 0, 0, image->width, image->height);
        wl_surface_commit(surface);

        _grwl.wl.cursorPreviousName = cursorName;
    }
}

static void pointerHandleButton(void* userData, struct wl_pointer* pointer, uint32_t serial, uint32_t time,
                                uint32_t button, uint32_t state)
{
    _GRWLwindow* window = _grwl.wl.pointerFocus;
    int grwlButton;
    uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

    if (!window)
    {
        return;
    }

    // On weston, pressing the title bar will cause leave event and never emit
    // enter event even though back to content area by pressing mouse button
    // just after it. So activate it here explicitly.
    activateTextInputV1(window);

    if (button == BTN_LEFT)
    {
        switch (window->wl.decorations.focus)
        {
            case GRWL_MAIN_WINDOW:
                break;
            case GRWL_TOP_DECORATION:
                if (window->wl.cursorPosY < GRWL_BORDER_SIZE)
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                }
                else
                {
                    xdg_toplevel_move(window->wl.xdg.toplevel, _grwl.wl.seat, serial);
                }
                break;
            case GRWL_LEFT_DECORATION:
                if (window->wl.cursorPosY < GRWL_BORDER_SIZE)
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                }
                else
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                }
                break;
            case GRWL_RIGHT_DECORATION:
                if (window->wl.cursorPosY < GRWL_BORDER_SIZE)
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                }
                else
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                }
                break;
            case GRWL_BOTTOM_DECORATION:
                if (window->wl.cursorPosX < GRWL_BORDER_SIZE)
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                }
                else if (window->wl.cursorPosX > window->wl.width + GRWL_BORDER_SIZE)
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                }
                else
                {
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                }
                break;
            default:
                assert(0);
        }
        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
        {
            xdg_toplevel_resize(window->wl.xdg.toplevel, _grwl.wl.seat, serial, edges);
            return;
        }
    }
    else if (button == BTN_RIGHT)
    {
        if (window->wl.decorations.focus != GRWL_MAIN_WINDOW && window->wl.xdg.toplevel)
        {
            xdg_toplevel_show_window_menu(window->wl.xdg.toplevel, _grwl.wl.seat, serial, window->wl.cursorPosX,
                                          window->wl.cursorPosY);
            return;
        }
    }

    // Don’t pass the button to the user if it was related to a decoration.
    if (window->wl.decorations.focus != GRWL_MAIN_WINDOW)
    {
        return;
    }

    _grwl.wl.serial = serial;

    /* Makes left, right and middle 0, 1 and 2. Overall order follows evdev
     * codes. */
    grwlButton = button - BTN_LEFT;

    _grwlInputMouseClick(window, grwlButton, state == WL_POINTER_BUTTON_STATE_PRESSED ? GRWL_PRESS : GRWL_RELEASE,
                         _grwl.wl.xkb.modifiers);
}

static void pointerHandleAxis(void* userData, struct wl_pointer* pointer, uint32_t time, uint32_t axis,
                              wl_fixed_t value)
{
    _GRWLwindow* window = _grwl.wl.pointerFocus;
    double x = 0.0, y = 0.0;
    // Wayland scroll events are in pointer motion coordinate space (think two
    // finger scroll).  The factor 10 is commonly used to convert to "scroll
    // step means 1.0.
    const double scrollFactor = 1.0 / 10.0;

    if (!window)
    {
        return;
    }

    assert(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL || axis == WL_POINTER_AXIS_VERTICAL_SCROLL);

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
    {
        x = -wl_fixed_to_double(value) * scrollFactor;
    }
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
    {
        y = -wl_fixed_to_double(value) * scrollFactor;
    }

    _grwlInputScroll(window, x, y);
}

static const struct wl_pointer_listener pointerListener = {
    pointerHandleEnter, pointerHandleLeave, pointerHandleMotion, pointerHandleButton, pointerHandleAxis,
};

static void keyboardHandleKeymap(void* userData, struct wl_keyboard* keyboard, uint32_t format, int fd, uint32_t size)
{
    struct xkb_keymap* keymap;
    struct xkb_state* state;
    struct xkb_compose_table* composeTable;
    struct xkb_compose_state* composeState;

    char* mapStr;
    const char* locale;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        close(fd);
        return;
    }

    mapStr = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapStr == MAP_FAILED)
    {
        close(fd);
        return;
    }

    keymap = xkb_keymap_new_from_string(_grwl.wl.xkb.context, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1, 0);
    munmap(mapStr, size);
    close(fd);

    if (!keymap)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to compile keymap");
        return;
    }

    state = xkb_state_new(keymap);
    if (!state)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    // Look up the preferred locale, falling back to "C" as default.
    locale = getenv("LC_ALL");
    if (!locale)
    {
        locale = getenv("LC_CTYPE");
    }
    if (!locale)
    {
        locale = getenv("LANG");
    }
    if (!locale)
    {
        locale = "C";
    }

    composeTable = xkb_compose_table_new_from_locale(_grwl.wl.xkb.context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState = xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
        {
            _grwl.wl.xkb.composeState = composeState;
        }
        else
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create XKB compose state");
        }
    }
    else
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create XKB compose table");
    }

    xkb_keymap_unref(_grwl.wl.xkb.keymap);
    xkb_state_unref(_grwl.wl.xkb.state);
    _grwl.wl.xkb.keymap = keymap;
    _grwl.wl.xkb.state = state;

    _grwl.wl.xkb.controlIndex = xkb_keymap_mod_get_index(_grwl.wl.xkb.keymap, "Control");
    _grwl.wl.xkb.altIndex = xkb_keymap_mod_get_index(_grwl.wl.xkb.keymap, "Mod1");
    _grwl.wl.xkb.shiftIndex = xkb_keymap_mod_get_index(_grwl.wl.xkb.keymap, "Shift");
    _grwl.wl.xkb.superIndex = xkb_keymap_mod_get_index(_grwl.wl.xkb.keymap, "Mod4");
    _grwl.wl.xkb.capsLockIndex = xkb_keymap_mod_get_index(_grwl.wl.xkb.keymap, "Lock");
    _grwl.wl.xkb.numLockIndex = xkb_keymap_mod_get_index(_grwl.wl.xkb.keymap, "Mod2");
}

static void keyboardHandleEnter(void* userData, struct wl_keyboard* keyboard, uint32_t serial,
                                struct wl_surface* surface, struct wl_array* keys)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
    {
        return;
    }

    if (wl_proxy_get_tag((struct wl_proxy*)surface) != &_grwl.wl.tag)
    {
        return;
    }

    _GRWLwindow* window = wl_surface_get_user_data(surface);
    if (surface != window->wl.surface)
    {
        return;
    }

    _grwl.wl.serial = serial;
    _grwl.wl.keyboardFocus = window;
    _grwlInputWindowFocus(window, true);
}

static void keyboardHandleLeave(void* userData, struct wl_keyboard* keyboard, uint32_t serial,
                                struct wl_surface* surface)
{
    _GRWLwindow* window = _grwl.wl.keyboardFocus;

    if (!window)
    {
        return;
    }

    struct itimerspec timer = { 0 };
    timerfd_settime(_grwl.wl.keyRepeatTimerfd, 0, &timer, nullptr);

    _grwl.wl.serial = serial;
    _grwl.wl.keyboardFocus = nullptr;
    _grwlInputWindowFocus(window, false);
}

static void keyboardHandleKey(void* userData, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time,
                              uint32_t scancode, uint32_t state)
{
    _GRWLwindow* window = _grwl.wl.keyboardFocus;
    if (!window)
    {
        return;
    }

    const int key = translateKey(scancode);
    const int action = state == WL_KEYBOARD_KEY_STATE_PRESSED ? GRWL_PRESS : GRWL_RELEASE;

    _grwl.wl.serial = serial;

    struct itimerspec timer = { 0 };

    if (action == GRWL_PRESS)
    {
        const xkb_keycode_t keycode = scancode + 8;

        if (xkb_keymap_key_repeats(_grwl.wl.xkb.keymap, keycode) && _grwl.wl.keyRepeatRate > 0)
        {
            _grwl.wl.keyRepeatScancode = scancode;
            if (_grwl.wl.keyRepeatRate > 1)
            {
                timer.it_interval.tv_nsec = 1000000000 / _grwl.wl.keyRepeatRate;
            }
            else
            {
                timer.it_interval.tv_sec = 1;
            }

            timer.it_value.tv_sec = _grwl.wl.keyRepeatDelay / 1000;
            timer.it_value.tv_nsec = (_grwl.wl.keyRepeatDelay % 1000) * 1000000;
        }
    }

    timerfd_settime(_grwl.wl.keyRepeatTimerfd, 0, &timer, nullptr);

    _grwlInputKey(window, key, scancode, action, _grwl.wl.xkb.modifiers);

    if (action == GRWL_PRESS)
    {
        inputText(window, scancode);
    }
}

static void keyboardHandleModifiers(void* userData, struct wl_keyboard* keyboard, uint32_t serial,
                                    uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
    _grwl.wl.serial = serial;

    if (!_grwl.wl.xkb.keymap)
    {
        return;
    }

    xkb_state_update_mask(_grwl.wl.xkb.state, modsDepressed, modsLatched, modsLocked, 0, 0, group);

    _grwl.wl.xkb.modifiers = 0;

    struct
    {
        xkb_mod_index_t index;
        unsigned int bit;
    } modifiers[] = {
        { _grwl.wl.xkb.controlIndex, GRWL_MOD_CONTROL },    { _grwl.wl.xkb.altIndex, GRWL_MOD_ALT },
        { _grwl.wl.xkb.shiftIndex, GRWL_MOD_SHIFT },        { _grwl.wl.xkb.superIndex, GRWL_MOD_SUPER },
        { _grwl.wl.xkb.capsLockIndex, GRWL_MOD_CAPS_LOCK }, { _grwl.wl.xkb.numLockIndex, GRWL_MOD_NUM_LOCK }
    };

    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++)
    {
        if (xkb_state_mod_index_is_active(_grwl.wl.xkb.state, modifiers[i].index, XKB_STATE_MODS_EFFECTIVE) == 1)
        {
            _grwl.wl.xkb.modifiers |= modifiers[i].bit;
        }
    }

    if (_grwl.wl.xkb.group != group)
    {
        _grwl.wl.xkb.group = group;
        _grwlInputKeyboardLayout();
    }
}

    #ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
static void keyboardHandleRepeatInfo(void* userData, struct wl_keyboard* keyboard, int32_t rate, int32_t delay)
{
    if (keyboard != _grwl.wl.keyboard)
    {
        return;
    }

    _grwl.wl.keyRepeatRate = rate;
    _grwl.wl.keyRepeatDelay = delay;
}
    #endif

static const struct wl_keyboard_listener keyboardListener = {
    keyboardHandleKeymap,     keyboardHandleEnter, keyboardHandleLeave, keyboardHandleKey, keyboardHandleModifiers,
    #ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    keyboardHandleRepeatInfo,
    #endif
};

static void seatHandleCapabilities(void* userData, struct wl_seat* seat, enum wl_seat_capability caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !_grwl.wl.pointer)
    {
        _grwl.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(_grwl.wl.pointer, &pointerListener, nullptr);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && _grwl.wl.pointer)
    {
        wl_pointer_destroy(_grwl.wl.pointer);
        _grwl.wl.pointer = nullptr;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !_grwl.wl.keyboard)
    {
        _grwl.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(_grwl.wl.keyboard, &keyboardListener, nullptr);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && _grwl.wl.keyboard)
    {
        wl_keyboard_destroy(_grwl.wl.keyboard);
        _grwl.wl.keyboard = nullptr;
    }
}

static void seatHandleName(void* userData, struct wl_seat* seat, const char* name)
{
}

static const struct wl_seat_listener seatListener = {
    seatHandleCapabilities,
    seatHandleName,
};

static void dataOfferHandleOffer(void* userData, struct wl_data_offer* offer, const char* mimeType)
{
    for (unsigned int i = 0; i < _grwl.wl.offerCount; i++)
    {
        if (_grwl.wl.offers[i].offer == offer)
        {
            if (strcmp(mimeType, "text/plain;charset=utf-8") == 0)
            {
                _grwl.wl.offers[i].text_plain_utf8 = true;
            }
            else if (strcmp(mimeType, "text/uri-list") == 0)
            {
                _grwl.wl.offers[i].text_uri_list = true;
            }

            break;
        }
    }
}

static const struct wl_data_offer_listener dataOfferListener = { dataOfferHandleOffer };

static void dataDeviceHandleDataOffer(void* userData, struct wl_data_device* device, struct wl_data_offer* offer)
{
    _GRWLofferWayland* offers = _grwl_realloc(_grwl.wl.offers, sizeof(_GRWLofferWayland) * (_grwl.wl.offerCount + 1));
    if (!offers)
    {
        _grwlInputError(GRWL_OUT_OF_MEMORY, nullptr);
        return;
    }

    _grwl.wl.offers = offers;
    _grwl.wl.offerCount++;

    _grwl.wl.offers[_grwl.wl.offerCount - 1] = (_GRWLofferWayland) { offer };
    wl_data_offer_add_listener(offer, &dataOfferListener, nullptr);
}

static void dataDeviceHandleEnter(void* userData, struct wl_data_device* device, uint32_t serial,
                                  struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer* offer)
{
    if (_grwl.wl.dragOffer)
    {
        wl_data_offer_destroy(_grwl.wl.dragOffer);
        _grwl.wl.dragOffer = nullptr;
        _grwl.wl.dragFocus = nullptr;
    }

    for (unsigned int i = 0; i < _grwl.wl.offerCount; i++)
    {
        if (_grwl.wl.offers[i].offer == offer)
        {
            _GRWLwindow* window = nullptr;

            if (surface)
            {
                if (wl_proxy_get_tag((struct wl_proxy*)surface) == &_grwl.wl.tag)
                {
                    window = wl_surface_get_user_data(surface);
                }
            }

            if (surface == window->wl.surface && _grwl.wl.offers[i].text_uri_list)
            {
                _grwl.wl.dragOffer = offer;
                _grwl.wl.dragFocus = window;
                _grwl.wl.dragSerial = serial;
            }

            _grwl.wl.offers[i] = _grwl.wl.offers[_grwl.wl.offerCount - 1];
            _grwl.wl.offerCount--;
            break;
        }
    }

    if (wl_proxy_get_tag((struct wl_proxy*)surface) != &_grwl.wl.tag)
    {
        return;
    }

    if (_grwl.wl.dragOffer)
    {
        wl_data_offer_accept(offer, serial, "text/uri-list");
    }
    else
    {
        wl_data_offer_accept(offer, serial, nullptr);
        wl_data_offer_destroy(offer);
    }
}

static void dataDeviceHandleLeave(void* userData, struct wl_data_device* device)
{
    if (_grwl.wl.dragOffer)
    {
        wl_data_offer_destroy(_grwl.wl.dragOffer);
        _grwl.wl.dragOffer = nullptr;
        _grwl.wl.dragFocus = nullptr;
    }
}

static void dataDeviceHandleMotion(void* userData, struct wl_data_device* device, uint32_t time, wl_fixed_t x,
                                   wl_fixed_t y)
{
}

static void dataDeviceHandleDrop(void* userData, struct wl_data_device* device)
{
    if (!_grwl.wl.dragOffer)
    {
        return;
    }

    char* string = readDataOfferAsString(_grwl.wl.dragOffer, "text/uri-list");
    if (string)
    {
        int count;
        char** paths = _grwlParseUriList(string, &count);
        if (paths)
        {
            _grwlInputDrop(_grwl.wl.dragFocus, count, (const char**)paths);
        }

        for (int i = 0; i < count; i++)
        {
            _grwl_free(paths[i]);
        }

        _grwl_free(paths);
    }

    _grwl_free(string);
}

static void dataDeviceHandleSelection(void* userData, struct wl_data_device* device, struct wl_data_offer* offer)
{
    if (_grwl.wl.selectionOffer)
    {
        wl_data_offer_destroy(_grwl.wl.selectionOffer);
        _grwl.wl.selectionOffer = nullptr;
    }

    for (unsigned int i = 0; i < _grwl.wl.offerCount; i++)
    {
        if (_grwl.wl.offers[i].offer == offer)
        {
            if (_grwl.wl.offers[i].text_plain_utf8)
            {
                _grwl.wl.selectionOffer = offer;
            }
            else
            {
                wl_data_offer_destroy(offer);
            }

            _grwl.wl.offers[i] = _grwl.wl.offers[_grwl.wl.offerCount - 1];
            _grwl.wl.offerCount--;
            break;
        }
    }
}

const struct wl_data_device_listener dataDeviceListener = {
    dataDeviceHandleDataOffer, dataDeviceHandleEnter, dataDeviceHandleLeave,
    dataDeviceHandleMotion,    dataDeviceHandleDrop,  dataDeviceHandleSelection,
};

void _grwlAddSeatListenerWayland(struct wl_seat* seat)
{
    wl_seat_add_listener(seat, &seatListener, nullptr);
}

void _grwlAddDataDeviceListenerWayland(struct wl_data_device* device)
{
    wl_data_device_add_listener(device, &dataDeviceListener, nullptr);
}

// Callbacks for text_input_unstable_v3 protocol.
//
// This protocol is widely supported by major desktop environments such as GNOME
// or KDE.
//
static void textInputV3Enter(void* data, struct zwp_text_input_v3* textInputV3, struct wl_surface* surface)
{
    zwp_text_input_v3_enable(textInputV3);
    zwp_text_input_v3_commit(textInputV3);
}

static void textInputV3Reset(_GRWLwindow* window)
{
    _GRWLpreedit* preedit = &window->preedit;

    preedit->textCount = 0;
    preedit->blockSizesCount = 0;
    preedit->focusedBlockIndex = 0;
    preedit->caretIndex = 0;

    _grwlInputPreedit(window);
}

static void textInputV3Leave(void* data, struct zwp_text_input_v3* textInputV3, struct wl_surface* surface)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    zwp_text_input_v3_disable(textInputV3);
    zwp_text_input_v3_commit(textInputV3);

    // Although this should be handled by IM via preedit callback, it seems that
    // the behavior varies depending on implemention. It's cleared by IM on
    // Ubuntu 22.04 but not cleared on Ubuntu 20.04.
    textInputV3Reset(window);
}

static void textInputV3PreeditString(void* data, struct zwp_text_input_v3* textInputV3, const char* text,
                                     int32_t cursorBegin, int32_t cursorEnd)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    _GRWLpreedit* preedit = &window->preedit;
    const char* cur = text;
    unsigned int cursorLength = 0;

    preedit->textCount = 0;
    preedit->blockSizesCount = 0;
    preedit->focusedBlockIndex = 0;
    preedit->caretIndex = 0;

    // Store preedit text
    while (cur && *cur)
    {
        uint32_t codepoint = _grwlDecodeUTF8(&cur);

        ++preedit->textCount;

        if (cur == text + cursorBegin)
        {
            preedit->caretIndex = preedit->textCount;
        }
        if (cursorBegin != cursorEnd && cur == text + cursorEnd)
        {
            cursorLength = preedit->textCount - cursorBegin;
        }

        if (preedit->textBufferCount < preedit->textCount + 1)
        {
            int bufSize = preedit->textBufferCount;

            while (bufSize < preedit->textCount + 1)
            {
                bufSize = (bufSize == 0) ? 1 : bufSize * 2;
            }
            preedit->text = _grwl_realloc(preedit->text, sizeof(unsigned int) * bufSize);
            if (!preedit->text)
            {
                return;
            }
            preedit->textBufferCount = bufSize;
        }
        preedit->text[preedit->textCount - 1] = codepoint;
    }
    if (preedit->text)
    {
        preedit->text[preedit->textCount] = 0;
    }

    // Store preedit blocks
    if (preedit->textCount)
    {
        int* blocks = preedit->blockSizes;
        int blockCount = preedit->blockSizesCount;
        int cursorPos = preedit->caretIndex;
        int textCount = preedit->textCount;

        if (!preedit->blockSizes)
        {
            int bufSize = 3;

            preedit->blockSizesBufferCount = bufSize;
            preedit->blockSizes = _grwl_calloc(sizeof(int), bufSize);
            if (!preedit->blockSizes)
            {
                return;
            }
            blocks = preedit->blockSizes;
        }

        if (cursorLength && cursorPos)
        {
            blocks[blockCount++] = cursorPos;
        }

        preedit->focusedBlockIndex = blockCount;
        blocks[blockCount++] = cursorLength ? cursorLength : textCount;

        if (cursorLength && cursorPos + cursorLength != textCount)
        {
            blocks[blockCount++] = textCount - cursorPos - cursorLength;
        }

        preedit->blockSizesCount = blockCount;
    }
}

static void textInputV3CommitString(void* data, struct zwp_text_input_v3* textInputV3, const char* text)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    const char* cur = text;

    while (cur && *cur)
    {
        uint32_t codepoint = _grwlDecodeUTF8(&cur);
        window->callbacks.character((GRWLwindow*)window, codepoint);
    }
}

static void textInputV3DeleteSurroundingText(void* data, struct zwp_text_input_v3* textInputV3, uint32_t beforeLength,
                                             uint32_t afterLength)
{
}

static void textInputV3Done(void* data, struct zwp_text_input_v3* textInputV3, uint32_t serial)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    _grwlUpdatePreeditCursorRectangleWayland(window);
    _grwlInputPreedit(window);
}

static const struct zwp_text_input_v3_listener textInputV3Listener = { textInputV3Enter,
                                                                       textInputV3Leave,
                                                                       textInputV3PreeditString,
                                                                       textInputV3CommitString,
                                                                       textInputV3DeleteSurroundingText,
                                                                       textInputV3Done };

// Callbacks for text_input_unstable_v1 protocol
//
// This protocol isn't so popular but Weston which is the reference Wayland
// implementation supports only this protocol and doesn't support
// text_input_unstable_v3.
//
static void textInputV1Enter(void* data, struct zwp_text_input_v1* textInputV1, struct wl_surface* surface)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    activateTextInputV1(window);
}

static void textInputV1Reset(_GRWLwindow* window)
{
    _GRWLpreedit* preedit = &window->preedit;

    preedit->textCount = 0;
    preedit->blockSizesCount = 0;
    preedit->focusedBlockIndex = 0;
    preedit->caretIndex = 0;

    _grwl_free(window->wl.textInputV1Context.preeditText);
    _grwl_free(window->wl.textInputV1Context.commitTextOnReset);
    window->wl.textInputV1Context.preeditText = nullptr;
    window->wl.textInputV1Context.commitTextOnReset = nullptr;

    _grwlInputPreedit(window);
}

static void textInputV1Leave(void* data, struct zwp_text_input_v1* textInputV1)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    char* commitText = window->wl.textInputV1Context.commitTextOnReset;

    textInputV3CommitString(data, nullptr, commitText);
    textInputV1Reset(window);
    deactivateTextInputV1(window);
}

static void textInputV1ModifiersMap(void* data, struct zwp_text_input_v1* textInputV1, struct wl_array* map)
{
}

static void textInputV1InputPanelState(void* data, struct zwp_text_input_v1* textInputV1, uint32_t state)
{
}

static void textInputV1PreeditString(void* data, struct zwp_text_input_v1* textInputV1, uint32_t serial,
                                     const char* text, const char* commit)
{
    _GRWLwindow* window = (_GRWLwindow*)data;

    _grwl_free(window->wl.textInputV1Context.preeditText);
    _grwl_free(window->wl.textInputV1Context.commitTextOnReset);
    window->wl.textInputV1Context.preeditText = strdup(text);
    window->wl.textInputV1Context.commitTextOnReset = strdup(commit);

    textInputV3PreeditString(data, nullptr, text, 0, 0);
    _grwlInputPreedit(window);
}

static void textInputV1PreeditStyling(void* data, struct zwp_text_input_v1* textInputV1, uint32_t index,
                                      uint32_t length, uint32_t style)
{
}

static void textInputV1PreeditCursor(void* data, struct zwp_text_input_v1* textInputV1, int32_t index)
{
    _GRWLwindow* window = (_GRWLwindow*)data;
    _GRWLpreedit* preedit = &window->preedit;
    const char* text = window->wl.textInputV1Context.preeditText;
    const char* cur = text;

    preedit->caretIndex = 0;
    if (index <= 0 || preedit->textCount == 0)
    {
        return;
    }

    while (cur && *cur)
    {
        _grwlDecodeUTF8(&cur);
        ++preedit->caretIndex;
        if (cur >= text + index)
        {
            break;
        }
        if (preedit->caretIndex > preedit->textCount)
        {
            break;
        }
    }
}

static void textInputV1CommitString(void* data, struct zwp_text_input_v1* textInputV1, uint32_t serial,
                                    const char* text)
{
    _GRWLwindow* window = (_GRWLwindow*)data;

    textInputV1Reset(window);
    textInputV3CommitString(data, nullptr, text);
}

static void textInputV1CursorPosition(void* data, struct zwp_text_input_v1* textInputV1, int32_t index, int32_t anchor)
{
    // It's for surrounding text feature which isn't supported by GRWL.
}

static void textInputV1DeleteSurroundingText(void* data, struct zwp_text_input_v1* textInputV1, int32_t index,
                                             uint32_t length)
{
}

static void textInputV1Keysym(void* data, struct zwp_text_input_v1* textInputV1, uint32_t serial, uint32_t time,
                              uint32_t sym, uint32_t state, uint32_t modifiers)
{
    uint32_t scancode;

    // This code supports only weston-keyboard because we aren't aware
    // of any other input methods that actually support this API.
    // Supporting all keysyms is overkill for now.

    switch (sym)
    {
        case XKB_KEY_Left:
            scancode = KEY_LEFT;
            break;
        case XKB_KEY_Right:
            scancode = KEY_RIGHT;
            break;
        case XKB_KEY_Up:
            scancode = KEY_UP;
            break;
        case XKB_KEY_Down:
            scancode = KEY_DOWN;
            break;
        case XKB_KEY_BackSpace:
            scancode = KEY_BACKSPACE;
            break;
        case XKB_KEY_Tab:
            scancode = KEY_TAB;
            break;
        case XKB_KEY_KP_Enter:
            scancode = KEY_KPENTER;
            break;
        case XKB_KEY_Return:
            scancode = KEY_ENTER;
            break;
        default:
            return;
    }

    _grwl.wl.xkb.modifiers = modifiers;

    keyboardHandleKey(data, _grwl.wl.keyboard, serial, time, scancode, state);
}

static void textInputV1Language(void* data, struct zwp_text_input_v1* textInputV1, uint32_t serial,
                                const char* language)
{
}

static void textInputV1TextDirection(void* data, struct zwp_text_input_v1* textInputV1, uint32_t serial,
                                     uint32_t direction)
{
}

static const struct zwp_text_input_v1_listener textInputV1Listener = {
    textInputV1Enter,          textInputV1Leave,
    textInputV1ModifiersMap,   textInputV1InputPanelState,
    textInputV1PreeditString,  textInputV1PreeditStyling,
    textInputV1PreeditCursor,  textInputV1CommitString,
    textInputV1CursorPosition, textInputV1DeleteSurroundingText,
    textInputV1Keysym,         textInputV1Language,
    textInputV1TextDirection
};

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

bool _grwlCreateWindowWayland(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                              const _GRWLfbconfig* fbconfig)
{
    if (!createNativeSurface(window, wndconfig, fbconfig))
    {
        return false;
    }

    if (ctxconfig->client != GRWL_NO_API)
    {
        if (ctxconfig->source == GRWL_EGL_CONTEXT_API || ctxconfig->source == GRWL_NATIVE_CONTEXT_API)
        {
            window->wl.egl.window = wl_egl_window_create(window->wl.surface, wndconfig->width, wndconfig->height);
            if (!window->wl.egl.window)
            {
                _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create EGL window");
                return false;
            }

            if (!_grwlInitEGL())
            {
                return false;
            }
            if (!_grwlCreateContextEGL(window, ctxconfig, fbconfig))
            {
                return false;
            }
        }
        else if (ctxconfig->source == GRWL_OSMESA_CONTEXT_API)
        {
            if (!_grwlInitOSMesa())
            {
                return false;
            }
            if (!_grwlCreateContextOSMesa(window, ctxconfig, fbconfig))
            {
                return false;
            }
        }

        if (!_grwlRefreshContextAttribs(window, ctxconfig))
        {
            return false;
        }
    }

    if (wndconfig->mousePassthrough)
    {
        _grwlSetWindowMousePassthroughWayland(window, true);
    }

    if (window->monitor || wndconfig->visible)
    {
        if (!createShellObjects(window))
        {
            return false;
        }
    }

    if (_grwl.wl.textInputManagerV3)
    {
        window->wl.textInputV3 = zwp_text_input_manager_v3_get_text_input(_grwl.wl.textInputManagerV3, _grwl.wl.seat);
        zwp_text_input_v3_add_listener(window->wl.textInputV3, &textInputV3Listener, window);
    }
    else if (_grwl.wl.textInputManagerV1)
    {
        window->wl.textInputV1 = zwp_text_input_manager_v1_create_text_input(_grwl.wl.textInputManagerV1);
        zwp_text_input_v1_add_listener(window->wl.textInputV1, &textInputV1Listener, window);
    }

    // Reset progress state as it gets saved between application runs
    if (_grwl.dbus.connection)
    {
        // Window nullptr is safe here because it won't get
        // used inside the SetWindowTaskbarProgress function
        _grwlSetWindowProgressIndicatorWayland(nullptr, GRWL_PROGRESS_INDICATOR_DISABLED, 0.0);
    }

    return true;
}

void _grwlDestroyWindowWayland(_GRWLwindow* window)
{
    if (window == _grwl.wl.pointerFocus)
    {
        _grwl.wl.pointerFocus = nullptr;
    }

    if (window == _grwl.wl.keyboardFocus)
    {
        _grwl.wl.keyboardFocus = nullptr;
    }

    if (window->wl.textInputV1)
    {
        zwp_text_input_v1_destroy(window->wl.textInputV1);
        _grwl_free(window->wl.textInputV1Context.preeditText);
        _grwl_free(window->wl.textInputV1Context.commitTextOnReset);
    }

    if (window->wl.textInputV3)
    {
        zwp_text_input_v3_destroy(window->wl.textInputV3);
    }

    if (window->wl.activationToken)
    {
        xdg_activation_token_v1_destroy(window->wl.activationToken);
    }

    if (window->wl.idleInhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);
    }

    if (window->wl.relativePointer)
    {
        zwp_relative_pointer_v1_destroy(window->wl.relativePointer);
    }

    if (window->wl.lockedPointer)
    {
        zwp_locked_pointer_v1_destroy(window->wl.lockedPointer);
    }

    if (window->wl.confinedPointer)
    {
        zwp_confined_pointer_v1_destroy(window->wl.confinedPointer);
    }

    if (window->context.destroy)
    {
        window->context.destroy(window);
    }

    destroyShellObjects(window);

    if (window->wl.decorations.buffer)
    {
        wl_buffer_destroy(window->wl.decorations.buffer);
    }

    if (window->wl.egl.window)
    {
        wl_egl_window_destroy(window->wl.egl.window);
    }

    if (window->wl.surface)
    {
        wl_surface_destroy(window->wl.surface);
    }

    _grwl_free(window->wl.title);
    _grwl_free(window->wl.appId);
    _grwl_free(window->wl.scales);
}

void _grwlSetWindowTitleWayland(_GRWLwindow* window, const char* title)
{
    char* copy = _grwl_strdup(title);
    _grwl_free(window->wl.title);
    window->wl.title = copy;

    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_title(window->wl.libdecor.frame, title);
    }
    else if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_title(window->wl.xdg.toplevel, title);
    }
}

void _grwlSetWindowIconWayland(_GRWLwindow* window, int count, const GRWLimage* images)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The platform does not support setting the window icon");
}

void _grwlSetWindowProgressIndicatorWayland(_GRWLwindow* window, const int progressState, double value)
{
    () window;

    const dbus_bool_t progressVisible = (progressState != GRWL_PROGRESS_INDICATOR_DISABLED);

    _grwlUpdateTaskbarProgressDBusPOSIX(progressVisible, value);
}

void _grwlSetWindowBadgeWayland(_GRWLwindow* window, int count)
{
    if (window != nullptr)
    {
        _grwlInputError(
            GRWL_FEATURE_UNAVAILABLE,
            "Wayland: Cannot set a badge for a window. Pass nullptr to set the application's shared badge.");
        return;
    }

    const dbus_bool_t badgeVisible = (count > 0);

    _grwlUpdateBadgeDBusPOSIX(badgeVisible, count);
}

void _grwlSetWindowBadgeStringWayland(_GRWLwindow* window, const char* string)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE,
                    "Wayland: Unable to set a string badge. Only integer badges are supported.");
}

void _grwlGetWindowPosWayland(_GRWLwindow* window, int* xpos, int* ypos)
{
    // A Wayland client is not aware of its position, so just warn and leave it
    // as (0, 0)

    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The platform does not provide the window position");
}

void _grwlSetWindowPosWayland(_GRWLwindow* window, int xpos, int ypos)
{
    // A Wayland client can not set its position, so just warn

    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The platform does not support setting the window position");
}

void _grwlGetWindowSizeWayland(_GRWLwindow* window, int* width, int* height)
{
    if (width)
    {
        *width = window->wl.width;
    }
    if (height)
    {
        *height = window->wl.height;
    }
}

void _grwlSetWindowSizeWayland(_GRWLwindow* window, int width, int height)
{
    if (window->monitor)
    {
        // Video mode setting is not available on Wayland
    }
    else
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeWindow(window);

        if (window->wl.libdecor.frame)
        {
            struct libdecor_state* frameState = libdecor_state_new(width, height);
            libdecor_frame_commit(window->wl.libdecor.frame, frameState, nullptr);
            libdecor_state_free(frameState);
        }

        if (window->wl.visible)
        {
            _grwlInputWindowDamage(window);
        }
    }
}

void _grwlSetWindowSizeLimitsWayland(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight)
{
    if (window->wl.libdecor.frame)
    {
        if (minwidth == GRWL_DONT_CARE || minheight == GRWL_DONT_CARE)
        {
            minwidth = minheight = 0;
        }

        if (maxwidth == GRWL_DONT_CARE || maxheight == GRWL_DONT_CARE)
        {
            maxwidth = maxheight = 0;
        }

        libdecor_frame_set_min_content_size(window->wl.libdecor.frame, minwidth, minheight);
        libdecor_frame_set_max_content_size(window->wl.libdecor.frame, maxwidth, maxheight);
    }
    else if (window->wl.xdg.toplevel)
    {
        if (minwidth == GRWL_DONT_CARE || minheight == GRWL_DONT_CARE)
        {
            minwidth = minheight = 0;
        }
        else
        {
            if (window->wl.decorations.top.surface)
            {
                minwidth += GRWL_BORDER_SIZE * 2;
                minheight += GRWL_CAPTION_HEIGHT + GRWL_BORDER_SIZE;
            }
        }

        if (maxwidth == GRWL_DONT_CARE || maxheight == GRWL_DONT_CARE)
        {
            maxwidth = maxheight = 0;
        }
        else
        {
            if (window->wl.decorations.top.surface)
            {
                maxwidth += GRWL_BORDER_SIZE * 2;
                maxheight += GRWL_CAPTION_HEIGHT + GRWL_BORDER_SIZE;
            }
        }

        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
        wl_surface_commit(window->wl.surface);
    }
}

void _grwlSetWindowAspectRatioWayland(_GRWLwindow* window, int numer, int denom)
{
    if (window->wl.maximized || window->wl.fullscreen)
    {
        return;
    }

    int width = window->wl.width, height = window->wl.height;

    if (numer != GRWL_DONT_CARE && denom != GRWL_DONT_CARE)
    {
        const float aspectRatio = (float)width / (float)height;
        const float targetRatio = (float)numer / (float)denom;
        if (aspectRatio < targetRatio)
        {
            height /= targetRatio;
        }
        else if (aspectRatio > targetRatio)
        {
            width *= targetRatio;
        }
    }

    if (width != window->wl.width || height != window->wl.height)
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeWindow(window);

        if (window->wl.libdecor.frame)
        {
            struct libdecor_state* frameState = libdecor_state_new(width, height);
            libdecor_frame_commit(window->wl.libdecor.frame, frameState, nullptr);
            libdecor_state_free(frameState);
        }

        _grwlInputWindowSize(window, width, height);

        if (window->wl.visible)
        {
            _grwlInputWindowDamage(window);
        }
    }
}

void _grwlGetFramebufferSizeWayland(_GRWLwindow* window, int* width, int* height)
{
    _grwlGetWindowSizeWayland(window, width, height);
    if (width)
    {
        *width *= window->wl.contentScale;
    }
    if (height)
    {
        *height *= window->wl.contentScale;
    }
}

void _grwlGetWindowFrameSizeWayland(_GRWLwindow* window, int* left, int* top, int* right, int* bottom)
{
    if (window->wl.decorations.top.surface)
    {
        if (top)
        {
            *top = GRWL_CAPTION_HEIGHT;
        }
        if (left)
        {
            *left = GRWL_BORDER_SIZE;
        }
        if (right)
        {
            *right = GRWL_BORDER_SIZE;
        }
        if (bottom)
        {
            *bottom = GRWL_BORDER_SIZE;
        }
    }
}

void _grwlGetWindowContentScaleWayland(_GRWLwindow* window, float* xscale, float* yscale)
{
    if (xscale)
    {
        *xscale = (float)window->wl.contentScale;
    }
    if (yscale)
    {
        *yscale = (float)window->wl.contentScale;
    }
}

void _grwlIconifyWindowWayland(_GRWLwindow* window)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_minimized(window->wl.libdecor.frame);
    }
    else if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
    }
}

void _grwlRestoreWindowWayland(_GRWLwindow* window)
{
    if (window->monitor)
    {
        // There is no way to unset minimized, or even to know if we are
        // minimized, so there is nothing to do in this case.
    }
    else
    {
        // We assume we are not minimized and act only on maximization

        if (window->wl.maximized)
        {
            if (window->wl.libdecor.frame)
            {
                libdecor_frame_unset_maximized(window->wl.libdecor.frame);
            }
            else if (window->wl.xdg.toplevel)
            {
                xdg_toplevel_unset_maximized(window->wl.xdg.toplevel);
            }
            else
            {
                window->wl.maximized = false;
            }
        }
    }
}

void _grwlMaximizeWindowWayland(_GRWLwindow* window)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_maximized(window->wl.libdecor.frame);
    }
    else if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
    }
    else
    {
        window->wl.maximized = true;
    }
}

void _grwlShowWindowWayland(_GRWLwindow* window)
{
    if (!window->wl.libdecor.frame && !window->wl.xdg.toplevel)
    {
        // NOTE: The XDG surface and role are created here so command-line applications
        //       with off-screen windows do not appear in for example the Unity dock
        createShellObjects(window);
    }
}

void _grwlHideWindowWayland(_GRWLwindow* window)
{
    if (window->wl.visible)
    {
        window->wl.visible = false;
        destroyShellObjects(window);

        wl_surface_attach(window->wl.surface, nullptr, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

static void xdgActivationHandleDone(void* userData, struct xdg_activation_token_v1* activationToken, const char* token)
{
    _GRWLwindow* window = userData;

    if (activationToken != window->wl.activationToken)
    {
        return;
    }

    xdg_activation_v1_activate(_grwl.wl.activationManager, token, window->wl.surface);
    xdg_activation_token_v1_destroy(window->wl.activationToken);
    window->wl.activationToken = nullptr;
}

static const struct xdg_activation_token_v1_listener xdgActivationListener = { xdgActivationHandleDone };

void _grwlRequestWindowAttentionWayland(_GRWLwindow* window)
{
    if (!_grwl.wl.activationManager)
    {
        return;
    }

    // We're about to overwrite this with a new request
    if (window->wl.activationToken)
    {
        xdg_activation_token_v1_destroy(window->wl.activationToken);
    }

    window->wl.activationToken = xdg_activation_v1_get_activation_token(_grwl.wl.activationManager);
    xdg_activation_token_v1_add_listener(window->wl.activationToken, &xdgActivationListener, window);

    xdg_activation_token_v1_commit(window->wl.activationToken);
}

void _grwlFocusWindowWayland(_GRWLwindow* window)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The platform does not support setting the input focus");
}

void _grwlSetWindowMonitorWayland(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                                  int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
        {
            _grwlSetWindowSizeWayland(window, width, height);
        }

        return;
    }

    if (window->monitor)
    {
        releaseMonitor(window);
    }

    _grwlInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        acquireMonitor(window);
    }
    else
    {
        _grwlSetWindowSizeWayland(window, width, height);
    }
}

bool _grwlWindowFocusedWayland(_GRWLwindow* window)
{
    return _grwl.wl.keyboardFocus == window;
}

bool _grwlWindowIconifiedWayland(_GRWLwindow* window)
{
    // xdg-shell doesn’t give any way to request whether a surface is
    // iconified.
    return false;
}

bool _grwlWindowVisibleWayland(_GRWLwindow* window)
{
    return window->wl.visible;
}

bool _grwlWindowMaximizedWayland(_GRWLwindow* window)
{
    return window->wl.maximized;
}

bool _grwlWindowHoveredWayland(_GRWLwindow* window)
{
    return window->wl.hovered;
}

bool _grwlFramebufferTransparentWayland(_GRWLwindow* window)
{
    return window->wl.transparent;
}

void _grwlSetWindowResizableWayland(_GRWLwindow* window, bool enabled)
{
    if (window->wl.libdecor.frame)
    {
        if (enabled)
        {
            libdecor_frame_set_capabilities(window->wl.libdecor.frame, LIBDECOR_ACTION_RESIZE);
        }
        else
        {
            libdecor_frame_unset_capabilities(window->wl.libdecor.frame, LIBDECOR_ACTION_RESIZE);
        }
    }
    else
    {
        // TODO
        _grwlInputError(GRWL_FEATURE_UNIMPLEMENTED, "Wayland: Window attribute setting not implemented yet");
    }
}

void _grwlSetWindowDecoratedWayland(_GRWLwindow* window, bool enabled)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_visibility(window->wl.libdecor.frame, enabled);
    }
    else if (window->wl.xdg.decoration)
    {
        uint32_t mode;

        if (enabled)
        {
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        }
        else
        {
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
        }

        zxdg_toplevel_decoration_v1_set_mode(window->wl.xdg.decoration, mode);
    }
    else if (window->wl.xdg.toplevel)
    {
        if (enabled)
        {
            createFallbackDecorations(window);
        }
        else
        {
            destroyFallbackDecorations(window);
        }
    }
}

void _grwlSetWindowFloatingWayland(_GRWLwindow* window, bool enabled)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: Platform does not support making a window floating");
}

void _grwlSetWindowMousePassthroughWayland(_GRWLwindow* window, bool enabled)
{
    if (enabled)
    {
        struct wl_region* region = wl_compositor_create_region(_grwl.wl.compositor);
        wl_surface_set_input_region(window->wl.surface, region);
        wl_region_destroy(region);
    }
    else
    {
        wl_surface_set_input_region(window->wl.surface, 0);
    }
}

float _grwlGetWindowOpacityWayland(_GRWLwindow* window)
{
    return 1.f;
}

void _grwlSetWindowOpacityWayland(_GRWLwindow* window, float opacity)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The platform does not support setting the window opacity");
}

void _grwlSetRawMouseMotionWayland(_GRWLwindow* window, bool enabled)
{
    // This is handled in relativePointerHandleRelativeMotion
}

bool _grwlRawMouseMotionSupportedWayland()
{
    return true;
}

void _grwlPollEventsWayland()
{
    double timeout = 0.0;
    handleEvents(&timeout);
}

void _grwlWaitEventsWayland()
{
    handleEvents(nullptr);
}

void _grwlWaitEventsTimeoutWayland(double timeout)
{
    handleEvents(&timeout);
}

void _grwlPostEmptyEventWayland()
{
    wl_display_sync(_grwl.wl.display);
    flushDisplay();
}

void _grwlGetCursorPosWayland(_GRWLwindow* window, double* xpos, double* ypos)
{
    if (xpos)
    {
        *xpos = window->wl.cursorPosX;
    }
    if (ypos)
    {
        *ypos = window->wl.cursorPosY;
    }
}

void _grwlSetCursorPosWayland(_GRWLwindow* window, double x, double y)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The platform does not support setting the cursor position");
}

void _grwlSetCursorModeWayland(_GRWLwindow* window, int mode)
{
    _grwlSetCursorWayland(window, window->wl.currentCursor);
}

const char* _grwlGetScancodeNameWayland(int scancode)
{
    if (scancode < 0 || scancode > 255 || _grwl.wl.keycodes[scancode] == GRWL_KEY_UNKNOWN)
    {
        _grwlInputError(GRWL_INVALID_VALUE, "Wayland: Invalid scancode %i", scancode);
        return nullptr;
    }

    const int key = _grwl.wl.keycodes[scancode];
    const xkb_keycode_t keycode = scancode + 8;
    const xkb_layout_index_t layout = xkb_state_key_get_layout(_grwl.wl.xkb.state, keycode);
    if (layout == XKB_LAYOUT_INVALID)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to retrieve layout for key name");
        return nullptr;
    }

    const xkb_keysym_t* keysyms = nullptr;
    xkb_keymap_key_get_syms_by_level(_grwl.wl.xkb.keymap, keycode, layout, 0, &keysyms);
    if (keysyms == nullptr)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to retrieve keysym for key name");
        return nullptr;
    }

    const uint32_t codepoint = _grwlKeySym2Unicode(keysyms[0]);
    if (codepoint == GRWL_INVALID_CODEPOINT)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to retrieve codepoint for key name");
        return nullptr;
    }

    const size_t count = _grwlEncodeUTF8(_grwl.wl.keynames[key], codepoint);
    if (count == 0)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to encode codepoint for key name");
        return nullptr;
    }

    _grwl.wl.keynames[key][count] = '\0';
    return _grwl.wl.keynames[key];
}

int _grwlGetKeyScancodeWayland(int key)
{
    return _grwl.wl.scancodes[key];
}

const char* _grwlGetKeyboardLayoutNameWayland()
{
    if (!_grwl.wl.xkb.keymap)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Keymap missing");
        return nullptr;
    }

    const char* name = xkb_keymap_layout_get_name(_grwl.wl.xkb.keymap, _grwl.wl.xkb.group);
    if (!name)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to query keyboard layout name");
        return nullptr;
    }

    free(_grwl.wl.keyboardLayoutName);
    _grwl.wl.keyboardLayoutName = _grwl_strdup(name);
    return _grwl.wl.keyboardLayoutName;
}

bool _grwlCreateCursorWayland(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot)
{
    cursor->wl.buffer = createShmBuffer(image);
    if (!cursor->wl.buffer)
    {
        return false;
    }

    cursor->wl.width = image->width;
    cursor->wl.height = image->height;
    cursor->wl.xhot = xhot;
    cursor->wl.yhot = yhot;
    return true;
}

bool _grwlCreateStandardCursorWayland(_GRWLcursor* cursor, int shape)
{
    const char* name = nullptr;

    if (!_grwl.wl.cursorTheme)
    {
        _grwlInputError(GRWL_CURSOR_UNAVAILABLE, "Wayland: No cursor theme");
        return false;
    }

    // Try the XDG names first
    switch (shape)
    {
        case GRWL_ARROW_CURSOR:
            name = "default";
            break;
        case GRWL_IBEAM_CURSOR:
            name = "text";
            break;
        case GRWL_CROSSHAIR_CURSOR:
            name = "crosshair";
            break;
        case GRWL_POINTING_HAND_CURSOR:
            name = "pointer";
            break;
        case GRWL_RESIZE_EW_CURSOR:
            name = "ew-resize";
            break;
        case GRWL_RESIZE_NS_CURSOR:
            name = "ns-resize";
            break;
        case GRWL_RESIZE_NWSE_CURSOR:
            name = "nwse-resize";
            break;
        case GRWL_RESIZE_NESW_CURSOR:
            name = "nesw-resize";
            break;
        case GRWL_RESIZE_ALL_CURSOR:
            name = "all-scroll";
            break;
        case GRWL_NOT_ALLOWED_CURSOR:
            name = "not-allowed";
            break;
    }

    cursor->wl.cursor = wl_cursor_theme_get_cursor(_grwl.wl.cursorTheme, name);

    if (_grwl.wl.cursorThemeHiDPI)
    {
        cursor->wl.cursorHiDPI = wl_cursor_theme_get_cursor(_grwl.wl.cursorThemeHiDPI, name);
    }

    if (!cursor->wl.cursor)
    {
        // Fall back to the core X11 names
        switch (shape)
        {
            case GRWL_ARROW_CURSOR:
                name = "left_ptr";
                break;
            case GRWL_IBEAM_CURSOR:
                name = "xterm";
                break;
            case GRWL_CROSSHAIR_CURSOR:
                name = "crosshair";
                break;
            case GRWL_POINTING_HAND_CURSOR:
                name = "hand2";
                break;
            case GRWL_RESIZE_EW_CURSOR:
                name = "sb_h_double_arrow";
                break;
            case GRWL_RESIZE_NS_CURSOR:
                name = "sb_v_double_arrow";
                break;
            case GRWL_RESIZE_ALL_CURSOR:
                name = "fleur";
                break;
            default:
                _grwlInputError(GRWL_CURSOR_UNAVAILABLE, "Wayland: Standard cursor shape unavailable");
                return false;
        }

        cursor->wl.cursor = wl_cursor_theme_get_cursor(_grwl.wl.cursorTheme, name);
        if (!cursor->wl.cursor)
        {
            _grwlInputError(GRWL_CURSOR_UNAVAILABLE, "Wayland: Failed to create standard cursor \"%s\"", name);
            return false;
        }

        if (_grwl.wl.cursorThemeHiDPI)
        {
            if (!cursor->wl.cursorHiDPI)
            {
                cursor->wl.cursorHiDPI = wl_cursor_theme_get_cursor(_grwl.wl.cursorThemeHiDPI, name);
            }
        }
    }

    return true;
}

void _grwlDestroyCursorWayland(_GRWLcursor* cursor)
{
    // If it's a standard cursor we don't need to do anything here
    if (cursor->wl.cursor)
    {
        return;
    }

    if (cursor->wl.buffer)
    {
        wl_buffer_destroy(cursor->wl.buffer);
    }
}

static void relativePointerHandleRelativeMotion(void* userData, struct zwp_relative_pointer_v1* pointer,
                                                uint32_t timeHi, uint32_t timeLo, wl_fixed_t dx, wl_fixed_t dy,
                                                wl_fixed_t dxUnaccel, wl_fixed_t dyUnaccel)
{
    _GRWLwindow* window = userData;
    double xpos = window->virtualCursorPosX;
    double ypos = window->virtualCursorPosY;

    if (window->cursorMode != GRWL_CURSOR_DISABLED)
    {
        return;
    }

    if (window->rawMouseMotion)
    {
        xpos += wl_fixed_to_double(dxUnaccel);
        ypos += wl_fixed_to_double(dyUnaccel);
    }
    else
    {
        xpos += wl_fixed_to_double(dx);
        ypos += wl_fixed_to_double(dy);
    }

    _grwlInputCursorPos(window, xpos, ypos);
}

static const struct zwp_relative_pointer_v1_listener relativePointerListener = { relativePointerHandleRelativeMotion };

static void lockedPointerHandleLocked(void* userData, struct zwp_locked_pointer_v1* lockedPointer)
{
}

static void lockedPointerHandleUnlocked(void* userData, struct zwp_locked_pointer_v1* lockedPointer)
{
}

static const struct zwp_locked_pointer_v1_listener lockedPointerListener = { lockedPointerHandleLocked,
                                                                             lockedPointerHandleUnlocked };

static void lockPointer(_GRWLwindow* window)
{
    if (!_grwl.wl.relativePointerManager)
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Wayland: The compositor does not support pointer locking");
        return;
    }

    window->wl.relativePointer =
        zwp_relative_pointer_manager_v1_get_relative_pointer(_grwl.wl.relativePointerManager, _grwl.wl.pointer);
    zwp_relative_pointer_v1_add_listener(window->wl.relativePointer, &relativePointerListener, window);

    window->wl.lockedPointer = zwp_pointer_constraints_v1_lock_pointer(_grwl.wl.pointerConstraints, window->wl.surface,
                                                                       _grwl.wl.pointer, nullptr,
                                                                       ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    zwp_locked_pointer_v1_add_listener(window->wl.lockedPointer, &lockedPointerListener, window);
}

static void unlockPointer(_GRWLwindow* window)
{
    zwp_relative_pointer_v1_destroy(window->wl.relativePointer);
    window->wl.relativePointer = nullptr;

    zwp_locked_pointer_v1_destroy(window->wl.lockedPointer);
    window->wl.lockedPointer = nullptr;
}

static void confinedPointerHandleConfined(void* userData, struct zwp_confined_pointer_v1* confinedPointer)
{
}

static void confinedPointerHandleUnconfined(void* userData, struct zwp_confined_pointer_v1* confinedPointer)
{
}

static const struct zwp_confined_pointer_v1_listener confinedPointerListener = { confinedPointerHandleConfined,
                                                                                 confinedPointerHandleUnconfined };

static void confinePointer(_GRWLwindow* window)
{
    window->wl.confinedPointer =
        zwp_pointer_constraints_v1_confine_pointer(_grwl.wl.pointerConstraints, window->wl.surface, _grwl.wl.pointer,
                                                   nullptr, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

    zwp_confined_pointer_v1_add_listener(window->wl.confinedPointer, &confinedPointerListener, window);
}

static void unconfinePointer(_GRWLwindow* window)
{
    zwp_confined_pointer_v1_destroy(window->wl.confinedPointer);
    window->wl.confinedPointer = nullptr;
}

void _grwlSetCursorWayland(_GRWLwindow* window, _GRWLcursor* cursor)
{
    if (!_grwl.wl.pointer)
    {
        return;
    }

    window->wl.currentCursor = cursor;

    // If we're not in the correct window just save the cursor
    // the next time the pointer enters the window the cursor will change
    if (window != _grwl.wl.pointerFocus || window->wl.decorations.focus != GRWL_MAIN_WINDOW)
    {
        return;
    }

    // Update pointer lock to match cursor mode
    if (window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        if (window->wl.confinedPointer)
        {
            unconfinePointer(window);
        }
        if (!window->wl.lockedPointer)
        {
            lockPointer(window);
        }
    }
    else if (window->cursorMode == GRWL_CURSOR_CAPTURED)
    {
        if (window->wl.lockedPointer)
        {
            unlockPointer(window);
        }
        if (!window->wl.confinedPointer)
        {
            confinePointer(window);
        }
    }
    else if (window->cursorMode == GRWL_CURSOR_NORMAL || window->cursorMode == GRWL_CURSOR_HIDDEN)
    {
        if (window->wl.lockedPointer)
        {
            unlockPointer(window);
        }
        else if (window->wl.confinedPointer)
        {
            unconfinePointer(window);
        }
    }

    if (window->cursorMode == GRWL_CURSOR_NORMAL || window->cursorMode == GRWL_CURSOR_CAPTURED)
    {
        if (cursor)
        {
            setCursorImage(window, &cursor->wl);
        }
        else
        {
            struct wl_cursor* defaultCursor = wl_cursor_theme_get_cursor(_grwl.wl.cursorTheme, "left_ptr");
            if (!defaultCursor)
            {
                _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Standard cursor not found");
                return;
            }

            struct wl_cursor* defaultCursorHiDPI = nullptr;
            if (_grwl.wl.cursorThemeHiDPI)
            {
                defaultCursorHiDPI = wl_cursor_theme_get_cursor(_grwl.wl.cursorThemeHiDPI, "left_ptr");
            }

            _GRWLcursorWayland cursorWayland = { defaultCursor, defaultCursorHiDPI, nullptr, 0, 0, 0, 0, 0 };

            setCursorImage(window, &cursorWayland);
        }
    }
    else if (window->cursorMode == GRWL_CURSOR_HIDDEN || window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        wl_pointer_set_cursor(_grwl.wl.pointer, _grwl.wl.pointerEnterSerial, nullptr, 0, 0);
    }
}

static void dataSourceHandleTarget(void* userData, struct wl_data_source* source, const char* mimeType)
{
    if (_grwl.wl.selectionSource != source)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Unknown clipboard data source");
        return;
    }
}

static void dataSourceHandleSend(void* userData, struct wl_data_source* source, const char* mimeType, int fd)
{
    // Ignore it if this is an outdated or invalid request
    if (_grwl.wl.selectionSource != source || strcmp(mimeType, "text/plain;charset=utf-8") != 0)
    {
        close(fd);
        return;
    }

    char* string = _grwl.wl.clipboardString;
    size_t length = strlen(string);

    while (length > 0)
    {
        const ssize_t result = write(fd, string, length);
        if (result == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Error while writing the clipboard: %s", strerror(errno));
            break;
        }

        length -= result;
        string += result;
    }

    close(fd);
}

static void dataSourceHandleCancelled(void* userData, struct wl_data_source* source)
{
    wl_data_source_destroy(source);

    if (_grwl.wl.selectionSource != source)
    {
        return;
    }

    _grwl.wl.selectionSource = nullptr;
}

static const struct wl_data_source_listener dataSourceListener = {
    dataSourceHandleTarget,
    dataSourceHandleSend,
    dataSourceHandleCancelled,
};

void _grwlSetClipboardStringWayland(const char* string)
{
    if (_grwl.wl.selectionSource)
    {
        wl_data_source_destroy(_grwl.wl.selectionSource);
        _grwl.wl.selectionSource = nullptr;
    }

    char* copy = _grwl_strdup(string);
    if (!copy)
    {
        _grwlInputError(GRWL_OUT_OF_MEMORY, nullptr);
        return;
    }

    _grwl_free(_grwl.wl.clipboardString);
    _grwl.wl.clipboardString = copy;

    _grwl.wl.selectionSource = wl_data_device_manager_create_data_source(_grwl.wl.dataDeviceManager);
    if (!_grwl.wl.selectionSource)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create clipboard data source");
        return;
    }
    wl_data_source_add_listener(_grwl.wl.selectionSource, &dataSourceListener, nullptr);
    wl_data_source_offer(_grwl.wl.selectionSource, "text/plain;charset=utf-8");
    wl_data_device_set_selection(_grwl.wl.dataDevice, _grwl.wl.selectionSource, _grwl.wl.serial);
}

const char* _grwlGetClipboardStringWayland()
{
    if (!_grwl.wl.selectionOffer)
    {
        _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "Wayland: No clipboard data available");
        return nullptr;
    }

    if (_grwl.wl.selectionSource)
    {
        return _grwl.wl.clipboardString;
    }

    _grwl_free(_grwl.wl.clipboardString);
    _grwl.wl.clipboardString = readDataOfferAsString(_grwl.wl.selectionOffer, "text/plain;charset=utf-8");
    return _grwl.wl.clipboardString;
}

void _grwlUpdatePreeditCursorRectangleWayland(_GRWLwindow* window)
{
    _GRWLpreedit* preedit = &window->preedit;
    int x = preedit->cursorPosX;
    int y = preedit->cursorPosY;
    int w = preedit->cursorWidth;
    int h = preedit->cursorHeight;

    if (window->wl.textInputV3)
    {
        zwp_text_input_v3_set_cursor_rectangle(window->wl.textInputV3, x, y, w, h);
        zwp_text_input_v3_commit(window->wl.textInputV3);
    }
    else if (window->wl.textInputV1)
    {
        zwp_text_input_v1_set_cursor_rectangle(window->wl.textInputV1, x, y, w, h);
    }
}

void _grwlResetPreeditTextWayland(_GRWLwindow* window)
{
}

void _grwlSetIMEStatusWayland(_GRWLwindow* window, int active)
{
}

int _grwlGetIMEStatusWayland(_GRWLwindow* window)
{
    return false;
}

EGLenum _grwlGetEGLPlatformWayland(EGLint** attribs)
{
    if (_grwl.egl.EXT_platform_base && _grwl.egl.EXT_platform_wayland)
    {
        return EGL_PLATFORM_WAYLAND_EXT;
    }
    else
    {
        return 0;
    }
}

EGLNativeDisplayType _grwlGetEGLNativeDisplayWayland()
{
    return _grwl.wl.display;
}

EGLNativeWindowType _grwlGetEGLNativeWindowWayland(_GRWLwindow* window)
{
    return window->wl.egl.window;
}

void _grwlGetRequiredInstanceExtensionsWayland(char** extensions)
{
    if (!_grwl.vk.KHR_surface || !_grwl.vk.KHR_wayland_surface)
    {
        return;
    }

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_wayland_surface";
}

bool _grwlGetPhysicalDevicePresentationSupportWayland(VkInstance instance, VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkGetInstanceProcAddr(
            instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_NULL_HANDLE;
    }

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device, queuefamily, _grwl.wl.display);
}

VkResult _grwlCreateWindowSurfaceWayland(VkInstance instance, _GRWLwindow* window,
                                         const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    VkResult err;
    VkWaylandSurfaceCreateInfoKHR sci;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;

    vkCreateWaylandSurfaceKHR =
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
    if (!vkCreateWaylandSurfaceKHR)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = _grwl.wl.display;
    sci.surface = window->wl.surface;

    err = vkCreateWaylandSurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Wayland: Failed to create Vulkan surface: %s",
                        _grwlGetVulkanResultString(err));
    }

    return err;
}

_GRWLusercontext* _grwlCreateUserContextWayland(_GRWLwindow* window)
{
    if (window->context.egl.handle)
    {
        return _grwlCreateUserContextEGL(window);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI struct wl_display* grwlGetWaylandDisplay()
{
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    if (_grwl.platform.platformID != GRWL_PLATFORM_WAYLAND)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "Wayland: Platform not initialized");
        return nullptr;
    }

    return _grwl.wl.display;
}

GRWLAPI struct wl_surface* grwlGetWaylandWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(nullptr);

    if (_grwl.platform.platformID != GRWL_PLATFORM_WAYLAND)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "Wayland: Platform not initialized");
        return nullptr;
    }

    return window->wl.surface;
}

#endif // _GRWL_WAYLAND
