#include "shared.h"
#include "gs_file.h"

#include "core/mutex.h"
#include "tdu_instance.h"
#include "config/gs_config.h"
#include "player_data/user_profile.h"
#include "gs_dirty_disk.h"
#include "flash/modal_message_box.h"
#include "file_path_register.h"

#if defined( OTDU_WIN32 )
#include <Shlobj.h>
#endif

#include <locale>
#include <codecvt>

GSFile* gpFile = nullptr;

static TestDriveMutex gUnknownMutex = {};

TestDriveMutex* gpFilemapMutex = nullptr; // TODO: Define the thread routine in GSFile to avoid doing this crap

static constexpr const char* kCameraBinPath = "Euro/BNK/DataBase/Cameras.bin";

static bool CreateFolders( const std::wstring& path ) 
{
    std::error_code ec;

    bool bSuccess = std::filesystem::create_directories(path, ec);
    if (!bSuccess) {
        OTDU_LOG_ERROR("Failed to create directory '%S': %s\n", path.c_str(), ec.message().c_str());
    }

    return bSuccess;
}

static uint32_t ThreadRoutine( WorkerThreadContext* param_1 )
{
    FileDirectAccess local_c48 = {};

    do {
        param_1->pEvent->waitForEvent();
        gpFilemapMutex->lock();
        if (param_1->pAsyncData->pFilemap == nullptr) {
            local_c48.bNeedName = true;
        } else {
            local_c48.bNeedName = param_1->pAsyncData->pFilemap->bNeedName;

            if (param_1->pAsyncData->pFilemap->Context.bIsValid) {
                bool bVar3 = local_c48.openContext(0ull, 
                    param_1->pAsyncData->pFilemap->Context.Filename.c_str(), 
                    param_1->pAsyncData->pFilemap->Context.Source
                );

                if (!bVar3) {
                    param_1->bJobDone = true;
                    gpFilemapMutex->unlock();
                    continue;
                }
            }
        }

        bool bVar4 = false;
        {
            ScopedMutexLock unknownLock(&gUnknownMutex);

            if (param_1->pAsyncData->bCompressed) {
                OTDU_UNIMPLEMENTED; // TODO: AFAIK zlib compressed banks are only used on Xbox (we'll keep this assert around just in case)
            } else {
                bVar4 = local_c48.loadContentIntoMemory(
                    param_1->pAsyncData->Filename.c_str(),
                    param_1->pAsyncData->Size,
                    param_1->pAsyncData->Offset,
                    &param_1->pAsyncData->pBuffer,
                    nullptr
                );
            }
        }
        
        if (!bVar4 && param_1->pAsyncData->pFilemap != nullptr) {
            gpDirtyDisk->onDataLoadFailure();
        }

        if (param_1->pAsyncData->pFilemap != nullptr && param_1->pAsyncData->pFilemap->Context.bIsValid) {
            local_c48.reset();
        }
        
        gpFilemapMutex->unlock();
        param_1->bJobDone = true;
    } while ( true );

    return 0;
}

GSFile::GSFile()
    : pFilemapMutex( new TestDriveMutex() )
    , pWorkerThread( new TestDriveThread() )
    , initStep( 0 )
    , bFilemapActive( false )
    , bTaskInProgress( false )
    , bIsCurrentContextReady( false )
    , bDeletingInProgress( false )
{
    gpFile = this;

    filemaps.resize( 127u );
    for ( FileMap& filemap : filemaps ) {
        filemap.initializeCRCLUT();
    }

    gpFilemapMutex = pFilemapMutex;

    workerThreadContext.pEvent = new TestDriveEvent();
    pWorkerThread->initialize( (TestDriveThread::StartRoutine_t)ThreadRoutine, &workerThreadContext, 0, 0 );
}

GSFile::~GSFile()
{
    delete pFilemapMutex;
}

