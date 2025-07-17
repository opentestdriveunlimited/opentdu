#pragma once

#include "game/gs_base.h"

class GSClothSystem : public GameSystem {
public:
    const char* getName() const override { return "Service : ClothSystem"; }

public:
    GSClothSystem();
    ~GSClothSystem();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSClothSystem* gpClothSystem;
