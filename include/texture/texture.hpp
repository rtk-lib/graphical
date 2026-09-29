#pragma once

#include <cstdint>
#include <limits>

namespace rtk
{
    class Texture {
        public:
            Texture() noexcept = default;

            [[nodiscard]]
            bool isValid() const noexcept
            {
                return _handle != INVALID_HANDLE;
            }

        private:
            static constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max() + 1;

            explicit Texture(uint32_t handle) : _handle(handle) {}

            uint32_t _handle = INVALID_HANDLE;

            friend class TextureManager;
            friend class RenderWindow;
            friend class Sprite;
            friend class SpriteRenderer;
    };
}