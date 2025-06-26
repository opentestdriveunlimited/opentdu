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
    colorBankPath += "Vehicules/colors.bnk";

    bool bLoadingResult = loadBank( gpFile, colorBankPath.c_str() );
    if ( !bLoadingResult ) {
        return false;
    }

    loadContentAsync();

    p2DMFileResource = (char*)getFirstEntry( 0x2, 0xe, nullptr );
    render2DM.initialize( p2DMFileResource );

    collection2D.register2DM( &render2DM );

    return true;
}

void GSCarColors::terminate()
{
    collection2D.unregister2DM( &render2DM );
    render2DM.destroy();

    TestDrive::Free( pBankFile );
    pBankFile = nullptr;
    bLoaded = false;
}
