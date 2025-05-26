#pragma once

#include "flash/flash_list.h"

class GMCarShowcase;

class FlashListCarShowcase : public FlashList 
{
public:
    FlashListCarShowcase();
    virtual ~FlashListCarShowcase();

private:
    GMCarShowcase* pGameModeInstance;
};
