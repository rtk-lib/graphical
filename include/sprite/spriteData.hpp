#pragma once

#include "utils/vec2.hpp"

#include <stdint.h>
#include <vector>

#include <vulkan/vulkan.h>
#include <array>

namespace rtk
{

    struct alignas(8) TextureRectU16 {
        uint16_t left;
        uint16_t top;
        uint16_t width;
        uint16_t height;
    };

    struct alignas(4) ColorRGBA8 {
        uint8_t r{255};
        uint8_t g{255};
        uint8_t b{255};
        uint8_t a{255};
    };

    /**
     * @brief SpriteData struct who contain information about the sprite
     * 64 byte struct btw
    */
    struct alignas(64) SpriteData {
        vec2 position;
        vec2 scale{1.f, 1.f};
        vec2 size;
        vec2 origin;

        TextureRectU16 textureRect{};
        ColorRGBA8 color{};
        float rotation{0.f};

        uint32_t textureId{0};
        int32_t layer{0};
        uint32_t flags{0};
        uint32_t reserved{0};
    };

    static_assert(sizeof(TextureRectU16) == 8);
    static_assert(sizeof(ColorRGBA8) == 4);
    static_assert(sizeof(SpriteData) == 64);
}

/**
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

    static std::array<VkVertexInputAttributeDescription, 9>
    getSpriteAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 9> attributes{};

        /*Position*/
        attributes[0].binding = 1;
        attributes[0].location = 1;
        attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[0].offset = offsetof(SpriteData, position);

        /*Scale*/
        attributes[1].binding = 1;
        attributes[1].location = 2;
        attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[1].offset = offsetof(SpriteData, scale);

        /*Size*/
        attributes[2].binding = 1;
        attributes[2].location = 3;
        attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[2].offset = offsetof(SpriteData, size);

        /*Origin*/
        attributes[3].binding = 1;
        attributes[3].location = 4;
        attributes[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[3].offset = offsetof(SpriteData, origin);

        /*Texture rectangle*/
        attributes[4].binding = 1;
        attributes[4].location = 5;
        attributes[4].format = VK_FORMAT_R16G16B16A16_UINT;
        attributes[4].offset = offsetof(SpriteData, textureRect);

        /*Color*/
        attributes[5].binding = 1;
        attributes[5].location = 6;
        attributes[5].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributes[5].offset = offsetof(SpriteData, color);

        /*Rotation*/
        attributes[6].binding = 1;
        attributes[6].location = 7;
        attributes[6].format = VK_FORMAT_R32_SFLOAT;
        attributes[6].offset = offsetof(SpriteData, rotation);

        /*Texture ID*/
        attributes[7].binding = 1;
        attributes[7].location = 8;
        attributes[7].format = VK_FORMAT_R32_UINT;
        attributes[7].offset = offsetof(SpriteData, textureId);

        /*Flags*/
        attributes[8].binding = 1;
        attributes[8].location = 9;
        attributes[8].format = VK_FORMAT_R32_UINT;
        attributes[8].offset = offsetof(SpriteData, flags);

        return attributes;
    }
}