#include "shared.h"
#include "gs_physicsfx.h"

GSPhysicsFX* gpPhysicsFX = nullptr;

GSPhysicsFX::GSPhysicsFX()
    : GameSystem()
{
    
}

GSPhysicsFX::~GSPhysicsFX()
{

}

bool GSPhysicsFX::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSPhysicsFX::terminate()
{

}