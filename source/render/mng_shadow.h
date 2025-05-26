#pragma once

#include "game/mng_base.h"

class MngShadow : public Manager {
public:
    const char* getName() const override { return "Shadow"; }

public:
    MngShadow();
    ~MngShadow();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float ) override;
    virtual void draw() override;
    virtual void terminate()  override;
};

extern MngShadow* gpMngShadow;
