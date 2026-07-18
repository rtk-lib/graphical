#pragma once

#include "../window/window.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace rtk
{
    struct GBuffer {
        VkImage positionImage;
        VkDeviceMemory positionMemory;
        VkImageView positionView;

        VkImage normalImage;
        VkDeviceMemory normalMemory;
        VkImageView normalView;

        VkImage albedoImage;
        VkDeviceMemory albedoMemory;
        VkImageView albedoView;

        VkImage depthImage;
        VkDeviceMemory depthMemory;
        VkImageView depthView;
    };

    class Render
    {
        public:
            Render(Window& window);
            ~Render();
            void draw();

        private:
            void createInstance();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createSwapChain();
            void createRenderPass();
            void createGBuffer();

            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
            VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
            VkFormat findDepthFormat();

            Window& _window;

            VkInstance _instance;
            VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
            VkDevice _device;
            VkQueue _graphicsQueue;
            VkQueue _presentQueue;
            VkSurfaceKHR _surface;

            VkSwapchainKHR _swapChain;
            std::vector<VkImage> _swapChainImages;
            VkFormat _swapChainImageFormat;
            VkExtent2D _swapChainExtent;

            VkRenderPass _renderPass;
            GBuffer _gBuffer;
    };
}
