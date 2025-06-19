#include "shared.h"
#include "material.h"

static uint32_t gNumRegisteredMaterial = 0u;

Material::Material()
{
    setDefaultMaterial();
}

Material::~Material()
{

}

void Material::setDefaultMaterial()
{
    pContent->OT = 0;
    pContent->FXFlags = 0;
    pContent->FXFlags2 = 0;
    pContent->pOTNode = ( void* )0x0;
    pContent->AlphaSource = '\0';
    pContent->FillMode = '\0';
    pContent->MatrixParam = '\0';
    pContent->AlphaRef = '\0';
    pContent->ColorDisable = '\0';
    pContent->UsageFlags = 0;
    pContent->StencilEnable = '\0';
    pContent->StencilFail = '\0';
    pContent->StencilZFail = '\0';
    pContent->StencilRef = 0;
    pContent->StencilFailCW = '\0';
    pContent->StencilZFailCW = '\0';
    pContent->StencilRefCW = 0;
    pContent->BackfaceCullingMode = '\x03';
    pContent->SrcBlend = '\x02';
    pContent->BlendOP = '\x06';
    pContent->StencilPass = '\x02';
    pContent->StencilFunc = '\a';
    pContent->StencilPassCW = '\x02';
    pContent->StencilFuncCW = '\a';
    pContent->NumParams = 1;
    pContent->DepthTest = '\x01';
    pContent->DepthWrite = '\x01';
    pContent->AlphaTest = '\x01';
    pContent->DstBlend = '\x01';
    pContent->StencilMask = 0xffffffff;
    pContent->StencilWriteMask = 0xffffffff;
    pContent->StencilMaskCW = 0xffffffff;
    pContent->StenwilWriteMaskCW = 0xffffffff;
    pContent->pVertexShaders[0] = ( void* )0x0;
    pContent->pPixelShaders[0] = ( void* )0x0;
    pContent->pVertexShaders[1] = ( void* )0x0;
    pContent->pPixelShaders[1] = ( void* )0x0;
    pContent->pVertexShaders[2] = ( void* )0x0;
    pContent->pPixelShaders[2] = ( void* )0x0;
    pContent->pVertexShaders[3] = ( void* )0x0;
    pContent->pPixelShaders[3] = ( void* )0x0;
    pContent->pVertexShaders[4] = ( void* )0x0;
    pContent->pPixelShaders[4] = ( void* )0x0;
    pContent->pVertexShaders[5] = ( void* )0x0;
    pContent->pPixelShaders[5] = ( void* )0x0;
    pContent->pVertexShaders[6] = ( void* )0x0;
    pContent->pPixelShaders[6] = ( void* )0x0;
    pContent->pVertexShaders[7] = ( void* )0x0;
    pContent->pPixelShaders[7] = ( void* )0x0;
    pContent->AmbientColor = Eigen::Vector4<float>( 1.0f, 1.0f, 1.0f, 1.0f );
    pContent->DiffuseColor = Eigen::Vector4<float>( 1.0f, 1.0f, 1.0f, 1.0f );
    pContent->SpecularColor = Eigen::Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
    pContent->EmissiveColor = Eigen::Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
}

int32_t FUN_0098f8e0( Material* param_1, MaterialParameter* param_2 )
{
    int32_t iVar6 = 0x2d;

    switch ( param_2->Hashcode ) {
    case 0xccddeeffffeeddcc:
        return 0x2b;

    case 0x31d3942a1b383483:
    case 0x55ae5ef8c288dfc4:
    case 0x719bf86269f07f72:
    case 0xa00d5c4ecab12190:
    {
        MaterialShaderParameterArray* peVar6 = ( MaterialShaderParameterArray* )param_1->getParameterByIndex( 0 );

        uint32_t local_4 = 0;
        MaterialShaderParameter* peVar7 = peVar6->getParameter( 0x3e, &local_4 );
        peVar7->bBindToVS = 1;
        // Fallthrough
    }
    case 0x1c6c7357183157b8:
    case 0x5789e65cb39c3501:
    case 0xdcbaabcdabcdeffe:
    case 0x31d6646666383483:
    {
        if ( param_1->AlphaTest == '\0' ) {
            iVar6 = 1;
        }

        if ( param_1->BlendOP == '\x06' ) {
            if ( iVar6 == 0x2d ) {
                iVar6 = 0;
            }
        } else {
            iVar6 = 0x22;
        }

        return iVar6;
    }
    case 0x5455291bc8ad93c9:
        param_1->BlendOP = '\x06';
        param_1->AlphaTest = '\x01';
        return 10;

    case 0xf123473b0f1234b:
    {
        MaterialLayer* iVar4 = param_2->getLayer( 0 );
        OTDU_UNIMPLEMENTED;
        //*( undefined4* )( iVar4 + 0x58 ) = 1;
        //*( undefined4* )( iVar4 + 0x5c ) = 0;
        param_2->Flags.UnknownBytes2[4] = '\x01';
        return 0x2d;
    }

    case 0xcaca336d73078865:
    case 0xcacacaca73078865:
    {
        if ( param_1->AlphaTest == '\0' ) {
            iVar6 = 7;
        }
        if ( param_1->BlendOP != '\x06' ) {
            iVar6 = 0x23;
        }
        if ( param_1->DepthWrite == '\x02' ) {
            param_1->AlphaTest = '\x01';
            return 0x24;
        }
        if ( iVar6 == 0x2d ) {
            return 6;
        }
    } break;

    default:
        OTDU_UNIMPLEMENTED;
        break;
    };
}

