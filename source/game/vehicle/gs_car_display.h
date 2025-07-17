#pragma once

#include "game/gs_base.h"

class GSCarDisplay : public GameSystem {
public:
    const char* getName() const override { return "Service : CarDisplay"; }

public:
    GSCarDisplay();
    ~GSCarDisplay();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSCarDisplay* gpCarDisplay;
