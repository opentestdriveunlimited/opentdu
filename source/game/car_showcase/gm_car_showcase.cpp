#include "shared.h"
#include "gm_car_showcase.h"

#include "tdu_instance.h"
#include "render/draw_list.h"
#include "database/gs_database.h"

GMCarShowcase::GMCarShowcase()
    : GameMode()
    , pDrawList( new DrawList() )
    , rotationAngleY( 0.0f )
    , wheelRotationSpeed( 0.0f )
    , floorHeight( -0.25f )
    , backgroundColor( 29, 41, 44, 0xff )
    , clearColor( 0u )
    , currentCarHashcode( 0 )
    , doorAngle( 0.0f )
    , doorSpeed( 1.0f )
    , doorMinAngle( 0.3f )
    , numLights( 0 )
    , pVehicleState( nullptr )
    , flashList( this )
    , mngFlash()
    , mngNumber()
    , currentCarID( 0 )
    , currentCarColor( 0 )
    , currentCarInterior( 0 )
    , currentCarRims( 0 )
    , ambient( Eigen::Vector4f::Zero() )
    , diffuse( Eigen::Vector4f::Zero() )
    , specular( Eigen::Vector4f::Zero() )
    , streamOrigin( Eigen::Vector4f::Zero() )
    , bUsePhysicsInput( false )
    , bFreezeCarSwitch( false ) 
{
    registerManager(&mngFlash);
    registerManager(&mngNumber);
}

GMCarShowcase::~GMCarShowcase()
{
    delete pDrawList;
}

void GMCarShowcase::initialize()
{
    carHashes.reserve( gpDatabase->getNumCars() );
    for ( const CarConfig& car : gpDatabase->getCarList() ) {
        carHashes.push_back( car.Hash );
    }
    OTDU_LOG_DEBUG("Found %u vehicles in database\n");

    pDrawList->initialize(0x20, 0x200, 0, true, nullptr, nullptr, false);
}

void GMCarShowcase::reset()
{
    OTDU_UNIMPLEMENTED;
}

bool GMCarShowcase::onMessage(FlashMessage &pMessage, FlashPlayer *pPlayer)
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
