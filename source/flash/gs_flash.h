#pragma once

#include "game/gs_base.h"
#include "game/gm_base.h"

#include "render/2dm.h"
#include "bank_common_flash.h"
#include "flash_resources.h"
#include "movie_player.h"
#include "flash_entity.h"
#include "flash_player.h"

class GSFlash : public GameSystem {
public:
    const char* getName() const override { return "Service : Flash"; }

public:
    GSFlash();
    ~GSFlash();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

    void setCommonPath( const char* pNewPath );
    void setRootPath( const char* pNewPath );

    void setMaxFlashMovies( const int32_t maxNumMovies );

    void initGameMode( const eGameMode gameMode, FlashEntity* pEntity, const bool param_4 );

    FlashPlayer* getFlashPlayer(const char* pName);
    bool getFlashMovieIndex(uint32_t *pOutIsValidGameMode,uint32_t *pOutMovieIndex, const char *param_4);

    int32_t getNumMovies( const int32_t param_1 );
    MoviePlayer* getMoviePlayer(int32_t levelIndex, int32_t movieIndex);

    void addMovieForGameMode(const int32_t param_1, FlashEntity* pEntity);
    void addMovie(FlashMovie& movie, FlashEntity* pEntity);

    void flashCallback(const char* param_1, FlashPlayer* param_2);

private:
    std::string commonPath;
    std::string rootPath;
    
    std::vector<FlashMovie> flashMovies;
    std::vector<MoviePlayer> moviePlayers;

    int32_t maxFlashMovies;
    int32_t currentLevel;
    int32_t unicodeFontID;

    Material* pNullMaterial;
    Material* pMaskMaterial;

    Render2DM p2DMs[2];

    BankCommonFlash commonResources;
    BankCommonFlash fontResources;
    
    MoviePlayer* pActiveMoviePlayer;

    uint8_t bInitialized : 1;
    uint8_t bLevelSetupInProgress : 1;

private:
    void initLevel(FlashEntity* param_1);
    void updateUnicodeFontBank();
    void setupLevel(const bool param_1);
};

extern GSFlash* gpFlash;
