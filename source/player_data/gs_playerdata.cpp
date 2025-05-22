#include "shared.h"
#include "gs_playerdata.h"

#include "game/mng_hud.h"
#include "input/gs_device.h"

#include "core/mutex.h"

#include "tdu_instance.h"

GSPlayerData* gpPlayerData = nullptr;

GSPlayerData::GSPlayerData()
    : bInitialized( false )
    , bUnknownBoolLanguage( false )
    , bUseUSFontPage( true )
    , bTutorialCompleted( false )
    , padSensitivity( -7.0f * 0.06666667f )
    , barSensitivity( 0.5f )
    , mouseSensitivity( 0.5f )
    , seatPositionHeight( 0.0f )
    , seatPositionPosition( 0.0f )
    , resolutionWidth( 1280 )
    , resolutionHeight( 720 )
    , refreshRate( 60 )
    , language( "us" )
    , timeout( 0.0f )
    , pMutex( new TestDriveMutex() )
{
    pMutex->initialize();
    //bOnlineOps[0] = DAT_014d5608 == 0;
    //bOnlineOps[1] = true;
    //bOnlineOps[2] = false;
    //bOnlineOps[3] = true;
    //bOnlineOps[4] = true;
    //bOnlineOps[5] = true;
    //bOnlineOps[6] = true;
    //bOnlineOps[7] = true;
    //bOnlineOps[8] = true;

    //{
    //    std::unique_lock lock( parameterMutex );

    //    FUN_008980c0( param_1->Language );
    //    if ( PTR_011403d0 != ( undefined* )0x0 ) {
    //        FUN_00896bd0( &param_1->field_0x184, PTR_011403d0 );
    //    }

    //    timeout = -144000.1;
    //    bUnusedFlag = true;
    //}

    gpPlayerData = this;
}

GSPlayerData::~GSPlayerData()
{

}

bool GSPlayerData::initialize( TestDriveGameInstance* )
{
    if ( bInitialized ) {
        return true;
    }
    padSensitivity = ( static_cast<float>( gDefaultSensitivity ) - 1.0f ) * 0.06666667f;

    return true;
}

void GSPlayerData::tick()
{

}

void GSPlayerData::terminate()
{

}

void GSPlayerData::setDefaultLanguage( const char* pDefaultLanguage, bool bUnknownBoolean )
{
    language = PlayerDataLanguage( pDefaultLanguage );

    bUnknownBoolLanguage = bUnknownBoolean;

    if ( gpTestDriveInstance && gpTestDriveInstance->getActiveGameMode() != GM_Login ) {
        updateFontPage();
    }
}

void GSPlayerData::setDefaultSeatPosition( const float height, const float depth )
{

}

void GSPlayerData::updateFontPage()
{
    loadPage( language == kLangUS );
}

void GSPlayerData::loadPage( const bool bLoadDefaultFontPage )
{
    bUseUSFontPage = bLoadDefaultFontPage;
    if ( gpMngHud != nullptr ) {
        gpMngHud->updateSettings();
    }

    // TODO: Requires GSFlash to be implemented
    OTDU_UNIMPLEMENTED;
    //iVar1 = FUN_0099a780( &gGSFlash, 1, 0 );
    //if ( ( ( iVar1 != 0 ) && ( *( int* )( iVar1 + 0x2dc ) == 1 ) ) && ( *( int* )( iVar1 + 0x2a8 ) != 0 ) ) {
    //    if ( bUseUSFontPage ) {
    //        iVar1 = FUN_0099a780( &gGSFlash, 1, 0 );
    //        if ( ( iVar1 != 0 ) && ( *( int* )( iVar1 + 0x2dc ) == 1 ) ) {
    //            FUN_00440ba0( "/:state_Metric", ( TDUFlashVariable* )0x0, *( int** )( iVar1 + 0x2a8 ) );
    //            return;
    //        }
    //        FUN_00440ba0( "/:state_Metric", ( TDUFlashVariable* )0x0, ( int* )0x0 );
    //        return;
    //    }
    //    iVar1 = FUN_0099a780( &gGSFlash, 1, 0 );
    //    if ( ( iVar1 != 0 ) && ( *( int* )( iVar1 + 0x2dc ) == 1 ) ) {
    //        FUN_00440ba0( "/:state_Metric", ( TDUFlashVariable* )&DAT_00000001, *( int** )( iVar1 + 0x2a8 ) );
    //        return;
    //    }
    //    FUN_00440ba0( "/:state_Metric", ( TDUFlashVariable* )&DAT_00000001, ( int* )0x0 );
    //}
}
