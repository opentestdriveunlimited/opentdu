#pragma once

#include "game/gs_base.h"

class GSPCOptions : public GameSystem {
public:
    const char* getName() const override { return "PC_Options"; }

public:
    GSPCOptions();
    ~GSPCOptions();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSPCOptions* gpPCOptions;
