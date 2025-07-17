#pragma once

#include "game/gs_base.h"

class GSCinematics : public GameSystem {
public:
    const char* getName() const override { return "Service : Cinematics"; }

public:
    GSCinematics();
    ~GSCinematics();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSCinematics* gpCinematics;
