#include "shared.h"
#include "gs_cd_key.h"

#include "filesystem/gs_file.h"
#include "core/hash/crc32.h"

GSCDKey* gpCDKey = nullptr;

GSCDKey::GSCDKey()
    : GameSystem()
    , bValidGameKey( false )
    , bValidAddonKey( false )
{
    memset( pGameCDKey, 0, sizeof(char) * kKeyLength );
    memset( pAddonCDKey, 0, sizeof(char) * kKeyLength );

    gpCDKey = this;
}

GSCDKey::~GSCDKey()
{
    gpCDKey = nullptr;
}

bool GSCDKey::initialize( TestDriveGameInstance* )
{  
    loadKeyFromCache();

    char cVar1 = '\0';
    char* pcVar3 = pGameCDKey;
    do {
        cVar1 = *pcVar3;
        pcVar3 = pcVar3 + 1;
    } while (cVar1 != '\0');

    if (pcVar3 == pGameCDKey + 1) {
        bValidGameKey = false;
        return true;
    }

    bValidGameKey = isValidKey(pGameCDKey);

    OTDU_LOG_INFO("bValidGameKey: %i bValidAddonKey: %\n", bValidGameKey, bValidAddonKey);

    return true;
}

void GSCDKey::terminate()
{
    memset( pGameCDKey, 0, sizeof(char) * kKeyLength );
    memset( pAddonCDKey, 0, sizeof(char) * kKeyLength );
    bValidAddonKey = false;
    bValidGameKey = false;
}

bool GSCDKey::isValidKey(const char* param_1)
{
    static constexpr const char* abStack_28 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    size_t keyLength = strlen(param_1);
    if (keyLength == 0x13) {
        uint32_t uVar3 = TestDriveCRC32((uint8_t*)param_1, 0xe);

        if (abStack_28[(uVar3 >> 0x18) % 0x24] == param_1[0xf] && 
            abStack_28[(uVar3 >> 0x10 & 0xff) % 0x24] == param_1[0x10] &&
            abStack_28[(uVar3 >> 8 & 0xff) % 0x24] == param_1[0x11] && 
            abStack_28[(uVar3 & 0xff) % 0x24] == param_1[0x12]) {
            return true;
        }
    }

    return false;
}

void GSCDKey::loadKeyFromCache()
{
    std::wstring pathToKeyCache = gpFile->getSaveFolder();
    pathToKeyCache += L"CDKey.txt";

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string narrowFilename = converter.to_bytes(pathToKeyCache);

    char pvStack_e84[0x20];
    memset(pvStack_e84, 0, sizeof(char) * 0x20);

    FileDirectAccess directFileAccess;
    bool bVar1 = directFileAccess.loadContentIntoMemory(narrowFilename.c_str(), 0x20, 0, (void**)&pvStack_e84, nullptr);
    if (bVar1) {
        pvStack_e84[0x1f] = '\0';
        updateGameKey(pvStack_e84);
    }

    bValidAddonKey = checkAddonKey();
}
    
void GSCDKey::writeKeyToCache()
{
    std::wstring pathToKeyCache = gpFile->getSaveFolder();
    pathToKeyCache += L"CDKey.txt";

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string narrowFilename = converter.to_bytes(pathToKeyCache);

    FileDirectAccess directFileAccess;
    bool bVar1 = directFileAccess.openFile(narrowFilename.c_str(), false, true);
    if (bVar1) {
        directFileAccess.StreamWrite.write(pGameCDKey, sizeof(char) * kKeyLength);
    }
    directFileAccess.close();
}

void GSCDKey::updateGameKey(const char* param_1)
{
    strncpy(pGameCDKey,param_1,0x13);
    pGameCDKey[0x13] = '\0';
    strupr(pGameCDKey);

    writeKeyToCache();

    char cVar1 = '\0';
    char* pcVar3 = pGameCDKey;
    do {
        cVar1 = *pcVar3;
        pcVar3 = pcVar3 + 1;
    } while (cVar1 != '\0');

    if (pcVar3 == pGameCDKey + 1) {
        bValidGameKey = false;
    } else {
        bValidGameKey = isValidKey(pGameCDKey);
    }
}

bool GSCDKey::checkAddonKey()
{
    std::wstring pathToKeyCache = gpFile->getSaveFolder();
    pathToKeyCache += L"Addon.txt";

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string narrowFilename = converter.to_bytes(pathToKeyCache);

    char pvStack_e84[kKeyLength];
    memset(pvStack_e84, 0, sizeof(char) * kKeyLength);

    FileDirectAccess directFileAccess;
    bool bVar1 = directFileAccess.loadContentIntoMemory(narrowFilename.c_str(), kKeyLength, 0, (void**)&pvStack_e84, nullptr);

    if (bVar1) {
        return isValidKey(pvStack_e84);
    }

    return false;
}
