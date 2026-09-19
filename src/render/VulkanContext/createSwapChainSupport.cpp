#include "render/VulkanContext.hpp"

namespace rtk
{

    /**
     * @brief try to take the B8G8R8A8 Format SRGB
    */
    VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        return availableFormats[0];
    }

    /**
     * @brief check triple buffering, or classic V-Sync
    */
    VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    /**
     * @brief return the current resolution hardware of the screen
    */
    VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        else {
            VkExtent2D actualExtent = {_width, _height};
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    /**
     * @brief recover all the information for color, resolution, or Swap Mode
    */
    SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device) const {
        SwapChainSupportDetails details;
        checkVkR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities));

        uint32_t formatCount;
        checkVkR(vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr));
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            checkVkR(vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data()));
        }

        uint32_t presentModeCount;
        checkVkR(vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr));
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            checkVkR(vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data()));
        }
        return details;
    }

    void VulkanContext::createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = _surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain!");

        checkVkR(vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr));
        _swapChainImages.resize(imageCount);
        checkVkR(vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data()));

        _swapChainImageFormat = surfaceFormat.format;
        _swapChainExtent = extent;
    }


    void VulkanContext::cleanupSwapChain()
    {
        for (auto imageView : _swapChainImageViews)
            vkDestroyImageView(_device, imageView, nullptr);
        if (_swapChain)
            vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    }

    void VulkanContext::recreateSwapChain()
    {
        vkDeviceWaitIdle(_device);
        cleanupSwapChain();
        createSwapChain();
        createImageViews();
    }
}