#include "shared.h"
#include "gs_flash.h"

#include "movie_player.h"

#include "config/gs_config.h"
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
    , bInitialized( false )
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

    if (!fontResources.isLoaded()) {
        fontResources.setFilepath( commonPath.c_str() );

        OTDU_ASSERT( unicodeFontID < kNumFontBanks );
        const char* pFontBank = kFontBanks[unicodeFontID];
        fontResources.setFilepath( pFontBank );
    }
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
