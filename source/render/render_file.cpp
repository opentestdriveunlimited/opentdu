#include "shared.h"
#include "render_file.h"

static constexpr const uint32_t kRealMagic = 0x4c414552; // LAER

RenderFile::RenderFile()
    : pHeader( nullptr )
    , pSections( nullptr )
{

}

RenderFile::~RenderFile()
{

}

bool RenderFile::parseHeader( void* pFileStream )
{
    pHeader = (Header*)pFileStream;
    pSections = (Section*)( pHeader + 1 );
    return true;
}

bool RenderFile::parseFile()
{
    uint8_t* pSectionIt = (uint8_t*)pSections;
    uint8_t* pSectionsEnd = (uint8_t*)pHeader + pHeader->Size;   

    // TODO: Is it really needed? I have no idea what's the point of doing that...
    //   while (pSection != (Section *)0x0) {
    //     if (pSection->Type == 0x4c414552) {
    //       pHeader = param_1->pHeader;
    //       sectionSize = pSection->Size - 0x10 >> 2;
    //       pNextSection = pSection + 1;
        
    //     }
    //     pSection = (Section *)((int)&pSection->Type + pSection->Size);
    //     if ((pFileEnd != (Section *)0x0) &&
    //        ((uVar3 < (uint)((int)pSection >> 0x1f) ||
    //         ((uVar3 <= (uint)((int)pSection >> 0x1f) && (pFileEnd <= pSection)))))) break;
    //   }

    bool cVar3 = true;
    while (pSectionIt != nullptr && pSectionIt < pSectionsEnd && cVar3) {
        Section* pSection = (Section*)pSectionIt;

        cVar3 = parseSection( pSection );
        readSection( pSection );

        pSectionIt += pSection->Size;
    }

    if (!cVar3) {
        return false;
    }

    pHeader->Flags |= 1;
    return cVar3;
}

bool RenderFile::readSection(Section *pSection)
{
    uint8_t* pSectionEnd  = (uint8_t*)pSection + pSection->Size;
    uint8_t* pSectionIt   = (uint8_t*)pSection + pSection->DataSize;

    if (pSectionEnd <= pSectionIt) {
        pSectionIt = nullptr;
    }

    bool cVar2 = true;
    if (pSectionIt != nullptr) {
        while (cVar2 != false) {
            Section* pFoundSection = (Section*)pSectionIt;
            if ( parseSection( pFoundSection ) ) {
                readSection( pFoundSection );
            }

            pSectionIt += pFoundSection->Size;

            if (pSectionIt == nullptr) {
                return cVar2;
            }

            if (pSectionEnd <= pSectionIt) {
                return cVar2;
            }
        }
    }
    return cVar2;
}
