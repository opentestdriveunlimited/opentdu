#pragma once

#include "game/gm_base.h"

#include "movie/mng_movie.h"
#include "flash/mng_flash_localize.h"
#include "mng_video_bumper.h"
#include "render/draw_list.h"

class GMVideoBumper : public GameMode {
public:
    GMVideoBumper();
    ~GMVideoBumper();

    void tick(float deltaTime);

    static void SetNextGameMode( eGameMode mode );
    static void SetActiveBumper( const std::string& bumperFilename );
    static void ClearActiveBumper();
    static void OnPlaybackComplete(const char* pFlashMovieName, const char* pFlashVariable, const char* pState, const bool param_4);
    static void SetSkippable();

private:
    static std::string ActiveVideoName;
    static eGameMode NextGameMode;
    static bool bCanSkipBumper;

    static bool BOOL_010e7d3d;
    static bool BOOL_010e7da0; // param_4 for FlashPlayer::ShowFrame
    static std::string FlashMovie;
    static std::string FlashVariable;
    static std::string FlashVariableValue;

private:
    MngMovie movieManager;
    MngFlashLocalize flashLocalization;
    MngVideoBumper videoBumperManager;
    DrawList drawList;
};

