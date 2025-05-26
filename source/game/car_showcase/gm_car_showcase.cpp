#include "shared.h"
#include "gm_car_showcase.h"

#include "tdu_instance.h"

GMCarShowcase::GMCarShowcase()
    : GameMode()
{

}

GMCarShowcase::~GMCarShowcase()
{

}

bool GMCarShowcase::onMessage(FlashMessage& pMessage, FlashPlayer *pPlayer)
{
    if (bFreezeCarSwitch) {
        return false;
    }

    switch (pMessage) {
        case GMCarShowcase::kExit: {
            bExitRequested = true;
            gpTestDriveInstance->setNextGameMode(GM_Home);
            return true;
        }
        default:
            OTDU_LOG_WARN("Unknown flash message '%s' (%x)\n", pMessage.pAsChar, pMessage.AsInteger);
            break;
    }

    return false;
}
