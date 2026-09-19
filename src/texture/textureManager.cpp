#include "texture/textureManager.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>

#include "stb_image.h"

namespace rtk {

    TextureManager::TextureManager(const VulkanContext& context)
        : _context(context)
    {
        createSampler();
        createDescriptorResources();

        loadTexture("assets/MissingTexture.png");
        const std::string missingPath =
        "assets/MissingTexture.png";

        TextureData data = createVulkanTexture(missingPath);

        _textures.push_back(data);
        _textureCache[missingPath] = MISSING_TEXTURE_IDX;
        updateDescriptorSet(MISSING_TEXTURE_IDX, data.view);

        missingTexture = Texture(MISSING_TEXTURE_IDX);
    }

    TextureManager::~TextureManager()
    {
        VkDevice device = _context.getDevice();

        for (const auto& tex : _textures) {
            vkDestroyImageView(device, tex.view, nullptr);
            vkDestroyImage(device, tex.image, nullptr);
            vkFreeMemory(device, tex.memory, nullptr);
        }

        if (_textureSampler)
            vkDestroySampler(device, _textureSampler, nullptr);

        if (_descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);

        if (_descriptorPool)
            vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
    }

    void TextureManager::createSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(_context.getDevice(), &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture sampler!");
    }

    void TextureManager::createDescriptorResources()
    {
        VkDevice device = _context.getDevice();

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = MAX_BINDLESS_TEXTURES;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create bindless descriptor pool!");

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = MAX_BINDLESS_TEXTURES;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

        VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
        extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        extendedInfo.bindingCount = 1;
        extendedInfo.pBindingFlags = &bindlessFlags;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = &extendedInfo;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &samplerLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create bindless descriptor set layout!");

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &_descriptorSetLayout;

        if (vkAllocateDescriptorSets(device, &allocInfo, &_descriptorSet) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate bindless descriptor set!");
    }

    rtk::Texture TextureManager::loadTexture(const std::string& filepath)
    {
        if (_textureCache.find(filepath) != _textureCache.end())
            return Texture(_textureCache[filepath]);

        uint32_t index = static_cast<uint32_t>(_textures.size());

        try {
            if (index >= MAX_BINDLESS_TEXTURES)
                throw std::runtime_error("Exceeded maximum number of bindless textures!");

            TextureData texData = createVulkanTexture(filepath);
            _textures.push_back(texData);
            _textureCache[filepath] = index;

            updateDescriptorSet(index, texData.view);
        }
        catch(const std::exception& e)
        {
            LOG_WARN(e.what());
            LOG_WARN("Missing texture will be load");
            return Texture(MISSING_TEXTURE_IDX);
        }

        return Texture(index);
    }

    const TextureData& TextureManager::getTexture(uint32_t id) const
    {
        return _textures.at(id);
    }

    TextureData TextureManager::createVulkanTexture(const std::string& filepath)
    {
        int texWidth, texHeight;
        VkDeviceSize imageSize;
        stbi_uc* pixels = nullptr;
        bool isSynthetic = false;

        if (filepath.empty() || filepath == "assets/MissingTexture.png") {
            texWidth = 2;
            texHeight = 2;
            imageSize = texWidth * texHeight * 4;
            pixels = new stbi_uc[imageSize];
            for (size_t i = 0; i < imageSize; i += 4) {
                pixels[i] = 255;
                pixels[i+1] = 0;
                pixels[i+2] = 255;
                pixels[i+3] = 255;
            }
            isSynthetic = true;
        } else {
            int texChannels;
            pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            imageSize = texWidth * texHeight * 4;
        }

        if (!pixels)
            throw std::runtime_error("Failed to load texture image: " + filepath);

        VkDevice device = _context.getDevice();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create staging buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate staging buffer memory!");

        checkVkR(vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0));

        void *data;
        checkVkR(vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data));
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        if (isSynthetic) {
            delete[] pixels;
        } else {
            stbi_image_free(pixels);
        }

        VkImage image;
        VkDeviceMemory imageMemory;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image!");

        vkGetImageMemoryRequirements(device, image, &memRequirements);
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate image memory!");

        checkVkR(vkBindImageMemory(device, image, imageMemory, 0));

        VkCommandBuffer commandBuffer = _context.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        _context.endSingleTimeCommands(commandBuffer);

        commandBuffer = _context.beginSingleTimeCommands();
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};

        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        _context.endSingleTimeCommands(commandBuffer);

        commandBuffer = _context.beginSingleTimeCommands();
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        _context.endSingleTimeCommands(commandBuffer);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        VkImageView imageView;
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture image view!");

        TextureData texData{};
        texData.image = image;
        texData.memory = imageMemory;
        texData.view = imageView;

        return texData;
    }

    void TextureManager::updateDescriptorSet(uint32_t index, VkImageView imageView)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = _textureSampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = index;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_context.getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}