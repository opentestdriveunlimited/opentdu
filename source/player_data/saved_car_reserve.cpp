#include "shared.h"
#include "saved_car_reserve.h"

SavedCarReserve::SavedCarReserve()
    : timeSinceRestock( 0 )
    , lastRestock( 0 )
    , unknown( 0 )
    , seedUpdate( 0 )
    , seedRestock( 0 )
{
    
}

SavedCarReserve::~SavedCarReserve()
{

}
