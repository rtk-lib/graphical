#include "../include/graphical.hpp"
#include "../sprite/sprite.hpp"
#include "../Logger/Logger.hpp"

#include <time.h>
#include <unistd.h>

int main()
{
    rtk::RenderWindow rWindow({1920, 1080}, "Vulkan");

    rtk::Texture sonicTex = rWindow.loadTexture("./test/sonic.png");
    rtk::Texture marioTex = rWindow.loadTexture("./test/mario.png");

    rtk::Sprite sonicSprite(sonicTex);
    rtk::Sprite marioSprite(marioTex);

    sonicSprite.setPosition({500.0f, 400.0f});
    sonicSprite.setSize({300.0f, 300.0f});
    sonicSprite.setRotation(0.0f);

    marioSprite.setPosition({1100.0f, 400.0f});
    marioSprite.setSize({300.0f, 300.0f});
    marioSprite.setRotation(0.0f);

    marioSprite.setFlipY(true);

    rtk::Event rtkEvent;
    rtk::RGB clearColor = {100, 100, 100};

    while (rWindow.pollEvents(rtkEvent)) {
        if (!rWindow.beginFrame(clearColor))
            continue;

        rWindow.drawSprite(sonicSprite);
        rWindow.drawSprite(marioSprite);

        rWindow.endFrame();
    }

    return 0;
}