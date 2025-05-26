#pragma once

#include "game/mng_base.h"

class MngHud : public Manager {
public:
    const char* getName() const override { return "Hud"; }

public:
    MngHud();
    ~MngHud();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float )  override;
    virtual void terminate()  override;

    void updateSettings();
};

extern MngHud* gpMngHud;
