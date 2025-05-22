#include "shared.h"
#include "gs_file.h"

#include "core/mutex.h"
#include "tdu_instance.h"

#if defined( OTDU_WIN32 )
#include <Shlobj.h>
#endif

GSFile* gpFile = nullptr;

wchar_t GSFile::SaveFolder[OTDU_MAX_PATH];

static TestDriveMutex gFilemapMutex = {};

static bool CreateFolderWin32( char* pFolderPath, const size_t pathLength )
{
    /*char cVar1;
    int iVar2;
    char* pcVar3;
    int iVar4;
    BOOL BVar5;
    DWORD DVar6;
    int iVar7;
    char local_200[512];

    iVar7 = 0;
    pcVar3 = pFolderPath;
    if ( pathLength < 0x1fe ) {
        iVar2 = -( int )pFolderPath;
        do {
            cVar1 = *pFolderPath;
            pFolderPath[( int )( local_200 + iVar2 )] = cVar1;
            pFolderPath = pFolderPath + 1;
        } while ( cVar1 != '\0' );
        if ( ( local_200[iVar4 + -1] != '\\' ) && ( local_200[iVar4 + -1] != '/' ) ) {
            local_200[iVar4] = '\\';
            local_200[iVar4 + 1] = '\0';
        }
        do {
            if ( local_200[0] == '\0' ) {
                return true;
            }
            if ( ( ( ( local_200[iVar7] == '\\' ) || ( local_200[iVar7] == '/' ) ) && ( 0 < iVar7 ) ) &&
                 ( local_200[iVar7 + -1] != ':' ) ) {
                local_200[iVar7] = '\0';
                BVar5 = CreateDirectory( local_200, ( LPSECURITY_ATTRIBUTES )0x0 );
                if ( ( BVar5 == 0 ) && ( DVar6 = GetLastError(), DVar6 != 0xb7 ) ) {
                    return false;
                }
                local_200[iVar7] = '\\';
            }
            local_200[0] = local_200[iVar7 + 1];
            iVar7 = iVar7 + 1;
        } while ( true );
    }*/
    OTDU_UNIMPLEMENTED; // TODO:
    return false;
}

static uint32_t ThreadRoutine( WorkerThreadContext* param_1 )
{
    AsyncFileOpen* pAVar1;
    //FileMap* pCVar2;
    //bool bVar3;
    //char cVar4;
    //uint32_t uVar5;
    FileDirectAccess local_c48 = {};

    do {
        param_1->pEvent->waitForEvent();

        pAVar1 = param_1->pAsyncData;
        gFilemapMutex.lock();
        OTDU_UNIMPLEMENTED; // TODO:
        gFilemapMutex.unlock();
        param_1->bJobDone = true;
    } while ( true );

    return 0;
}

GSFile::GSFile()
    : pFilemapMutex( new TestDriveMutex() )
    , pWorkerThread( new TestDriveThread() )
    , bFilemapActive( false )
    , bTaskInProgress( false )
    , bIsCurrentContextReady( false )
    , bDeletingInProgress( false )
{
    gpFile = this;

    memset( SaveFolder, 0, sizeof( wchar_t ) * OTDU_MAX_PATH );

    asyncData.resize( 32u );

    filemaps.resize( 8u );
    for ( FileMap& filemap : filemaps ) {
        filemap.initializeCRCLUT();
    }

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
    return bInitialized;
}

void GSFile::tick()
{

}

void GSFile::terminate()
{
    pFilemapMutex->terminate();
}

bool GSFile::retrieveSaveFolder()
{
    savegameFolder = L"\\";

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

FileDevice::FileDevice()
    : pDeviceName( nullptr )
    , pPreviousDevice( nullptr )
    , pNextDevice( nullptr )
    , pFirstDeviceHandle( nullptr )
    , flagCaps( eFileOpenMode::FOM_Default )
    , name( "" )
{

}

FileDevice::~FileDevice()
{

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

static std::wstring GetSaveLocation( eContextSource type )
{
    std::wstring saveLocation = GSFile::SaveFolder;

    switch ( type ) {
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

FileDirectAccess::FileDirectAccess()
    : CurrentHashcode( 0 )
    , pContext( nullptr )
    , pHandle( nullptr )
    , StoredOnHDD( 0 )
    , bNeedName( false )
    , bIsOpen( false )
{

}

bool FileDirectAccess::openContext( const uint64_t hashcode, const char* pFilename, const eContextSource source )
{
    OTDU_UNIMPLEMENTED; // TODO:

    //char cVar1;
    //uint64_t uVar3;
    //bool bVar4;
    ////TDUProfileList* pTVar5;
    //char* pProfileName;
    //int iVar6;
    //char* pcVar7;
    //char pSaveLocation[512];

    //FolderPath.clear();
    //ContainerFolderPath.clear();
    //CurrentHashcode = 0ull;
    //StoredOnHDD = false;

    //std::wstring saveLocation = GetSaveLocation( source );
    //if ( !saveLocation.empty() ) {
    //    bIsOpen = true;
    //    CurrentHashcode = hashcode;

    //    OTDU_UNIMPLEMENTED;

    // /*   iVar6 = ( int )pcVar7 - ( int )pFilename;
    //    do {
    //        cVar1 = *pFilename;
    //        pFilename[iVar6] = cVar1;
    //        pFilename = pFilename + 1;
    //    } while ( cVar1 != '\0' );
    //    pProfileName = pSaveLocation;
    //    iVar6 = 0x408 - ( int )pProfileName;
    //    do {
    //        cVar1 = *pProfileName;
    //        pProfileName[( int )this + iVar6] = cVar1;
    //        pProfileName = pProfileName + 1;
    //    } while ( cVar1 != '\0' );*/

    //   /* pTVar5 = TestDrive::GetProfileList();
    //    pProfileName = ( char* )( *( code* )pTVar5->lpvtbl->GetProfileNameByHash )( CurrentHashcode );
    //    if ( ( pProfileName != ( char* )0x0 ) && ( *pProfileName != '\0' ) ) {
    //        FolderName += FolderPath;
    //        FolderName += "/";
    //        FolderName += pProfileName;
    //        FolderName += "/";
    //        FolderName += ContainerFolderPath;

    //        CreateFolderWin32( FolderName.c_str(), FolderName.length() );
    //        return true;
    //    }*/
    //}
    
    return false;
}
