#include "shared.h"
#include "mng_flash_car_showcase.h"

#include "gm_car_showcase.h"

MngFlashCarShowcase::MngFlashCarShowcase()
    : MngFlash()
    , pGameModeInstance( nullptr )
{

}

MngFlashCarShowcase::~MngFlashCarShowcase()
{

}

bool MngFlashCarShowcase::onMessage(char *pMessage, FlashPlayer *pPlayer)
{
    if (pGameModeInstance == nullptr) {
        return false;
    }

    return pGameModeInstance->onMessage(pMessage, pPlayer);
}
