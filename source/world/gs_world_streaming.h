#pragma once

#include "game/gs_base.h"

class GSWorldStreaming : public GameSystem {
public:
    const char* getName() const override { return "NoName"; }

public:
    GSWorldStreaming();
    ~GSWorldStreaming();
};

extern GSWorldStreaming* gpWorldStreaming;