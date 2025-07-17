#pragma once

#include "game/gs_base.h"

class GSWorldStreaming : public GameSystem {
public:
    const char* getName() const override { return "NoName"; }

public:
    GSWorldStreaming();
    ~GSWorldStreaming();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSWorldStreaming* gpWorldStreaming;