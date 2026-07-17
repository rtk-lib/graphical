#pragma once

#include "Logger/Logger.hpp"

namespace rtk 
{

    struct RGB {
        uint8_t r, g, b;
    };

    class Window 
    {
        public:

            /*
            ** @brief Create a window 
            */
            Window() 
            {
                LOG_INFO("create window");
            }

            /*
            ** @brief Destroy the window
            */
            ~Window()
            {
                LOG_INFO("destroy window");
            }

            /*
            ** @brief swap the buffer and clear
            ** @param clearColor to choose the background color
            */
            void display(RGB clearColor = {0, 0, 0}) 
            {

            }

            uint64_t getSurface() const { return _surface; }

        private:

            void* _display;
            uint64_t _windowHandle;
            
            void* _vkInstance;
            uint64_t _surface;
    };
}