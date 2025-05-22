#pragma once

#include "game/gs_base.h"
#include "bank.h"

class TestDriveMutex;

class GSBootDatas : public GameSystem, public Bank {
public:
    const char* getName() const override { return "BootDatas"; }

public:
    GSBootDatas();
    ~GSBootDatas();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;

    void loadDataBanks();

private:
    uint8_t bInitialized : 1;
};

extern GSBootDatas* gpBootDatas;
