#pragma once

#include "game/gs_base.h"

class GSAmbiance : public GameSystem {
public:
    const char* getName() const override { return "Service : Ambiance"; }

public:
    GSAmbiance();
    ~GSAmbiance();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSAmbiance* gpAmbiance;
