#include "shared.h"
#include "file_handle.h"

#include "virtual_file_system.h"

FileIterator::FileIterator()
    : pVFS( nullptr )
    , ItHandle( 0 )
    , ItInfos()
    , SearchPattern("")
    , SearchDirectory("")
{
}

bool FileIterator::findFirstMatch(const char *pPattern)
{
    if (pVFS != nullptr) {
        pVFS->findFile(this, FileIterator::eOpenMode::FIOM_Close);
    }
    pVFS = nullptr;
    ItHandle = 0;
    ItInfos = FileFindInfos();
    SearchPattern.clear();
    SearchDirectory.clear();
    
    SearchPattern = pPattern;
    pVFS = gVirtualFileSystemRegister.get(SearchDirectory, SearchPattern, false);
    if (pVFS != nullptr) {
        bool bVar1 = pVFS->findFile(this, FileIterator::eOpenMode::FIOM_FindFirst);
        if (bVar1) {
            return true;
        }

        if (pVFS != nullptr) {
            pVFS->findFile(this, FileIterator::eOpenMode::FIOM_Close);
            pVFS = nullptr;
        }
    }
    
    return false;
}

bool FileIterator::closeHandle()
{
    if (pVFS == nullptr) {
        return false;
    }

    bool bClosedSuccessfully = pVFS->findFile(this, FileIterator::eOpenMode::FIOM_Close); 
    pVFS = nullptr;
    return bClosedSuccessfully;
}

int32_t FileIterator::getSize() const
{
    return ItInfos.Size;
}
