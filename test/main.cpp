#include "../src/window/window.hpp"
#include "../src/render/render.hpp"

int main()
{
    rtk::Window window(800, 600, "Vulkan");
    rtk::Render render(window);

    while (window.pollEvents()) {
        render.draw();
    }

    return 0;
}
