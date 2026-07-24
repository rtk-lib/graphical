#pragma once

#include "../window/window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include <array>

namespace rtk
{
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }
    };

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

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool hasGraphics = false;
        bool hasPresent = false;
        bool isComplete() { return hasGraphics && hasPresent; }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    class Render
    {
        public:
            Render(Window& window);
            ~Render();
            void draw(RGB clearColor = {0, 0, 0});

        private:
            void createInstance();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createSwapChain();
            void createRenderPass();
            void createGBuffer();
            void createFramebuffers();
            void createGraphicsPipelines();
            void createCommandPool();
            void loadModel();
            void createVertexBuffer();
            void createIndexBuffer();
            void createUniformBuffers();
            void createDescriptorPool();
            void createDescriptorSets();
            void createDescriptorSetLayout();
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void updateUniformBuffer(uint32_t currentImage);
            void createCommandBuffers();
            void createSyncObjects();

            bool checkValidationLayerSupport();
            std::vector<const char*> getRequiredExtensions();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
            bool isDeviceSuitable(VkPhysicalDevice device);

            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
            VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
            VkFormat findDepthFormat();
            VkShaderModule createShaderModule(const std::vector<char>& code);

            Window& _window;

            VkInstance _instance = VK_NULL_HANDLE;
            VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
            VkDevice _device = VK_NULL_HANDLE;
            VkQueue _graphicsQueue = VK_NULL_HANDLE;
            VkQueue _presentQueue = VK_NULL_HANDLE;
            VkSurfaceKHR _surface = VK_NULL_HANDLE;

            VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
            std::vector<VkImage> _swapChainImages;
            std::vector<VkImageView> _swapChainImageViews;
            VkFormat _swapChainImageFormat;
            VkExtent2D _swapChainExtent;

            VkCommandPool _commandPool = VK_NULL_HANDLE;
            VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;

            VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE;
            VkSemaphore _renderFinishedSemaphore = VK_NULL_HANDLE;
            VkFence _inFlightFence = VK_NULL_HANDLE;

            VkRenderPass _renderPass = VK_NULL_HANDLE;
            
            VkPipelineLayout _geometryPipelineLayout = VK_NULL_HANDLE;
            VkPipeline _geometryPipeline = VK_NULL_HANDLE;

            GBuffer _gBuffer = {};
            std::vector<VkFramebuffer> _swapChainFramebuffers;

            std::vector<Vertex> _vertices;
            std::vector<uint32_t> _indices;
            VkBuffer _vertexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
            VkBuffer _indexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;

            std::vector<VkBuffer> _uniformBuffers;
            std::vector<VkDeviceMemory> _uniformBuffersMemory;
            std::vector<void*> _uniformBuffersMapped;

            VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
            VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
            std::vector<VkDescriptorSet> _descriptorSets;
            
            #ifdef NDEBUG
                const bool enableValidationLayers = false;
            #else
                const bool enableValidationLayers = true;
            #endif

            const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
            };
    };
}
