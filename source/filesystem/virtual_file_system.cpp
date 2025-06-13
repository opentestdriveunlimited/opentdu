#include "shared.h"
#include "virtual_file_system.h"
#include "file_path_register.h"

VirtualFileSystemRegister gVirtualFileSystemRegister = {};

void PathSplit(char *param_1,char *param_2,char *param_3,char *param_4, char *param_5)
{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  
  if ((*param_5 != '\0') && (*param_5 == '<')) {
    do {
      cVar1 = *param_5;
      if (param_1 != (char *)0x0) {
        *param_1 = cVar1;
        param_1 = param_1 + 1;
      }
    } while ((cVar1 != '\0') && (param_5 = param_5 + 1, cVar1 != '>'));
  }
  if (param_1 != (char *)0x0) {
    *param_1 = '\0';
  }
  if ((*param_5 != '\0') && ((pcVar2 = strchr(param_5,'/'), pcVar2 != (char *)0x0) || (pcVar2 = strchr(param_5,':'), pcVar2 != (char *)0x0))) {
    do {
      cVar1 = *param_5;
      if (param_2 != (char *)0x0) {
        *param_2 = cVar1;
        param_2 = param_2 + 1;
      }
    } while ((cVar1 != '\0') && (param_5 = param_5 + 1, cVar1 != ':'));
  }
  if (param_2 != (char *)0x0) {
    *param_2 = '\0';
  }
  if ((*param_5 != '\0') && ((pcVar2 = strchr(param_5,'/'), pcVar2 != (char *)0x0) || (pcVar2 = strchr(param_5,'\\'), pcVar2 != (char *)0x0))) {
    pcVar2 = param_5;
    do {
      cVar1 = *pcVar2;
      pcVar2 = pcVar2 + 1;
    } while (cVar1 != '\0');
    pcVar3 = param_5 + ((size_t)pcVar2 - (size_t)(param_5 + 1));
    cVar1 = param_5[(size_t)pcVar2 - (size_t)(param_5 + 1)];
    while (cVar1 != '\\') {
      pcVar2 = pcVar3 + -1;
      pcVar3 = pcVar3 + -1;
      cVar1 = *pcVar2;
    }
    for (; (param_5 != pcVar3 + 1 && (*param_5 != '\0')); param_5 = param_5 + 1) {
      if (param_3 != (char *)0x0) {
        *param_3 = *param_5;
        param_3 = param_3 + 1;
      }
    }
  }
  if (param_3 != (char *)0x0) {
    *param_3 = '\0';
  }
  cVar1 = *param_5;
  while (cVar1 != '\0') {
    if (param_4 != (char *)0x0) {
      *param_4 = cVar1;
      param_4 = param_4 + 1;
    }
    if (cVar1 != '\0') {
      param_5 = param_5 + 1;
    }
    cVar1 = *param_5;
  }
  if (param_4 != (char *)0x0) {
    *param_4 = '\0';
  }
}

VirtualFileSystem::VirtualFileSystem()
    : pFirstDeviceHandle( nullptr )
    , flagCaps( 0x2F )
    , name( "<PC>" )
    , defaultUnit("")
{
    
}

VirtualFileSystem::~VirtualFileSystem()
{

}

void VirtualFileSystem::initialize()
{
    // TODO: Support multiple type of VFS? (TDU1 only support HDD)
    char pWorkingDirectory[512];
    
#ifdef OTDU_WIN32
    GetCurrentDirectoryA(512,pWorkingDirectory);
#elif defined( OTDU_UNIX )
    getcwd(pWorkingDirectory, 512 * sizeof(char));
#endif
    
    setDefaultPath( pWorkingDirectory );

    std::string path = name;
    path += pWorkingDirectory;
    path += "\\";

    gFilePathRegister.registerPath(path);
}

bool VirtualFileSystem::findFile(FileIterator* pIterator, const FileIterator::eOpenMode mode)
{
    // TODO: Remove me (original game was using the crappy WinAPI for directory it)
    if (mode == FileIterator::eOpenMode::FIOM_Close) {
        return true;
    }

    auto it = pIterator->SearchPattern.find( '>' );
    std::string searchPath = pIterator->SearchPattern;
    if (it != std::string::npos) {
        searchPath = searchPath.substr(it + 1);
    }

    // YOLO: Check for direct access (we're on PC and don't really need to check disk or other kind of external devices)
    std::string completeResourcePath = pIterator->SearchDirectory;
    completeResourcePath += "/";
    completeResourcePath += pIterator->SearchPattern;
    if (std::filesystem::exists(completeResourcePath)) {
        OTDU_LOG_DEBUG("File '%s' exists!\n", completeResourcePath.c_str());

        std::filesystem::file_status status = std::filesystem::status(completeResourcePath);
        pIterator->ItInfos.bIsFolder = std::filesystem::is_directory(status);
        pIterator->ItInfos.Filename = completeResourcePath;
        pIterator->ItInfos.Size = static_cast<uint32_t>( std::filesystem::file_size(completeResourcePath) );

        // TODO: AFAIK those are useless so we can skip all this crap for now...
        //pIterator->ItInfos.CreationDate = std::filesystem::last_write_time(completeResourcePath);
        //pIterator->ItInfos.ModificationDate = std::filesystem::last_write_time(completeResourcePath);

        return true;
    }

    // TODO: Re-implement old iterative search TDU1 uses (enumerate directory content and walk the list until we have a match)
    // AFAIK it's pretty much useless to reimplement (as we have direct access to HDD on PC)
    OTDU_UNIMPLEMENTED;
    return false;

    // for (const auto& fileEntry : std::filesystem::directory_iterator(searchPath.c_str())) {
    //     return true;
    // }
    
    // return false;
}

