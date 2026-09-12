#include "../render/SpriteRender.hpp
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
                _renderer(_context, _texture)
            {}

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

        };
};