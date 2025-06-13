#include "shared.h"
#include "gs_world.h"

GSWorld* gpWorld = nullptr;

GSWorld::GSWorld()
    : pStreamingManager( nullptr )
    , bEnabled( true )
    , bForceUpdate( false )
    , bDirtyStreaming( false )
    , bAreaStreamingEnabled( true )
    , bIsCameraDirty( false )
{
    OTDU_ASSERT( gpWorld == nullptr );
    gpWorld = this;
}

GSWorld::~GSWorld()
{

}

bool GSWorld::initialize( TestDriveGameInstance* pGameInstance )
{
    bPaused = false;
    return true;
}

void GSWorld::tick(float deltaTime)
{

}

void GSWorld::terminate()
{

}
