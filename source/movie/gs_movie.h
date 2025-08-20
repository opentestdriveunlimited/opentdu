#pragma once

#include "game/gs_base.h"

class GSMovie : public GameSystem {
public:
    const char* getName() const override { return "Service : Movie"; }

public:
    GSMovie();
    ~GSMovie();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float totalTime, float deltaTime) override;
    void terminate() override;
};

extern GSMovie* gpMovie;
