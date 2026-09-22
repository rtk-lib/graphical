#pragma once

#include <filesystem>

namespace rtk
{
    namespace utils
    {
        inline bool fileExists(const std::string& path) {
        return std::filesystem::exists(path);
    }
    }
}