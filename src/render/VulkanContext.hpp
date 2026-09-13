#pragma once

#include "../window/window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <optional>
#include <string>

namespace rtk
{
    inline const char* vkResultToString(VkResult result)
    {
        switch (result) {
            case VK_SUCCESS:
                return "VK_SUCCESS";

            case VK_NOT_READY:
                return "VK_NOT_READY";

            case VK_TIMEOUT:
                return "VK_TIMEOUT";

            case VK_EVENT_SET:
                return "VK_EVENT_SET";

            case VK_EVENT_RESET:
                return "VK_EVENT_RESET";

            case VK_INCOMPLETE:
                return "VK_INCOMPLETE";

            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "VK_ERROR_OUT_OF_HOST_MEMORY";

            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "VK_ERROR_OUT_OF_DEVICE_MEMORY";

            case VK_ERROR_INITIALIZATION_FAILED:
                return "VK_ERROR_INITIALIZATION_FAILED";

            case VK_ERROR_DEVICE_LOST:
                return "VK_ERROR_DEVICE_LOST";

            case VK_ERROR_MEMORY_MAP_FAILED:
                return "VK_ERROR_MEMORY_MAP_FAILED";

            case VK_ERROR_LAYER_NOT_PRESENT:
                return "VK_ERROR_LAYER_NOT_PRESENT";

            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";

            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "VK_ERROR_FEATURE_NOT_PRESENT";

            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "VK_ERROR_INCOMPATIBLE_DRIVER";

            case VK_ERROR_TOO_MANY_OBJECTS:
                return "VK_ERROR_TOO_MANY_OBJECTS";

            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "VK_ERROR_FORMAT_NOT_SUPPORTED";

            case VK_ERROR_FRAGMENTED_POOL:
                return "VK_ERROR_FRAGMENTED_POOL";

            case VK_ERROR_OUT_OF_DATE_KHR:
                return "VK_ERROR_OUT_OF_DATE_KHR";

            case VK_SUBOPTIMAL_KHR:
                return "VK_SUBOPTIMAL_KHR";

            default:
                return "VK_RESULT_UNKNOWN";
        }
    }

    inline void checkVkR(VkResult result, const char *error = {"Vulkan error"})
    {
        if (result != VK_SUCCESS)
            throw std::runtime_error(std::string(error) + " failed with " + vkResultToString(result) + " (" + std::to_string(static_cast<int>(result)) + ")");
    }

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

        void recreateSwapChain();

    private:
        void cleanupSwapChain();

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
        bool isDeviceSuitable(VkPhysicalDevice device, bool useVirtualGpu);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        Window& _window;

        VkInstance _instance = VK_NULL_HANDLE;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device = VK_NULL_HANDLE;

        std::vector<VkPhysicalDevice> _virtualGpuPool = {};

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
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,

            #ifdef __APPLE__
            "VK_KHR_portability_subset"
            #endif
        };

        const uint32_t _width;
        const uint32_t _height;

#ifdef NDEBUG
        const bool _enableValidationLayers = false;
#else
        const bool _enableValidationLayers = true;
#endif
    };
}
