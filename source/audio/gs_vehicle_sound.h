#pragma once

#include "game/gs_base.h"

class GSVehicleSound : public GameSystem {
public:
    const char* getName() const override { return "Service : Vehicle Sound"; }

public:
    GSVehicleSound();
    ~GSVehicleSound();
};

extern GSVehicleSound* gpVehicleSound;
