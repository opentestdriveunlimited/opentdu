#include "shared.h"
#include "gm_video_bumper.h"

#include "config/gs_config.h"

std::string GMVideoBumper::ActiveVideoName = "bumper.avi";
eGameMode GMVideoBumper::NextGameMode = eGameMode::GM_Living;
bool GMVideoBumper::bCanSkipBumper = true;

bool GMVideoBumper::BOOL_010e7d3d = false;
bool GMVideoBumper::BOOL_010e7da0 = false;
std::string GMVideoBumper::FlashMovie = "";
std::string GMVideoBumper::FlashVariable = "";
std::string GMVideoBumper::FlashVariableValue = "";

GMVideoBumper::GMVideoBumper()
    : GameMode()
    , movieManager()
    , flashLocalization()
    , videoBumperManager( this )
    , drawList()
{
    

    registerManager( &movieManager );
    registerManager( &flashLocalization );
    registerManager( &videoBumperManager );
}

GMVideoBumper::~GMVideoBumper()
{

}

void GMVideoBumper::tick(float deltaTime)
{
    /*int iVar1;
    undefined4 uVar2;
    uint local_8;
    int local_4;

    if ( gpConfig->bEnableMovie ) {
        LoadMovie( movieManager, gActiveVideoName );
    }

    DAT_00fa4942._2_1_ = 0;
    UNK_0129cfa8._8_1_ = 0;
    if ( DAT_010e7d3d != '\0' ) {
        iVar1 = FUN_0099a950( gpFLashPage, &gGSFlash );
        if ( iVar1 != 0 ) {
            FUN_0099b3f0( gpFLashPage, &gGSFlash );
        }
    }
    uVar2 = FUN_0099a490( &local_8, &local_4, "CONFIGPC", &gGSFlash );
    if ( ( char )uVar2 != '\0' ) {
        iVar1 = FUN_0099a780( &gGSFlash, local_8, local_4 );
        if ( iVar1 != 0 ) {
            *( uint* )( iVar1 + 0xac4 ) = *( uint* )( iVar1 + 0xac4 ) | 2;
        }
    }*/
}

void GMVideoBumper::SetNextGameMode( eGameMode mode )
{
    NextGameMode = mode;
    OTDU_LOG_DEBUG("Next game mode set to '%s'\n", kGameModeNames[NextGameMode]);
}

void GMVideoBumper::SetActiveBumper( const std::string& videoFilename )
{
    ActiveVideoName = videoFilename;
    OTDU_LOG_DEBUG("Next video set to '%s'\n", ActiveVideoName.c_str());
}

void GMVideoBumper::ClearActiveBumper()
{
    ActiveVideoName.clear();
}

void GMVideoBumper::SetSkippable()
{
    bCanSkipBumper = true;
}

void GMVideoBumper::OnPlaybackComplete(const char* pFlashMovieName, const char* pFlashVariable, const char* pState, const bool param_4)
{
    FlashMovie = pFlashMovieName;
    FlashVariable = pFlashVariable;
    FlashVariableValue = pState;
    BOOL_010e7da0 = param_4;
}
