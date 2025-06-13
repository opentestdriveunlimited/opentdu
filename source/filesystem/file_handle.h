#pragma once

class VirtualFileSystem;

#include <stack>

enum class eFileCommandType {
    FCT_Default	= 0,
    FCT_FileSeek = 1,
    FCT_FileRead = 2,
    FCT_FileWrite = 3,
    FCT_FileClose = 4,
    FCT_FileOpen = 5,
    FCT_Count
};

enum class eFileQueueState {
    FQS_Idle	= 0,
    FQS_Process = 1,
    FQS_Last	= 2
};

struct FileHandle;

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

struct FileQueue {
    eFileQueueState             State;
    std::stack< FileCommand >   Commands;
};

enum class eFileOpenMode : uint32_t {
    FOM_Default = 0,
    FOM_Read = 1,
    FOM_Write = 2,
    FOM_NoWait = 8,
    FOM_VideoStream = 16,
    FOM_NoRelease = 32
};

struct FileHandle {
    eFileOpenMode   OpenMode;
    uint32_t        FilePosition;
    int32_t         CurrentComandIndex;
    int32_t         NumPendingCommands;

    void*           pLocalHandle;
    void*           pUserData;
    VirtualFileSystem* pVFS;
    FileHandle*     pNextFile;

    std::string     NormalizedFilename;

    FileQueue       CommandStack;

    uint8_t         bActive : 1;
    uint8_t         bCommandStatus : 1;
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

struct FileIterator {
    enum class eOpenMode {
        FIOM_FindFirst = 0,
        FIOM_FindNext = 1,
        FIOM_Close = 2
    };

    VirtualFileSystem*  pVFS;
    int32_t             ItHandle;
    FileFindInfos       ItInfos;
    std::string         SearchPattern;
    std::string         SearchDirectory;

    FileIterator();

    bool findFirstMatch(const char* pPattern);
    bool closeHandle();
    int32_t getSize() const;
};
