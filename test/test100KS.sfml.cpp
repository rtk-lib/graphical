#include <SFML/Graphics.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <vector>

struct ParticleMotion {
    float radius;
    float baseAngle;
    float speed;
};

int main()
{
    constexpr std::size_t ParticleCount = 1'000'000;
    constexpr unsigned int WindowWidth = 1920;
    constexpr unsigned int WindowHeight = 1080;
    constexpr float Pi = std::numbers::pi_v<float>;

    sf::RenderWindow window(
        sf::VideoMode({WindowWidth, WindowHeight}),
        "SFML 3.0.2 - 100K individual draw calls"
    );

    window.setVerticalSyncEnabled(false);
    window.setFramerateLimit(0);

    sf::Texture texture;

    if (!texture.loadFromFile("texture.png"))
        throw std::runtime_error("Failed to load texture.png");

    const sf::Vector2u textureSize = texture.getSize();

    if (textureSize.x == 0 || textureSize.y == 0)
        throw std::runtime_error("Texture has an invalid size");

    std::vector<sf::Sprite> particles;
    std::vector<ParticleMotion> motions(ParticleCount);

    particles.reserve(ParticleCount);

    for (std::size_t i = 0; i < ParticleCount; ++i) {
        const float normalizedIndex = static_cast<float>(i) / static_cast<float>(ParticleCount);
        const float branch = static_cast<float>(i % 8) * (2.0f * Pi / 8.0f);
        const float spiral = normalizedIndex * Pi * 20.0f;
        const float radius = std::sqrt(normalizedIndex) * 520.0f;
        const float size = 1.0f + static_cast<float>(i % 3);

        sf::Sprite& sprite = particles.emplace_back(texture);

        sprite.setOrigin({
            static_cast<float>(textureSize.x) * 0.5f,
            static_cast<float>(textureSize.y) * 0.5f
        });

        sprite.setScale({
            size / static_cast<float>(textureSize.x),
            size / static_cast<float>(textureSize.y)
        });

        motions[i] = {
            radius,
            branch + spiral,
            0.1f + 1.2f / (radius * 0.02f + 1.0f)
        };
    }

    std::uint64_t frameCount = 0;
    bool benchmarkDisplayed = false;

    const auto startTime = std::chrono::steady_clock::now();
    const auto benchmarkStart = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const auto currentTime = std::chrono::steady_clock::now();
        const float time = std::chrono::duration<float>(currentTime - startTime).count();

        for (std::size_t i = 0; i < ParticleCount; ++i) {
            const float angle = motions[i].baseAngle + time * motions[i].speed;
            const float wave = std::sin(time * 2.0f + motions[i].baseAngle) * 10.0f;
            const float radius = motions[i].radius + wave;

            particles[i].setPosition({
                static_cast<float>(WindowWidth) * 0.5f + std::cos(angle) * radius,
                static_cast<float>(WindowHeight) * 0.5f + std::sin(angle) * radius * 0.55f
            });

            particles[i].setRotation(sf::degrees(angle * (180.0f / Pi)));
        }

        window.clear(sf::Color(5, 5, 15));

        for (const sf::Sprite& particle : particles)
            window.draw(particle);

        window.display();

        frameCount++;

        const auto benchmarkNow = std::chrono::steady_clock::now();
        const double elapsedSeconds = std::chrono::duration<double>(benchmarkNow - benchmarkStart).count();

        if (!benchmarkDisplayed && elapsedSeconds >= 20.0) {
            const double averageFps = static_cast<double>(frameCount) / elapsedSeconds;
            const double averageFrameTimeMs = 1000.0 / averageFps;

            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Particles: " << ParticleCount << '\n';
            std::cout << "Rendered frames: " << frameCount << '\n';
            std::cout << "Average FPS: " << averageFps << '\n';
            std::cout << "Average frame time: " << averageFrameTimeMs << " ms\n";

            benchmarkDisplayed = true;
        }
    }

    return 0;
}