bool GSFile::initialize( TestDriveGameInstance* )
{
    bool bInitialized = retrieveSaveFolder();

    hardDriveVFS.initialize();
    gVirtualFileSystemRegister.registerVFS(&hardDriveVFS);

    gUserProfileRegister.deserialize();
    
    // std::string defaultRootPath = "<pc>";
    // std::string local_200;
    // VirtualFileSystem* peVar1 = gVirtualFileSystemRegister.get(defaultRootPath, local_200, true);
    // if (peVar1 != nullptr) {
    //     gFilePathRegister.registerPath(local_200);
    //     gFilePathRegister.setDefaultPath(local_200);
    //     peVar1->setDefaultPath(local_200.c_str());
    // }

    return bInitialized;
}

void GSFile::tick(float deltaTime)
{
    if (gpTestDriveInstance == nullptr || !gpTestDriveInstance->hasRequestedExit()) {
        if (initStep == 1) {
            //gpFlashMessageBox->display(0x3b863c2, 3.0f, 0, nullptr, nullptr, false);
            
            // puVar1 = (undefined4 *)&gGSDatabase.field_0x43fc;
            // do {
            //     puVar1[-1] = 0;
            //     *puVar1 = 0;
            //     puVar1 = puVar1 + 2;
            // } while ((int)puVar1 < 0x11f30bc);
            initStep = 2;
        } else if (initStep == 2) {
            initStep = 0;
        }
    }
}

void GSFile::terminate()
{
    pFilemapMutex->terminate();
}

bool GSFile::loadFile(const char *pFilename, void **pOutContent, uint32_t *pOutContentSize)
{
    *pOutContent = (void *)0x0;
    if (pOutContentSize != nullptr) {
        *pOutContentSize = 0u;
    }

    FileMapEntry* pEntry = nullptr;
    FileMap* pFilemap = nullptr;
    uint32_t fileSize = 0;
    if (!bFilemapActive) {
        bool bVar2 = findFileOutsideFilemap(pFilename, &fileSize);
        if (!bVar2) {
            return false;
        }
    } else {
        bool bVar3 = findMapEntriesForFile(pFilename, &pEntry, &pFilemap);
        
        // If not found in the map, check external
        if (!bVar3) {
            std::string filename = pFilename;
            const char* pResRoot = gpConfig->getResRootPath();
            size_t iVar5 = filename.rfind(pResRoot, 0);
            if (iVar5 == 0) {
                return false;
            }
            bool bVar2 = findFileOutsideFilemap(pFilename, &fileSize);
            if (!bVar2) {
                return false;
            }
        }

        fileSize = pEntry->Size;
    }

    // TODO: What's the 0x801 required for?

    void* pFileAlloc = TestDrive::Alloc(fileSize + 0x801);
    OTDU_LOG_DEBUG("Allocated %llu for file '%s'\n", fileSize + 0x801, pFilename);
    *pOutContent = pFileAlloc;
    if (pFileAlloc != nullptr) {
        *(uint32_t*)*pOutContent = fileSize;

        bool bLoadedContentFromDisk = loadFileRaw(pFilename, pFilemap, pEntry, fileSize, pOutContent);
        if (bLoadedContentFromDisk) {
            uint8_t* pFileContentByte = (uint8_t*)*pOutContent;
            pFileContentByte[fileSize] = 0;
            if (pOutContentSize != nullptr) {
                *pOutContentSize = fileSize;
            }
            return true;
        }

        TestDrive::Free(pFileAlloc);
        *pOutContent = (void *)0x0;
        
        return false;
    }

    return false;
}

bool GSFile::retrieveSaveFolder()
{
    savegameFolder = L"\\";

    // TODO: Move platform specific under system/
#if defined( OTDU_WIN32 )
    HANDLE tokenHandle = NULL;
    HANDLE ProcessHandle = GetCurrentProcess();
    OpenProcessToken( ProcessHandle, 0x200ec, &tokenHandle );

    PWSTR pSavegameFolderPath = nullptr;
    HRESULT operationResult = SHGetKnownFolderPath( FOLDERID_Documents, 
                                                    KF_FLAG_DEFAULT, 
                                                    tokenHandle, 
                                                    &pSavegameFolderPath );
    if ( operationResult != S_OK ) {
        return false;
    }

    savegameFolder = pSavegameFolderPath;
    savegameFolder += L"\\Test Drive Unlimited\\";
#elif defined( OTDU_UNIX )
    std::string homeDirectory = getenv("HOME");
    if (homeDirectory.empty()) {
        return false;
    }

    savegameFolder = std::wstring(homeDirectory.begin(), homeDirectory.end());
#ifdef OTDU_MACOS
    savegameFolder += L"/Library/Application Support";
#endif

    savegameFolder += L"/Test Drive Unlimited/";
#else
    static_assert(false, "Unimplemented" );
    return false;
#endif

    OTDU_LOG_DEBUG("Active savegame folder: '%S'\n", savegameFolder.c_str());

    return true;
}

