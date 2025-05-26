#include "shared.h"
#include "saved_garage.h"

SavedGarage::SavedGarage()
    : Notifiable()
    , currentCarIndex( 0xffffffff )
{

}

SavedGarage::~SavedGarage()
{

}
