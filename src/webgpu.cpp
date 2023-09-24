//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#if defined(_GRWL_BUILD_WEBGPU)

    #include <webgpu/webgpu.h>

    #define WGPU_TARGET_MACOS 1
    #define WGPU_TARGET_LINUX_X11 2
    #define WGPU_TARGET_WINDOWS 3
    #define WGPU_TARGET_LINUX_WAYLAND 4

    #if defined(_WIN32)
        #define WGPU_TARGET WGPU_TARGET_WINDOWS
    #elif defined(__APPLE__)
        #define WGPU_TARGET WGPU_TARGET_MACOS
    #elif defined(_GRWL_WAYLAND)
        #define WGPU_TARGET WGPU_TARGET_LINUX_WAYLAND
    #else
        #define WGPU_TARGET WGPU_TARGET_LINUX_X11
    #endif

    #if WGPU_TARGET == WGPU_TARGET_MACOS
        #include <Foundation/Foundation.h>
        #include <QuartzCore/CAMetalLayer.h>
    #endif

    #include <GRWL/grwl.h>
    #if WGPU_TARGET == WGPU_TARGET_MACOS
        #define GRWL_EXPOSE_NATIVE_COCOA
    #elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
        #define GRWL_EXPOSE_NATIVE_X11
    #elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
        #define GRWL_EXPOSE_NATIVE_WAYLAND
    #elif WGPU_TARGET == WGPU_TARGET_WINDOWS
        #define GRWL_EXPOSE_NATIVE_WIN32
    #endif
    #include <GRWL/grwlnative.h>

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI WGPUSurface grwlCreateWindowWGPUSurface(WGPUInstance instance, GRWLwindow* window)
{
    #if WGPU_TARGET == WGPU_TARGET_MACOS
    {
        id metal_layer = nullptr;
        NSWindow* ns_window = grwlGetCocoaWindow(window);
        [ns_window.contentView setWantsLayer:YES];
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer:metal_layer];
        return wgpuInstanceCreateSurface(instance,
                                         &(WGPUSurfaceDescriptor) {
                                             .label = nullptr,
                                             .nextInChain =
                                                 (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromMetalLayer) {
                                                     .chain =
                                                         (WGPUChainedStruct) {
                                                             .next = nullptr,
                                                             .sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
                                                         },
                                                     .layer = metal_layer,
                                                 },
                                         });
    }
    #elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
    {
        Display* x11_display = grwlGetX11Display();
        Window x11_window = grwlGetX11Window(window);
        return wgpuInstanceCreateSurface(instance,
                                         &(WGPUSurfaceDescriptor) {
                                             .label = nullptr,
                                             .nextInChain =
                                                 (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromXlibWindow) {
                                                     .chain =
                                                         (WGPUChainedStruct) {
                                                             .next = nullptr,
                                                             .sType = WGPUSType_SurfaceDescriptorFromXlibWindow,
                                                         },
                                                     .display = x11_display,
                                                     .window = x11_window,
                                                 },
                                         });
    }
    #elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
    {
        struct wl_display* wayland_display = grwlGetWaylandDisplay();
        struct wl_surface* wayland_surface = grwlGetWaylandWindow(window);
        return wgpuInstanceCreateSurface(instance,
                                         &(WGPUSurfaceDescriptor) {
                                             .label = nullptr,
                                             .nextInChain =
                                                 (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromWaylandSurface) {
                                                     .chain =
                                                         (WGPUChainedStruct) {
                                                             .next = nullptr,
                                                             .sType = WGPUSType_SurfaceDescriptorFromWaylandSurface,
                                                         },
                                                     .display = wayland_display,
                                                     .surface = wayland_surface,
                                                 },
                                         });
    }
    #elif WGPU_TARGET == WGPU_TARGET_WINDOWS
    {
        HWND hwnd = grwlGetWin32Window(window);
        HINSTANCE hinstance = GetModuleHandle(nullptr);
        return wgpuInstanceCreateSurface(instance,
                                         &(WGPUSurfaceDescriptor) {
                                             .label = nullptr,
                                             .nextInChain =
                                                 (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromWindowsHWND) {
                                                     .chain =
                                                         (WGPUChainedStruct) {
                                                             .next = nullptr,
                                                             .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
                                                         },
                                                     .hinstance = hinstance,
                                                     .hwnd = hwnd,
                                                 },
                                         });
    }
    #else
        #error "Unsupported WGPU_TARGET"
    #endif
}
#endif /* _GRWL_BUILD_WEBGPU */
