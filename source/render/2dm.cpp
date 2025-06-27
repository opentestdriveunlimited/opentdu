#include "shared.h"
#include "2dm.h"

#include "2db.h"
#include "render/material.h"

static constexpr uint32_t k2DMMagic             = 0x4d44322e; // MD2. (.2DM)
static constexpr uint32_t kMaterialArrayMagic   = 0x4154414d; // MATA (MATerial Array)
static constexpr uint32_t kMaterialMagic        = 0x2e54414d; // MAT. (MATerial)
static constexpr uint32_t kParameterArrayMagic  = 0x41524150; // PARA (PARameter Array)
static constexpr uint32_t kLayerArrayMagic      = 0x4159414c; // LAYA (LAYer Array)
static constexpr uint32_t kHashcodesMagic       = 0x48534148; // HASH (HASHcodes)
static constexpr uint32_t kLayerMagic           = 0x2e59414c; // LAY. (LAYer)

Render2DM::Render2DM()
    : RenderFile()
    , pMatArray( nullptr )
    , pHashcodes( nullptr )
    , pParameters( nullptr )
    , pLayers( nullptr )
{

}

Render2DM::~Render2DM()
{

}

bool Render2DM::initialize( void* pStream )
{
    if ( pStream == nullptr ) {
        return false;
    }

    parseHeader( pStream );
    if ( pHeader->Hashcode != k2DMMagic ) {
        return false;
    }

    pMatArray = nullptr;
    pHashcodes = nullptr;
    pParameters = nullptr;
    pLayers = nullptr;

    return parseFile();
}

bool Render2DM::parseSection(RenderFile::Section *pSection)
{
    switch (pSection->Type) {
    case kMaterialArrayMagic:
        pMatArray = pSection;
        return true;
        
    case kMaterialMagic:
        gMaterialRegister.pushMaterial( pSection );
        return true;

    case kParameterArrayMagic:
        pParameters = pSection;
        return true;

    case kLayerArrayMagic:
        pLayers = pSection;
        return true;
        
    case kHashcodesMagic:
        pHashcodes = pSection;
        return true;

    default:
        return true;
    };
}

void Render2DM::bindBitmapReference( const Render2DB* param_2 )
{
    OTDU_ASSERT( pLayers );

    const uint64_t bitmapHashcode = param_2->getBitmapHashcode();

    uint8_t* piVar1 = ( uint8_t* )pLayers + pLayers->Size;
    uint8_t* piVar2 = ( uint8_t* )pLayers + pLayers->DataSize;

    RenderFile::Section* pLayerSection = ( RenderFile::Section* )piVar1;
    while ( pLayerSection->Type == kLayerMagic ) {
        MaterialLayer* pLayer = ( MaterialLayer* )( pLayerSection + 1 );
        for ( uint32_t i = 0; i < pLayer->getNumUsedTextureSlots(); i++ ) {
            if ( pLayer->pLayerTextures[i].Hashcode == bitmapHashcode ) {
                pLayer->pLayerTextures[i].pTextureSection = param_2->getBitmapSection();
            }
        }

        piVar1 += pLayerSection->Size;
    }
}

void Render2DM::bindUVAnimationReference( const RenderUVA* param_2 )
{
    OTDU_UNIMPLEMENTED;
}

Material* Render2DM::create( void* param_1, uint64_t param_2, int32_t param_3, uint32_t param_4, uint32_t param_5, uint32_t param_6 )
{
    OTDU_UNIMPLEMENTED;
    return nullptr;
}

RenderFile::Section* Render2DM::getMaterial( uint64_t param_2 )
{
    uint8_t* piVar1 = ( uint8_t* )pMatArray + pMatArray->DataSize;
    RenderFile::Section* pHashcodeSection = ( RenderFile::Section* )piVar1;
    OTDU_ASSERT( pHashcodeSection->Type == kHashcodesMagic );

    uint32_t numHashcodes = ( pHashcodeSection->Size - 0x10 ) / sizeof( HashTableEntry );
    HashTableEntry* pIterator = ( HashTableEntry* )( pHashcodeSection + 1 );

    for ( uint32_t i = 0; i < numHashcodes; i++ ) {
        if ( pIterator->Hashcode == param_2 ) {
            return pIterator->pMaterial;
        }
        pIterator++;
    }

    OTDU_LOG_WARN( "Failed to find material %p in 2DM hashtable (nullptr will be returned)\n", param_2 );
    return nullptr;
}
