#include "audio/soundManager.hpp"
#include "Logger/Logger.hpp"
#include "utils/fileExists.hpp"

    SoundManager::SoundManager() {
        if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
            LOG_FATAL("Failed to initialize miniaudio.");
        }
    }

    SoundManager::~SoundManager() {
        for (auto& data : sounds) {
            if (data && (data->active || data->dying)) {
                ma_sound_stop(&data->audio);
                ma_sound_uninit(&data->audio);
            }
        }
        sounds.clear();
        ma_engine_uninit(&engine);
    }

    Sound SoundManager::addSound(const std::string& path) {
        if (pathToIndex.find(path) != pathToIndex.end()) {
            int index = pathToIndex[path];
            return Sound(index, sounds[index]->generation);
        }

        if (!rtk::utils::fileExists(path)) {
            LOG_ERROR("File not found:" + path);
            return Sound(-1, -1);
        }

        for (size_t i = 0; i < sounds.size(); ++i) {
            if (!sounds[i]->active && !sounds[i]->dying) {
                if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &sounds[i]->audio) != MA_SUCCESS) {
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
        if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &newData->audio) != MA_SUCCESS) {
            LOG_ERROR("Miniaudio format error" + path);
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
                sounds[index]->dying = true;
                pathToIndex.erase(sounds[index]->path);
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

        ma_sound_seek_to_pcm_frame(&sounds[index]->audio, 0);
        ma_sound_start(&sounds[index]->audio);
    }

void SoundManager::update() {
    for (auto& data : sounds) {
        if (data && data->dying) {
            if (ma_sound_at_end(&data->audio) || !ma_sound_is_playing(&data->audio)) {
                ma_sound_uninit(&data->audio);
                data->dying = false;
            }
        }
    }
}