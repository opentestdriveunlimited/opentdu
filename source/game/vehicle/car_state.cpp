#include "shared.h"
#include "car_state.h"

CarState::CarState()
    : GameObject()
    , name( "UNKNOWN" )
    , carInstanceID( 0xffffffff )
    , numWheels( 0 )
    , carID( 0xffffffff )
    , tireID( 0xffffffff )
    , rimID( 0xffffffff )
    , colorID( 0xffffffff )
    , bInstanceAlive( false )
    , bIsIndoor( false )
    , bUseShowcaseControl( false )
    , packHashcode( 0 )
{
    
}

CarState::~CarState()
{
}

Eigen::Vector3f CarState::getWorldPosition() const
{
    Eigen::Vector4f position = body.WorldMatrix.row(3);
    return Eigen::Vector3f(position.x(), position.y(), position.z());
}

bool CarState::isIndoor() const
{
    return bIsIndoor;
}
