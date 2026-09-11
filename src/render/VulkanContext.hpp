#pragma once

#include "../window/window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <optional>

namespace rtk
{
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanContext
    {
    public:
        VulkanContext(Window& window);
        ~VulkanContext();

        VulkanContext(const VulkanContext&) = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;

        VkInstance getInstance() const { return _instance; }
        VkPhysicalDevice getPhysicalDevice() const { return _physicalDevice; }
        VkDevice getDevice() const { return _device; }
        VkQueue getGraphicsQueue() const { return _graphicsQueue; }
        VkQueue getPresentQueue() const { return _presentQueue; }
        VkSurfaceKHR getSurface() const { return _surface; }
        VkSwapchainKHR getSwapChain() const { return _swapChain; }
        const std::vector<VkImage>& getSwapChainImages() const { return _swapChainImages; }
        const std::vector<VkImageView>& getSwapChainImageViews() const { return _swapChainImageViews; }
        VkFormat getSwapChainImageFormat() const { return _swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() const { return _swapChainExtent; }
        VkCommandPool getCommandPool() const { return _commandPool; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        VkCommandBuffer beginSingleTimeCommands() const;
        void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    private:
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();
        void createCommandPool();

        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
        bool isDeviceSuitable(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        Window& _window;

        VkInstance _instance = VK_NULL_HANDLE;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device = VK_NULL_HANDLE;

        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _presentQueue = VK_NULL_HANDLE;

        VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> _swapChainImages;
        std::vector<VkImageView> _swapChainImageViews;
        VkFormat _swapChainImageFormat;
        VkExtent2D _swapChainExtent;

        VkCommandPool _commandPool = VK_NULL_HANDLE;

        const std::vector<const char*> _validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        const std::vector<const char*> _deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

#ifdef NDEBUG
        const bool _enableValidationLayers = false;
#else
        const bool _enableValidationLayers = true;
#endif
    };
}
