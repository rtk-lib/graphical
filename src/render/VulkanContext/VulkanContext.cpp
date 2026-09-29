#include "render/VulkanContext.hpp"

namespace rtk
{
    VulkanContext::VulkanContext(Window& window) : _window(window), _logicalExtent({window.getWindowSizeWidth(), window.getWindowSizeHeight()})
    {
        try {
            createInstance();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createSwapChain();
            createImageViews();
            createCommandPool();
        } catch (...) {
            if (_commandPool) vkDestroyCommandPool(_device, _commandPool, nullptr);
            cleanupSwapChain();
            if (_device) vkDestroyDevice(_device, nullptr);
            if (_surface) vkDestroySurfaceKHR(_instance, _surface, nullptr);
            if (_instance) vkDestroyInstance(_instance, nullptr);
            throw;
        }
    }

    VulkanContext::~VulkanContext()
    {
        if (_commandPool)
            vkDestroyCommandPool(_device, _commandPool, nullptr);

        cleanupSwapChain();

        if (_device)
            vkDestroyDevice(_device, nullptr);

        if (_surface)
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
        if (_instance)
            vkDestroyInstance(_instance, nullptr);
    }
}
