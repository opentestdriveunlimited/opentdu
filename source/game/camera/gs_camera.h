#pragma once

#include "game/gs_base.h"

class GSCamera : public GameSystem {
public:
    float DeadZone;

public:
    const char* getName() const override { return "Camera"; }

public:
    GSCamera();
    ~GSCamera();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

private:
    float NearPlane;
    float FarPlane;
};

extern GSCamera* gpCamera;