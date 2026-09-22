#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "audio/sound.hpp"
#include "stb/miniaudio.h"

class SoundManager {
private:
    struct SoundData {
        ma_sound audio;
        int generation = 0;
        std::string path;
        bool active = false;
        bool dying = false;
    };

    ma_engine engine;
    std::unordered_map<std::string, int> pathToIndex;

    std::vector<std::unique_ptr<SoundData>> sounds;

public:
    SoundManager();
    ~SoundManager();

    Sound addSound(const std::string& path);
    Sound getSound(int index);
    Sound getSound(const std::string& path);

    void removeSound(Sound sound);
    void playSound(Sound sound);

    void update(); 
};