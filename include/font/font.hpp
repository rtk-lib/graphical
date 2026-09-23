#pragma once

#include <cstdint>
#include <limits>

namespace rtk {

class FontManager;

class Font {
public:
    [[nodiscard]]
    constexpr uint32_t getIndex() const noexcept
    {
        return _index;
    }

    [[nodiscard]]
    constexpr uint32_t getGeneration() const noexcept
    {
        return _generation;
    }

    [[nodiscard]]
    constexpr bool isValid() const noexcept
    {
        return _index != InvalidIndex;
    }

    friend constexpr bool operator==( const Font& lhs, const Font& rhs) noexcept = default;

private:
    static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

    uint32_t _index = InvalidIndex;
    uint32_t _generation = 0;

    constexpr Font(uint32_t index, uint32_t generation) noexcept
    : _index(index),
        _generation(generation)
    {
    }

    friend class FontManager;
};

}