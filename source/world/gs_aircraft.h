#pragma once

#include "game/gs_base.h"

class GSAircraft : public GameSystem {
public:
    const char* getName() const override { return "Service : Aircraft"; }

public:
    GSAircraft();
    ~GSAircraft();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSAircraft* gpAircraft;