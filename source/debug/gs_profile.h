#pragma once

#include "game/gs_base.h"

class GSProfile : public GameSystem {
public:
    const char* getName() const override { return "Service : Profile"; }

public:
    GSProfile();
    ~GSProfile();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float totalTime, float deltaTime) override;
    void terminate() override;
};