uint16_t GetOTNumber( SerializedMaterial* param_1, Material* param_2 )
{
    MaterialParameter* peVar1 = nullptr;
    uint16_t iVar2 = 0x2d;
    uint64_t uVar3 = 0;

    if ( param_1->NumParams != 0 ) {
        peVar1 = param_2->getParameterByIndex( 0 );
        if ( peVar1->Type == 2 ) {
            iVar2 = FUN_0098f8e0( param_1, peVar1 );
        }
        if ( peVar1->Type == 1 ) {
            if ( param_1->AlphaTest == '\0' ) {
                iVar2 = 0xc;
            }
            if ( param_1->BlendOP == '\x06' ) {
                if ( iVar2 == 0x2d ) {
                    iVar2 = 0xb;
                }
            } else {
                iVar2 = 0xd;
            }
            goto LAB_00990061;
        }
        if ( iVar2 != 0x2d ) goto LAB_00990061;
    }
    if ( param_1->AlphaTest == '\0' ) {
        iVar2 = 9;
    }
    if ( ( param_1->BlendOP == '\x06' ) || ( iVar2 = 0x26, param_1->DepthWrite != '\x02' ) ) {
        if ( iVar2 == 0x2d ) {
            iVar2 = 8;
        }
    } else {
        iVar2 = 0x28;
    }
LAB_00990061:
    uVar3 = __allshl();
    if ( ( uVar3 & 0x82000005fff ) != 0 ) {
        param_1->StencilEnable = '\x01';
        param_1->StencilFail = '\0';
        param_1->StencilZFail = '\0';
        param_1->StencilPass = '\x02';
        param_1->StencilFunc = '\a';
        param_1->StencilRef = 0;
        param_1->StencilMask = 0xffffffff;
        param_1->StencilWriteMask = 0xffffffff;
    }
    return iVar2;
}

void Material::initializeFromFile( RenderFile::Section* pFile )
{
    OTDU_ASSERT( pFile );
    
    pContent = ( SerializedMaterial* )( pFile + 1 );
    pParameters = ( MaterialParameter* )( ( uint8_t* )pContent + 0xd0 );

    pContent->OT = GetOTNumber( pContent, this );
    
    OTDU_UNIMPLEMENTED;
    // RetrieveShaderPermutations( peVar2, peVar2->VertexShader, peVar2->PixelShader );

    if ( pContent->NumParams != 0 && pParameters->Flags.bAffectedByFog ) {
        pContent->FXFlags2 |= 1;
        gNumRegisteredMaterial++;
        return;
    }

    gNumRegisteredMaterial++;
}

void Material::unregister()
{
    gNumRegisteredMaterial--;
}

MaterialParameter* Material::getParameterByIndex( const uint32_t index )
{
    return ( NumParams == 0 ) ? nullptr : pParameters[index];
}

MaterialShaderParameter* MaterialShaderParameterArray::getParameter( const uint32_t type, uint32_t* pOutParamIndex /*= nullptr */ )
{
    MaterialShaderParameter* pParameter = nullptr;

    MaterialShaderParameter* pParameterIt = ( MaterialShaderParameter* )pParameters;
    uint32_t parameterIndex = 0;
    while ( parameterIndex < NumParameters ) {
        if ( pParameterIt->Type == type ) {
            break;
        }

        parameterIndex++;
        pParameterIt++;
    }

    if ( pParameter && pOutParamIndex != nullptr ) {
        *pOutParamIndex = parameterIndex;
    }
    return pParameter;
}

MaterialLayer* MaterialParameter::getLayer( const uint32_t index )
{
    // TODO: This is a direct translation from ASM (and is quite frankly horrible to read). Refactor.
    if ( Type == 1 && index < NumLayers) { // Terrain (or multilayered material?)
        return ( MaterialTerrainParameter* )( this )->pLayers[index];
    } else if ( Type == 2 && index == 0 ) { // Regular
        return ( MaterialShaderParameterArray* )(this)->pLayer;
    }

    return nullptr;
}
