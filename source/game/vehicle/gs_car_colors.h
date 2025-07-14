#pragma once

#include "game/gs_base.h"
#include "filesystem/bank.h"
#include "render/file_collection_2d.h"
#include "render/2dm.h"

class GSCarColors : public GameSystem, public Bank {
public:
    const char* getName() const override { return "GServiceCarColors"; }

public:
    GSCarColors();
    ~GSCarColors();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;

private:
    FileCollection2D collection2D;
    Render2DM render2DM;
    char* p2DMFileResource;
};

extern GSCarColors* gpCarColors;
