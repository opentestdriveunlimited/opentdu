#pragma once

#include "game/gm_base.h"

class GMFreeride : public GameMode {
public:
    GMFreeride();
    ~GMFreeride();

    void tick(float deltaTime);
};
