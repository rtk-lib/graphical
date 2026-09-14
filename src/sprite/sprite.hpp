#pragma once

#include "spriteData.hpp"

namespace rtk {

    #define FLIP_X 1u << 0
    #define FLIP_Y 1u << 1

    class Sprite {
        public:
            explicit Sprite(Texture texture) noexcept
            {
                _data.position = {0.f, 0.f};
                _data.scale = {1.f, 1.f};
                _data.size = {1.f, 1.f};
                _data.origin = {0.f, 0.f};
                _data.textureRect = {0, 0, 0, 0};
                _data.color = {255, 255, 255, 255};
                _data.rotation = 0.f;
                _data.textureId = texture._handle;
                _data.layer = 0;
                _data.flags = 0;
                _data.reserved = 0;
            }

            void setPosition(rtk::vec2 position) noexcept
            {
                _data.position = position;
            }

            void setScale(rtk::vec2 scale) noexcept
            {
                _data.scale = scale;
            }

            void setSize(rtk::vec2 size) noexcept
            {
                _data.size = size;
            }

            void setOrigin(rtk::vec2 origin) noexcept
            {
                _data.origin = origin;
            }

            void setRotation(float rotation) noexcept
            {
                _data.rotation = rotation;
            }

            void setTexture(Texture texture) noexcept
            {
                _data.textureId = texture._handle;
            }

            void setTextureRect(TextureRectU16 rectangle) noexcept
            {
                _data.textureRect = rectangle;
            }

            void setColor(ColorRGBA8 color) noexcept
            {
                _data.color = color;
            }

            void setFlipX(bool enabled) noexcept
            {
                if (enabled)
                    _data.flags |= FLIP_X;
                else
                    _data.flags &= ~FLIP_X;
            }

            void setFlipY(bool enabled) noexcept
            {
                if (enabled)
                    _data.flags |= FLIP_Y;
                else
                    _data.flags &= ~FLIP_Y;
            }

            void setLayer(int32_t layer) noexcept
            {
                _data.layer = layer;
            }

            void setFlags(uint32_t flags) noexcept
            {
                _data.flags = flags;
            }

            [[nodiscard]]
            rtk::vec2 getPosition() const noexcept
            {
                return _data.position;
            }

            [[nodiscard]]
            rtk::vec2 getScale() const noexcept
            {
                return _data.scale;
            }

            [[nodiscard]]
            rtk::vec2 getSize() const noexcept
            {
                return _data.size;
            }

            [[nodiscard]]
            rtk::vec2 getOrigin() const noexcept
            {
                return _data.origin;
            }

            [[nodiscard]]
            float getRotation() const noexcept
            {
                return _data.rotation;
            }

            [[nodiscard]]
            TextureRectU16 getTextureRect() const noexcept
            {
                return _data.textureRect;
            }

            [[nodiscard]]
            ColorRGBA8 getColor() const noexcept
            {
                return _data.color;
            }

            [[nodiscard]]
            bool isFlippedX() const noexcept
            {
                return (_data.flags & FLIP_X) != 0;
            }

            [[nodiscard]]
            bool isFlippedY() const noexcept
            {
                return (_data.flags & FLIP_Y) != 0;
            }

            [[nodiscard]]
            int32_t getLayer() const noexcept
            {
                return _data.layer;
            }

            [[nodiscard]]
            uint32_t getFlags() const noexcept
            {
                return _data.flags;
            }

            [[nodiscard]]
            const SpriteData& data() const noexcept
            {
                return _data;
            }

        private:
            [[nodiscard]]
            uint32_t getTextureId() const noexcept
            {
                return _data.textureId;
            }

            friend class RenderWindow;

            SpriteData _data {};
    };

}