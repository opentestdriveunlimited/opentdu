#include "shared.h"
#include "bank_flash.h"

#include "render/draw_list.h"

BankFlash::BankFlash()
    : Bank()
    , StreamedResource()
    , filepath( "" )
    , filename( "" )
    , pCommonBank( nullptr )
    , pFlashPlayer( nullptr )
    , drawList( new DrawList() )
    , maxNumStrips( 0 )
    , maxNumVertices( 0 )
    , maxNumFlashVars( 0 )
    , state( eBankFlashInit::BFI_Unknown )
    , bInUse( false )
    , bLoaded( false )
{

}

BankFlash::~BankFlash()
{
    delete drawList;
}

void BankFlash::setUsed() 
{ 
    OTDU_ASSERT(!isInUse()); 
    bInUse = true; 
}

void BankFlash::setCommonBankRef(BankCommonFlash *pBankRef)
{
    pCommonBank = pBankRef;
}

void BankFlash::setMaxNumVars(const uint32_t numMaxVertices, const uint32_t numMaxStrips, const uint32_t numMaxVars)
{
    maxNumVertices = numMaxVertices;
    maxNumStrips = numMaxStrips;
    maxNumFlashVars = numMaxVars;
}

void BankFlash::updateFilePaths(const char *pFilePath, const char *pFileName)
{
    filepath = pFilePath;
    filename = pFileName;
}
