#include "render/VulkanContext.hpp"

namespace rtk
{
    void VulkanContext::createSurface()
    {
        /*Create the surface into the window class*/
        _window.createSurface(_instance);

        /*Get the surface from the window*/
        _surface = (VkSurfaceKHR)_window.getSurface();

        if (!_surface)
            throw std::runtime_error("Failed to get Vulkan surface from Window!");
    }
}