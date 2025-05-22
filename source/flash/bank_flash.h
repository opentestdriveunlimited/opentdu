#pragma once

#include "filesystem/bank.h"
#include "filesystem/streamed_resource.h"

#include "render/file_collection_2d.h"

class DrawList;
class BankCommonFlash;
struct FlashPlayer;
struct FlashMaterial;
struct Flash2DM;
struct FlashSound;
class Render2DB;

class BankFlash : public Bank, public StreamedResource
{
public:
    using UserCallback_t = std::function<void(char *, FlashPlayer*)>;

    // TODO: Looks the same as eBankCommonFlashInit. 
    // Quantify to avoid useless dupe?
    enum class eBankFlashInit {
        BFI_Unknown = 0,
        BFI_Loaded = 1,
        BFI_Unknown2 = 2,
        BFI_Unknown3 = 3,
        BCFI_MustBeSync = 4,
        BCFI_MissingBank = 5,
        BCFI_Unknown4 = 6,
        BFI_NotEnoughMemory = 7
    };

public:
    inline bool isInUse() const { return bInUse; }
    inline BankFlash::eBankFlashInit getState() const { return state; }
    inline FlashPlayer* getFlashPlayer() const { return pFlashPlayer; }

public:
    BankFlash();
    ~BankFlash();

private:
    const char* pFilepath;
    const char* pFilename;
    BankCommonFlash* pCommonBank;
    FlashPlayer* pFlashPlayer;
    DrawList* drawList;
    FileCollection2D fileCollection;
    UserCallback_t playerCallback;
    int32_t maxNumStrips;
    int32_t maxNumVertices;
    int32_t maxNumFlashVars;
    eBankFlashInit state;
    std::vector<FlashMaterial*> pMaterials;
    std::vector<Flash2DM*> p2DMs;
    std::vector<Render2DB*> p2DBs;
    std::vector<FlashSound*> pSounds;
    void* pResource;
    void* pArena;
    uint32_t arenaSize;
    uint8_t bInUse : 1;
    uint8_t bLoaded : 1;
};
