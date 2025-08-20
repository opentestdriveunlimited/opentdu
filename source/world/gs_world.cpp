#include "shared.h"
#include "gs_world.h"

GSWorld* gpWorld = nullptr;

GSWorld::GSWorld()
    : GameSystem()
    , pStreamingManager( nullptr )
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

void GSWorld::tick(float totalTime, float deltaTime)
{

}

void GSWorld::terminate()
{

}

void GSWorld::pushGlobal2DM( Render2DM* pRender2DM )
{
    OTDU_ASSERT( pRender2DM );
    collection2D.register2DM( pRender2DM );
}

void GSWorld::pushGlobal2DB( Render2DB* pRender2DB )
{
    OTDU_ASSERT( pRender2DB );
    collection2D.register2DB( pRender2DB );
}

void GSWorld::removeGlobal2DB(Render2DB *pRender2DB)
{
    collection2D.unregister2DB(pRender2DB);
}
