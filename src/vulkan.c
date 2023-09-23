//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define _GLFW_FIND_LOADER 1
#define _GLFW_REQUIRE_LOADER 2

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwInitVulkan(int mode)
{
    VkResult err;
    VkExtensionProperties* ep;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    uint32_t i, count;

    if (_glfw.vk.available)
    {
        return GLFW_TRUE;
    }

    if (_glfw.hints.init.vulkanLoader)
    {
        _glfw.vk.GetInstanceProcAddr = _glfw.hints.init.vulkanLoader;
    }
    else
    {
#if defined(_GLFW_VULKAN_LIBRARY)
        _glfw.vk.handle = _glfwPlatformLoadModule(_GLFW_VULKAN_LIBRARY);
#elif defined(_GLFW_WIN32)
        _glfw.vk.handle = _glfwPlatformLoadModule("vulkan-1.dll");
#elif defined(_GLFW_COCOA)
        _glfw.vk.handle = _glfwPlatformLoadModule("libvulkan.1.dylib");
        if (!_glfw.vk.handle)
        {
            _glfw.vk.handle = _glfwLoadLocalVulkanLoaderCocoa();
        }
#elif defined(__OpenBSD__) || defined(__NetBSD__)
        _glfw.vk.handle = _glfwPlatformLoadModule("libvulkan.so");
#else
        _glfw.vk.handle = _glfwPlatformLoadModule("libvulkan.so.1");
#endif
        if (!_glfw.vk.handle)
        {
            if (mode == _GLFW_REQUIRE_LOADER)
            {
                _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Loader not found");
            }

            return GLFW_FALSE;
        }

        _glfw.vk.GetInstanceProcAddr =
            (PFN_vkGetInstanceProcAddr)_glfwPlatformGetModuleSymbol(_glfw.vk.handle, "vkGetInstanceProcAddr");
        if (!_glfw.vk.GetInstanceProcAddr)
        {
            _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Loader does not export vkGetInstanceProcAddr");

            _glfwTerminateVulkan();
            return GLFW_FALSE;
        }
    }

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(
        NULL, "vkEnumerateInstanceExtensionProperties");
    if (!vkEnumerateInstanceExtensionProperties)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Failed to retrieve vkEnumerateInstanceExtensionProperties");

        _glfwTerminateVulkan();
        return GLFW_FALSE;
    }

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    if (err)
    {
        // NOTE: This happens on systems with a loader but without any Vulkan ICD
        if (mode == _GLFW_REQUIRE_LOADER)
        {
            _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Failed to query instance extension count: %s",
                            _glfwGetVulkanResultString(err));
        }

        _glfwTerminateVulkan();
        return GLFW_FALSE;
    }

    ep = _glfw_calloc(count, sizeof(VkExtensionProperties));

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, ep);
    if (err)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Failed to query instance extensions: %s",
                        _glfwGetVulkanResultString(err));

        _glfw_free(ep);
        _glfwTerminateVulkan();
        return GLFW_FALSE;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(ep[i].extensionName, "VK_KHR_surface") == 0)
        {
            _glfw.vk.KHR_surface = GLFW_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_win32_surface") == 0)
        {
            _glfw.vk.KHR_win32_surface = GLFW_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_MVK_macos_surface") == 0)
        {
            _glfw.vk.MVK_macos_surface = GLFW_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_EXT_metal_surface") == 0)
        {
            _glfw.vk.EXT_metal_surface = GLFW_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_xlib_surface") == 0)
        {
            _glfw.vk.KHR_xlib_surface = GLFW_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_xcb_surface") == 0)
        {
            _glfw.vk.KHR_xcb_surface = GLFW_TRUE;
        }
        else if (strcmp(ep[i].extensionName, "VK_KHR_wayland_surface") == 0)
        {
            _glfw.vk.KHR_wayland_surface = GLFW_TRUE;
        }
    }

    _glfw_free(ep);

    _glfw.vk.available = GLFW_TRUE;

    _glfw.platform.getRequiredInstanceExtensions(_glfw.vk.extensions);

    return GLFW_TRUE;
}

void _glfwTerminateVulkan(void)
{
    if (_glfw.vk.handle)
    {
        _glfwPlatformFreeModule(_glfw.vk.handle);
    }
}

const char* _glfwGetVulkanResultString(VkResult result)
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
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwVulkanSupported(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);
    return _glfwInitVulkan(_GLFW_FIND_LOADER);
}

GLFWAPI const char** glfwGetRequiredInstanceExtensions(uint32_t* count)
{
    assert(count != NULL);

    *count = 0;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_glfwInitVulkan(_GLFW_REQUIRE_LOADER))
    {
        return NULL;
    }

    if (!_glfw.vk.extensions[0])
    {
        return NULL;
    }

    *count = 2;
    return (const char**)_glfw.vk.extensions;
}

GLFWAPI GLFWvkproc glfwGetInstanceProcAddress(VkInstance instance, const char* procname)
{
    GLFWvkproc proc;
    assert(procname != NULL);

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_glfwInitVulkan(_GLFW_REQUIRE_LOADER))
    {
        return NULL;
    }

    // NOTE: Vulkan 1.0 and 1.1 vkGetInstanceProcAddr cannot return itself
    if (strcmp(procname, "vkGetInstanceProcAddr") == 0)
    {
        return (GLFWvkproc)vkGetInstanceProcAddr;
    }

    proc = (GLFWvkproc)vkGetInstanceProcAddr(instance, procname);
    if (!proc)
    {
        if (_glfw.vk.handle)
        {
            proc = (GLFWvkproc)_glfwPlatformGetModuleSymbol(_glfw.vk.handle, procname);
        }
    }

    return proc;
}

GLFWAPI int glfwGetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily)
{
    assert(instance != VK_NULL_HANDLE);
    assert(device != VK_NULL_HANDLE);

    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);

    if (!_glfwInitVulkan(_GLFW_REQUIRE_LOADER))
    {
        return GLFW_FALSE;
    }

    if (!_glfw.vk.extensions[0])
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Window surface creation extensions not found");
        return GLFW_FALSE;
    }

    return _glfw.platform.getPhysicalDevicePresentationSupport(instance, device, queuefamily);
}

GLFWAPI VkResult glfwCreateWindowSurface(VkInstance instance, GLFWwindow* handle,
                                         const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    _GLFWwindow* window = (_GLFWwindow*)handle;
    assert(instance != VK_NULL_HANDLE);
    assert(window != NULL);
    assert(surface != NULL);

    *surface = VK_NULL_HANDLE;

    _GLFW_REQUIRE_INIT_OR_RETURN(VK_ERROR_INITIALIZATION_FAILED);

    if (!_glfwInitVulkan(_GLFW_REQUIRE_LOADER))
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (!_glfw.vk.extensions[0])
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "Vulkan: Window surface creation extensions not found");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->context.client != GLFW_NO_API)
    {
        _glfwInputError(
            GLFW_INVALID_VALUE,
            "Vulkan: Window surface creation requires the window to have the client API set to GLFW_NO_API");
        return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;
    }

    return _glfw.platform.createWindowSurface(instance, window, allocator, surface);
}
