#include "../Logger/Logger.hpp"

#include "../src/window/window.hpp"
#include "../src/graphical/renderWindow.hpp"
#include "../src/render/VulkanContext.hpp"
#include "../src/render/SpriteRenderer.hpp"
#include "../src/texture/textureManager.hpp"
#include "../src/event/event.hpp"
#include "../sprite/sprite.hpp"
#include "../src/texture/texture.hpp"

#include <cstddef>
#include <sstream>
#include <string>
#include <type_traits>

template<typename T>
void logTypeInfo(const std::string& name)
{
    std::ostringstream message;

    message
        << name
        << " | sizeof: " << sizeof(T) << " bytes"
        << " | alignof: " << alignof(T) << " bytes"
        << " | trivial: "
        << std::boolalpha
        << std::is_trivially_copyable_v<T>
        << " | standard layout: "
        << std::is_standard_layout_v<T>;

    LOG_INFO(message.str());
}

int main()
{
    LOG_INFO("========== RTK PUBLIC TYPES ==========");

    logTypeInfo<rtk::RGB>("rtk::RGB");
    logTypeInfo<rtk::Texture>("rtk::Texture");
    logTypeInfo<rtk::SpriteData>("rtk::SpriteData");
    logTypeInfo<rtk::Sprite>("rtk::Sprite");
    logTypeInfo<rtk::Event>("rtk::Event");
    logTypeInfo<rtk::Window>("rtk::Window");
    logTypeInfo<rtk::RenderWindow>("rtk::RenderWindow");

    LOG_INFO("========== RTK INTERNAL TYPES ==========");

    logTypeInfo<rtk::VulkanContext>("rtk::VulkanContext");
    logTypeInfo<rtk::TextureManager>("rtk::TextureManager");
    logTypeInfo<rtk::SpriteRenderer>("rtk::SpriteRenderer");

    LOG_INFO("========== RTK MATH TYPES ==========");

    logTypeInfo<rtk::vec2>("rtk::math::vec2");
    return 0;
}