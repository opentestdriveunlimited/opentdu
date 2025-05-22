#include "shared.h"
#include "mng_flash.h"
#include "gs_flash.h"

#include "game/gm_base.h"

MngFlash::MngFlash()
    : Manager()
    , bInitialized( false )
    , bWantAsyncInstall( false )
    , bNeedDatabaseInit( false )
{
    flashWidgets.reserve( 8 );
}

MngFlash::~MngFlash()
{
    flashWidgets.clear();
}

bool MngFlash::initialize(TestDriveGameInstance *)
{
    gpFlash->initGameMode(gpActiveGameMode->getCurrentGameMode(), this, false);
    return true;
}

void MngFlash::tick(float, float)
{
}

void MngFlash::terminate()
{
}

void MngFlash::addWidget(const char pName[4], FlashEntity *pWidget)
{
    FlashWidget widget;
    widget.WidgetID = (((uint32_t)*pName << 8 | (uint32_t)pName[1]) << 8 | (uint32_t)pName[2]) << 8 | (uint32_t)pName[3];
    widget.pWidgetInstance = pWidget;

    flashWidgets.push_back( widget );
}

void MngFlash::removeWidgets()
{
    flashWidgets.clear();
}
