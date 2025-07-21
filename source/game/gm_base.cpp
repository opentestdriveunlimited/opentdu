#include "shared.h"
#include "gm_base.h"

#include "render/draw_list.h"
#include "tdu_instance.h"
#include "gs_timer.h"
#include "system/gs_system.h"
#include "render/gs_render.h"

#include "game/mng_base.h"

GameMode* gpActiveGameMode = nullptr;

GameMode::GameMode()
    : transitionFlags( 0x39 )
    , transitionTime( 1000.0f )
    , numTicksPerTransition( 60 )
    , currentGameMode( GM_Invalid )
    , pTransitionDrawList( new DrawList() )
    , bExitRequested( false )
    , bMessageBoxVisible( false )
    , bAsync( false )
    , bLoadedAsync( false )
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

bool GameMode::initializeAsync()
{
    initialize();
    return false;
}

void GameMode::mainLoop(TestDriveGameInstance* param_1)
{
    gGSTimer.PreviousGameSpeed = 1.0;
    gGSTimer.GameSpeed = 1.0;
    gGSTimer.GameDeltaTime = 0.0;
    gGSTimer.GameTotalTime = 0.0;
    gGSTimer.PrevGameTime = 0.0;

    bExitRequested = false;
    bMessageBoxVisible = false;

    do {
        gpSystem->tick(gGSTimer.GameDeltaTime);
        gpRender->flushDrawCommands(false);
    } while (!gpSystem->isWindowActivated());

    if (!bLoadedAsync) {
        // TODO: Not sure what's the point
        // (I suspect it's either platform specific or scrapped late)
        bool bVar1 = initializeAsync();
        while (bVar1) {
            bVar1 = initializeAsync();
        }

        for (Manager* pManager : registeredManagers) {
            bool bVar2 = pManager->initializeAsync(param_1);
            while (bVar2) {
                bVar2 = pManager->initializeAsync(param_1);
            }
        }
        
        this->reset();

        for (Manager* pManager : registeredManagers) {
            pManager->reset();
        }

        gpTestDriveInstance->flushPendingFileInstanciation(false);
        if ((transitionFlags & 1) != 0) {
            int32_t iVar6 = numTicksPerTransition;
            while (iVar6 != 0) {
                stepLogic(gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);
                iVar6--;
            }

            transitionFlags &= 0xfffffffe;
        }
    }
    OTDU_UNIMPLEMENTED;
}

void GameMode::stepLogic(float deltaTime, float totalTime)
{
    OTDU_UNIMPLEMENTED;
}

void GameMode::registerManager(Manager* pManager)
{
    registeredManagers.push_back(pManager);
    OTDU_LOG_DEBUG("Registered manager '%s' (for game mode '%s')\n", pManager->getName(), kGameModeNames[currentGameMode]);
}

void GameMode::setTransitionFlags( const uint32_t flags )
{
    transitionFlags = flags;
}
