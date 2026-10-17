#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <list>

#include "audio/sound.hpp"

namespace rtk {

class SoundManager {
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;

public:
    SoundManager();
    ~SoundManager();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    Sound addSound(const std::string& path);
    Sound getSound(int index);
    Sound getSound(const std::string& path);

    void removeSound(Sound sound);
    void playSound(Sound sound);

    void update(); 
};

} // namespace rtk