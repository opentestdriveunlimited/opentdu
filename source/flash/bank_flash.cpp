#include "shared.h"
#include "bank_flash.h"

#include "render/draw_list.h"

BankFlash::BankFlash()
    : Bank()
    , StreamedResource()
    , pFilepath( nullptr )
    , pFilename( nullptr )
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
