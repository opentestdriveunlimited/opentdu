#pragma once

#include "flash/mng_flash.h"

class GMVideoBumper;

class MngVideoBumper : public MngFlash {
public:
    MngVideoBumper(GMVideoBumper* param_1);
    ~MngVideoBumper();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float )  override;
    virtual void terminate()  override;

private:
    GMVideoBumper* pParent;
};
