#include "shared.h"
#include "gs_scenaric.h"

GSScenaric* gpScenaric = nullptr;

GSScenaric::GSScenaric()
    : bIsRoadLocked( 1 )
    , TraceSize( 300 )
{

}

GSScenaric::~GSScenaric()
{

}

bool GSScenaric::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSScenaric::tick(float totalTime, float deltaTime)
{

}

void GSScenaric::terminate()
{

}
