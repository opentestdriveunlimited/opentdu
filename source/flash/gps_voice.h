#pragma once

#include "filesystem/bank.h"

class GPSVoice : public Bank {
public:
    GPSVoice();
    ~GPSVoice();

    bool initialize();

private:
    enum class eAudioState {
        GVAS_Unknown = 0,
        GVAS_WaitingForParsing = 1,
        GVAS_Ready = 2
    };

    struct Sample {
        void* pContent = nullptr;
        char* pName = nullptr;
    };

private:
    void* pMemoryPool;
    eAudioState audioState;
    std::vector<Sample> audioSamples;
    uint8_t bEnabled : 1;
};
