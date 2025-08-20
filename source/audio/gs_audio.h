#pragma once

#include "game/gs_base.h"

struct TDUMixer {

};

class GSAudio : public GameSystem {
public:
    int32_t NbSpeakers;
    float OutputSpatialFactor;
    float OutputDopplerFactor;
    
public:
    const char* getName() const override { return "[GSAudio]"; }

public:
    GSAudio();
    ~GSAudio();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float totalTime, float deltaTime) override;
    void terminate() override;

    void addMixer( TDUMixer* mixer );
};

extern GSAudio* gpAudio;
