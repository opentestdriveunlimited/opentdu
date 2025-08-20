#pragma once

#include "game/gs_base.h"

// TODO: Move those globals to the device service instance
static uint32_t gNbMaxKeyboards = 1;
static uint32_t gNbMaxMice = 1;
static uint32_t gNbMaxJoysticks = 1;
static uint32_t gNbMaxActions = 0;
static uint32_t gDefaultSensitivity = 8;

static float gFFBFactor = 0.0f;
static float gSteeringDeadZone = 0.20f;
static float gSteeringLinearity = 50.0f;
static float gSteeringTurn = 270.0f;
static float gBrakeDeadZone = 0.20f;
static float gAccelDeadZone = 0.20f;
static float gFFBLoadFactor[3] = { 0.0f, 0.0f, 0.0f };
static float gFFBLoadFromAccel[3] = { 0.0f, 0.0f, 0.0f };
static bool gPedalsSeparated = false;

class GSDevice : public GameSystem {
public:
    const char* getName() const override { return "Service : Device"; }

public:
    GSDevice();
    ~GSDevice();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float totalTime, float deltaTime) override;
    void terminate() override;
};
