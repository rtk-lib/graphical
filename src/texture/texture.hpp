#pragma once

#include <cstdint>

namespace rtk
{
    class Texture {
        private:
            explicit Texture(uint32_t handle) : _handle(handle) {}

            uint32_t _handle;

            friend class TextureManager;
            friend class RenderWindow;
            friend class Sprite;
            friend class SpriteRenderer;
        };

}