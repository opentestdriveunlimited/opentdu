#include "shared.h"
#include "gm_base.h"

#include "render/draw_list.h"
#include "tdu_instance.h"
#include "gs_timer.h"
#include "system/gs_system.h"
#include "render/gs_render.h"

#include "game/mng_base.h"
#include "flash/gs_flash.h"
#include "audio/audio_device.h"

#include "player_data/save_game_manager.h"
#include "player_data/gs_playerdata.h"
#include "filesystem/gs_file.h"
#include "render/particles/gs_particles.h"
#include "flash/modal_message_box.h"
#include "database/gs_database.h"
#include "core/locale.h"
#include "flash/mng_flash_localize.h"

GameMode* gpActiveGameMode = nullptr;

bool DAT_00fa4030 = false;
bool DAT_00fa4031 = false;
bool DAT_010e72fb = false;

// Loading screen fade to color
uint8_t DAT_010e72f4 = 0; // R
uint8_t DAT_010e72f5 = 0; // G
uint8_t DAT_010e72f6 = 0; // B

uint32_t DAT_01691498[2] = { 0, 0 };
uint32_t DAT_016914a0 = 0;
uint32_t DAT_016914a4 = 0;

// DAT_00f77dac
static constexpr const char* kLoadingPleaseMessages[10] = {
    "Loading... Please wait...",
    "Chargement en cours, veuillez patienter...",
    "Caricamento in corso... Attendere...",
    "Es wird geladen ... Bitte warten ...",
    "Cargando... Un momento, por favor...",
    "_no trad", // TODO:
    "_no trad", // TODO:
    "_no trad", // TODO:
    "_no trad",
    "_no trad",
};


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
    , field7_0x1c( false )
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
    // FUN_0097e5e0
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
    // FUN_0097f260
    gGSTimer.PreviousGameSpeed = 1.0;
    gGSTimer.GameSpeed = 1.0;
    gGSTimer.GameDeltaTime = 0.0;
    gGSTimer.GameTotalTime = 0.0;
    gGSTimer.PrevGameTime = 0.0;

    bExitRequested = false;
    bMessageBoxVisible = false;

    do {
        gpSystem->tick(gGSTimer.GameTotalTime, gGSTimer.GameDeltaTime);
        gpRender->flushDrawCommands(false);
    } while (!gpSystem->isWindowActivated());
    gpRender->flushDrawCommands(false);

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
                stepLogic(0.0f,gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);
                iVar6--;
            }

            transitionFlags &= 0xfffffffe;
        }
    }

    gbRequestedExit = false;
    int32_t iVar6 = gpRender->flushDrawCommands(false);
    if (iVar6 == 4) {
        bExitRequested = true;
        gbRequestedExit = true;
        param_1->requestExit();
    } else {
        drawLoadingScreen();
    }

    while ((bExitRequested != true || (bMessageBoxVisible != false))) {
        stepLogic(0.0f,gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);
        if (!bDraw) {
            iVar6 = gpRender->flushDrawCommands(false);
            if (iVar6 == 4) {
                gbRequestedExit = true;
            }
            gpTestDriveInstance->flushPendingFileInstanciation(true);
            iVar6 = DAT_016914a4;
            DAT_016914a4 = DAT_016914a0;
        } else {
            submitDrawCommands(gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);
            iVar6 = gpRender->flushDrawCommands(false);
            if (iVar6 == 4) {
                gbRequestedExit = true;
            }
            gpTestDriveInstance->flushPendingFileInstanciation(true);
            iVar6 = DAT_016914a4;
            DAT_016914a4 = DAT_016914a0;
        }

        DAT_016914a0 = iVar6;
        DAT_01691498[iVar6] = 0;
        if (BOOL_010e7364 == false) {
            gSavegameManager.doSaveBlocking();
        }
        if (gbRequestedExit != false) {
            gpTestDriveInstance->requestExit();
            bExitRequested = true;
        }
    }

    // gGSFlash.field77_0xf8c[0x30] = 1; // TODO: No clue what's this?
    FUN_0097ed60();

    bool bNeedToExit = gpRender->getRenderDevice()->isDeviceLost() ||  gpRender->flushDrawCommands(false) == 4;
    if (bNeedToExit) {
        bExitRequested = true;
        gpTestDriveInstance->requestExit(); 
    }
    gbRequestedExit = true;
    
    do {
        gpSystem->tick(gGSTimer.GameTotalTime, gGSTimer.GameDeltaTime);
        gpRender->flushDrawCommands(false);
    } while (!gpSystem->isWindowActivated());
    gpRender->flushDrawCommands(false);
    
    gpFile->flushPendingAsync();
    gpParticles->FUN_0080d330();
    gpTestDriveInstance->flushPendingFileInstanciation(false);

    for (Manager* pMVar4 : registeredManagers) {
        pMVar4->pause();
    }
    this->pause();

    for (Manager* pMVar4 : registeredManagers) {
        pMVar4->terminate();
    }
    this->destroy();
}

