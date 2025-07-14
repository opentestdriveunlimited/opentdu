#pragma once

#include "game/gm_base.h"

class MngMovie;

class GMVideoBumper : public GameMode {
public:
    GMVideoBumper();
    ~GMVideoBumper();

    void tick(float deltaTime);

private:
    MngMovie* movieManager;
};

