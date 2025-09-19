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

void RenderFile::destroy()
{
    reset();
    unparse();
}

void RenderFile::reset()
{
    // FUN_00505eb0
    pHeader = nullptr;
    pSections = nullptr;
}

void RenderFile::unparse()
{
    // FUN_00506010
    uint8_t* peVar2 = (uint8_t*)pSections;
    uint8_t* peVar3 = (uint8_t*)pHeader + pHeader->Size;  
    while (peVar2 != nullptr) {
        RenderFile::Section* pSection = (RenderFile::Section*)peVar2;
        unparseSection(pSection);
        unparseSubSections(pSection);
        peVar2 += pSection->Size;

        if (peVar3 != nullptr & peVar3 <= peVar2) {     
            break;
        }
    }
    pHeader->Flags &= 0xfffffffe;
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

void RenderFile::unparseSubSections(Section *param_2)
{
    // FUN_00505f90
    uint8_t* peVar1 = (uint8_t*)param_2 + param_2->Size;
    uint8_t* peVar2 = (uint8_t*)param_2 + param_2->DataSize;

    if (peVar1 <= peVar2) {
        peVar2 = nullptr;
    }

    while( true ) {
        do {
            if (peVar2 == nullptr) {
                return;
            }
            Section* pSubSection = (Section*)peVar2;
            unparseSection(pSubSection);
            unparseSubSections(pSubSection);
            peVar2 += pSubSection->Size;
        } while (peVar1 == nullptr);

        if (peVar1 <= peVar2) {
            return;
        }
    }
}
