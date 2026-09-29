#pragma once

#include "render/SpriteRenderer.hpp"
#include "sprite/sprite.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "utils/vec2.hpp"
#include "font/fontManager.hpp"
#include "text/text.hpp"
#include <span>

namespace rtk {
    class RenderWindow
    {
        private:
            Window _window;
            VulkanContext _context;
            TextureManager _textures;
            FontManager _fonts;
            SpriteRenderer _renderer;

        public:

            /*no copying*/
            RenderWindow(const RenderWindow&) = delete;
            RenderWindow& operator=(const RenderWindow&) = delete;

            /*not movable*/
            RenderWindow(RenderWindow&&) = delete;
            RenderWindow& operator=(RenderWindow&&) = delete;

            /**
             * @brief init window, context, the textureManager, fontManager and the renderer
            */
            RenderWindow(rtk::vec2 windowSize, const char *windowName) :
                _window(windowSize.x, windowSize.y, windowName), 
                _context(_window),
                _textures(_context),
                _fonts(_textures),
                _renderer(_context, _textures)
            {}

            
            /********\
            * Window *
            \********/

            /** 
             * @brief get some event on the computer
             * @param event Event state to update. 
             * 
             * @return true if the window remains open, false otherwise.
            */
            [[nodiscard]]
            inline bool pollEvents(rtk::Event &event)
            {
                return _window.pollEvents(event);
            }

            /*****************\
            * Texture Manager *
            \*****************/

            /**
            * @brief load texture into the texture manager
            *
            * @param textureFile path of the file.
            * @return returns the ID of a texture
            */
            [[nodiscard]]
            rtk::Texture loadTexture(const char *textureFile)
            {
                return _textures.loadTexture(textureFile);
            }

            /**************\
            * Font Manager *
            \**************/

            [[nodiscard]]
            rtk::Font loadFont(const std::string& fontFile, float pixelSize)
            {
                return _fonts.loadFont(fontFile, pixelSize);
            }

            [[nodiscard]]
            rtk::Text createText(const std::string& content, rtk::Font font, const rtk::vec2& position = {}, const rtk::ColorRGBA8& color = {255, 255, 255, 255}) const
            {
                return _fonts.createText(content, font, position, color);
            }

            FontManager& getFontManager() { return _fonts; }
            const FontManager& getFontManager() const { return _fonts; }


            /***************\
            * Sprite Render *
            \***************/

            [[nodiscard]]
            bool beginFrame(const rtk::RGB &color)
            {
                return _renderer.beginFrame(color);
            }

            void drawSpriteFromRaw(const rtk::vec2& position, const rtk::vec2& size, float rotation, const rtk::Texture &texture)
            {
                _renderer.drawSprite(position, size, rotation, texture._handle);
            }

            void drawSprite(const rtk::Sprite &sprite)
            {
                _renderer.drawSprite(sprite);
            }

            void draw(std::span<const SpriteData> sprites)
            {
                _renderer.submit(sprites);
            }
            
            void draw(const rtk::Text& text)
            {
                _renderer.submit(text.getSprites());
            }

            void endFrame()
            {
                _renderer.endFrame();
            }

        };
};
