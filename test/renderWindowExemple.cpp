//#include "../src/window/window.hpp"
//#include "../src/render/VulkanContext.hpp"
//#include "../src/texture/textureManager.hpp"
//#include "../src/render/SpriteRenderer.hpp"
//#include "../src/event/event.hpp"

#include "../include/graphical.hpp"

#include "../sprite/sprite.hpp"

#include "../Logger/Logger.hpp"

#include <time.h>
#include <unistd.h>

/*Test Open Window*/
int main()
{
    rtk::RenderWindow rWindow({1920, 1080}, "Vulkan");

    uint32_t testTexId = rWindow.loadTexture("texture.png");

    rtk::SpriteData sprite;

    sprite.position = {1000.0f, 0.0f};

    rtk::Event rtkEvent;

    rtk::RGB clearColor = {100, 100, 100};

    while (rWindow.pollEvents(rtkEvent)) {
        rWindow.beginFrame(clearColor);
        rWindow.drawSpriteFromRaw({1920.0f / 2 - (1920 / 4), 0.0f / 2}, {1000.0f, 1000.0f}, 0.0f, testTexId);
        rWindow.endFrame();
    }

    return 0;
}
