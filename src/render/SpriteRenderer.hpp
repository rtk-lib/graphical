#pragma once

#include "VulkanContext.hpp"
#include "../texture/textureManager.hpp"
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <string>

/*quad for display*/
#define QUAD 4
#define QUAD_MEMORY_SIZE sizeof(SpriteVertex)

#define INDICE_MEMORY sizeof(uint32_t)
#define INDICE_MEMORY_SIZE INDICE_MEMORY * 6

namespace rtk {

    /**
     * @brief Structure representing a single vertex of a 2D sprite.
     */
    struct SpriteVertex {
        glm::vec2 position;
        glm::vec2 uv;
        uint32_t textureId;

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
        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(SpriteVertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(SpriteVertex, uv);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[2].offset = offsetof(SpriteVertex, textureId);

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
     * @brief A 2D sprite renderer utilizing dynamic batching and bindless textures.
     */
    class SpriteRenderer {
    public:
        /**
         * @brief Constructs a new SpriteRenderer.
         * @param context The VulkanContext reference.
         * @param textureManager The TextureManager reference.
         */
        SpriteRenderer(const VulkanContext& context, const TextureManager& textureManager);

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
         * @brief Adds a sprite to the current rendering batch.
         * @param position The position of the sprite.
         * @param size The size of the sprite.
         * @param rotation The rotation of the sprite in degrees.
         * @param textureId The ID of the texture from the bindless array.
         */
        void drawSprite(const glm::vec2& position, const glm::vec2& size, float rotation, uint32_t textureId);

        /**
         * @brief Ends the current frame, flushing all batched sprites to the GPU and presenting.
         */
        void endFrame();

    private:
        const VulkanContext& _context;
        const TextureManager& _textureManager;

        VkRenderPass _renderPass = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> _swapChainFramebuffers;

        const size_t MAX_SPRITES = 10000;
        const size_t MAX_VERTICES = MAX_SPRITES * 4;
        const size_t MAX_INDICES = MAX_SPRITES * 6;

        VkBuffer _vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
        void* _mappedVertices = nullptr;

        VkBuffer _indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;
        void* _mappedIndices = nullptr;

        std::vector<SpriteVertex> _cpuVertices;
        std::vector<uint32_t> _cpuIndices;
        uint32_t _quadCount = 0;

        const int MAX_FRAMES_IN_FLIGHT = 2;
        std::vector<VkCommandBuffer> _commandBuffers;
        std::vector<VkSemaphore> _imageAvailableSemaphores;
        std::vector<VkSemaphore> _renderFinishedSemaphores;
        std::vector<VkFence> _inFlightFences;

        uint32_t _currentFrame = 0;
        uint32_t _imageIndex = 0;
        bool _isFrameStarted = false;

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
    };
}
