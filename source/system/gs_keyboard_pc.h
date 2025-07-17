#pragma once

#include "game/gs_base.h"

class GSKeyboardPC : public GameSystem {
public:
    const char* getName() const override { return "Service : Keyboard PC"; }

public:
    GSKeyboardPC();
    ~GSKeyboardPC();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSKeyboardPC* gpKeyboardPC;
