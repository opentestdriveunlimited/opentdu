#pragma once

#include "game/gs_base.h"

class GSBoat : public GameSystem {
public:
    const char* getName() const override { return "Service : Boat"; }

public:
    GSBoat();
    ~GSBoat();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSBoat* gpBoat;