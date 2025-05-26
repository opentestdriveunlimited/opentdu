#include "shared.h"
#include "gs_flash.h"

#include "movie_player.h"

#include "config/gs_config.h"
#include "player_data/gs_playerdata.h"
#include "core/locale.h"
#include "tdu_instance.h"

static constexpr const char* kCommonFlashBankName = "common.bnk";

static constexpr int32_t kNumFontBanks = 4;
static constexpr const char* kFontBanks[kNumFontBanks] = {
    "font_jap.bnk",
    "font_cor.bnk",
    "font_chn.bnk",
    "font_ru.bnk"
};

GSFlash* gpFlash = nullptr;

GSFlash::GSFlash()
    : commonPath( "FRONTEND" )
    , rootPath( "FRONTEND" )
    , maxFlashMovies( 32 )
    , currentLevel( 0 )
    , unicodeFontID( 0 )
    , pNullMaterial( nullptr )
    , pMaskMaterial( nullptr )
    , pActiveMoviePlayer( nullptr )
    , bInitialized( false )
    , bLevelSetupInProgress( false )
{
    OTDU_ASSERT( gpFlash == nullptr );
    gpFlash = this;

    flashMovies.resize( maxFlashMovies );
}

GSFlash::~GSFlash()
{

}

bool GSFlash::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSFlash::tick()
{

}

void GSFlash::terminate()
{

}

void GSFlash::setCommonPath( const char* pNewPath )
{
    commonPath.clear();
    commonPath += gpConfig->getResRootPath();
    commonPath += pNewPath;
    commonPath += "\\";

    if (!commonResources.isLoaded()) {
        commonResources.setFilepath( commonPath.c_str() );
        commonResources.setFilename( kCommonFlashBankName );
    }

    updateUnicodeFontBank();
}

void GSFlash::setRootPath( const char* pNewPath )
{
    // "%s%s\\%s\\"
    rootPath.clear();
    rootPath += gpConfig->getResRootPath();
    rootPath += pNewPath;
    rootPath += "\\";
    rootPath += gpConfig->getAssetQualityFolder();
    rootPath += "\\";
}

void GSFlash::setMaxFlashMovies( const int32_t maxNumMovies )
{
    if ( !bInitialized ) {
        flashMovies.resize( maxNumMovies );
        maxFlashMovies = maxNumMovies;
    }
}

void GSFlash::initGameMode(const eGameMode gameMode, FlashEntity *pEntity, const bool param_4)
{
    if (currentLevel == 0) {
        initLevel(nullptr);
    }
    addMovieForGameMode(gameMode, pEntity);
    
    
    // GSFlash::PushInstallLevel(param_1,param_4);
    // uVar1 = 0;
    // if (param_1->NumFlashMovies != 0) {
    //   iVar3 = 0;
    //   iVar2 = 0;
    //   do {
    //     if (*(int *)(param_1->pFlashMovies->pFilename + iVar2 + 0x20) == -1) {
    //       *(IListenerFlash **)((int)&param_1->pMoviePlayers->pUnknown + iVar3) = param_3;
    //     }
    //     uVar1 = uVar1 + 1;
    //     iVar2 = iVar2 + 0x30;
    //     iVar3 = iVar3 + 0xae0;
    //   } while (uVar1 < param_1->NumFlashMovies);
    // }
    // if (param_2 != 0) {
    //   FUN_0099e9b0(&param_1->Audio);
    // }
}

FlashPlayer *GSFlash::getFlashPlayer(const char *pName)
{ 
    uint32_t validMode = 0u;
    uint32_t movieIndex = 0u;
    
    bool bVar1 = getFlashMovieIndex(&validMode, &movieIndex, pName);
    if (bVar1) {
        MoviePlayer* pMVar2 = getMoviePlayer(validMode,movieIndex);
        if (pMVar2 != nullptr && pMVar2->MovieBank.getState() == BankFlash::eBankFlashInit::BFI_Loaded) {
            return pMVar2->MovieBank.getFlashPlayer();
        }
    }
    
    return nullptr;
}

