#pragma once

#include "game/gs_base.h"

class AudioDevice {
public:
    AudioDevice();

    void setGlobalVolume(float param_2);
};

extern AudioDevice gAudioDevice;