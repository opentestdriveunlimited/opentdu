#include "shared.h"
#include "uva.h"

RenderUVA::RenderUVA()
    : pHeaderSection( nullptr )
    , pDataSection( nullptr )
{
}

RenderUVA::~RenderUVA()
{
}

bool RenderUVA::initialize(void *pBuffer, const size_t length)
{
    pHeaderSection = nullptr;
    pDataSection = nullptr;

    if (pBuffer == nullptr) { 
        return false;
    }
    
    uint8_t* pBufferIt = static_cast<uint8_t*>( pBuffer );
    for (uint32_t i = 0; i < length; i += 4) {
        uint32_t magic = *(uint32_t*)pBufferIt;
        if (magic == 0x21464f45) {
            break;
        }

        if (magic == 0x214d4e41) {
            pHeaderSection = reinterpret_cast<RenderUVA::Header*>( pBufferIt + 8 );
        } else if (magic == 0x41544144) {
            pDataSection = reinterpret_cast<RenderUVA::Data*>( pBufferIt + 8 );
        } else {
            OTDU_LOG_WARN( "Unknown section with magic 0x%x found!\n", magic );
        }
    }

    if (pHeaderSection == nullptr || pDataSection == nullptr) {
        return false;
    }

    if (pHeaderSection->Version != 1.1f) {
        return false;
    }
   
    return true;
}
