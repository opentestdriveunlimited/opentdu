#pragma once

#include "game/gs_base.h"

class GSSubtitles : public GameSystem {
public:
    const char* getName() const override { return "Service : Subtitles"; }

public:
    GSSubtitles();
    ~GSSubtitles();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
};

extern GSSubtitles* gpSubtitles;