bool GSFile::findFileOutsideFilemap(const char* pFilename, uint32_t* pOutFilesize)
{
    OTDU_ASSERT(pOutFilesize);

    FileIterator it;
    bool bVar2 = it.findFirstMatch(pFilename);
    if (!bVar2) {
        OTDU_LOG_ERROR("Failed to open file '%s': file not found\n", pFilename);
        return false;
    }

    uint32_t fileSize = it.getSize();
    *pOutFilesize = fileSize;
    it.closeHandle();

    if (fileSize == 0) {
        OTDU_LOG_ERROR("Failed to open file '%s': file size is 0\n", pFilename);
        return false;
    }

    return true;
}

bool GSFile::findMapEntriesForFile(const char *pEntryName, FileMapEntry** ppOutMapEntry, FileMap **ppOutMap)
{
    ScopedMutexLock filemapLock(pFilemapMutex);
    *ppOutMapEntry = nullptr;
    *ppOutMap = nullptr;

    FileMap* pBestMap = nullptr;
    FileMapEntry* pBestEntry = nullptr;
    int32_t filemapIndex = 0;
    for (FileMap& filemap : filemaps) {
        filemapIndex++;

        if (filemap.MapSize == 0) {
            continue;
        }

        FileMapEntry* pFoundEntry = nullptr;
        bool bFoundEntryInMap = filemap.findEntryInSet(pEntryName, &pFoundEntry);
        if (bFoundEntryInMap) {
            if (pBestEntry != nullptr) {
                if (pBestEntry->Version < pFoundEntry->Version) {
                    OTDU_LOG_DEBUG("(found best revision=>%08X on map %d)", pFoundEntry->Version, filemapIndex);
                    pBestEntry = pFoundEntry;
                    pBestMap = &filemap;
                }
            } else {
                pBestEntry = pFoundEntry;
                pBestMap = &filemap;
                if (pFoundEntry->Version != 0) {
                    OTDU_LOG_DEBUG("(found revision=>%08X on map %d)", pFoundEntry->Version, filemapIndex);
                }
            }
        }
    }

    if (pBestEntry == nullptr) {
        *ppOutMap = nullptr;
        *ppOutMapEntry = nullptr;
        return false;
    }

    *ppOutMap = pBestMap;
    *ppOutMapEntry = pBestEntry;
    return true;
}

bool GSFile::loadFileRaw(const char *pFilename, FileMap* pFilemap, FileMapEntry* pEntry, const uint32_t size, void** ppOutContent)
{
    FileDirectAccess local_c48;
    bTaskInProgress = true;

    std::string filename = "";
    uint32_t offset = 0;
    bool bCompressed = false;
    if (pFilemap == nullptr || pEntry == nullptr) {
        FileIterator it;
        it.findFirstMatch(pFilename);

        filename = it.SearchDirectory;
        filename += "/";
        filename += it.SearchPattern;
            
        tmpFilename.clear();
        tmpFilename += pFilename;
        
        local_c48.bNeedName = true;
    } else {
        local_c48.bNeedName = pFilemap->bNeedName;
        bCompressed = pEntry->bCompressed;

        if (!pFilemap->bUseBigFile) {
            filename = pFilemap->resolveName(pFilename, bCompressed);
        } else {
            offset = pEntry->Offset;
            filename = pFilemap->FilenameBigFile;
        }
    }

    bool bVar1 = false;
    if (pFilemap == nullptr || !pFilemap->Context.bIsValid) {
        bVar1 = true;
    } else {
        bVar1 = local_c48.openContext(0, pFilemap->Context.Filename.c_str(), pFilemap->Context.Source);
    }

    {
        ScopedMutexLock unknownMutexLock(&gUnknownMutex);
        if (bVar1) {
            if (bCompressed) {
                OTDU_ASSERT(false); // TODO: AFAIK zlib compressed banks are only used on Xbox (we'll keep this assert around just in case)
            } else {
                bVar1 = local_c48.loadContentIntoMemory(filename.c_str(), size + 0x7ffU & 0xfffff800, offset, ppOutContent, nullptr);
            }
        }
    }
    if (!bVar1) {
        if (pFilemap != nullptr) {
            gpDirtyDisk->onDataLoadFailure();
        }
    }

    if (pFilemap != nullptr && pFilemap->Context.bIsValid) {
        local_c48.reset();
    }
    
    bTaskInProgress = false;
    return bVar1;
}

