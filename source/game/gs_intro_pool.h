#pragma once

#include "game/gs_base.h"

class GSIntroPool : public GameSystem {
public:
    const char* getName() const override { return "GSIntroPool"; }

public:
    GSIntroPool();
    ~GSIntroPool();

    bool initialize( TestDriveGameInstance* pGameInstance ) override;
    void tick() override {}
    void terminate() override;

private:
    void* pIntroPool;
};

extern GSIntroPool* gpIntroPool;