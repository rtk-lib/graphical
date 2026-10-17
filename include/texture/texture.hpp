#pragma once

#include "../utils/vec2.hpp"
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

            [[nodiscard]]
            vec2 getSize() const noexcept
            {
                return _size;
            }

             [[nodiscard]]
            const uint32_t getHandle() const noexcept
            {
                return _handle;
            }

        private:
            static constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max();

            explicit Texture(uint32_t handle, vec2 size = {0.f, 0.f}) : _handle(handle), _size(size) {}

            uint32_t _handle = INVALID_HANDLE;
            vec2 _size{0.f, 0.f};

            friend class TextureManager;
            friend class RenderWindow;
            friend class Sprite;
            friend class SpriteRenderer;
    };
}