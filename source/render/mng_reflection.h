#pragma once

#include "game/mng_base.h"

class MngReflection : public Manager {
public:
    const char* getName() const override { return "Shadow"; }

public:
    MngReflection();
    ~MngReflection();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float ) override;
    virtual void draw() override;
    virtual void terminate()  override;
};

extern MngReflection* gpMngReflection;
