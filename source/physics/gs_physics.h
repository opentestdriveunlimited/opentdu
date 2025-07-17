#pragma once

#include "game/gs_base.h"

class GSPhysics : public GameSystem {
public:
    const char* getName() const override { return "Service : Physics"; }

public:
    GSPhysics();
    ~GSPhysics();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSPhysics* gpPhysics;
