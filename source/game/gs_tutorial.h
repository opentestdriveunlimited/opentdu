#pragma once

#include "game/gs_base.h"

class GSTutorial : public GameSystem {
public:
    const char* getName() const override { return "Service : Tutorial"; }

public:
    GSTutorial();
    ~GSTutorial();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSTutorial* gpTutorial;
