#pragma once

#include "game/gs_base.h"

class GSPostFX : public GameSystem {
public:
    const char* getName() const override { return "GSPostFX"; }

public:
    GSPostFX();
    ~GSPostFX();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};
