#pragma once

#include "game/gs_base.h"

class GSScenaric : public GameSystem {
public:
    uint8_t bIsRoadLocked : 1;
    uint32_t TraceSize;

public:
    const char* getName() const override { return "Service : Scenaric"; }

public:
    GSScenaric();
    ~GSScenaric();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;
};

extern GSScenaric* gpScenaric;