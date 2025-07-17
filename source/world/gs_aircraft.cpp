#include "shared.h"
#include "gs_aircraft.h"

GSAircraft* gpAircraft = nullptr;

GSAircraft::GSAircraft()
    : GameSystem()
{
    OTDU_ASSERT( gpAircraft == nullptr );
    gpAircraft = this;
}

GSAircraft::~GSAircraft()
{

}

bool GSAircraft::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSAircraft::terminate()
{

}