#pragma once

#include "spriteData.hpp"

namespace rtk {

    class Sprite {
        public:
            explicit Sprite(Texture texture) noexcept
            {
                _data.position = {0.f, 0.f};
                _data.scale = {1.f, 1.f};
                _data.rotation = 0.f;
                _data.textureId = texture._handle;
                _data.colorTint = 0xFFFFFFFF;
                _data.flags = 0;
            }

            void setPosition(math::vec2 position) noexcept
            {
                _data.position = position;
            }

            void setSize(math::vec2 size) noexcept
            {
                _data.scale = size;
            }

            void setRotation(float rotation) noexcept
            {
                _data.rotation = rotation;
            }

            void setTexture(Texture texture) noexcept
            {
                _data.textureId = texture._handle;
            }

            void setColor(rtk::RGB color) noexcept
            {
                _data.colorTint = color.toRGBA();
            }

            void setFlags(uint32_t flags) noexcept
            {
                _data.flags = flags;
            }

            [[nodiscard]]
            math::vec2 getPosition() const noexcept
            {
                return _data.position;
            }

            [[nodiscard]]
            math::vec2 getSize() const noexcept
            {
                return _data.scale;
            }

            [[nodiscard]]
            float getRotation() const noexcept
            {
                return _data.rotation;
            }

            [[nodiscard]]
            const SpriteData& data() const noexcept
            {
                return _data;
            }

        private:
            SpriteData _data {};    
    };
}