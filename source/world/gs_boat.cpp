#include "shared.h"
#include "gs_boat.h"

GSBoat* gpBoat = nullptr;

GSBoat::GSBoat()
    : GameSystem()
{
    OTDU_ASSERT( gpBoat == nullptr );
    gpBoat = this;
}

GSBoat::~GSBoat()
{

}

bool GSBoat::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSBoat::terminate()
{

}