#include "shared.h"
#include "gs_car_state.h"

GSCarState* gpCarState = nullptr;

GSCarState::GSCarState()
    : GameSystem()
{
    gpCarState = this;
}

GSCarState::~GSCarState()
{

}

bool GSCarState::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSCarState::terminate()
{

}
