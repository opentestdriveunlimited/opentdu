#pragma once

#include "game/gs_base.h"

class GSConsole : public GameSystem {
public:
    const char* getName() const override { return "Service : Console"; }

public:
    GSConsole();
    ~GSConsole();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;
};

