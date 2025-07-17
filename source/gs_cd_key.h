#pragma once

#include "game/gs_base.h"

class GSCDKey : public GameSystem {
public:
    const char* getName() const override { return "NoName"; }

public:
    GSCDKey();
    ~GSCDKey();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSCDKey* gpCDKey;
