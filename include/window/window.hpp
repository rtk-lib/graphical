#pragma once

#include <cstdint>
#include <vector>

#include "event/event.hpp"

#include <vulkan/vulkan_core.h>

namespace rtk 
{
    /**
     * @brief Simple struct representing an RGB color.
     */
    class RGB {
        public:
            uint8_t r, g, b;

            [[nodiscard]]
            constexpr uint32_t toRGBA(uint8_t alpha = 255) const noexcept
            {
                return static_cast<uint32_t>(r)
                    | (static_cast<uint32_t>(g) << 8)
                    | (static_cast<uint32_t>(b) << 16)
                    | (static_cast<uint32_t>(alpha) << 24);
            }
    };

    /*
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
            Window(uint32_t width = 1920, uint32_t height = 1080, const char* title = "rtk-lib");

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

            std::vector<const char*> getRequiredExtensions() const;
            void createSurface(void *vkInstance);
            VkSurfaceKHR getSurface() const;

            /**
             * @brief Poll window events (like close, resize, inputs).
             * 
             * @return true if the window is still open and running.
             * @return false if the window received a close request.
             */
            bool pollEvents(rtk::Event &rtkEvent);

            void setWindowSize(uint32_t width, uint32_t height) {
                _width = width;
                _height = height;
            }

            const uint32_t &getWindowSizeWidth(){return _width;};
            const uint32_t &getWindowSizeHeight(){return _height;};

        private:
            void *_display;
            uint64_t _windowHandle;
            
            void *_vkInstance;
            VkSurfaceKHR _surface;
            bool _isOpen;

            uint32_t _width;
            uint32_t _height;
    };
}