#pragma once

#include "flash/flash_list.h"

class GMCarShowcase;

class FlashListCarShowcase : public FlashList 
{
public:
    FlashListCarShowcase(GMCarShowcase* pInstance);
    virtual ~FlashListCarShowcase();

private:
    GMCarShowcase* pGameModeInstance;
};
