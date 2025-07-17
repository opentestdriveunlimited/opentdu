#pragma once

#include "game/gs_base.h"

class GSBrandCar : public GameSystem {
public:
    const char* getName() const override { return "Service : Brand Car"; }

public:
    GSBrandCar();
    ~GSBrandCar();
};

extern GSBrandCar* gpBrandCar;
