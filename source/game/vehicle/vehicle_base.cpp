#include "shared.h"
#include "vehicle_base.h"

VehicleBase::VehicleBase()
    : pCarState( nullptr )
    , numHiearchyNodes( 0 )
    , bValidPosition( false )
    , bLightsDirty( false )
    , bInitialized( false )
    , bShared( false )
    , bShouldDraw( false )
    , bVisible( false )
    , bNeedToUnstream( false )
    , bUseHQInterior( false )
    , bUnknownFlag( false )
{

}

VehicleBase::~VehicleBase()
{
}
