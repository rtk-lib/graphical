#pragma once

#include "render/VulkanContext.hpp"

#include "texture/texture.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

#include <stdexcept>


#define MISSING_TEXTURE_IDX 0

namespace rtk {

    class mistickError : public std::exception {};

    struct TextureData {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
    };

    class TextureManager {
    public:
        TextureManager(const VulkanContext& context);
        ~TextureManager();

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        rtk::Texture loadTexture(const std::string& filepath);
        Texture loadTextureFromMemory(std::span<const std::uint8_t> pixels, uint32_t width, uint32_t height);
        const TextureData& getTexture(uint32_t id) const;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return _descriptorSetLayout; }
        VkDescriptorSet getDescriptorSet() const { return _descriptorSet; }

    private:
        const VulkanContext& _context;

        std::unordered_map<std::string, uint32_t> _textureCache;
        std::vector<TextureData> _textures;

        VkSampler _textureSampler = VK_NULL_HANDLE;

        VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;

        const uint32_t MAX_BINDLESS_TEXTURES = 1000;

        TextureData createVulkanTexture(const std::string& filepath);
        void createSampler();
        void createDescriptorResources();
        void updateDescriptorSet(uint32_t index, VkImageView imageView);

        Texture missingTexture{0};
    };
}