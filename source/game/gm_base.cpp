#include "shared.h"
#include "gm_base.h"

#include "render/draw_list.h"
#include "tdu_instance.h"
#include "gs_timer.h"

GameMode* gpActiveGameMode = nullptr;

GameMode::GameMode()
    : transitionFlags( 0x39 )
    , transitionTime( 1000.0f )
    , ticksPerLoop( 60 )
    , currentGameMode( GM_Invalid )
    , pTransitionDrawList( new DrawList() )
    , bExitRequested( false )
    , bMessageBoxVisible( false )
    , bAsync( false )
    , bLoadAsync( false )
    , bDraw( true )
{
    if (gpTestDriveInstance != nullptr) {
        currentGameMode = gpTestDriveInstance->getActiveGameMode();
    }

    gpActiveGameMode = this;

    DrawStreams transitionStreams;
    transitionStreams.Tangent = '\0';
    transitionStreams.Binormal = '\0';
    transitionStreams.Index = '\0';
    transitionStreams.Custom = '\0';
    transitionStreams.Normals = '\0';
    transitionStreams.Diffuse = '\0';
    transitionStreams.Specular = '\0';
    transitionStreams.UVMap = '\0';

    pTransitionDrawList->initialize(10, 100, 0, true, &transitionStreams, "Trans_rlist2d", false);
}

GameMode::~GameMode()
{
    pTransitionDrawList->destroy();
    delete pTransitionDrawList;

    gpActiveGameMode = nullptr;
}

void GameMode::mainLoop()
{
    gGSTimer.PreviousGameSpeed = 1.0;
    gGSTimer.GameSpeed = 1.0;
    gGSTimer.GameDeltaTime = 0.0;
    gGSTimer.GameTotalTime = 0.0;
    gGSTimer.PrevGameTime = 0.0;

    bExitRequested = false;
    bMessageBoxVisible = false;

    OTDU_UNIMPLEMENTED;
}