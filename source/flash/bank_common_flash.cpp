#include "shared.h"
#include "bank_common_flash.h"

#include "filesystem/gs_file.h"

BankCommonFlash::BankCommonFlash()
{
}

BankCommonFlash::~BankCommonFlash()
{
}

BankCommonFlash::eBankCommonFlashInit BankCommonFlash::initialize(const bool param_1)
{
    if (bLoaded) {
        return eBankCommonFlashInit::BCFI_Loaded;
    }

    if (param_1) {
        return eBankCommonFlashInit::BCFI_Unknown2;
    }

    OTDU_UNIMPLEMENTED; // TODO:
    return eBankCommonFlashInit::BCFI_Unknown;

    // std::string bankPath = filepath + filename;
    // bVar1 = GSFile::LoadFile(&gGSFile,&param_1->pBankFile,local_400,&param_1->BankSize,
    // &param_1->pMemory->super,".\\Sources\\FlashLoader.cpp",0x4a);
    // if (!bVar1) {
    //     bVar1 = GSFile::FileExists(&gGSFile,local_400);
    //     return (-(uint)bVar1 & 2) + ERROR_NO_FILE_FOUND;
    //}
}
