#pragma once

#include "game/gs_base.h"

class GSAI : public GameSystem {
public:
    const char* getName() const override { return "Service : AI"; }

public:
    GSAI();
    ~GSAI();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSAI* gpAI;