#pragma once

#include "font/font.hpp"
#include "sprite/spriteData.hpp"
#include "texture/textureManager.hpp"
#include "utils/vec2.hpp"
#include "text/text.hpp"
#include <unordered_map>
#include <string>

namespace rtk {

struct Glyph {
    rtk::TextureRectU16 textureRect{};
    rtk::vec2 size{};
    rtk::vec2 bearing{};
    float advance = 0.f;
};

struct FontData {
    rtk::Texture atlas;
    std::unordered_map<char32_t, Glyph> glyphs;

    std::string path;
    float pixelSize = 0.f;
    float lineHeight = 0.f;

    uint32_t generation = 1;
    bool active = false;
};

class FontManager {
public:
    explicit FontManager(rtk::TextureManager& textures) : _textures(textures) {}

    rtk::Font loadFont(const std::string& path, float pixelSize);

    [[nodiscard]]
    const FontData *getFont(rtk::Font font) const;

    [[nodiscard]]
    rtk::Text createText(const std::string& content, rtk::Font font, const rtk::vec2& position = {}, const rtk::ColorRGBA8& color = {255, 255, 255, 255}) const;

private:
    rtk::TextureManager& _textures;
    std::vector<FontData> _fonts;
};

} // namespace rtk