void GSFile::initCarPacks(bool param_1)
{
    // TODO: This could be re-used to support standalone mods (just need to re-implement the missing
    // XML parsing)
    ScopedMutexLock filemapLock(pFilemapMutex);

    FileDirectAccess file;
    file.bNeedName = false;

    uint64_t profileHashcode = gUserProfileRegister.getActiveProfileHashcode();

    uint32_t uStack_aa58 = 0;
    EnumerateContextItem aeStack_9e00[129];
    bool bVar1 = file.enumerateContext(
        profileHashcode,
        eContextSource::CS_Marketplace,
        0,
        nullptr,
        nullptr,
        0x80,
        aeStack_9e00,
        &uStack_aa58
    );

    OTDU_LOG_INFO("=>EnumerateContext() => %s, %d items\n", ((bVar1) ? "TRUE" : "FALSE"), uStack_aa58 );
    if (param_1) {
        OTDU_UNIMPLEMENTED;
        //FUN_008a3750(0x147aef0);
    }

    for (uint32_t i = 0; i < uStack_aa58; i++) {
        EnumerateContextItem& item = aeStack_9e00[i];
        if (!item.bIsValid) {
            continue;
        }

        OTDU_LOG_INFO("  -Found Pack: Name:%S  Type:%d DeviceID=%d\n", item.DisplayName.c_str(), item.Source, item.StoredOnHDD);
        
        OTDU_UNIMPLEMENTED; 
    }
}

std::wstring GSFile::getSaveLocation(eContextSource source)
{
    std::wstring saveLocation = getSaveFolder();

    switch ( source ) {
    case eContextSource::CS_Publisher:
        saveLocation += L"publisher";
        break;
    case eContextSource::CS_Marketplace:
        saveLocation += L"marketplace";
        break;
    case eContextSource::CS_Savegame:
        saveLocation += L"savegame";
        break;
    default:
        break;
    }

    return saveLocation;
}

bool GSFile::hasPendingAsyncRequests()
{
    OTDU_UNIMPLEMENTED;
    return true;
}

void GSFile::flushPendingAsync()
{
    bool bVar1 = hasPendingAsyncRequests();
    if (bVar1) {
        do {
            syncAsyncWorkerThread();
            bVar1 = hasPendingAsyncRequests();
        } while (bVar1);
    }

    OTDU_IMPLEMENTATION_SKIPPED( "FUN_00609a20" );
    gpTestDriveInstance->flushPendingFileInstanciation(false);
}

void GSFile::syncAsyncWorkerThread()
{
    if (!workerThreadContext.bRunning) {
        auto it = asyncData.find(workerThreadContext.pAsyncData);

        // Find the next async request in the set.
        // TODO: Use a queue instead of whatever the game used?
        for ( ; it != asyncData.end(); it++ ) {
            if ((*it)->bInUse && !(*it)->bCanceled) {
                break;
            }
        }

        if (it == asyncData.end()) {
            return;
        }
        
        workerThreadContext.bRunning = true;
        workerThreadContext.bJobDone = false;
        workerThreadContext.pAsyncData = (*it);
        workerThreadContext.pEvent->reset();
    } else if (workerThreadContext.bJobDone) {
        workerThreadContext.bRunning = false;
        workerThreadContext.bJobDone = false;
        bTaskInProgress = false;

        if (!workerThreadContext.pAsyncData->bCanceled) {
            if (workerThreadContext.pAsyncData->pCompletionCallback != nullptr) {
                workerThreadContext.pAsyncData->pCompletionCallback(4, 1);
                return;
            }
        }
    }
}

