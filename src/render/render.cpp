/*
 * Render implementation
 */
#include "render.hpp"
#include "Logger/Logger.hpp"

namespace rtk
{
    Render::Render(Window& window) : _window(window)
    {
        LOG_INFO("Render initialized");
    }

    Render::~Render()
    {
        LOG_INFO("Render destroyed");
    }

    void Render::draw()
    {
        _window.display({0, 0, 0});
    }
}
