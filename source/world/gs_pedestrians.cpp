#include "shared.h"
#include "gs_pedestrians.h"

GSPedestrians* gpPedestrians = nullptr;

GSPedestrians::GSPedestrians()
    : GameSystem()
{
    OTDU_ASSERT( gpPedestrians == nullptr );
    gpPedestrians = this;
}

GSPedestrians::~GSPedestrians()
{

}

bool GSPedestrians::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSPedestrians::terminate()
{

}