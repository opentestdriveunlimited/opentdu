#pragma once

#include "game/gs_base.h"

class GSNoise : public GameSystem {
public:
    const char* getName() const override { return "GSNoise"; }

public:
    GSNoise();
    ~GSNoise();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};
