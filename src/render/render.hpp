#pragma once

#include "../window/window.hpp"

namespace rtk
{
    /**
     * @brief Renderer class responsible for Vulkan logic.
     */
    class Render
    {
        public:
            /**
             * @brief Create a Render instance.
             * 
             * @param window The window to render to.
             */
            Render(Window& window);

            /**
             * @brief Destroy the Render instance.
             */
            ~Render();

            /**
             * @brief Draw a frame.
             */
            void draw();

        private:
            Window& _window;
    };
}
