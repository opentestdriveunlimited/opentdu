#include "shared.h"
#include "gs_world.h"

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

}

void GSWorld::tick()
{

}

void GSWorld::terminate()
{

}
