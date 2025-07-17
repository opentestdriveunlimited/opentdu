#include "shared.h"
#include "gs_mng_resources.h"

GSMngResources* gpMngResources = nullptr;

GSMngResources::GSMngResources()
    : GameSystem()
{
    OTDU_ASSERT( gpMngResources == nullptr );
    gpMngResources = this;
}

GSMngResources::~GSMngResources()
{

}

bool GSMngResources::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSMngResources::terminate()
{

}