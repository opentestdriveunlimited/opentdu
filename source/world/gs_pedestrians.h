#pragma once

#include "game/gs_base.h"

class GSPedestrians : public GameSystem {
public:
    const char* getName() const override { return "Service : Pedestrian"; }

public:
    GSPedestrians();
    ~GSPedestrians();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSPedestrians* gpPedestrians;