#pragma once

#include "filesystem/bank.h"
#include "filesystem/streamed_resource.h"
#include "flash_resources.h"
#include "bank_flash.h"

class BankCommonFlash
{
public:
    enum class eBankCommonFlashInit {
        BCFI_Unknown = 0,
        BCFI_Loaded = 1,
        BCFI_Unknown3 = 2,
        BCFI_MustBeSync = 3,
        BCFI_MissingBank = 4,
        BCFI_Unknown2 = 5,
        BCFI_NotEnoughMemory = 6
    };

public:
    inline bool isLoaded() const { return bLoaded; }
    inline void setFilepath( const char* pPath ) { filepath = pPath; }
    inline void setFilename( const char* pFilename ) { filename = pFilename; }

public:
    BankCommonFlash();
    ~BankCommonFlash();

    eBankCommonFlashInit initialize( const bool param_1 );

private:
    std::string filepath;
    std::string filename;
    std::vector<FlashMaterial*> pMaterials;
    std::vector<Flash2DM*> p2DMs;
    std::vector<Render2DB*> p2DBs;
    void* pBankFile;
    uint32_t BankSize;
    BankInstance FlashBankInstance;
    uint8_t bLoaded : 1;
};
