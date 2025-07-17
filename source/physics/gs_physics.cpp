#include "shared.h"
#include "gs_physics.h"

GSPhysics* gpPhysics = nullptr;

GSPhysics::GSPhysics()
    : GameSystem()
{
    gpPhysics = this;
}

GSPhysics::~GSPhysics()
{

}

bool GSPhysics::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSPhysics::terminate()
{

}
