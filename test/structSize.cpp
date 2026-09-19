#include "../Logger/Logger.hpp"

#include "../include/window/window.hpp"
#include "../include/graphical/renderWindow.hpp"
#include "../include/render/VulkanContext.hpp"
#include "../include/render/SpriteRenderer.hpp"
#include "../include/texture/textureManager.hpp"
#include "../include/event/event.hpp"
#include "../sprite/sprite.hpp"
#include "../include/texture/texture.hpp"

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