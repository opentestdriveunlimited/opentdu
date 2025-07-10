#include "shared.h"
#include "gs_flash.h"

#include "movie_player.h"

#include "config/gs_config.h"
#include "player_data/gs_playerdata.h"
#include "core/locale.h"
#include "tdu_instance.h"
#include "render/material.h"
#include "world/gs_world.h"

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
    , p2DMMemory{ nullptr, nullptr }
    , pActiveMoviePlayer( nullptr )
    , bInitialized( false )
    , bLevelSetupInProgress( false )
{
    OTDU_ASSERT( gpFlash == nullptr );
    gpFlash = this;

    flashMovies.reserve( maxFlashMovies );
    moviePlayers.reserve( 4 );
}

GSFlash::~GSFlash()
{

}

bool GSFlash::initialize( TestDriveGameInstance* )
{
    if ( !bInitialized ) {
        return true;
    }

    uint32_t nullMaterialSize = Render2DM::CalcSize(0, 0);

    // Null Material Fallback
    p2DMMemory[0] = TestDrive::Alloc(nullMaterialSize);

    pNullMaterial = render2DM[0].create(p2DMMemory[0], 0x230588678a67836d, 0, 0, 0, 0);
    pNullMaterial->DepthTest = '\x01';
    pNullMaterial->DepthWrite = '\x02';
    pNullMaterial->AlphaTest = '\0';
    pNullMaterial->SrcBlend = '\x05';
    pNullMaterial->DstBlend = '\x06';
    pNullMaterial->BlendOP = '\x01';
    pNullMaterial->AlphaSource = '\0';
    
    MaterialParameter* peVar8 = pNullMaterial->getParameterByIndex(0);
    (peVar8->Flags).NumPrelight = '\x01';
    (peVar8->Flags).NumTextures = '\0';

    MaterialLayer* iVar9 = peVar8->getLayer(0);
    iVar9->pLayerTextures[0].SamplerAddress[0] = 2;
    iVar9->pLayerTextures[0].SamplerAddress[1] = 2;

    render2DM[0].initialize(p2DMMemory[0]);
    pNullMaterial->OT = 0x2a; // TODO: I assume 0x2a maps to Flash RenderLayer (or smthing like that)

    gpWorld->pushGlobal2DM(&render2DM[0]);

    // Masked Material Fallback
    p2DMMemory[1] = TestDrive::Alloc(nullMaterialSize);

    pMaskMaterial = render2DM[1].create(p2DMMemory[0], 0x230588678a67836d, 0, 0, 0, 0);
    pMaskMaterial->DepthTest = '\x02';
    pMaskMaterial->DepthWrite = '\x01';
    pMaskMaterial->AlphaTest = '\x01';
    pMaskMaterial->SrcBlend = '\x05';
    pMaskMaterial->DstBlend = '\x06';
    pMaskMaterial->BlendOP = '\x01';
    pMaskMaterial->AlphaSource = '\0';

    peVar8 = pMaskMaterial->getParameterByIndex(0);
    (peVar8->Flags).NumPrelight = '\x01';
    (peVar8->Flags).NumTextures = '\0';

    iVar9 = peVar8->getLayer(0);
    iVar9->pLayerTextures[0].SamplerAddress[0] = 2;
    iVar9->pLayerTextures[0].SamplerAddress[1] = 2;

    render2DM[1].initialize(p2DMMemory[1]);
    pMaskMaterial->OT = 0x2a;

    gpWorld->pushGlobal2DM(&render2DM[1]);

    mutex.initialize("Flash");

    void* pAudioPool = TestDrive::AllocAligned(0x120002, 0x10);
    audio.initialize(pAudioPool);

    return true;
}

void GSFlash::tick(float deltaTime)
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
    setupLevel(param_4);

    // TODO: Flash resources seems to be paired with a single player. Is it always the case/safe to assume?
    for (uint32_t i = 0; i < flashMovies.size(); i++) {
        FlashMovie& movie = flashMovies[i];
        MoviePlayer& player = moviePlayers[i];

        if (movie.GameMode == -1) {
            player.pEntityInstance = pEntity;
        }
    }

    if (gameMode != eGameMode::GM_BootMenu) {
        OTDU_UNIMPLEMENTED;
    //   FUN_0099e9b0(&param_1->Audio);
    }
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

    bool bHasInitPlayers = false;
    for ( MoviePlayer& player : moviePlayers ) {
        if ( player.MovieBank.isInUse() && player.Level == currentLevel && !player.MovieBank.isLoaded() ) {
            initMoviePlayer( param_1, player );
            bHasInitPlayers = true;
        }
    }

    if ( param_1 ) {
        currentLevel++;
        return;
    }
    bLevelSetupInProgress = param_1;
    
    int32_t iVar5 = 0;
    if ( streamingListeners.empty() ) {
        iVar5 = currentLevel;
    } else {
        iVar5 = currentLevel;
        if ( 0 < iVar5 ) {
            if ( param_1 ) {
                for ( FlashStreamListener* pListener : streamingListeners ) {
                    pListener->onResourceStreamed();
                }
            }
            streamingListeners.clear();
            iVar5 = currentLevel;
        }
    }

    if ( iVar5 == 0 ) {
        uint32_t local_8 = 0;
        uint32_t local_4 = 0;
        FlashPlayer* peVar6 = nullptr;
        bool bVar1 = getFlashMovieIndex( &local_8, &local_4, "GENERAL" );
        if ( bVar1 ) {
            MoviePlayer* pMVar3 = getMoviePlayer( local_8, local_4 );
            if ( pMVar3 == nullptr || !pMVar3->MovieBank.isLoaded() ) {
                peVar6 = nullptr;
            } else {
                peVar6 = pMVar3->MovieBank.getFlashPlayer();
            }
        }

        getFlashMovieIndex( &local_8, &local_4, "CONFIGPC" );
        if ( peVar6 != nullptr ) {
            OTDU_UNIMPLEMENTED; // TODO:
//          param_1->pFlashMessageBox->pPlayer = peVar6;
//          param_1->field_0xf8c->field_0x18 = peVar6;
            peVar6->setVariableValue( "/:state_Cheats", 0u );
        }
    }

    currentLevel++;
}

void GSFlash::initMoviePlayer( bool param_1, MoviePlayer& param_2 )
{
    if ( param_2.MovieBank.isLoaded() ) {
        OTDU_UNIMPLEMENTED; // TODO:
    }
}
