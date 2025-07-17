#pragma once

#include "game/gs_base.h"

class GSCarState : public GameSystem {
public:
    const char* getName() const override { return "CAR_STATE"; }

public:
    GSCarState();
    ~GSCarState();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSCarState* gpCarState;
