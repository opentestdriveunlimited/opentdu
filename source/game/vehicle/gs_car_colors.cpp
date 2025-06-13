#include "shared.h"
#include "gs_car_colors.h"

#include "config/gs_config.h"
#include "filesystem/gs_file.h"

GSCarColors gCarColors = {};

GSCarColors::GSCarColors()
    : GameSystem()
    , Bank()
    , collection2D()
    , render2DM()
    , p2DMFileResource( nullptr )
{

}

GSCarColors::~GSCarColors()
{

}

bool GSCarColors::initialize( TestDriveGameInstance* )
{
    if ( bLoaded ) {
        return true;
    }

    std::string colorBankPath = gpConfig->getResRootPath();
    colorBankPath += "Vehicules\\colors.bnk";

    bool bLoadingResult = loadBank( gpFile, colorBankPath.c_str() );
    if ( !bLoadingResult ) {
        return false;
    }

    loadContentAsync();

    p2DMFileResource = (char*)getFirstEntry( 0xe, 0x2, nullptr );
    render2DM.initialize( p2DMFileResource );

    /*collection2D.AddToSet( render2DM );
    collection2D.Finalize( render2DM );*/

    return true;
}

void GSCarColors::terminate()
{
    //collection2D.RemoveFromSet( render2DM );
    //collection2D.Destroy();
    TestDrive::Free( pBankFile );
    pBankFile = nullptr;
    bLoaded = false;
}
