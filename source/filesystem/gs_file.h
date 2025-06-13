#pragma once

#include "game/gs_base.h"
#include <stack>
#include <queue>

#include "file_handle.h"
#include "virtual_file_system.h"

class TestDriveMutex;
class TestDriveEvent;
class TestDriveThread;

struct FileHandle;

struct FileMapEntry {
    uint32_t CRC;
    uint32_t SizeWithPadding;
    uint32_t Offset;
    uint32_t Size;
    uint32_t Version;
    uint32_t StoredAsUint16;
    uint8_t  bCompressed : 1;

    int32_t deserialize(void* pMemory);
};

enum class eContextSource {
    CS_None = 0,
    CS_Publisher = 1,
    CS_Marketplace = 2,
    CS_Savegame = 4
};

struct EnumerateContextItem {
    uint8_t bIsValid : 1;

    uint32_t        StoredOnHDD;
    eContextSource  Source;

    std::wstring    DisplayName;
    std::string     Filename;
};

struct FileMap {
    uint32_t pCRCLUT[256];
    std::string FilenameBigFile;

    void* pMapStart;
    uint32_t MapSize;

    EnumerateContextItem Context;
    FileMapEntry         Entry;

    std::string CurrentFile;
    std::string HeaderToSkip;
    std::string TempFilename;

    uint8_t bUseBigFile : 1;
    uint8_t bNeedName : 1;
    uint8_t bUnknown : 1;

    void initializeCRCLUT();
    bool findEntryInSet(const char* pFilename, FileMapEntry** ppOutEntry);
    FileMapEntry* findEntry(const char* pFilename);
    FileMapEntry* findEntry(const uint32_t crc);
    void normalizeFilename(const char* pFilename);
    uint32_t hashFilename(const char* pFilename);
    uint32_t crc32(const char* pString, size_t length);
    std::string& resolveName(const char* pFilename, const bool bCompressed);
};

struct AsyncFileOpen {
    FileHandle  Handle;
    std::string Filename;
    void*       pUserData;
    void*       pBuffer;
    void*       pCompletionCallback;

    uint32_t    Size;
    uint32_t    Offset;
    uint8_t     bInUse : 1;
    uint8_t     bCanceled : 1;
    uint8_t     bCompressed : 1;
    uint8_t     bHighPriority : 1;

    FileMap*    pFilemap;

    AsyncFileOpen();
};

struct FileDirectAccess {
    std::string NormalizedFilename;
    std::string AbsoluteFilename;
    std::string FolderPath;
    std::string ContainerFolderPath;
    std::wstring FolderName;

    FileFindInfos FindInfos;

    std::ifstream Stream;
    uint64_t CurrentHashcode;
    void* pContext;
    void* pHandle;
    uint32_t StoredOnHDD;

    uint8_t bNeedName : 1;
    uint8_t bIsOpen : 1;

    FileDirectAccess();

    bool enumerateContext(   
        const uint64_t hashcode, 
        const eContextSource source,
        const uint32_t param_3,
        char *param_5,
        char *param_6,
        uint32_t param_7,
        EnumerateContextItem *param_8,
        uint32_t *param_9
    );
    bool openContext( const uint64_t hashcode, const char* pFilename, const eContextSource source );
    bool loadContentIntoMemory( const char* pFilename, int32_t sizeToRead, int32_t offset, void** ppOutContent, uint32_t* pContentSize );
    bool close();
    bool reset();
};

struct FileToSave {
    std::string Filename;
    void* pContent;
    uint32_t ContentSize;
    uint8_t bOwnContentBuffer : 1;

    FileToSave()
        : pContent( nullptr )
        , ContentSize( 0 )
        , bOwnContentBuffer( false )
    {
         
    }
};

struct SaveQueue {
    std::string             ContextName;
    std::wstring            DisplayName;
    std::queue<FileToSave>  FilesToSave;
    TestDriveMutex*         pMutex;
    uint8_t                 bPauseFlush : 1;

    SaveQueue();
    ~SaveQueue();
};

struct WorkerThreadContext {
    TestDriveEvent* pEvent;
    AsyncFileOpen* pAsyncData;
    uint8_t bRunning : 1;
    uint8_t bJobDone : 1;

    WorkerThreadContext()
        : pEvent( nullptr )
        , pAsyncData( nullptr )
        , bRunning( false )
        , bJobDone( false )
    {

    }
};

class GSFile : public GameSystem {
public:
    const char* getName() const override { return "Service : File"; }
    const std::wstring& getSaveFolder() const { return savegameFolder; }

public:
    GSFile();
    ~GSFile();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    bool loadFile(const char *pFilename, void **pOutContent, uint32_t *pOutContentSize);
    bool findMapEntriesForFile(const char *pEntryName, FileMapEntry** ppOutMapEntry, FileMap **ppOutMap);
    bool loadFileRaw(const char *pFilename, FileMap* pFilemap, FileMapEntry* pEntry, const uint32_t size, void** ppOutContent);

    void initCarPacks( bool param_1 );
    std::wstring getSaveLocation( eContextSource source );

private:
    std::vector<AsyncFileOpen>  asyncData;
    std::vector<FileMap>        filemaps;
    TestDriveMutex*             pFilemapMutex;
    TestDriveThread*            pWorkerThread;
    WorkerThreadContext         workerThreadContext;
    FileDirectAccess            directAccess;
    FileDirectAccess            currentContextDirectAccess;
    SaveQueue                   saveQueue;

    VirtualFileSystem           hardDriveVFS;

    std::string                 tmpFilename;
    std::wstring                savegameFolder;
    
    int32_t                     initStep;

    uint8_t                     bFilemapActive : 1;
    uint8_t                     bTaskInProgress : 1;
    uint8_t                     bIsCurrentContextReady : 1;
    uint8_t                     bDeletingInProgress : 1;

private:
    bool retrieveSaveFolder();
    bool findFileOutsideFilemap(const char* pFilename, uint32_t* pOutFilesize);
};

extern GSFile* gpFile;

