#pragma once

#include "game/gs_base.h"
#include <stack>
#include <queue>

class TestDriveMutex;
class TestDriveEvent;
class TestDriveThread;

struct FileHandle;

enum class eFileOpenMode {
    FOM_Default = 0,
    FOM_Read = 1,
    FOM_Write = 2,
    FOM_NoWait = 8,
    FOM_VideoStream = 16,
    FOM_NoRelease = 32
};

class FileDevice {
public:
    FileDevice();
    ~FileDevice();

private:
    char*           pDeviceName;
    FileDevice*     pPreviousDevice;
    FileDevice*     pNextDevice;
    FileHandle*     pFirstDeviceHandle;
    eFileOpenMode   flagCaps;
    std::string     name;
};

enum class eFileCommandType {
    FCT_Default	= 0,
    FCT_FileSeek = 1,
    FCT_FileRead = 2,
    FCT_FileWrite = 3,
    FCT_FileClose = 4,
    FCT_FileOpen = 5,
    FCT_Count
};

struct FileCommand {
    using PostExecCallback_t = bool  ( * )( FileHandle*, FileCommand*, bool );

    eFileCommandType    Type;
    FileHandle*         pFileHandle;
    PostExecCallback_t  pCallback;

    const char*         pFilepathToOpen;
    void*               pBuffer;
    uint32_t            SizeToReadOrWrite;
    uint32_t            SeekOffset;
};

enum class eFileQueueState {
    FQS_Idle	= 0,
    FQS_Process = 1,
    FQS_Last	= 2
};

struct FileQueue {
    eFileQueueState             State;
    std::stack< FileCommand >   Commands;
};

struct FileHandle {
    eFileOpenMode   OpenMode;
    uint32_t        FilePosition;
    int32_t         CurrentComandIndex;
    int32_t         NumPendingCommands;

    void*           pLocalHandle;
    void*           pUserData;
    FileDevice*     pDevice;
    FileHandle*     pNextFile;

    std::string     NormalizedFilename;

    FileQueue       CommandStack;

    uint8_t         bActive : 1;
    uint8_t         bCommandStatus : 1;
};

struct FileMapEntry {
    uint32_t CRC;
    uint32_t SizeWithPadding;
    uint32_t Offset;
    uint32_t Size;
    uint32_t Version;
    uint32_t Unknown;
    uint8_t  bCompressed : 1;
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

    void initializeCRCLUT();
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

struct FileFindInfos {
    std::string Filename;
    uint32_t    Size;
    uint8_t     bIsFolder : 1;
    uint8_t     CreationDate[8];
    uint8_t     ModificationDate[8];

    FileFindInfos()
        : Size( 0 )
        , bIsFolder( false )
        , CreationDate{ 0, 0, 0, 0, 0, 0, 0, 0 }
        , ModificationDate{ 0, 0, 0, 0, 0, 0, 0, 0 }
    {

    }
};

struct FileDirectAccess {
    std::string NormalizedFilename;
    std::string AbsoluteFilename;
    std::string FolderPath;
    std::string ContainerFolderPath;
    std::string FolderName;

    FileFindInfos FindInfos;

    uint64_t CurrentHashcode;
    void* pContext;
    void* pHandle;
    uint32_t StoredOnHDD;

    uint8_t bNeedName : 1;
    uint8_t bIsOpen : 1;

    FileDirectAccess();

    bool openContext( const uint64_t hashcode, const char* pFilename, const eContextSource source );
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
    static wchar_t SaveFolder[OTDU_MAX_PATH];

public:
    const char* getName() const override { return "Service : File"; }

public:
    GSFile();
    ~GSFile();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

private:
    std::vector<AsyncFileOpen>  asyncData;
    std::vector<FileMap>        filemaps;
    TestDriveMutex*             pFilemapMutex;
    TestDriveThread*            pWorkerThread;
    WorkerThreadContext         workerThreadContext;
    FileDirectAccess            directAccess;
    FileDirectAccess            currentContextDirectAccess;
    SaveQueue                   saveQueue;

    std::wstring                savegameFolder;
    
    uint8_t                     bFilemapActive : 1;
    uint8_t                     bTaskInProgress : 1;
    uint8_t                     bIsCurrentContextReady : 1;
    uint8_t                     bDeletingInProgress : 1;

private:
    bool retrieveSaveFolder();
};

extern GSFile* gpFile;

