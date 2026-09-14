#include "../VulkanContext.hpp"

namespace rtk
{
    VulkanContext::VulkanContext(Window& window) : _window(window), _width(window.getWindowSizeWidth()), _height(window.getWindowSizeHeight())
    {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createCommandPool();
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
