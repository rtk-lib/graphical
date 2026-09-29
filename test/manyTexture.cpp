#include "../include/graphical.hpp"
#include "../include/sprite/sprite.hpp"
#include "../include/Logger/Logger.hpp"

#include <cstdint>

int main()
{
    rtk::RenderWindow rWindow({1920, 1080}, "Vulkan");

    rtk::Texture sonicTex = rWindow.loadTexture("./test/sonic.png");
    rtk::Texture marioTex = rWindow.loadTexture("./test/mario.png");

    rtk::Sprite sonicSprite(sonicTex);
    rtk::Sprite marioSprite(marioTex);

    float sonicX = 500.0f;
    float sonicY = 400.0f;
    sonicSprite.setPosition({sonicX, sonicY});
    sonicSprite.setSize({300.0f, 300.0f});
    sonicSprite.setRotation(0.0f);

    std::uint16_t moveX = 0;
    sonicSprite.setTextureRect({moveX, 0, 607, 794});

    float x = 1100.0f;
    marioSprite.setPosition({x, 400.0f});
    marioSprite.setSize({300.0f, 300.0f});
    marioSprite.setRotation(0.0f);
    marioSprite.setFlipX(true);

    rtk::Event rtkEvent;
    rtk::RGB clearColor = {100, 100, 100};

    while (rWindow.pollEvents(rtkEvent)) {
        if (!rWindow.beginFrame(clearColor))
            continue;

        float speed = 5.0f;

        if (rtkEvent.isKeyPressed(rtk::Key::Z)) sonicY -= speed;
        if (rtkEvent.isKeyPressed(rtk::Key::S)) sonicY += speed;
        if (rtkEvent.isKeyPressed(rtk::Key::Q)) sonicX -= speed;
        if (rtkEvent.isKeyPressed(rtk::Key::D)) sonicX += speed;

        sonicSprite.setPosition({sonicX, sonicY});

        if (rtkEvent.isKeyPressed(rtk::Key::A)) {
            x++;
            moveX++;

            marioSprite.setPosition({x, 400.0f});
            sonicSprite.setTextureRect({moveX, 0, 607, 794});
        }

        rWindow.drawSprite(marioSprite);
        rWindow.drawSprite(sonicSprite);

        rWindow.endFrame();
    }

    return 0;
}