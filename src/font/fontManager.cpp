#include "sprite/sprite.hpp"
#include "font/fontManager.hpp"
#include "stb_truetype.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <vector>

namespace rtk {

Font FontManager::loadFont(const std::string& path, float pixelSize)
{
    if (path.empty())
        throw std::invalid_argument("Font path cannot be empty");

    if (pixelSize <= 0.f)
        throw std::invalid_argument("Font pixel size must be positive");

    for (uint32_t i = 0; i < _fonts.size(); ++i) {
        const FontData& font = _fonts[i];

        if (font.active && font.path == path && font.pixelSize == pixelSize) {
            return Font(i, font.generation);
        }
    }

    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("Failed to open font file: " + path);

    std::vector<std::uint8_t> fontBytes{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    if (fontBytes.empty())
        throw std::runtime_error("Font file is empty: " + path);

    const int fontOffset = stbtt_GetFontOffsetForIndex(fontBytes.data(), 0);

    if (fontOffset < 0)
        throw std::runtime_error("Invalid TrueType font: " + path);

    stbtt_fontinfo fontInfo{};

    if (!stbtt_InitFont(&fontInfo, fontBytes.data(), fontOffset)) {
        throw std::runtime_error("Failed to initialize font: " + path);
    }

    constexpr int FirstCodepoint = 32;
    constexpr int LastCodepoint = 255;
    constexpr int GlyphCount = LastCodepoint - FirstCodepoint + 1;

    std::vector<stbtt_packedchar> packedGlyphs(GlyphCount);

    int atlasSize = 512;
    bool packedSuccessfully = false;
    std::vector<std::uint8_t> alphaAtlas;

    while (atlasSize <= 2048 && !packedSuccessfully) {
        alphaAtlas.assign(static_cast<std::size_t>(atlasSize) * atlasSize, 0);

        std::fill(packedGlyphs.begin(), packedGlyphs.end(), stbtt_packedchar{});

        stbtt_pack_context packContext{};

        if (!stbtt_PackBegin(&packContext, alphaAtlas.data(), atlasSize, atlasSize, 0, 1, nullptr))
            throw std::runtime_error("Failed to initialize font atlas");

        stbtt_PackSetOversampling(&packContext, 2, 2);

        packedSuccessfully = stbtt_PackFontRange(&packContext, fontBytes.data(), 0,
            pixelSize,FirstCodepoint,GlyphCount,packedGlyphs.data()) != 0;

        stbtt_PackEnd(&packContext);

        if (!packedSuccessfully)
            atlasSize *= 2;
    }

    if (!packedSuccessfully)
        throw std::runtime_error("Font glyphs do not fit inside the maximum atlas: " + path);


    std::vector<std::uint8_t> rgbaAtlas(alphaAtlas.size() * 4);

    for (std::size_t i = 0; i < alphaAtlas.size(); ++i) {
        rgbaAtlas[i * 4 + 0] = 255;
        rgbaAtlas[i * 4 + 1] = 255;
        rgbaAtlas[i * 4 + 2] = 255;
        rgbaAtlas[i * 4 + 3] = alphaAtlas[i];
    }

    FontData data;
    data.path = path;
    data.pixelSize = pixelSize;
    data.active = true;
    data.generation = 1;

    data.atlas = _textures.loadTextureFromMemory(
        std::span<const std::uint8_t>(rgbaAtlas.data(), rgbaAtlas.size()),
        static_cast<uint32_t>(atlasSize),
        static_cast<uint32_t>(atlasSize)
    );

    int ascent = 0;
    int descent = 0;
    int lineGap = 0;

    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    const float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelSize);

    data.lineHeight = static_cast<float>(ascent - descent + lineGap) * scale;

    for (int i = 0; i < GlyphCount; ++i) {
        const char32_t codepoint = static_cast<char32_t>(FirstCodepoint + i);
        const stbtt_packedchar& packed = packedGlyphs[i];
        Glyph glyph{};

        glyph.textureRect = {
            static_cast<uint16_t>(packed.x0),
            static_cast<uint16_t>(packed.y0),
            static_cast<uint16_t>(packed.x1 - packed.x0),
            static_cast<uint16_t>(packed.y1 - packed.y0)
        };

        glyph.size = {packed.xoff2 - packed.xoff, packed.yoff2 - packed.yoff };

        glyph.bearing = {packed.xoff, packed.yoff};

        glyph.advance = packed.xadvance;

        data.glyphs.emplace(codepoint, glyph);
    }

    const uint32_t index = static_cast<uint32_t>(_fonts.size());
    _fonts.push_back(std::move(data));

    return Font(index, _fonts[index].generation);
}

const FontData* FontManager::getFont(Font font) const
{
    if (!font.isValid())
        return nullptr;

    if (font.getIndex() >= _fonts.size())
        return nullptr;

    const FontData& data = _fonts[font.getIndex()];

    if (!data.active)
        return nullptr;

    if (data.generation != font.getGeneration())
        return nullptr;

    return &data;
}


Text FontManager::createText(const std::string& content, Font font, const vec2& position, const ColorRGBA8& color) const
{
    Text text(font, content);
    text._position = position;
    text._color = color;

    const FontData* fontData = getFont(font);
    if (!fontData) {
        return text;
    }

    std::vector<SpriteData> sprites;
    sprites.reserve(content.size());

    float cursorX = position.x;
    float cursorY = position.y;

    for (const unsigned char character : content) {
        if (character == '\n') {
            cursorX = position.x;
            cursorY += fontData->lineHeight;
            continue;
        }

        const auto glyphIterator = fontData->glyphs.find(static_cast<char32_t>(character));
        if (glyphIterator == fontData->glyphs.end())
            continue;

        const Glyph& glyph = glyphIterator->second;

        if (glyph.size.x > 0.f && glyph.size.y > 0.f) {
            Sprite sprite(fontData->atlas);

            sprite.setPosition({
                cursorX + glyph.bearing.x,
                cursorY + glyph.bearing.y
            });

            sprite.setSize(glyph.size);
            sprite.setTextureRect(glyph.textureRect);
            sprite.setColor(color);
            sprite.setLayer(0);

            sprites.push_back(sprite.data());
        }

        cursorX += glyph.advance;
    }

    text._glyphs = std::move(sprites);
    return text;
}

} // namespace rtk
