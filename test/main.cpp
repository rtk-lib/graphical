#include "../src/window/window.hpp"
#include "../src/render/render.hpp"

#include <time.h>
#include <unistd.h>

/*Test Open Window*/
int main()
{
    rtk::Window window(1920, 1080, "Vulkan");
    rtk::Render render(window);

    while (window.pollEvents()) {
        render.draw({255, 255, 255});
        sleep(1);
        render.draw({0, 0, 0});
        sleep(1);
    }

    return 0;
}
