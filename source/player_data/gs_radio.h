#pragma once

#include "game/gs_base.h"

class GSRadio : public GameSystem {
public:
    const char* getName() const override { return "[GSRadio]"; }

public:
    GSRadio();
    ~GSRadio();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSRadio* gpRadio;
