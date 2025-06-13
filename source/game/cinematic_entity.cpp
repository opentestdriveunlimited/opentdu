#include "shared.h"
#include "cinematic_entity.h"

CinematicEntity::CinematicEntity()
    : cutsceneName( "" )
    , cutsceneIndex( 0 )
    , pParticleInstance( nullptr )
    , bInitialized( false )
    , bHighPriorityInit( true )
    , bNeedRegisterEntity( true )
{
    
}

CinematicEntity::~CinematicEntity()
{
}