bool GSFlash::getFlashMovieIndex(uint32_t *pOutIsValidGameMode, uint32_t *pOutMovieIndex, const char *param_4)
{
    if (flashMovies.empty()) {
        OTDU_LOG_WARN("Trying to retrieve flash movie but cache is empty...");
        *pOutIsValidGameMode = 0;
        *pOutMovieIndex = 0;
        return false;
    }

    uint32_t matchingMovieIndex = 0;
    bool bVar1 = false;
    int32_t gameMode = -1;
    for (FlashMovie& movie : flashMovies) {
        bool bGMMatch = (movie.GameMode == gpTestDriveInstance->getActiveGameMode() || movie.GameMode < 0);
        if (movie.Filename == param_4 && bGMMatch) {
            gameMode = movie.GameMode;
            *pOutIsValidGameMode = static_cast<uint32_t>(-1 < gameMode);
            bVar1 = true;
            break;
        }
        matchingMovieIndex++;
    }

    if (!bVar1) {
        OTDU_LOG_WARN("Unknown flash movie '%s'", param_4);
        *pOutIsValidGameMode = 0;
        *pOutMovieIndex = 0;
        return false;
    }
    
    uint32_t movieIndex = 0;
    for (uint32_t i = 0; i < flashMovies.size(); i++) {
        if (i == matchingMovieIndex) {
            *pOutMovieIndex = movieIndex;
            return true;
        }

        const FlashMovie& movie = flashMovies[i];
        if ((movie.GameMode == gameMode) || ((movie.GameMode < 0 && (gameMode < 0)))) {
            movieIndex++;
        }
    }

    *pOutIsValidGameMode = 0;
    *pOutMovieIndex = 0;
    return false;
}

int32_t GSFlash::getNumMovies(const int32_t param_1)
{
    int32_t numPlayers = 0;

    for (const MoviePlayer& player : moviePlayers) {
        if (player.MovieBank.isInUse() && player.Level == param_1) {
            numPlayers++;
        }
    }

    return numPlayers;
}

MoviePlayer* GSFlash::getMoviePlayer(int32_t levelIndex, int32_t movieIndex)
{
    for (uint32_t i = 0; i < moviePlayers.size(); i++) {
        MoviePlayer& player = moviePlayers[i];

        if (player.MovieBank.isInUse()) {
            if (player.Level == levelIndex && (i + 1) == movieIndex) {
                return &moviePlayers[i];
            }
        }
    }
    OTDU_LOG_WARN( "Could not find movie player with level index %i and movie index %i\n", levelIndex, movieIndex);
    return nullptr;
}

void GSFlash::addMovieForGameMode(const int32_t param_1, FlashEntity *pEntity)
{
    for (FlashMovie& movie : flashMovies) {
        if (movie.GameMode == param_1) {
            addMovie(movie, pEntity);
        }
    }
}

void GSFlash::addMovie(FlashMovie &movie, FlashEntity *pEntity)
{
    for (MoviePlayer& player : moviePlayers) {
        if (!player.MovieBank.isInUse()) {
            player.Level = currentLevel;
            player.pEntityInstance = pEntity;

            BankFlash::eBankFlashInit bankState = player.MovieBank.getState();
            if (bankState != BankFlash::eBankFlashInit::BFI_Loaded) {
                player.MovieBank.setUsed();
                player.MovieBank.updateFilePaths( rootPath.c_str(), movie.Filename.c_str() );
                player.MovieBank.setCommonBankRef( &commonResources );
            }
            player.MovieBank.setMaxNumVars(movie.NumDrawListVertices, movie.NumDrawListStrips, movie.MaxNumVars);
            player.MovieBank.setPlayerCallback( std::bind(&GSFlash::flashCallback, this, std::placeholders::_1, std::placeholders::_2) );
            return;
        }
    }

    OTDU_LOG_ERROR("Failed to add movie: no more movie player available!");
}

void GSFlash::flashCallback(const char *param_1, FlashPlayer* param_2)
{
    OTDU_UNIMPLEMENTED; // TODO:
}

