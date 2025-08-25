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

bool CarState::isReversing() const
{
    return body.bIsReversing;
}

bool CarState::isBraking() const
{
    return body.bIsBraking;
}

bool CarState::isIndicatingLeft() const
{
    return body.bIndicatingLeft;
}

bool CarState::isIndicatingRight() const
{
    return body.bIndicatingRight;
}

bool CarState::headlightsOn() const
{
    return body.bHeadlightsOn;
}

bool CarState::hazardLightsOn() const
{
    return body.bHazardLightsOn;
}
