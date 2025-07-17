#pragma once

#include "game/gs_base.h"

class GSMngResources : public GameSystem {
public:
    const char* getName() const override { return "Service : MngResources"; }

public:
    GSMngResources();
    ~GSMngResources();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSMngResources* gpMngResources;