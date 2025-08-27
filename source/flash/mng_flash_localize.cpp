#include "shared.h"
#include "mng_flash_localize.h"

#include "flash_player.h"
#include "tdu_instance.h"

#include "database/gs_database.h"

MngFlashLocalize* gpMngFlashLocalize = nullptr;
char MngFlashLocalize::VersionString[64] = "VERSION UKNOW";

MngFlashLocalize::MngFlashLocalize()
    : Manager()
    , dbContext( 0x0 )
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

int32_t MngFlashLocalize::FUN_00716f60(char* param_1)
{
    OTDU_UNIMPLEMENTED;
    return -1;
}

void MngFlashLocalize::FUN_00717ff0(FlashPlayer* param_2, char* param_3, char* param_4, void* param_5)
{
    // FUN_00717ff0
    if (param_3 != nullptr) {
        size_t pcVar3 = strlen(param_3);
        if (pcVar3 < 0x41) {
            int32_t iVar4 = FUN_00716f60(param_3);
            if (iVar4 < 0) {
                param_2->setVariableValue(param_4, "(ERR:1)");
                return;
            }
            
            int32_t iVar5 = 0;
            char* local_228[10] = {
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            };

            char** ppcVar7 = nullptr;
            if (0 < iVar4) {
                int32_t iVar6 = 0;
                ppcVar7 = &param_3;
                do {
                    ppcVar7 = ppcVar7 + 1;
                    if ((iVar5 < 0) || (iVar6 = iVar5, 9 < iVar5)) {
                        iVar6 = 0;
                    }
                    local_228[iVar6] = *ppcVar7;
                    if ((iVar5 < 0) || (iVar6 = iVar5, 9 < iVar5)) {
                        iVar6 = 0;
                    }
                } while ((local_228[iVar6] != (char *)0x0) && (iVar5 = iVar5 + 1, iVar5 < iVar4));
            }
            ppcVar7 = local_228;
          
            OTDU_UNIMPLEMENTED;
        }
        OTDU_UNIMPLEMENTED;
    }

    OTDU_UNIMPLEMENTED;
}

void MngFlashLocalize::FUN_00718120(FlashPlayer *param_2, char *param_3, uint64_t param_4)
{
    // FUN_00718120
    char* pcVar1 = gpDatabase->getStringByHashcode(dbContext, param_4);                       
    void* pcVar3 = nullptr; //FUN_00416450(local_2c,(char *)&param_1); // TODO: Not sure wtf this is (the arg is never consumed by FUN_00717ff0)
    FUN_00717ff0(param_2, param_3, pcVar1, pcVar3);
}

void  MngFlashLocalize::resetMenuLocalization()
{
    menuLocalizedStrings.clear();
    menuLocalizedStrings.reserve( 0xa ); // Original code uses c-array of 10 - 00716d1d 83 f8 0a        CMP        EAX,0xa
}