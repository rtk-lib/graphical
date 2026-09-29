#pragma once

class SoundManager;

class Sound {
private:
    int index;
    int generation;

    friend class SoundManager;
    Sound(int idx, int gen) : index(idx), generation(gen) {}

public:
    Sound() : index(-1), generation(-1) {}
    int getIndex() const { return index; }
    int getGeneration() const { return generation; }
    bool isValid() const { return index != -1 && generation != -1; }
};
