#pragma once

#include "VulkanContext.hpp"
#include "../texture/textureManager.hpp"

#include "../utils/vec2.hpp"

#include "../sprite/sprite.hpp"

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <string>
#include <limits>
#include <span>

/*quad for display*/
#define QUAD 4
#define QUAD_MEMORY_SIZE sizeof(SpriteVertex) * QUAD

#define INDICE_MEMORY sizeof(uint32_t)
#define INDICE_MEMORY_SIZE INDICE_MEMORY * 6


#define MAX_SPRITES 10000
#define MAX_VERTICES QUAD
#define MAX_INDICES 6

namespace rtk {


    struct FrameResources {
       VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

       VkSemaphore imageAvailable = VK_NULL_HANDLE;
       VkSemaphore renderFinished = VK_NULL_HANDLE;
       VkFence inFlightFence = VK_NULL_HANDLE;

       VkBuffer instanceBuffer = VK_NULL_HANDLE;
       VkDeviceMemory instanceMemory = VK_NULL_HANDLE;
       void* mappedInstances = nullptr;

       std::size_t instanceCount = 0;
       std::size_t instanceCapacity = 0;
       std::size_t uploadedBytes = 0;
       std::size_t reallocationCount = 0;
    };

    /**
     * @brief Structure representing a single vertex of a 2D sprite.
     */
    struct SpriteVertex {
        rtk::vec2 position;

        /**
         * @brief Gets the binding description for the vertex input.
         * @return The Vulkan binding description.
         */
        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(SpriteVertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }

        /**
         * @brief Gets the attribute descriptions for the vertex input.
         * @return An array of Vulkan attribute descriptions.
         */
        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(SpriteVertex, position);

            return attributeDescriptions;
        }
    };

    /**
     * @brief Push constants structure containing the projection-view matrix.
     */
    struct SpritePushConstants {
        glm::mat4 projectionView;
    };

    /**
     * @brief A 2D sprite renderer utilizing hardware instancing and bindless textures.
     */
    class SpriteRenderer {
    public:
        /**
         * @brief Constructs a new SpriteRenderer.
         * @param context The VulkanContext reference.
         * @param textureManager The TextureManager reference.
         */
        SpriteRenderer(VulkanContext& context, const TextureManager& textureManager);

        /**
         * @brief Destroys the SpriteRenderer and releases Vulkan resources.
         */
        ~SpriteRenderer();

        SpriteRenderer(const SpriteRenderer&) = delete;
        SpriteRenderer& operator=(const SpriteRenderer&) = delete;

        /**
         * @brief Begins a new rendering frame.
         * @param clearColor The background clear color (r, g, b). Defaults to black.
         */
        void beginFrame(const RGB& clearColor = {0, 0, 0});

        /**
         * @brief Adds a sprite to the current rendering batch from raw info.
         * @param position The position of the sprite.
         * @param size The size of the sprite.
         * @param rotation The rotation of the sprite in degrees.
         * @param textureId The ID of the texture from the bindless array.
         */
        void drawSprite(const rtk::vec2& position, const rtk::vec2& size, float rotation, const uint32_t textureId);


        /**
         * @brief Adds a sprite to the current rendering batch from sprite data.
         * @param sprite sprite data from rtk::sprite
         */
        void submit(const SpriteData& sprite);

        /**
         * @brief Adds an array of sprites to the current rendering batch, from an array of sprite data.
         * @param sprite sprite data from rtk::sprite
         */
        void submit(std::span<const SpriteData> sprites);


        /**
         * @brief Ends the current frame, flushing all batched sprites to the GPU and presenting.
         */
        void endFrame();

    private:
        VulkanContext& _context;
        const TextureManager& _textureManager;

        VkRenderPass _renderPass = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> _swapChainFramebuffers;

        VkBuffer _quadVertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _quadVertexBufferMemory = VK_NULL_HANDLE;

        VkBuffer _quadIndexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _quadIndexBufferMemory = VK_NULL_HANDLE;

        uint32_t _imageIndex = 0;
        bool _isFrameStarted = false;

        std::vector<rtk::SpriteData> _instances;

        VkBuffer _instanceBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _instanceBufferMemory = VK_NULL_HANDLE;
        void* _mappedInstanceData = nullptr;

        static constexpr std::size_t MaxFramesInFlight = 2;
        static constexpr std::size_t SpritesPerPage = 16'384;

        std::array<FrameResources, MaxFramesInFlight> _frames{};
        std::size_t _currentFrame = 0;

        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createBuffers();
        void createSyncObjects();
        void createCommandBuffers();
        void flush();
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        std::vector<char> readFile(const std::string& filename);

        void createInstanceBuffer(FrameResources& frame, std::size_t capacity);
        void ensureInstanceCapacity(FrameResources& frame, std::size_t requiredCapacity);

        void recreateSwapChain();
    };
}