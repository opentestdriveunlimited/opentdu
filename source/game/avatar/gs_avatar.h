#pragma once

#include "game/gs_base.h"

class GSAvatar : public GameSystem {
public:
    const char* getName() const override { return "Service : Avatar"; }

public:
    GSAvatar();
    ~GSAvatar();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSAvatar* gpAvatar;
