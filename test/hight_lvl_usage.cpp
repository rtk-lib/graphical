

#include "../include/window/window.hpp"
#include "../include/render/VulkanContext.hpp"
#include "../include/texture/textureManager.hpp"
#include "../include/render/SpriteRenderer.hpp"
#include "../include/event/event.hpp"



#include "../sprite/sprite.hpp"

#include "../Logger/Logger.hpp"

#include <time.h>
#include <unistd.h>

/*Test Open Window*/
int main()
{
    rtk::Window window(1920, 1080, "Vulkan");
    rtk::VulkanContext context(window);
    rtk::TextureManager textureManager(context);
    rtk::SpriteRenderer spriteRenderer(context, textureManager);

    uint32_t testTexId = textureManager.loadTexture("texture.png");

    rtk::SpriteData sprite;

    sprite.position = {1000.0f, 0.0f};

    rtk::Event rtkEvent;

    rtk::RGB clearColor = {100, 100, 100};

    while (window.pollEvents(rtkEvent)) {
        spriteRenderer.beginFrame(clearColor);
        spriteRenderer.drawSprite({1920.0f / 2 - (1920 / 4), 0.0f / 2}, {1000.0f, 1000.0f}, 0.0f, testTexId);
        spriteRenderer.endFrame();
    }

    return 0;
}

"