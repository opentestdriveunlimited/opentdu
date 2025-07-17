#pragma once

#include "game/gs_base.h"

class GSDetails : public GameSystem {
public:
    const char* getName() const override { return "Details"; }

public:
    GSDetails();
    ~GSDetails();
    
    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSDetails* gpDetails;