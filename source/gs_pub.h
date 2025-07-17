#pragma once

#include "game/gs_base.h"

class GSPub : public GameSystem {
public:
    const char* getName() const override { return "Service : Pub"; }

public:
    GSPub();
    ~GSPub();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSPub* gpPub;
