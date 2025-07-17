#pragma once

#include "game/gs_base.h"

class GSCarCommon : public GameSystem {
public:
    const char* getName() const override { return "GSCarCommon"; }

public:
    GSCarCommon();
    ~GSCarCommon();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSCarCommon* gpCarCommon;
