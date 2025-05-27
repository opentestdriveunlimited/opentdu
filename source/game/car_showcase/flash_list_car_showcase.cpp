#include "shared.h"
#include "flash_list_car_showcase.h"

#include "gm_car_showcase.h"

FlashListCarShowcase::FlashListCarShowcase(GMCarShowcase* pInstance)
    : FlashList()
    , pGameModeInstance( pInstance )
{

}

FlashListCarShowcase::~FlashListCarShowcase()
{

}
