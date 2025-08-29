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
static constexpr uint32_t kParameterMagic       = 0x2e524150; // PAR. (PARameter)

static constexpr uint32_t k2DMVersionMajor      = 2;
static constexpr uint32_t k2DMVersionMinor      = 0;

static constexpr uint32_t kMaxNumLayers         = 8;

static uint32_t DAT_005f52b0 = 2; // DAT_005f52b0

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

Material* Render2DM::Create( void* param_1, uint64_t param_2, int32_t param_3, uint32_t param_4, uint32_t param_5, uint32_t param_6 )
{
    int32_t iVar3 = param_4 * 0x10 + param_5;
    memset(param_1, 0, iVar3 + 0x2d3U & 0xfffffff0);

    int8_t* pStreamPointer = (int8_t*)param_1;
    pStreamPointer = (pStreamPointer + 0xfu);

    RenderFile::Header* pHeader = (RenderFile::Header*)pStreamPointer;
    pHeader->VersionMajor = k2DMVersionMajor;
    pHeader->VersionMinor = k2DMVersionMinor;
    pHeader->Size = iVar3;
    pHeader->Hashcode = k2DMMagic;
    pHeader->Flags = 0;

    RenderFile::Section* pLayerArraySection = (RenderFile::Section*)( pStreamPointer + 0x10 );
    pLayerArraySection->Type = kLayerArrayMagic;
    pLayerArraySection->VersionMajor = 0;
    pLayerArraySection->VersionMinor = 0;
    pLayerArraySection->Size = 0x130;
    pLayerArraySection->DataSize = 0x10;

    RenderFile::Section* pPrimaryLayer = (RenderFile::Section*)( pStreamPointer + 0x20 );
    pPrimaryLayer->Type = kLayerMagic;
    pPrimaryLayer->VersionMajor = 0;
    pPrimaryLayer->VersionMinor = 0;
    pPrimaryLayer->Size = 0x120;
    pPrimaryLayer->DataSize = 0x120;

    pStreamPointer[0x3c] = static_cast<uint8_t>( param_3 );

    int8_t* pLayerParams = (pStreamPointer + 0x40);
    for ( int32_t iVar7 = 0; iVar7 < 8; iVar7++ ) {
        // NOTE Original ASM has relative neg offsets
        pLayerParams[0x16] = 0; // MOV        byte ptr [ECX + -0x1],BL
        pLayerParams[0x17] = 0; // Original offset is 0x57
        pLayerParams[0x18] = 0;
        pLayerParams[0x19] = 0;

        if (iVar7 < param_3) {
            pLayerParams[0x8] = 1;
            for ( uint32_t i = 0x9; i < 0x10; i++ ) {
                pLayerParams[i] = 0;
            }
        }

        pLayerParams += 0x20;
    }

    RenderFile::Section* pParameterArray = (RenderFile::Section*)( pStreamPointer + 0x140 );
    pPrimaryLayer->Type = kParameterArrayMagic;
    pPrimaryLayer->VersionMajor = 0;
    pPrimaryLayer->VersionMinor = 0;
    pPrimaryLayer->Size = (param_4 + 7) * 0x10 + param_5;
    pPrimaryLayer->DataSize = 0x10;

    RenderFile::Section* pPrimaryParameter = (RenderFile::Section*)( pStreamPointer + 0x150 );
    pPrimaryParameter->Type = kParameterMagic;
    pPrimaryParameter->VersionMajor = 0;
    pPrimaryParameter->VersionMinor = 0;
    uint32_t uVar5 = (param_4 + 6) * 0x10 + param_5;
    pPrimaryParameter->Size = uVar5;
    pPrimaryParameter->DataSize = uVar5;

    MaterialShaderParameterArray* pPrimaryShaderParam = (MaterialShaderParameterArray*)( pStreamPointer + 0x160 );
    pPrimaryShaderParam->pParameters = pPrimaryLayer;
    pPrimaryShaderParam->Flags.NumTextures = param_3;
    pPrimaryShaderParam->Flags.NumUVMaps = param_3;
    pPrimaryShaderParam->Flags.NumPrelight = param_6;
    pPrimaryShaderParam->Hashcode = param_2;
    pPrimaryShaderParam->Type = eMaterialParameterType::MPT_Generic;
    pPrimaryShaderParam->NumParameters = param_4;
    pPrimaryShaderParam->NumLayers = 1;
    
    MaterialShaderParameter* pParams = (MaterialShaderParameter*)( pStreamPointer + 0x1b0 );
    for (uint32_t i = 0; i < param_4; i++ ) {
        MaterialShaderParameter& param = pParams[i];
        param.Size = 0x10;
        param.Format = 2;
        param.bBindToVS = false;
        param.Type = 0;
    }

    RenderFile::Section* pMaterialArray = (RenderFile::Section*)( pStreamPointer + 0x170 + (param_4 * 0x10) + param_5 );
    pMaterialArray->Type = kMaterialArrayMagic;
    pMaterialArray->VersionMajor = 0;
    pMaterialArray->VersionMinor = 0;
    pMaterialArray->Size = 0x114;
    pMaterialArray->DataSize = 0x10;

    RenderFile::Section* pHashSection = ( pMaterialArray + 1 );
    pHashSection->Type = kHashcodesMagic;
    pHashSection->VersionMajor = 0;
    pHashSection->VersionMinor = 0;
    pHashSection->Size = 0x20;
    pHashSection->DataSize = 0x20;

    HashTableEntry* pMainEntry = (HashTableEntry*)(pHashSection + 1);
    pMainEntry->Hashcode = 0xdeadbeef;
    pMainEntry->pMaterial = nullptr;

    RenderFile::Section* pMaterialHeader = (RenderFile::Section*)(pHashSection + 1);
    pMaterialHeader->Type = kMaterialMagic;
    pMaterialHeader->VersionMajor = 0;
    pMaterialHeader->VersionMinor = 0;
    pMaterialHeader->Size = 0xe4;
    pMaterialHeader->DataSize = 0xe4;

    Material* pMaterial = (Material*)( pMaterialHeader + 1 );
    pMaterial->setDefaultMaterial();
    pMaterial->NumParams = 1;

    RenderFile::Section** pParamSectionPointer = (RenderFile::Section**)((uint8_t*)pMaterial + 0xd0);
    *pParamSectionPointer = pPrimaryParameter;

    return pMaterial;
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

StaticRender2DM::StaticRender2DM(bool param_2, uint64_t param_3)
{
    // FUN_005f52b0
    pMaterial = Render2DM::Create( pBuffer, param_3, 0, 0, 0, 0);
    pMaterial->OT = DAT_005f52b0;
    DAT_005f52b0++;
    pMaterial->NumParams = 1;
    pMaterial->FXFlags = 0;
    pMaterial->FXFlags2 = 0;
    pMaterial->UsageFlags = 0;
    pMaterial->pOTNode = nullptr;
    pMaterial->AlphaSource = '\0';
    pMaterial->DepthWrite = '\x01';
    pMaterial->DepthTest = '\x01';
    if (param_2) {
        pMaterial->SrcBlend = '\x05';
        pMaterial->DstBlend = '\x06';
        pMaterial->BlendOP = '\x01';
    } else {
        pMaterial->SrcBlend = '\x02';
        pMaterial->DstBlend = '\x01';
        pMaterial->BlendOP = '\x06';
    }
    pMaterial->BackfaceCullingMode = '\x03';
    pMaterial->FillMode = '\0';
    pMaterial->AlphaTest = '\x01';
    pMaterial->AmbientColor = Eigen::Vector4f(1.0f, 1.0f, 1.0f, 0.0f);
    pMaterial->DiffuseColor = Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    pMaterial->SpecularColor = Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
}

StaticRender2DM DAT_00fe6d50(true,  0x230588678a67836d); // DAT_00fe6d50
StaticRender2DM DAT_00fe67dc(false, 0xb9bae7f43dafe4f6); // DAT_00fe67dc
StaticRender2DM DAT_00fe6aa0(true,  0xb9bae7f43dafe4f6); // DAT_00fe6aa0
