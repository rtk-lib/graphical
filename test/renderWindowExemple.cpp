//#include "../include/window/window.hpp"
//#include "../include/render/VulkanContext.hpp"
//#include "../include/texture/textureManager.hpp"
//#include "../include/render/SpriteRenderer.hpp"
//#include "../include/event/event.hpp"

#include "../include/graphical.hpp"

#include "../sprite/sprite.hpp"

#include "../Logger/Logger.hpp"

#include <time.h>
#include <unistd.h>

/*Test Open Window*/
int main()
{
    rtk::RenderWindow rWindow({1920, 1080}, "Vulkan");

    rtk::Texture testTexId = rWindow.loadTexture("texture.png");
    rtk::Sprite sprite(testTexId);

    rtk::vec2 pSprite = {1920.0f / 2 - (1920 / 4), 0.0f / 2};

    sprite.setPosition(pSprite);
    sprite.setSize({1000.0f, 1000.0f});
    sprite.setRotation(0.0f);

    rtk::Event rtkEvent;

    rtk::RGB clearColor = {100, 100, 100};

    while (rWindow.pollEvents(rtkEvent)) {
        if (!rWindow.beginFrame(clearColor))
            continue;
        pSprite.x++;
        if (pSprite.x == 1920)
            pSprite.x = 0;
        sprite.setPosition(pSprite);
        rWindow.drawSprite(sprite);
        rWindow.endFrame();
    }

    return 0;
}