void GameMode::stepLogic(float param_1, float deltaTime, float totalTime)
{
    // TODO: Debug stuff? Never seen those called at runtime...
    if (DAT_00fa4031) {
       OTDU_UNIMPLEMENTED;
    } else if (DAT_00fa4030) {
        OTDU_UNIMPLEMENTED;
    }

    gpTestDriveInstance->tickGameMode(param_1, 0.0f, deltaTime, totalTime);
}

void GameMode::submitDrawCommands(float deltaTime, float totalTime)
{
    // FUN_0097e7c0
    gpTestDriveInstance->drawServices(deltaTime);
    draw(deltaTime, totalTime);
    if (DAT_010e72fb) {
        OTDU_UNIMPLEMENTED;
    } else {
        DAT_010e72fb = false;
        for (Manager* pMVar1 : registeredManagers) {
            pMVar1->draw();
        }
        gpTestDriveInstance->updateFileInstanciation();
    }
}

void GameMode::registerManager(Manager* pManager)
{
    registeredManagers.push_back(pManager);
    OTDU_LOG_DEBUG("Registered manager '%s' (for game mode '%s')\n", pManager->getName(), kGameModeNames[currentGameMode]);
}

void GameMode::drawLoadingScreen()
{
    // FUN_0097e990
    float local_20 = transitionTime;
    bool bVar9 = true;

    uint32_t local_c = 0;
    uint32_t local_10 = 0;
    uint32_t local_14 = 0;
    uint32_t local_18 = 0;
    FlashPlayer* peVar15 = nullptr;
    while( true ) {
        bool bVar12 = gpFlash->getFlashMovieIndex(&local_18, &local_14, "GENERAL");
        if (bVar12) {
            MoviePlayer* pMVar13 = gpFlash->getMoviePlayer(local_18, local_14);
            if (pMVar13 != nullptr && pMVar13->MovieBank.isLoaded() && pMVar13->MovieBank.getFlashPlayer() != nullptr) {
                bVar12 = gpFlash->getFlashMovieIndex(&local_10, &local_c, "GENERAL");
                if (bVar12) {
                    pMVar13 = gpFlash->getMoviePlayer(local_10, local_c);
                    peVar15 = ((pMVar13 == nullptr || !pMVar13->MovieBank.isLoaded()) ? nullptr : pMVar13->MovieBank.getFlashPlayer());
                } else {
                    peVar15 = nullptr;
                }
            }
            {
                ScopedMutexLock lock = gpFlash->scopedLock();    
                OTDU_UNIMPLEMENTED;
            }

            gbLoadingInProgress = false;
        }

        uint32_t uVar4 = transitionFlags;
        if (((uVar4 >> 3 & 1) == 0) && ((uVar4 >> 1 & 1) == 0)) {
            break;
        }

        float local_1c = -1.0f;
        float peVar7 = -1.0f;
        if (((uVar4 >> 3 & 1) == 0) && (peVar7 = local_1c, (uVar4 >> 1 & 1) != 0)) {
            peVar7 = 0.0f;
        }
        local_1c = peVar7;

        float fVar17 = gGSTimer.GameDeltaTime;
        if (isnan(gGSTimer.GameDeltaTime) != (gGSTimer.GameDeltaTime == 0.0f)) {
            fVar17 = 0.033333335f;
        }
        local_20 = local_20 - fVar17 * 1000.0f;
        if (0.0f <= local_20) {
            fVar17 = 1.0 - local_20 / transitionTime;
        } else {
            local_20 = transitionTime;
            transitionFlags = uVar4 & 0xfffffff5;
            bVar9 = false;
            fVar17 = 1.0;
        }

        gAudioDevice.setGlobalVolume(fVar17);
        float local_4 = (local_20 / transitionTime) * 255.0f;
        pTransitionDrawList->reset();
        pTransitionDrawList->beginPrimitive(ePrimitiveType::PT_TriangleStrip, 4);
        
        uint8_t iVar14 = (uint8_t)local_4;
        if (iVar14 < 0) {
            iVar14 = 0;
        } else if (0xff < iVar14) {
            iVar14 = 0xff;
        }
        pTransitionDrawList->setDiffuse(0, ColorRGBA(DAT_010e72f4, DAT_010e72f5, DAT_010e72f6, iVar14));

        const Viewport& viewport2D = gpRender->getViewport2D();
        pTransitionDrawList->pushVertex(0.0f,               0.0f,               local_1c);
        pTransitionDrawList->pushVertex(viewport2D.Width,   0.0f,               local_1c);
        pTransitionDrawList->pushVertex(0.0f,               viewport2D.Height,  local_1c);
        pTransitionDrawList->pushVertex(viewport2D.Width,   viewport2D.Height,  local_1c);
        pTransitionDrawList->commitPrimitive();
        
        RenderScene* peVar16 = gpRender->getRenderPass<eRenderPass::RP_After2D>().pScene;
        if (((transitionFlags >> 3 & 1) != 0) || (peVar16 = gpRender->getRenderPass<eRenderPass::RP_Before2D>().pScene, (transitionFlags >> 1 & 1) != 0)) {
            peVar16->enqueueDynamicDrawList(pTransitionDrawList);
        }
        if (!bVar9) {
            return;
        }
        stepLogic(0.0f,gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);
        submitDrawCommands(gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);

        iVar14 = gpRender->flushDrawCommands(true);
        if (iVar14 == 4) {
            bExitRequested = true;
            gbRequestedExit = true;
            gpTestDriveInstance->requestExit();
            return;
        }
    }

    gAudioDevice.setGlobalVolume(1.0f);
}

