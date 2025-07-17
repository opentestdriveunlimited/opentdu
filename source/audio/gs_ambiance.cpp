#include "shared.h"
#include "gs_ambiance.h"

GSAmbiance* gpAmbiance = nullptr;

GSAmbiance::GSAmbiance()
    : GameSystem()
{
    gpAmbiance = this;
}

GSAmbiance::~GSAmbiance()
{

}

bool GSAmbiance::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSAmbiance::terminate()
{

}
