#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <wayland-client.h>

int main() {
    // Initialize Wayland connection
    wl_display* display = wl_display_connect(nullptr);
    if (!display) {
        std::cout << "Failed to connect to Wayland display" << std::endl;
        return 1;
    }

    // Initialize Wayland registry
    wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &wl_registry_listener, nullptr);
    wl_display_dispatch(display);

    // Create the Wayland window
    wl_compositor* compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, 1, &wl_compositor_interface, 1));
    wl_surface* surface = wl_compositor_create_surface(compositor);
    wl_shell_surface* shell_surface = wl_shell_get_shell_surface(static_cast<wl_shell*>(wl_registry_bind(registry, 3, &wl_shell_interface, 1)), surface);
    wl_shell_surface_set_toplevel(shell_surface);

    // Create the Vulkan instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Window";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    VkInstance instance;
    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
        std::cout << "Failed to create Vulkan instance" << std::endl;
        return 1;
    }

    // Create the Vulkan surface
    VkWaylandSurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.display = display;
    surfaceInfo.surface = surface;

    VkSurfaceKHR vk_surface;
    if (vkCreateWaylandSurfaceKHR(instance, &surfaceInfo, nullptr, &vk_surface) != VK_SUCCESS) {
        std::cout << "Failed to create Vulkan surface" << std::endl;
        return 1;
    }

    // Find a physical device and queue family that support the surface
    uint32_t queueFamilyIndex = UINT32_MAX;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> physicalDevices;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    physicalDevices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
    for (const auto& device : physicalDevices) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk_surface, &presentSupport);
                if (presentSupport) {
                    physicalDevice = device;
                    queueFamilyIndex = i;
                    break;
                }
            }
        }
        if (physicalDevice != VK_NULL_HANDLE) {
            break;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        std::cout << "Failed to find a suitable physical device" << std::endl;
        return 1;
    }

    // Create the logical device
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamilyIndex;
    queueInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    VkDevice device;
        if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
        std::cout << "Failed to create logical device" << std::endl;
        return 1;
    }

    // Get the queue
    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    // Create the swapchain
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vk_surface, &surfaceCapabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vk_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vk_surface, &formatCount, surfaceFormats.data());

    VkSurfaceFormatKHR surfaceFormat;
    if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    } else {
        for (const auto& format : surfaceFormats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = format;
                break;
            }
        }
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = vk_surface;
    swapchainInfo.minImageCount = surfaceCapabilities.minImageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 1;
    swapchainInfo.pQueueFamilyIndices = &queueFamilyIndex;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.clipped = VK_TRUE;
    
    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &sw