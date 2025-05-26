#pragma once

#include "shared.h"
#include "game/gs_base.h"

class GSTimer : public GameSystem
{
public:
    GSTimer()
    : GameSystem()
    , SystemTotalTime( 0.0f )
    , SystemDeltaTime( 0.0f )
    , GameSpeed( 1.0f )
    , PreviousGameSpeed( 1.0f )
    , GameTotalTime( 0.0f )
    , GameDeltaTime( 0.0f )
    {

    }

    ~GSTimer()
    {

    }

    bool initialize( TestDriveGameInstance* ) override
    {
        return true;
    }

    void terminate() override
    {

    }

    float SystemTotalTime;
    float SystemDeltaTime;
    float GameSpeed;
    float PreviousGameSpeed;
    float GameTotalTime;
    float GameDeltaTime;
};

static GSTimer gGSTimer = {};
