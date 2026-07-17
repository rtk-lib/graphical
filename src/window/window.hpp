#pragma once

#include <cstdint>

namespace rtk 
{
    /**
     * @brief Simple struct representing an RGB color.
     */
    struct RGB {
        uint8_t r, g, b;
    };

    /**
     * @brief Cross-platform Window class.
     * 
     * This class abstracts the OS-specific window creation and event management.
     */
    class Window 
    {
        public:
            /**
             * @brief Create a Window instance.
             * 
             * @param width The width of the window.
             * @param height The height of the window.
             * @param title The title of the window.
             */
            Window(uint32_t width = 800, uint32_t height = 600, const char* title = "rtk-lib");

            /**
             * @brief Destroy the Window instance.
             */
            ~Window();

            /**
             * @brief Swap the buffers and clear the window.
             * 
             * @param clearColor The color used to clear the background.
             */
            void display(RGB clearColor = {0, 0, 0});

            /**
             * @brief Get the Vulkan surface handle.
             * 
             * @return uint64_t The surface handle cast to uint64_t.
             */
            uint64_t getSurface() const;

            /**
             * @brief Poll window events (like close, resize, inputs).
             * 
             * @return true if the window is still open and running.
             * @return false if the window received a close request.
             */
            bool pollEvents();

        private:
            void* _display;
            uint64_t _windowHandle;
            
            void* _vkInstance;
            uint64_t _surface;
            bool _isOpen;
    };
}