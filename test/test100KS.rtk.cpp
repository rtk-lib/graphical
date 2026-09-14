#include "../include/graphical.hpp"
#include "../sprite/sprite.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>
#include <vector>

struct ParticleMotion {
    float radius;
    float baseAngle;
    float speed;
};

int main()
{
    constexpr std::size_t ParticleCount = 1'000'000;
    constexpr float WindowWidth = 1920.0f;
    constexpr float WindowHeight = 1080.0f;
    constexpr float Pi = std::numbers::pi_v<float>;

    rtk::RenderWindow window({WindowWidth, WindowHeight}, "RTK - 100K Sprite Stress Test");

    const rtk::Texture texture = window.loadTexture("texture.png");
    const rtk::Sprite prototype(texture);

    std::vector<rtk::SpriteData> particles(ParticleCount);
    std::vector<ParticleMotion> motions(ParticleCount);

    std::uint64_t frameCount = 0;
    bool benchmarkDisplayed = false;
    const auto benchmarkStart = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < ParticleCount; ++i) {
        const float normalizedIndex = static_cast<float>(i) / static_cast<float>(ParticleCount);
        const float branch = static_cast<float>(i % 8) * (2.0f * Pi / 8.0f);
        const float spiral = normalizedIndex * Pi * 20.0f;
        const float radius = std::sqrt(normalizedIndex) * 520.0f;
        const float size = 5.0f + static_cast<float>(i % 3);

        particles[i] = prototype.data();
        particles[i].scale = {size, size};
        particles[i].rotation = 0.0f;
        particles[i].flags = 0;

        motions[i].radius = radius;
        motions[i].baseAngle = branch + spiral;
        motions[i].speed = 0.1f + 1.2f / (radius * 0.02f + 1.0f);
    }

    rtk::Event event;
    const rtk::RGB clearColor = {5, 5, 15};
    const auto startTime = std::chrono::steady_clock::now();

    while (window.pollEvents(event)) {
        if (event.isKeyPressed(rtk::Key::A))
            std::cout << "A\n";
    const auto currentTime = std::chrono::steady_clock::now();
    const float time = std::chrono::duration<float>(currentTime - startTime).count();

    for (std::size_t i = 0; i < ParticleCount; ++i) {
        const float angle = motions[i].baseAngle + time * motions[i].speed;
        const float wave = std::sin(time * 2.0f + motions[i].baseAngle) * 10.0f;
        const float radius = motions[i].radius + wave;

        particles[i].position = {
            WindowWidth * 0.5f + std::cos(angle) * radius,
            WindowHeight * 0.5f + std::sin(angle) * radius * 0.55f
        };

        particles[i].rotation = angle * (180.0f / Pi);
    }

    if (!window.beginFrame(clearColor))
        continue;
    window.draw(std::span<const rtk::SpriteData>{particles});
    window.endFrame();

    frameCount++;



    const auto benchmarkNow = std::chrono::steady_clock::now();
    const double elapsedSeconds = std::chrono::duration<double>(benchmarkNow - benchmarkStart).count();

    if (!benchmarkDisplayed && elapsedSeconds >= 10.0) {
        const double averageFps = static_cast<double>(frameCount) / elapsedSeconds;
        const double averageFrameTimeMs = 1000.0 / averageFps;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Benchmark duration: " << elapsedSeconds << " seconds\n";
        std::cout << "Rendered frames: " << frameCount << '\n';
        std::cout << "Average FPS: " << averageFps << '\n';
        std::cout << "Average frame time: " << averageFrameTimeMs << " ms\n";

        benchmarkDisplayed = true;
    }
}

    return 0;
}