#include "audio/soundManager.hpp"
#include "utils/fileExists.hpp"
#include "Logger/Logger.hpp"
#include <filesystem>
#include <miniaudio.h>
#include <unordered_map>
#include <vector>
#include <list>
#include <string>

namespace rtk {

struct SoundManager::Impl {
    struct SoundData {
        ma_sound templateAudio;
        std::list<ma_sound> activeVoices;
        int generation = 0;
        std::string path;
        bool active = false;
    };

    ma_engine engine;
    std::unordered_map<std::string, int> pathToIndex;
    std::vector<std::unique_ptr<SoundData>> sounds;
};

SoundManager::SoundManager() : pimpl(std::make_unique<Impl>()) {
    if (ma_engine_init(NULL, &pimpl->engine) != MA_SUCCESS) {
        LOG_FATAL("Failed to initialize miniaudio.");
    }
}

SoundManager::~SoundManager() {
    for (auto& data : pimpl->sounds) {
        if (data) {
            for (auto& voice : data->activeVoices) {
                ma_sound_stop(&voice);
                ma_sound_uninit(&voice);
            }
            data->activeVoices.clear();

            if (!data->path.empty()) {
                ma_sound_uninit(&data->templateAudio);
            }
        }
    }
    pimpl->sounds.clear();
    ma_engine_uninit(&pimpl->engine);
}

Sound SoundManager::addSound(const std::string& path) {
    if (pimpl->pathToIndex.find(path) != pimpl->pathToIndex.end()) {
        int index = pimpl->pathToIndex[path];

        if (!pimpl->sounds[index]->active) {
            pimpl->sounds[index]->active = true;
            pimpl->sounds[index]->generation++;
        }
        return Sound(index, pimpl->sounds[index]->generation);
    }

    if (!rtk::utils::fileExists(path)) {
        LOG_ERROR("File not found:" + path);
        return Sound(-1, -1);
    }

    for (size_t i = 0; i < pimpl->sounds.size(); ++i) {
        if (!pimpl->sounds[i]->active && pimpl->sounds[i]->activeVoices.empty()) {
            if (ma_sound_init_from_file(&pimpl->engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &pimpl->sounds[i]->templateAudio) != MA_SUCCESS) {
                LOG_ERROR("Miniaudio format error: " + path);
                return Sound(-1, -1);
            }
            pimpl->sounds[i]->path = path;
            pimpl->sounds[i]->active = true;
            pimpl->sounds[i]->generation++;
            pimpl->pathToIndex[path] = i;
            return Sound(i, pimpl->sounds[i]->generation);
        }
    }

    auto newData = std::make_unique<Impl::SoundData>();
    if (ma_sound_init_from_file(&pimpl->engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &newData->templateAudio) != MA_SUCCESS) {
        LOG_ERROR("Miniaudio format error: " + path);
        return Sound(-1, -1);
    }

    int index = static_cast<int>(pimpl->sounds.size());
    int generation = 1;

    newData->generation = generation;
    newData->path = path;
    newData->active = true;

    pimpl->sounds.push_back(std::move(newData));
    pimpl->pathToIndex[path] = index;

    return Sound(index, generation);
}

void SoundManager::removeSound(Sound sound) {
    if (!sound.isValid()) return;
    int index = sound.getIndex();

    if (index >= 0 && index < static_cast<int>(pimpl->sounds.size())) {
        if (pimpl->sounds[index]->generation == sound.getGeneration() && pimpl->sounds[index]->active) {
            pimpl->sounds[index]->active = false;
        }
    }
}

void SoundManager::playSound(Sound sound) {
    if (!sound.isValid()) return;
    int index = sound.getIndex();

    if (index < 0 || index >= static_cast<int>(pimpl->sounds.size()) || !pimpl->sounds[index]->active) {
        LOG_WARN("Play failed: Index out of bounds or inactive.");
        return;
    }
    if (pimpl->sounds[index]->generation != sound.getGeneration()) {
        LOG_WARN("Play failed: Sound has expired (invalid generation).");
        return;
    }

    pimpl->sounds[index]->activeVoices.emplace_back();

    ma_sound* newVoicePtr = &pimpl->sounds[index]->activeVoices.back();
    if (ma_sound_init_copy(&pimpl->engine, &pimpl->sounds[index]->templateAudio, 0, NULL, newVoicePtr) == MA_SUCCESS) {
        ma_sound_start(newVoicePtr);
    } else {
        LOG_ERROR("Play failed: Could not create sound copy from template.");
        pimpl->sounds[index]->activeVoices.pop_back();
    }
}

void SoundManager::update() {
    for (auto& data : pimpl->sounds) {
        if (!data) continue;

        for (auto it = data->activeVoices.begin(); it != data->activeVoices.end(); ) {
            if (ma_sound_at_end(&*it) || !ma_sound_is_playing(&*it)) {
                ma_sound_uninit(&*it);
                it = data->activeVoices.erase(it);
            } else {
                ++it;
            }
        }

        if (!data->active && data->activeVoices.empty()) {
            if (!data->path.empty()) {
                pimpl->pathToIndex.erase(data->path);
                ma_sound_uninit(&data->templateAudio);
                data->path.clear();
                LOG_DEBUG("Dead sound template cleanly uninitialized and memory freed.");
            }
        }
    }
}

} // namespace rtk