#include "audio/soundManager.hpp"
#include "utils/fileExists.hpp"
#include "Logger/Logger.hpp"
#include <filesystem>

    SoundManager::SoundManager() {
        if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
            LOG_FATAL("Failed to initialize miniaudio.");
        }
    }

    SoundManager::~SoundManager() {
        for (auto& data : sounds) {
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
        sounds.clear();
        ma_engine_uninit(&engine);
    }

    Sound SoundManager::addSound(const std::string& path) {
        if (pathToIndex.find(path) != pathToIndex.end()) {
            int index = pathToIndex[path];

            if (!sounds[index]->active) {
                sounds[index]->active = true;
                sounds[index]->generation++;
            }
            return Sound(index, sounds[index]->generation);
        }

        if (!rtk::utils::fileExists(path)) {
            LOG_ERROR("File not found:" + path);
            return Sound(-1, -1);
        }

        for (size_t i = 0; i < sounds.size(); ++i) {
            if (!sounds[i]->active && sounds[i]->activeVoices.empty()) {
                if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &sounds[i]->templateAudio) != MA_SUCCESS) {
                    LOG_ERROR("Miniaudio format error: " + path);
                    return Sound(-1, -1);
                }
                sounds[i]->path = path;
                sounds[i]->active = true;
                sounds[i]->generation++;
                pathToIndex[path] = i;
                return Sound(i, sounds[i]->generation);
            }
        }

        auto newData = std::make_unique<SoundData>();
        if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &newData->templateAudio) != MA_SUCCESS) {
            LOG_ERROR("Miniaudio format error: " + path);
            return Sound(-1, -1);
        }

        int index = static_cast<int>(sounds.size());
        int generation = 1;

        newData->generation = generation;
        newData->path = path;
        newData->active = true;

        sounds.push_back(std::move(newData));
        pathToIndex[path] = index;

        return Sound(index, generation);
    }

void SoundManager::removeSound(Sound sound) {
    if (!sound.isValid()) return;
    int index = sound.getIndex();

    if (index >= 0 && index < static_cast<int>(sounds.size())) {
        if (sounds[index]->generation == sound.getGeneration() && sounds[index]->active) {
            sounds[index]->active = false;
        }
    }
}

void SoundManager::playSound(Sound sound) {
    if (!sound.isValid()) return;
    int index = sound.getIndex();

    if (index < 0 || index >= static_cast<int>(sounds.size()) || !sounds[index]->active) {
        LOG_WARN("Play failed: Index out of bounds or inactive.");
        return;
    }
    if (sounds[index]->generation != sound.getGeneration()) {
        LOG_WARN("Play failed: Sound has expired (invalid generation).");
        return;
    }

    sounds[index]->activeVoices.emplace_back();

    ma_sound* newVoicePtr = &sounds[index]->activeVoices.back();
    if (ma_sound_init_copy(&engine, &sounds[index]->templateAudio, 0, NULL, newVoicePtr) == MA_SUCCESS) {
        ma_sound_start(newVoicePtr);
    } else {
        LOG_ERROR("Play failed: Could not create sound copy from template.");
        sounds[index]->activeVoices.pop_back();
    }
}

void SoundManager::update() {
    for (auto& data : sounds) {
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
                pathToIndex.erase(data->path);
                ma_sound_uninit(&data->templateAudio);
                data->path.clear();
                LOG_DEBUG("Dead sound template cleanly uninitialized and memory freed.");
            }
        }
    }
}