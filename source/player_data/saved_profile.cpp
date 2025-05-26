#include "shared.h"
#include "saved_profile.h"

SavedProfile::SavedProfile()
    : profileIndex( 0xffffffff )
    , clubIndex( 0 )
    , clubSpotIndex( 0 )
    , money( 0 )
    , numCoupons( 0 )
    , pendingMoneyOperation( 0 )
    , moneyOperationTimer( 0.0f )
    , currentCarIndex( 0 )
    , tutorialState( 0 )
    , unknownDword( 0 )
    , unknownDword2( 0 )
    , currentHouseHashcode( 0 )
    , lastNewsIndex( 0 )
    , level( 0.0f )
    , odometer( 0.0f )
    , bUnknownFlag( true )
{
    memset( pNickname, 0, sizeof( char ) * 32 );
    memset( pClubName, 0, sizeof( char ) * 16 );
}

SavedProfile::~SavedProfile()
{

}
