#include "shared.h"
#include "gs_boot_datas.h"

#include "config/gs_config.h"

GSBootDatas* gpBootDatas = nullptr;

GSBootDatas::GSBootDatas()
    : GameSystem()
    , Bank()
    , bInitialized( false )
{
    gpBootDatas = this;
}

GSBootDatas::~GSBootDatas()
{

}

bool GSBootDatas::initialize( TestDriveGameInstance* pInstance )
{
    pGameInstance = pInstance;
    bPaused = false;
    bInitialized = false;
    return true;
}

void GSBootDatas::terminate()
{
    pBankFile = nullptr;
    bLoaded = false;
    bInitialized = false;
}

void GSBootDatas::loadDataBanks()
{
    if ( bInitialized ) {
        return;
    }

    const char* pVFSRoot = gpConfig->getResRootPath();

    std::string bankPath = pVFSRoot;
    bankPath += "Boot/Datas.bnk";
        
    if ( !bLoaded ) {
        OTDU_UNIMPLEMENTED; // TODO:
    }

    bInitialized = true;
}
