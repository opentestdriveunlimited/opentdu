#pragma once

#include "game/gs_base.h"

class TestDriveThread;

class GSDirtyDisk : public GameSystem {
public:
    const char* getName() const override { return "BootDatas"; }

public:
    GSDirtyDisk();
    ~GSDirtyDisk();

    bool initialize( TestDriveGameInstance* ) override { return true; }
    void terminate() override { }
    void onDataLoadFailure();

private:
    TestDriveThread* pThread;
    uint8_t bFailureReported : 1;
};

extern GSDirtyDisk* gpDirtyDisk;