void GSFlash::initLevel(FlashEntity *param_1)
{
    const char* pLanguage = gpPlayerData->getLanguage();
    
    PlayerDataLanguage language( pLanguage );

    uint32_t languageFontID = 0;
    if ( language == kLangKO ) {
        languageFontID = 1;
    } else if ( language == kLangCH ) {
        languageFontID = 2;
    } else if ( language == kLangRU || language == kLangPO ) {
        languageFontID = 3;
    }

    if (languageFontID != unicodeFontID) {
        unicodeFontID = languageFontID;
        updateUnicodeFontBank();
    }

    addMovieForGameMode(-1, param_1);
    addMovieForGameMode(-2, param_1);
    setupLevel(false);
}

void GSFlash::updateUnicodeFontBank()
{
    if (!fontResources.isLoaded()) {
        fontResources.setFilepath( commonPath.c_str() );
        OTDU_ASSERT( unicodeFontID < kNumFontBanks );
        const char* pFontBank = kFontBanks[unicodeFontID];
        fontResources.setFilename( pFontBank );
    }
}

void GSFlash::setupLevel(const bool param_1)
{
    bLevelSetupInProgress = true;
    fontResources.initialize( false );


//     bool bVar1;
//     char *pcVar2;
//     MoviePlayer *pMVar3;
//     uint uVar4;
//     int iVar5;
//     edCFlashPlayer *peVar6;
//     uint local_8;
//     uint local_4;
    
//     bVar1 = false;
//     uVar4 = 0;
//     if (param_1->NumPlayers != 0) {
//       iVar5 = 0;
//       bVar1 = false;
//       do {
//         pcVar2 = (param_1->pMoviePlayers->edCFlashResource).super.BankEntry.pFilename + iVar5 + -0x75;
//         if (((*(char *)((int)((param_1->pMoviePlayers->edCFlashResource).RenderList.CurrentState.
//                               Matrix.Scalars + -5) + 0xcU + iVar5) != '\0') &&
//             (*(int *)(pcVar2 + 0xad4) == param_1->CurrentLevel)) && (*(int *)(pcVar2 + 0x2dc) != 1)) {
//           GSFlash::InstallMovie(param_2,(int)pcVar2,param_1);
//           bVar1 = true;
//         }
//         uVar4 = uVar4 + 1;
//         iVar5 = iVar5 + 0xae0;
//       } while (uVar4 < param_1->NumPlayers);
//     }
//     if (param_2) goto LAB_0099c496;
//     peVar6 = (edCFlashPlayer *)0x0;
//     param_1->bInstalling = param_2;
//     if (param_1->pActiveInstallListener == (IFlashInstallListener *)0x0) {
//   LAB_0099c414:
//       iVar5 = param_1->CurrentLevel;
//     }
//     else {
//       iVar5 = param_1->CurrentLevel;
//       if (0 < iVar5) {
//         if (bVar1) {
//           (**(code **)param_1->pActiveInstallListener->lpVtbl)();
//         }
//         param_1->pActiveInstallListener = (IFlashInstallListener *)0x0;
//         goto LAB_0099c414;
//       }
//     }
//     if (iVar5 == 0) {
//       bVar1 = GSFlash::GetFlashMovieIndex(param_1,&local_8,&local_4,"GENERAL");
//       if (bVar1) {
//         pMVar3 = GSFlash::GetMoviePlayer(param_1,local_8,local_4);
//         if ((pMVar3 == (MoviePlayer *)0x0) || ((pMVar3->edCFlashResource).LoadState != LOADED)) {
//           peVar6 = (edCFlashPlayer *)0x0;
//         }
//         else {
//           peVar6 = (pMVar3->edCFlashResource).pPlayer;
//         }
//       }
//       GSFlash::GetFlashMovieIndex(param_1,&local_8,&local_4,"CONFIGPC");
//       if (peVar6 != (edCFlashPlayer *)0x0) {
//         param_1->pFlashMessageBox->pPlayer = peVar6;
//         *(edCFlashPlayer **)(*(int *)&param_1->field_0xf8c + 0x18) = peVar6;
//         edCFlashPlayer::SetVar(peVar6,"/:state_Cheats",0);
//       }
//     }
//   LAB_0099c496:
//     param_1->CurrentLevel = param_1->CurrentLevel + 1;
//     return;
}
