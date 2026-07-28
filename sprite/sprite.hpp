#pragma once

#include <stdint.h>
#include <vector>

#include <vulkan/vulkan.h>
#include <array>

namespace math 
{
    struct vec2
    {
        float x, y;
    };
    
}

namespace rtk
{
    /*
     * @brief SpriteData struct who contain information about the sprite
     * 32 byte struct btw
    */
    struct SpriteData {
        math::vec2 position;
        math::vec2 scale;
        float rotation;
        
        uint32_t textureId;
        uint32_t colorTint; 

        uint32_t flags;
    };
}

/*
 * @brief Vulkan implementation
 * how to read the struct SpriteData
*/
namespace rtk {
    static VkVertexInputBindingDescription getSpriteBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(SpriteData);

        /*Change the way to read this buffer*/
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 6> getSpriteAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 6> attributes{};

        /*Position*/
        attributes[0].binding = 1;
        attributes[0].location = 3; 
        attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[0].offset = offsetof(SpriteData, position);

        /*Scale*/
        attributes[1].binding = 1;
        attributes[1].location = 4;
        attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[1].offset = offsetof(SpriteData, scale);

        /*Rotation*/
        attributes[2].binding = 1;
        attributes[2].location = 5;
        attributes[2].format = VK_FORMAT_R32_SFLOAT;
        attributes[2].offset = offsetof(SpriteData, rotation);

        /*Texture ID*/
        attributes[3].binding = 1;
        attributes[3].location = 6;
        attributes[3].format = VK_FORMAT_R32_UINT;
        attributes[3].offset = offsetof(SpriteData, textureId);

        /*Color Tint*/
        attributes[4].binding = 1;
        attributes[4].location = 7;
        attributes[4].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributes[4].offset = offsetof(SpriteData, colorTint);

        /*Flags*/
        attributes[5].binding = 1;
        attributes[5].location = 8;
        attributes[5].format = VK_FORMAT_R32_UINT;
        attributes[5].offset = offsetof(SpriteData, flags);

        return attributes;
    }
}