void VirtualFileSystem::setDefaultPath( const char* param_1 )
{
    char pDefaultUnit[512];
    char local_200[512];
    strcpy(local_200, param_1);

    PathSplit(nullptr,pDefaultUnit,nullptr,nullptr,local_200);

    defaultUnit = param_1;

    OTDU_LOG_DEBUG("Default unit set to '%s'\n", defaultUnit.c_str());
}

bool VirtualFileSystem::getDefaultPath(std::string& param_2)
{
    param_2 = defaultUnit;
    return !defaultUnit.empty();

    // char cVar1;
    // char *pcVar2;
    // char *pcVar3;

    // pcVar3 = this->pDefaultUnit;
    // pcVar2 = pcVar3;
    // do {
    //     cVar1 = *pcVar2;
    //     pcVar2[(size_t)param_2 - (size_t)pcVar3] = cVar1;
    //     pcVar2 = pcVar2 + 1;
    // } while (cVar1 != '\0');
    // do {
    //     cVar1 = *pcVar3;
    //     pcVar3 = pcVar3 + 1;
    // } while (cVar1 != '\0');
    // return (size_t)pcVar3 - (size_t)(this->pDefaultUnit + 1);
}

VirtualFileSystemRegister::VirtualFileSystemRegister()
{
    
}

VirtualFileSystemRegister::~VirtualFileSystemRegister()
{

}

VirtualFileSystem *VirtualFileSystemRegister::find(const char *param_1)
{
    for (VirtualFileSystem* pVFS : vfsList) {
        if (pVFS->getName() == param_1) {
            return pVFS;
        }
    }

    OTDU_LOG_WARN("Failed to find VFS with root '%s'", param_1);
    return nullptr;
}

VirtualFileSystem *VirtualFileSystemRegister::get(std::string& param_2, std::string& param_3, const bool param_4)
{
    if (param_3.empty()) {
        param_3 = "";
    }

    size_t iVar7 = 0;
    if (param_3[0] == '<') {
        bool bVar1 = false;
        for (uint32_t i = 1; i < param_3.length(); i++) {
            char cVar2 = param_3[i];
            if (cVar2 == '>') {
                bVar1 = true;
                break;
            }

            char cVar3 = cVar2;
            if (('`' < cVar2) && (cVar2 < '{')) {
                cVar3 = cVar2 + -0x20;
            }
            param_2 += cVar3;
            iVar7++;
        }

        if (!bVar1) {
            param_2.clear();
            return nullptr;
        }
    } else {
        iVar7 = gFilePathRegister.getFiler(param_2.c_str());
    }

    VirtualFileSystem* peVar4 = findVFS(param_2);
    if (peVar4 != nullptr) {
        if (param_3[0] == '|') {
            uint32_t i = 0;
            for (i = 0; i < param_3.length(); i++) {
                char cVar2 = param_3[i];

                if (('`' < cVar2) && (cVar2 < '{')) {
                    cVar2 = cVar2 + -0x20;
                }

                param_2 += cVar2;
                iVar7++;

                if (param_3[i + 1] == '|' || param_3[i + 1] == '\0') {
                    break;
                }
            }
            
            if (i < param_3.length()) {
                return nullptr;
            }
        } else {
            int32_t iVar5 = 0;
            auto pcVar8 = param_3.find(':');
            if (pcVar8 == std::string::npos) {
                iVar5 = peVar4->getDefaultPath(param_2);
                iVar7 += iVar5;
            } else {
                for (int32_t i = 1; i < param_3.length(); i++) {
                    char cVar3 = param_3[i];
                    if (('`' < cVar3) && (cVar3 < '{')) {
                        cVar3 = cVar3 + -0x20;
                    }

                    param_2[iVar7] = cVar3;
                    iVar7++;
                    if (cVar3 == ':') break;
                }
            }
            char local_200 [512];
            local_200[0] = '\0';
            char* pcVar9 = nullptr;
                   
            size_t pcVar7 = 0;
            if (!param_4) {
                auto pcVar7 = param_3.find( '/' );
                if (pcVar7 == std::string::npos) {
                    pcVar7 = param_3.find( '\\' );
                }
            }

        }

        return peVar4;
    }

    param_2.clear();
    return nullptr;
}

VirtualFileSystem *VirtualFileSystemRegister::findVFS(std::string &param_2)
{
    for (VirtualFileSystem* vfs : vfsList) {
        if (vfs->getName() == param_2 || vfs->getName() == "<PC>") { // TODO: FIXME:
            return vfs;
        }
    }
    return nullptr;
}

void VirtualFileSystemRegister::registerVFS(VirtualFileSystem * pVfs)
{
    vfsList.push_back(pVfs);
}
