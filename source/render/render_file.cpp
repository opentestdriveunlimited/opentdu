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

    // Resolve offsets (convert bank offsets to actual memory offsets)
    while ( pSectionIt < pSectionsEnd ) {
        Section* pSection = ( Section* )pSectionIt;
        if ( pSection->Type == kRealMagic ) {
            uint32_t sectionSize = pSection->Size - 0x10;
            
            pSectionIt += 0x10;
            for ( uint32_t i = 0; i < sectionSize; i += sizeof( uint32_t ) ) {
                uint32_t offsetPosition = *(uint32_t*)pSectionIt;

                int8_t* pRelativeOffset = (int8_t*)pHeader + offsetPosition;
                uint32_t relativeOffset = *( uint32_t* )pRelativeOffset;

                uint64_t* pRelativeOffsetAsPointer = ( uint64_t* )pRelativeOffset;
                *pRelativeOffsetAsPointer = (uint64_t)(( int8_t* )pHeader + relativeOffset);

                pSectionIt += sizeof( uint32_t );
            }
        } else {
            pSectionIt += 0x10;
        }
    }

    pSectionIt = ( uint8_t* )pSections;

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
