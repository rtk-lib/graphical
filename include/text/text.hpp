#pragma once

#include "font/font.hpp"
#include "sprite/spriteData.hpp"
#include "utils/vec2.hpp"
#include <string>
#include <vector>
#include <utility>

namespace rtk {

class Text {
public:
    Text() = default;
    Text(Font font, std::string content)
        : _font(font),
          _content(std::move(content))
    {
    }

    const std::vector<SpriteData>& getSprites() const {
        return _glyphs;
    }

    Font _font;
    std::string _content;

    vec2 _position{};
    ColorRGBA8 _color{255, 255, 255, 255};
    int32_t _layer = 0;

    std::vector<SpriteData> _glyphs;
};

} // namespace rtk
