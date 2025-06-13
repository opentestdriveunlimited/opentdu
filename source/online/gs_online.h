#pragma once

namespace TestDrive
{
    namespace CRC
    {
        static uint32_t Protocol = 1;
    }
}
#include "game/gs_base.h"

class GSOnline : public GameSystem {
public:
    const char* getName() const override { return "Service : Online"; }

public:
    GSOnline();
    ~GSOnline();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    void setFreeRideVersion( const char* pVersion );

private:
    char pFreerideVersion[32];
};

extern GSOnline* gpOnline;
