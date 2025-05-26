#include "shared.h"
#include "car_desc.h"

CarDesc::CarDesc()
    : carConfigID( 0 )
    , carID( 0 )
    , state( eState::CDS_None )
    , pConfig( nullptr )
    , upgradeLevel( 0.0f )
    , paintType( ePaintType::PT_Default )
    , paintIndex( 0u )
    , paintColor( 0u )
    , paintLogo( 0u )
    , paintSecondColor( 0u )
    , paintSecondColor2( 0u )
    , rimID( 0 )
    , interiorID( 0 )
    , price( 0 )
    , odometer( 0.0f )
    , usageTime( 0.0f )
    , packHashcode( 0ull )
{
}

CarDesc::~CarDesc()
{
}
