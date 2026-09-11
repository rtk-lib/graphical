#include "../src/window/window.hpp"
#include "../src/render/VulkanContext.hpp"
#include "../src/texture/textureManager.hpp"
#include "../src/render/SpriteRenderer.hpp"
#include "../src/event/event.hpp"

#include "../sprite/sprite.hpp"

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

    while (window.pollEvents(rtkEvent)) {
        spriteRenderer.beginFrame();
        spriteRenderer.drawSprite({100.0f, 0.0f}, {1000.0f, 1000.0f}, 0.0f, testTexId);
        spriteRenderer.drawSprite(glm::vec2(sprite.position.x, sprite.position.y), {100.0f, 100.0f}, 0.0f, testTexId);
        spriteRenderer.endFrame();
    }

    return 0;
}
