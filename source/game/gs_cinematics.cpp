#include "shared.h"
#include "gs_cinematics.h"

GSCinematics* gpCinematics = nullptr;

GSCinematics::GSCinematics()
    : GameSystem()
{
    gpCinematics = this;
}

GSCinematics::~GSCinematics()
{

}

bool GSCinematics::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSCinematics::terminate()
{

}
