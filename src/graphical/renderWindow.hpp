#include "../render/SpriteRenderer.hpp"

#include "../sprite/sprite.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace rtk {
    class RenderWindow
    {
        private:
            Window _window;
            VulkanContext _context;
            TextureManager _textures;
            SpriteRenderer _renderer;

        public:

            /*no copying*/
            RenderWindow(const RenderWindow&) = delete;
            RenderWindow& operator=(const RenderWindow&) = delete;

            /*not movable*/
            RenderWindow(RenderWindow&&) = delete;
            RenderWindow& operator=(RenderWindow&&) = delete;

            /**
             * @brief init window, context, the textureManager and the renderer
            */
            RenderWindow(glm::vec2 windowSize, const char *windowName) :
                _window(windowSize.x, windowSize.y, windowName), 
                _context(_window),
                _textures(_context),
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

            /***************\
            * Sprite Render *
            \***************/

            void beginFrame(const rtk::RGB &color)
            {
                _renderer.beginFrame(color);
            }

            void drawSpriteFromRaw(const glm::vec2& position, const glm::vec2& size, float rotation, const rtk::Texture &texture)
            {
                _renderer.drawSprite(position, size, rotation, texture._handle);
            }

            void drawSprite(const rtk::Sprite &sprite)
            {
                _renderer.drawSprite(sprite.getPosition(), sprite.getSize(), sprite.getRotation(), sprite.getTextureId());
            }

            void endFrame()
            {
                _renderer.endFrame();
            }

        };
};