AsyncFileOpen::AsyncFileOpen()
    : Handle()
    , Filename( "" )
    , pUserData( nullptr )
    , pBuffer( nullptr )
    , pCompletionCallback( nullptr )
    , Size( 0 )
    , Offset( 0 )
    , bInUse( false )
    , bCanceled( false )
    , bCompressed( false )
    , bHighPriority( false )
    , pFilemap( nullptr )
{

}

void FileMap::initializeCRCLUT()
{
    uint32_t uVar1 = 0;
    uint32_t uVar2 = 0;

    do {
        uint32_t uVar1 = uVar2 >> 1;
        if ( ( uVar2 & 1 ) != 0 ) {
            uVar1 = uVar1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        if ( ( uVar1 & 1 ) == 0 ) {
            uVar1 = uVar1 >> 1;
        } else {
            uVar1 = uVar1 >> 1 ^ 0xedb88320;
        }
        pCRCLUT[uVar2] = uVar1;
        uVar2 = uVar2 + 1;
    } while ( uVar2 < 256 );
}

bool FileMap::findEntryInSet(const char *pFilename, FileMapEntry **ppOutEntry)
{
    std::string local_104 = pFilename;
    local_104 += ".zc";
    
    if (MapSize == 0) {
        *ppOutEntry = nullptr;
        return false;
    }

    FileMapEntry* pFoundEntry = findEntry(local_104.c_str());
    if (pFoundEntry == nullptr) {
        pFoundEntry = findEntry(pFilename);
        if (pFoundEntry == nullptr) {
            *ppOutEntry = nullptr;
            return false;
        }
    }
    
    *ppOutEntry = pFoundEntry;
    return true;
}

FileMapEntry *FileMap::findEntry(const char *pFilename)
{
    if (HeaderToSkip.length() > 0) {
        std::string filename = pFilename;
        size_t iVar3 = filename.rfind(HeaderToSkip, 0);
        if (iVar3 == 0) {
            std::string filenameWithoutHeader = filename.substr(HeaderToSkip.length());
            uint32_t hashcode = hashFilename(filenameWithoutHeader.c_str());
            return findEntry(hashcode);
        }
    }

    uint32_t hashcode = hashFilename(pFilename);
    return findEntry(hashcode);
}

FileMapEntry *FileMap::findEntry(const uint32_t crc)
{
    if (pMapStart == nullptr) {
        return nullptr;
    }

    if (bUnknown) {
        int8_t* pMapIterator = (int8_t*)pMapStart; 
        for ( uint32_t i = 0; i < MapSize; i += 0x18 /* sizeof(SerializedFilemapEntry)*/ ) {
            Entry.deserialize(pMapIterator + i);
            if (Entry.CRC == crc) {
                return &Entry;
            }
        }
    }

    int32_t iVar6 = 0;
    int32_t iVar4 = MapSize / 0x18 - 1;
    if (-1 < iVar4) {
        int8_t* pMapIt = (int8_t*)pMapStart;
        do {
            int32_t iVar5 = (iVar4 - iVar6) / 2 + iVar6;
            Entry.deserialize(pMapIt + iVar5 * 0x18);
            
            if (crc == Entry.CRC) {
                return &Entry;
            }

            if (crc < Entry.CRC) {
                iVar4 = iVar5 + -1;
            } else {
                iVar6 = iVar5 + 1;
            }
        } while (iVar6 <= iVar4);
    }

    return nullptr;
}

void FileMap::normalizeFilename(const char *pFilename)
{
    CurrentFile.clear();
    
    std::string normalizedName = pFilename;
    std::replace( normalizedName.begin(), normalizedName.end(), '\\', '/' );

    std::transform( normalizedName.begin(), normalizedName.end(), normalizedName.begin(), tolower );

    CurrentFile = normalizedName;
}

uint32_t FileMap::hashFilename(const char *pFilename)
{
    normalizeFilename(pFilename);
    return crc32( CurrentFile.c_str(), CurrentFile.length() );
}

uint32_t FileMap::crc32(const char *param_1, size_t param_2)
{
    uint32_t uVar1 = 0xffffffff;
    if (0 < param_2) {
        do {
            uVar1 = uVar1 >> 8 ^ pCRCLUT[(*param_1 ^ uVar1) & 0xff];
            param_1 = (char *)(param_1 + 1);
            param_2 = param_2 + -1;
        } while (param_2 != 0);
    }

    return ~uVar1;
}

std::string &FileMap::resolveName(const char *pFilename, const bool bCompressed)
{
    TempFilename.clear();

    std::string filename = pFilename;
    if (HeaderToSkip.length() > 0) {
        size_t iVar3 = filename.rfind(HeaderToSkip, 0);
        if (iVar3 == 0) {
            filename = filename.substr(HeaderToSkip.length());
        }
    }

    std::string extension = bCompressed ? ".zc" : "";
    TempFilename += HeaderToSkip;
    TempFilename += filename;
    TempFilename += extension;

    return TempFilename;
}

SaveQueue::SaveQueue()
    : pMutex( new TestDriveMutex() )
    , bPauseFlush( false )
    , DisplayName( L"NO NAME" )
{

}

SaveQueue::~SaveQueue()
{
    delete pMutex;
}

FileDirectAccess::FileDirectAccess()
    : CurrentHashcode( 0 )
    , pContext( nullptr )
    , pHandle( nullptr )
    , StoredOnHDD( 0 )
    , bNeedName( false )
    , bIsOpen( false )
{

}

void FUN_006078d0(std::string& param_2)
{
    size_t pcVar2 = param_2.find('>');
    if (pcVar2 == std::string::npos) {
        return;
    }

    size_t iVar3 = param_2.length() - pcVar2;
    for (size_t i = pcVar2; i < param_2.length(); i++) {
        param_2[iVar3] = param_2[i];
    }
}

bool FileDirectAccess::enumerateContext(
    const uint64_t hashcode, 
    const eContextSource source,
    const uint32_t param_3,
    char *param_5,
    char *param_6,
    uint32_t param_7,
    EnumerateContextItem *param_8,
    uint32_t *param_9)
{  
    *param_9 = 0;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring saveFolder = gpFile->getSaveLocation( source );
    if (saveFolder.empty()) {
        return false;
    }

    std::string narrowSaveFolder = converter.to_bytes(saveFolder);
    if (!bNeedName) {
        AbsoluteFilename = narrowSaveFolder;
    } else {
        VirtualFileSystem* pVFS = gVirtualFileSystemRegister.get(NormalizedFilename, narrowSaveFolder, false);
        if (pVFS == nullptr) {
            return false;
        }
        FUN_006078d0(NormalizedFilename);
    }

    std::string profileName = gUserProfileRegister.getProfileName(hashcode);
    if (profileName.empty()) {
        return false;
    }

    FolderName = converter.from_bytes(AbsoluteFilename);
    FolderName += L"/";
    FolderName += converter.from_bytes(profileName);
    FolderName += L"/";
#if 0
    FolderName += L"/*.*/";
#endif

    char* pcVar5 = param_6;
    char* pcVar11 = param_5;
    if (param_5 != (char *)0x0) {
        do {
            char* pcVar12 = pcVar5;
            for (int32_t iVar9 = 0x4f; iVar9 != 0; iVar9 = iVar9 + -1) {
                pcVar12[0] = '\0';
                pcVar12[1] = '\0';
                pcVar12[2] = '\0';
                pcVar12[3] = '\0';
                pcVar12 = pcVar12 + 4;
            }
            *pcVar5 = '\0';
            pcVar5[4] = -1;
            pcVar5[5] = '\0';
            pcVar5[6] = '\0';
            pcVar5[7] = '\0';
            pcVar11 = pcVar11 + -1;
            pcVar5 = pcVar5 + 0x13c;
        } while (pcVar11 != (char *)0x0);
    }

    OTDU_LOG_DEBUG("Enumerating items in folder '%S'...\n", FolderName.c_str());
    if (!std::filesystem::exists(FolderName.c_str())) {
        OTDU_LOG_WARN("Directory '%S' does not exist; skipping item enumerate...\n", FolderName.c_str());
        return true;
    }

    OTDU_UNIMPLEMENTED; // This is only used on Xbox AFAIK; skip decomp unless it's needed
    for (const auto& fileEntry : std::filesystem::directory_iterator(FolderName.c_str())) {     
        char* pcVar10 = param_6 + 8;
        if (fileEntry.is_directory()) {
            continue;
        }

        OTDU_LOG_DEBUG("\t- Found file '%s'...\n", fileEntry.path().c_str());
    }

    return true;
}

bool FileDirectAccess::openContext(const uint64_t hashcode, const char *pFilename, const eContextSource source)
{
    FolderPath.clear();
    ContainerFolderPath.clear();
    CurrentHashcode = 0ull;
    StoredOnHDD = 0;

    std::wstring saveFolder = gpFile->getSaveLocation( source );
    if (saveFolder.empty()) {
        return false;
    }

    CurrentHashcode = hashcode;
    bIsOpen = true;
    std::string narrowFilename(pFilename);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring filename = converter.from_bytes(narrowFilename);
    filename += saveFolder;
    const std::string& profileName = gUserProfileRegister.getProfileName(CurrentHashcode);
    if (profileName.empty()) {
        return false;
    }
    
    FolderName += converter.from_bytes(FolderPath);
    FolderName += L"/";
    FolderName += converter.from_bytes(profileName);
    FolderName += L"/";
    FolderName += saveFolder;

    CreateFolders( FolderName );
    return true;
}

bool FileDirectAccess::close()
{
    Stream.close();
    return true;    
}

bool FileDirectAccess::reset()
{
    if (!bIsOpen) {
        return false;
    }

    bIsOpen = false;
    FolderPath.clear();
    ContainerFolderPath.clear();
    StoredOnHDD = 0;
    return true;
}

bool FileDirectAccess::loadContentIntoMemory(const char *pFilename, int32_t sizeToRead, int32_t offset, void **ppOutContent, uint32_t *pContentSize)
{
    Stream = std::ifstream( pFilename, std::ifstream::in | std::ifstream::binary);
    if (!Stream.is_open() || !Stream.good()) {
        return false;
    }
    Stream.seekg(0, std::ios::end);
    size_t fileSize = Stream.tellg();
    Stream.seekg(0, std::ios::beg);

    if (offset > fileSize) {
        close();
        return true;
    }

    Stream.seekg(offset);
    char* pOutContent = (char*)*ppOutContent;
//    pOutContent += 4;

    Stream.read((char*)pOutContent, sizeToRead);

    size_t numBytesRead = Stream.gcount();

    if (pContentSize != nullptr) {
        *pContentSize = static_cast<uint32_t>( numBytesRead );
    }

    close();
    return (numBytesRead != 0);
}

int32_t FileMapEntry::deserialize(void *pMemory)
{
    StoredAsUint16 = 0x1020304; // For some reason this is never deserialized?

    uint32_t* pMemoryAsDword = (uint32_t*)pMemory;
    OTDU_ASSERT(pMemoryAsDword);

    uint32_t crc                = pMemoryAsDword[0];
    uint32_t size               = pMemoryAsDword[1];
    uint32_t offset             = pMemoryAsDword[2];
    uint32_t uncompressedSize   = pMemoryAsDword[3];
    uint32_t version            = pMemoryAsDword[4];

    uint8_t* pbCompressed = (uint8_t*)(pMemory) + 0x14;
    bool bCompressed = *(bool*)pbCompressed;

    CRC = crc;
    SizeWithPadding = size;
    Offset = offset;
    Size = uncompressedSize;
    Version = version;

    return 0x18;
}
