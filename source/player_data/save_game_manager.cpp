#include "shared.h"
#include "save_game_manager.h"

#include "tdu_instance.h"
#include "user_profile.h"

#include "filesystem/gs_file.h"

bool BOOL_010e7364 = false; // Always false at runtime

static constexpr uint32_t kBufferSize = 0x2000;
static uint8_t DAT_01041f50[kBufferSize];

static std::wstring FormatPlayerSaveTimestamp(const wchar_t* pSaveType)
{
    // FUN_0097bed0
    // NOTE: Replaced GetLocalTime() with time (for x compatibility)
    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);

    wchar_t formatBuffer[260];
    memset(formatBuffer, 0, sizeof(wchar_t) * 260);
    swprintf(formatBuffer, 260, L"%s (%d/%d/%02d - %d:%02d'%02d'')", pSaveType, info->tm_mon, info->tm_wday, info->tm_year, 
        info->tm_hour, info->tm_min, info->tm_sec);

    return formatBuffer;
}

static bool FUN_0097c0d0(char *param_1,std::wstring& param_2)
{
    bool bVar7 = false;
    bool bVar8 = false;

    FileDirectAccess file;
    file.bNeedName = false;

    uint32_t uVar4 = gUserProfileRegister.FUN_00419de0();
    uint64_t hashcode = gUserProfileRegister.getActiveProfileHashcode();

    bool bVar1 = file.openContext(hashcode, param_1, eContextSource::CS_Savegame);
    if (bVar1) {
        OTDU_UNIMPLEMENTED;

        file.reset();
        
        if (bVar7 && bVar8) {
            return true;
        }
    }

    return false;
}

SavegameManager gSavegameManager = {};

SavegameManager::SavegameManager()
    : event()
    , mutex()
    , pendingSaveCount( 0 )
    , unknownDword( 0 )
    , bProfileChange( false )
    , bUnknownBool2( true )
    , unknownDword2( 0 )
    , bUnknownBool3( false )
    , bUnknownBool4( false )
    , bUnknownBool1( false )
{
    memset(DAT_01041f50, 0, sizeof(uint8_t) * kBufferSize);
}

SavegameManager::~SavegameManager()
{

}

void SavegameManager::create()
{
    // FUN_0097c830
    if (BOOL_010e7364) {
        // TODO: Looks like this is a dead branch (which somehow did not get culled at compile time)
        OTDU_UNIMPLEMENTED;
    }

    pendingSaveCount = 0;
    unknownDword = 0;
    bProfileChange = false;
    bUnknownBool2 = true;
    unknownDword2 = 0;
    bUnknownBool3 = false;
    bUnknownBool4 = false;
    bUnknownBool1 = false;
}

bool SavegameManager::doSaveBlocking()
{
    // FUN_0097c2f0
    if ((gpTestDriveInstance->hasRequestedExit()) || (bUnknownBool4 == false)) {
        return false;
    }

    ScopedMutexLock lock(&mutex);

    unknownDword2 = 0;
    uint32_t uVar1 = unknownDword;
    uint32_t uVar2 = pendingSaveCount;
    gUserProfileRegister.FUN_0060a160();

    OTDU_UNIMPLEMENTED;

    return true;
}

bool SavegameManager::isProfileChangeInProgress() const
{
    // FUN_00406630
    return bProfileChange;
}

void SavegameManager::toggleProfileChange()
{
    // FUN_0097bd90
    if (!bProfileChange) {
        bProfileChange = true;
    }
}

void SavegameManager::setUnknownBool2(bool param_1)
{
    // FUN_00406620
    bUnknownBool2 = param_1;
}

bool SavegameManager::getUnknownBool2() const
{
    // FUN_00406610
    return bUnknownBool2;
}

bool SavegameManager::isDword2Zero() const
{
    // FUN_00406600
    return unknownDword2 != 0;
}

bool SavegameManager::isDwordDiffFromPendingSaveCount() const
{
    // FUN_00406640
    return pendingSaveCount != unknownDword;
}

void SavegameManager::increaseNumSaveCount()
{
    // FUN_00406640
    pendingSaveCount++;
}