bool GameMode::FUN_0097ed60()
{
    // FUN_0097ed60
    float local_18 = transitionTime;
    bool bVar8 = true;
    bool bVar11 = false;
    while (true) {      
        if (bDraw == false && gpPlayerData->FUN_00897990() && !gpFile->hasTaskPending()) {
            bDraw = gpFlashMessageBox->display(0x37ba8e2, 3.0f, 0, nullptr, nullptr, false);
        }

        uint32_t uVar3 = transitionFlags;
        if (((uVar3 >> 4 & 1) == 0) && ((uVar3 >> 2 & 1) == 0)) {
            gAudioDevice.setGlobalVolume(0.0f);
            return bVar11;
        }

        float local_c = -1.0f;
        float peVar6 = -1.0f;
        if (((uVar3 >> 4 & 1) == 0) && (peVar6 = local_c, (uVar3 >> 2 & 1) != 0)) {
            peVar6 = 0.0f;
        }
        local_c = peVar6;
        float fVar17 = 1.0f / transitionTime;
        int32_t local_14 = (int32_t)(255.0 - fVar17 * local_18 * 255.0);
        if (local_14 < 0x100) {
            if (local_14 < 0) {
                local_14 = 0;
            }
        } else {
            local_14 = 0xff;
        }
        
        float fVar18 = gGSTimer.GameDeltaTime;
        if (isnan(gGSTimer.GameDeltaTime) != (gGSTimer.GameDeltaTime == 0.0f)) {
            fVar18 = 0.033333335f;
        }
            
        if (!field7_0x1c) {
            local_14 = 0;
        } else {
            local_18 = local_18 - fVar18 * 1000.0;
        }

        if ((0.0f <= local_18) || (local_14 != 0xff)) {
            fVar17 = fVar17 * local_18;
        } else {
            local_18 = transitionTime;
            transitionFlags = uVar3 & 0xffffffeb;
            bVar8 = false;
            fVar17 = 0.0;
        }

        gAudioDevice.setGlobalVolume(fVar17);

        uint32_t local_4 = 0;
        uint32_t local_8 = 0;
        bVar11 = gpFlash->getFlashMovieIndex(&local_8,&local_4,"GENERAL");

        FlashPlayer* pMVar14 = nullptr;
        if (bVar11) {
            MoviePlayer* pMVar12 = gpFlash->getMoviePlayer(local_8,local_4);
            if ((pMVar12 == nullptr) || (!pMVar12->MovieBank.isLoaded())) {
                pMVar14 = nullptr;
            } else {
                pMVar14 = pMVar12->MovieBank.getFlashPlayer();
            }
        }

        const char* pcVar19 = nullptr;
        if ((((transitionFlags >> 5 & 1) != 0) && (local_18 - fVar18 * 1000.0 <= 0.0))
        && (pMVar14 != (nullptr)) && (field7_0x1c != false)) {
            bVar11 = (bDraw != false || gpPlayerData->FUN_00897990());
            
            if (gpFlashMessageBox->getInputState() == ModalMessageBox::eInputState::MMBIS_None) {
                bVar11 = false;
            }
            
            if (!gpDatabase->isInitialized()) {
                int32_t iVar15 = 0;

                eLocale eVar13 = GetLocaleFromSystem();
                switch (eVar13) {
                case L_Japanese:
                    iVar15 = 5;
                    break;
                default:
                    iVar15 = 0;
                    break;
                case L_French:
                    iVar15 = 1;
                    break;
                case L_Spanish:
                    iVar15 = 4;
                    break;
                case L_German:
                    iVar15 = 3;
                    break;
                case L_Italian:
                    iVar15 = 2;
                    break;
                case L_Russian:
                    iVar15 = 8;
                    break;
                case L_Polish:
                    iVar15 = 9;
                    break;
                };

                const char* pcVar19 =  (!bVar11) ? kLoadingPleaseMessages[iVar15] : " ";
                FlashPlayer* peVar14 = gpFlash->getFlashPlayer("GENERAL");
                peVar14->setVariableValue("/:msg_loading", pcVar19);
                pcVar19 = "loading_start";
            } else {
                if (gpMngFlashLocalize != nullptr) {
                    pcVar19 = "loading";
                    if (bVar11) {
                        gpMngFlashLocalize->FUN_00717ff0(pMVar14, " ", "/:msg_loading");
                    } else {
                        gpMngFlashLocalize->FUN_00718120(pMVar14, "/:msg_loading", 0x36c66a2);
                    }
                }
            }
            
            pMVar14->showFrame(pcVar19, "/", false);
            gbLoadingInProgress = true;
        }

        pTransitionDrawList->reset();
        pTransitionDrawList->beginPrimitive(ePrimitiveType::PT_TriangleStrip, 4);
        pTransitionDrawList->setDiffuse(0, ColorRGBA(DAT_010e72f4, DAT_010e72f5, DAT_010e72f6, local_14));

        const Viewport& viewport2D = gpRender->getViewport2D();
        pTransitionDrawList->pushVertex(0.0f,               0.0f,               local_c);
        pTransitionDrawList->pushVertex(viewport2D.Width,   0.0f,               local_c);
        pTransitionDrawList->pushVertex(0.0f,               viewport2D.Height,  local_c);
        pTransitionDrawList->pushVertex(viewport2D.Width,   viewport2D.Height,  local_c);
        pTransitionDrawList->commitPrimitive();
        
        RenderScene* peVar16 = gpRender->getRenderPass<eRenderPass::RP_After2D>().pScene;
        if (((transitionFlags >> 4 & 1) != 0) || (peVar16 = gpRender->getRenderPass<eRenderPass::RP_Before2D>().pScene, (transitionFlags >> 2 & 1) != 0)) {
            peVar16->enqueueDynamicDrawList(pTransitionDrawList);
        }
        bVar11 = true;
        if (!bVar8) {
            return true;
        }
        stepLogic(1.4013e-45f,gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);
        submitDrawCommands(gGSTimer.GameDeltaTime,gGSTimer.GameTotalTime);

        int32_t iVar15 = gpRender->flushDrawCommands(true);
        if (iVar15 == 4) {
            bExitRequested = true;
            gbRequestedExit = true;
            gpTestDriveInstance->requestExit();
            return true;
        }
    }

    // Unreachable
    return false;
}

void GameMode::setTransitionFlags( const uint32_t flags )
{
    transitionFlags = flags;
}

void GameMode::tickManagers(float param_2, float param_3, int32_t param_4, bool param_5)
{
    // FUN_0097e630
    if (param_5) {
        OTDU_UNIMPLEMENTED;
    } else {
        for (Manager* manager : registeredManagers) {
            if (manager->getIndex() == param_4) {
                manager->tick(param_2,param_3);
            }
        }
    }
}
