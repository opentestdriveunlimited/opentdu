#include "shared.h"
#include "gs_vehicle_sound.h"

GSVehicleSound* gpVehicleSound = nullptr;

GSVehicleSound::GSVehicleSound()
    : GameSystem()
{
    gpVehicleSound = this;
}

GSVehicleSound::~GSVehicleSound()
{

}

bool GSVehicleSound::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSVehicleSound::terminate()
{

}
