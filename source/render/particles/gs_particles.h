#pragma once

#include "game/gs_base.h"

class GSParticles : public GameSystem {
public:
    const char* getName() const override { return "Service : Particles"; }

public:
    GSParticles();
    ~GSParticles();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};
