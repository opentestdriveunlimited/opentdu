#include "shared.h"
#include "2dm.h"

#include "render/material.h"

static constexpr uint32_t k2DMMagic             = 0x4d44322e; // MD2. (.2DM)
static constexpr uint32_t kMaterialArrayMagic   = 0x4154414d; // MATA (MATerial Array)
static constexpr uint32_t kMaterialMagic        = 0x2e54414d; // MAT. (MATerial)
static constexpr uint32_t kParameterMagic       = 0x41524150; // PARA (PARAmeter)
static constexpr uint32_t kLayerArrayMagic      = 0x4159414c; // LAYA (LAYer Array)
static constexpr uint32_t kHashcodesMagic       = 0x48534148; // HASH (HASHcodes)

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

    case kParameterMagic:
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

Material* Render2DM::create( void* param_1, uint64_t param_2, int32_t param_3, uint32_t param_4, uint32_t param_5, uint32_t param_6 )
{
    return nullptr;
}
