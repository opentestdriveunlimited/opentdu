#include "shared.h"
#include "mng_flash_localize.h"

#include "flash_player.h"
#include "tdu_instance.h"

MngFlashLocalize* gpMngFlashLocalize = nullptr;
char MngFlashLocalize::VersionString[64] = "VERSION UKNOW";

MngFlashLocalize::MngFlashLocalize()
    : Manager()
    , unknown( 0x0 )
{
    localizedStrings.reserve( 32 );
    reset();

    if (gpMngFlashLocalize == nullptr) {
        gpMngFlashLocalize = this;
    }
}

MngFlashLocalize::~MngFlashLocalize()
{

}

bool MngFlashLocalize::initialize( TestDriveGameInstance* )
{
    strcpy(MngFlashLocalize::VersionString, TestDrive::GetVersion());
    return true;
}

void MngFlashLocalize::reset()
{
    localizedStrings.clear();
    localizedStrings.reserve( 0x20 ); // Original code uses c-array of 32 - 00716f30 83 f8 20        CMP        EAX,0x20

    resetMenuLocalization();
}

int32_t MngFlashLocalize::FUN_00716f60(uint64_t param_1)
{
    OTDU_UNIMPLEMENTED;
    return -1;
}

void MngFlashLocalize::FUN_00717ff0(FlashPlayer* param_2, uint64_t param_3, uint64_t param_4, uint32_t param_5)
{
    OTDU_UNIMPLEMENTED;
}

void  MngFlashLocalize::resetMenuLocalization()
{
    menuLocalizedStrings.clear();
    menuLocalizedStrings.reserve( 0xa ); // Original code uses c-array of 10 - 00716d1d 83 f8 0a        CMP        EAX,0xa
}