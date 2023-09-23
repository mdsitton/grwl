//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#include <cassert>
#include <cstring>
#include <cstdlib>

#define _GRWL_FIND_LOADER 1
#define _GRWL_REQUIRE_LOADER 2

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

GRWLbool _grwlInitVulkan(int mode)
{
    VkResult err;
    VkExtensionProperties* ep;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    uint32_t i, count;

    if (_grwl.vk.available)
    {
        return GRWL_TRUE;
    }

    if (_grwl.hints.init.vulkanLoader)
    {
        _grwl.vk.GetInstanceProcAddr = _grwl.hints.init.vulkanLoader;
    }
    else
    {
#if defined(_GRWL_VULKAN_LIBRARY)
        _grwl.vk.handle = _grwlPlatformLoadModule(_GRWL_VULKAN_LIBRARY);
#elif defined(_GRWL_WIN32)
        _grwl.vk.handle = _grwlPlatformLoadModule("vulkan-1.dll");
#elif defined(_GRWL_COCOA)
        _grwl.vk.handle = _grwlPlatformLoadModule("libvulkan.1.dylib");
        if (!_grwl.vk.handle)
        {
            _grwl.vk.handle = _grwlLoadLocalVulkanLoaderCocoa();
        }
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        _grwl.vk.handle = _grwlPlatformLoadModule("libvulkan.so");
#else
        _grwl.vk.handle = _grwlPlatformLoadModule("libvulkan.so.1");
#endif
        if (!_grwl.vk.handle)
        {
            if (mode == _GRWL_REQUIRE_LOADER)
            {
                _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Loader not found");
            }

            return GRWL_FALSE;
        }

        _grwl.vk.GetInstanceProcAddr =
            (PFN_vkGetInstanceProcAddr)_grwlPlatformGetModuleSymbol(_grwl.vk.handle, "vkGetInstanceProcAddr");
        if (!_grwl.vk.GetInstanceProcAddr)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Loader does not export vkGetInstanceProcAddr");

            _grwlTerminateVulkan();
            return GRWL_FALSE;
        }
    }

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(
        NULL, "vkEnumerateInstanceExtensionProperties");
    if (!vkEnumerateInstanceExtensionProperties)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Failed to retrieve vkEnumerateInstanceExtensionProperties");

        _grwlTerminateVulkan();
        return GRWL_FALSE;
    }

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    if (err)
    {
        // NOTE: This happens on systems with a loader but without any Vulkan ICD
        if (mode == _GRWL_REQUIRE_LOADER)
        {
            _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Failed to query instance extension count: %s",
                            _grwlGetVulkanResultString(err));
        }

        _grwlTerminateVulkan();
        return GRWL_FALSE;
    }

    ep = (VkExtensionProperties*)_grwl_calloc(count, sizeof(VkExtensionProperties));

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, ep);
    if (err)
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Failed to query instance extensions: %s",
                        _grwlGetVulkanResultString(err));

        _grwl_free(ep);
        _grwlTerminateVulkan();
        return GRWL_FALSE;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(ep[i].extensionName, "VK_KHR_surface") == 0)
        {
            _grwl.vk.KHR_surface = GRWL_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_win32_surface") == 0)
        {
            _grwl.vk.KHR_win32_surface = GRWL_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_MVK_macos_surface") == 0)
        {
            _grwl.vk.MVK_macos_surface = GRWL_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_EXT_metal_surface") == 0)
        {
            _grwl.vk.EXT_metal_surface = GRWL_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_xlib_surface") == 0)
        {
            _grwl.vk.KHR_xlib_surface = GRWL_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_xcb_surface") == 0)
        {
            _grwl.vk.KHR_xcb_surface = GRWL_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_wayland_surface") == 0)
        {
            _grwl.vk.KHR_wayland_surface = GRWL_TRUE;
        }
    }

    _grwl_free(ep);

    _grwl.vk.available = GRWL_TRUE;

    _grwl.platform.getRequiredInstanceExtensions(_grwl.vk.extensions);

    return GRWL_TRUE;
}

void _grwlTerminateVulkan(void)
{
    if (_grwl.vk.handle)
    {
        _grwlPlatformFreeModule(_grwl.vk.handle);
    }
}

const char* _grwlGetVulkanResultString(VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS:
            return "Success";
        case VK_NOT_READY:
            return "A fence or query has not yet completed";
        case VK_TIMEOUT:
            return "A wait operation has not completed in the specified time";
        case VK_EVENT_SET:
            return "An event is signaled";
        case VK_EVENT_RESET:
            return "An event is unsignaled";
        case VK_INCOMPLETE:
            return "A return array was too small for the result";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST:
            return "The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "A requested format is not supported on this device";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "A surface is no longer available";
        case VK_SUBOPTIMAL_KHR:
            return "A swapchain no longer matches the surface properties exactly, but can still be used";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "A surface has changed in such a way that it is no longer compatible with the swapchain";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "The display used by a swapchain does not use the same presentable image layout";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "A validation layer found an error";
        default:
            return "ERROR: UNKNOWN VULKAN ERROR";
    }
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL public API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI int grwlVulkanSupported(void)
{
    _GRWL_REQUIRE_INIT_OR_RETURN(GRWL_FALSE);
    return _grwlInitVulkan(_GRWL_FIND_LOADER);
}

GRWLAPI const char** grwlGetRequiredInstanceExtensions(uint32_t* count)
{
    assert(count != NULL);

    *count = 0;

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_grwlInitVulkan(_GRWL_REQUIRE_LOADER))
    {
        return NULL;
    }

    if (!_grwl.vk.extensions[0])
    {
        return NULL;
    }

    *count = 2;
    return (const char**)_grwl.vk.extensions;
}

GRWLAPI GRWLvkproc grwlGetInstanceProcAddress(VkInstance instance, const char* procname)
{
    GRWLvkproc proc;
    assert(procname != NULL);

    _GRWL_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_grwlInitVulkan(_GRWL_REQUIRE_LOADER))
    {
        return NULL;
    }

    // NOTE: Vulkan 1.0 and 1.1 vkGetInstanceProcAddr cannot return itself
    if (strcmp(procname, "vkGetInstanceProcAddr") == 0)
    {
        return (GRWLvkproc)vkGetInstanceProcAddr;
    }

    proc = (GRWLvkproc)vkGetInstanceProcAddr(instance, procname);
    if (!proc)
    {
        if (_grwl.vk.handle)
        {
            proc = (GRWLvkproc)_grwlPlatformGetModuleSymbol(_grwl.vk.handle, procname);
        }
    }

    return proc;
}

GRWLAPI int grwlGetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily)
{
    assert(instance != VK_NULL_HANDLE);
    assert(device != VK_NULL_HANDLE);

    _GRWL_REQUIRE_INIT_OR_RETURN(GRWL_FALSE);

    if (!_grwlInitVulkan(_GRWL_REQUIRE_LOADER))
    {
        return GRWL_FALSE;
    }

    if (!_grwl.vk.extensions[0])
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Window surface creation extensions not found");
        return GRWL_FALSE;
    }

    return _grwl.platform.getPhysicalDevicePresentationSupport(instance, device, queuefamily);
}

GRWLAPI VkResult grwlCreateWindowSurface(VkInstance instance, GRWLwindow* handle,
                                         const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    assert(instance != VK_NULL_HANDLE);
    assert(window != NULL);
    assert(surface != NULL);

    *surface = VK_NULL_HANDLE;

    _GRWL_REQUIRE_INIT_OR_RETURN(VK_ERROR_INITIALIZATION_FAILED);

    if (!_grwlInitVulkan(_GRWL_REQUIRE_LOADER))
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (!_grwl.vk.extensions[0])
    {
        _grwlInputError(GRWL_API_UNAVAILABLE, "Vulkan: Window surface creation extensions not found");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->context.client != GRWL_NO_API)
    {
        _grwlInputError(
            GRWL_INVALID_VALUE,
            "Vulkan: Window surface creation requires the window to have the client API set to GRWL_NO_API");
        return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;
    }

    return _grwl.platform.createWindowSurface(instance, window, allocator, surface);
}
