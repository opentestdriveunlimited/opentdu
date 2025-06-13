#pragma once

#include "flash/mng_flash.h"

class GMCarShowcase;
class FlashPlayer;

class MngFlashCarShowcase : public MngFlash 
{
public:
    MngFlashCarShowcase();
    virtual ~MngFlashCarShowcase();

    virtual bool onMessage(FlashMessage& pMessage, FlashPlayer* pPlayer) override;

private:
    GMCarShowcase* pGameModeInstance;
    std::vector<int32_t> carList;
};
