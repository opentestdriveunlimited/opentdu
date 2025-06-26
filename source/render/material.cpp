#include "shared.h"
#include "material.h"

#include "shaders/shader_register.h"

MaterialRegister gMaterialRegister = {};

int64_t __allshl( int32_t param_1, int32_t param_2, int32_t param_3 )
{
    if ( 0x3f < param_1 ) {
        return 0;
    }
    if ( param_1 < 0x20 ) {
        return ( param_2 << ( param_1 & 0x1f ) | ( ( int64_t )( param_3 ) >> 0x20 ) - ( param_1 & 0x1f ) ) << 0x20 | ( int64_t )( param_3 << ( param_1 & 0x1f ) );
    }
    return ( int64_t )( uint32_t )( param_3 << ( param_1 & 0x1f ) ) << 0x20;
}

static bool IsFloatAlmostZero( float* param_1 )
{
    float fVar1;

    fVar1 = *param_1;
    if ( fVar1 < 0.0f ) {
        fVar1 = 0.0f - fVar1;
    }
    if ( 0.0001f < fVar1 ) {
        return false;
    }
    return true;
}

Material::Material()
{
    setDefaultMaterial();
}

Material::~Material()
{

}

void Material::setDefaultMaterial()
{
    OT = 0;
    FXFlags = 0;
    FXFlags2 = 0;
    pOTNodePointer32 = 0u;
    AlphaSource = '\0';
    FillMode = '\0';
    MatrixParam = '\0';
    AlphaRef = '\0';
    ColorDisable = '\0';
    UsageFlags = 0;
    StencilEnable = '\0';
    StencilFail = '\0';
    StencilZFail = '\0';
    StencilRef = 0;
    StencilFailCW = '\0';
    StencilZFailCW = '\0';
    StencilRefCW = 0;
    BackfaceCullingMode = '\x03';
    SrcBlend = '\x02';
    BlendOP = '\x06';
    StencilPass = '\x02';
    StencilFunc = '\a';
    StencilPassCW = '\x02';
    StencilFuncCW = '\a';
    NumParams = 1;
    DepthTest = '\x01';
    DepthWrite = '\x01';
    AlphaTest = '\x01';
    DstBlend = '\x01';
    StencilMask = 0xffffffff;
    StencilWriteMask = 0xffffffff;
    StencilMaskCW = 0xffffffff;
    StenwilWriteMaskCW = 0xffffffff;
    pVertexShaders[0] = ( void* )0x0;
    pPixelShaders[0] = ( void* )0x0;
    pVertexShaders[1] = ( void* )0x0;
    pPixelShaders[1] = ( void* )0x0;
    pVertexShaders[2] = ( void* )0x0;
    pPixelShaders[2] = ( void* )0x0;
    pVertexShaders[3] = ( void* )0x0;
    pPixelShaders[3] = ( void* )0x0;
    //pVertexShaders[4] = ( void* )0x0;
    //pPixelShaders[4] = ( void* )0x0;
    //pVertexShaders[5] = ( void* )0x0;
    //pPixelShaders[5] = ( void* )0x0;
    //pVertexShaders[6] = ( void* )0x0;
    //pPixelShaders[6] = ( void* )0x0;
    //pVertexShaders[7] = ( void* )0x0;
    //pPixelShaders[7] = ( void* )0x0;
    AmbientColor = Eigen::Vector4<float>( 1.0f, 1.0f, 1.0f, 1.0f );
    DiffuseColor = Eigen::Vector4<float>( 1.0f, 1.0f, 1.0f, 1.0f );
    SpecularColor = Eigen::Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
    EmissiveColor = Eigen::Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
}

uint16_t Material::getOTForParameter( MaterialParameter* param_2 )
{
    uint32_t iVar6 = 0x2d;
    uint32_t local_4 = 0x2d;

    switch ( param_2->Hashcode ) {
    case 0xccddeeffffeeddcc:
        return 0x2b;

    case 0x31d3942a1b383483:
    case 0x55ae5ef8c288dfc4:
    case 0x719bf86269f07f72:
    case 0xa00d5c4ecab12190:
    {
        MaterialShaderParameterArray* peVar6 = ( MaterialShaderParameterArray* )( getParameterByIndex( 0 ) );
        MaterialShaderParameter* peVar7 = peVar6->getParameter( 0x3e, &local_4 );
        peVar7->bBindToVS = 1;
        // Fallthrough
    }
    case 0x1c6c7357183157b8:
    case 0x5789e65cb39c3501:
    case 0xdcbaabcdabcdeffe:
    case 0x31d6646666383483:
    {
        if ( AlphaTest == '\0' ) {
            iVar6 = 1;
        }

        if ( BlendOP == '\x06' ) {
            if ( iVar6 == 0x2d ) {
                iVar6 = 0;
            }
        } else {
            iVar6 = 0x22;
        }

        return iVar6;
    }
    case 0x5455291bc8ad93c9:
        BlendOP = '\x06';
        AlphaTest = '\x01';
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
        if ( AlphaTest == '\0' ) {
            iVar6 = 7;
        }
        if ( BlendOP != '\x06' ) {
            iVar6 = 0x23;
        }
        if ( DepthWrite == '\x02' ) {
            AlphaTest = '\x01';
            return 0x24;
        }
        if ( iVar6 == 0x2d ) {
            return 6;
        }

        return 8;
    }

    case 0x27c8abacd453201:
    {
        FXFlags = FXFlags | 4;
        BlendOP = '\x01';
        SrcBlend = '\x05';
        DstBlend = '\x06';
        AlphaRef = '\n';
        AlphaTest = '\0';
        return 0x15;
    }

    case 0xe0d553158eaf208e:
    {
        if ( DepthWrite == '\x02' ) {
            return 0x12;
        }
        if ( BlendOP != '\x06' ) {
            return 0x11;
        }
        AlphaTest = '\x01';
        return 8;
    }

    case 0xca37333333378865:
    {
        return ( DepthWrite == '\x02' ) + 0x11;
    }

    case 0x232101231232101:
    case 0x562101231232104:
    {
        return ( DepthWrite == '\x02' ) + 0xf;
    }

    case 0xa8c8cacacafe8dec:
        return 0x19;

    case 0xdebdbaaffacaa96b:
        return 0x29;

    case 0xdb79dadbd5013ed0:
    case 0xdb79babad5013ed0:
    case 0xdb79eaead5013ed0:
    case 0xdbaabebed5013ed0:
    case 0xdb79dacbd5013ed0:
    case 0xdb79baedd5013ed0:
    case 0xdbdededed5013ed0:
    case 0xdb79bbbbd5013ed0:
    {
        if ( AlphaTest == '\0' ) {
            iVar6 = 3;
        }
        if ( BlendOP != '\x06' ) {
            iVar6 = 0x1b;
        }
        if ( DepthWrite == '\x02' ) {
            uint32_t uVar3 = param_2->Flags.UnknownBytes2[3] << 1 | param_2->Flags.UnknownBytes2[2];

            if ( uVar3 == 0 ) {
                return 0x1e;
            }
            if ( uVar3 == 1 ) {
                return 0x1f;
            }
            if ( uVar3 == 2 ) {
                return 0x20;
            }
            if ( uVar3 == 3 ) {
                return 0x21;
            }
        }
        if ( iVar6 != 0x2d ) {
            return iVar6;
        }
        return 2;
    }

    case 0xcaca336d83078865:
    {
        param_2->Flags.NumPrelight = '\x01';
        param_2->Flags.bUsingAmbient = '\x01';
        return 0x2d;
    }

    case 0xbc4589eeac78ce32:
    {
        AlphaTest = '\0';
        AlphaRef = '\0';
        DepthWrite = '\x02';
        return 0x28;
    }

    case 0xbc4589ffac78df32:
    {
        AlphaTest = '\x01';
        BlendOP = '\x06';
        ColorDisable = '\x04';
        // MaterialLayer* peVar2 = param_2->getLayer( 0 );
        param_2->Flags.NumPrelight = '\0';
        return ( -( int32_t )( DepthTest != '\x01' ) & 0x1d ) + 8;
    }

    case 0xc68e90cdfd9658ba:
    {
        return 0x14;
    }

    case 0x1446886d97abe189:
    {
        AlphaRef = '\n';
        AlphaTest = '\0';
        DepthWrite = '\x02';
        MaterialLayer* peVar2 = param_2->getLayer( 0 );

        // 0098fcb9 c7 40 58        MOV        dword ptr[param_2->Type + 0x58], 0x1
        // 0098fcc0 89 58 5c        MOV        dword ptr[param_2->Type + 0x5c], EBX
        peVar2->pLayerTextures[2].Hashcode = 1ull << 32;

        param_2->Flags.bUsingSpecular = '\0';
        param_2->Flags.bUsingBinormal = '\0';
        param_2->Flags.NumPrelight = '\x01';
        param_2->Flags.bUsingTangent = '\x01';
        return 0x13;
    }

    case 0x1c6c7357dddddddd:
    {
        MaterialLayer* peVar2 = param_2->getLayer( 0 );
        param_2->Flags.NumUVMaps = '\x02';
        return 0x2d;
    }

    case 0x2cea0000:
    {
        return 0xe;
    }

    case 0x8a4efd79643cdcb2:
    {
        MaterialLayer* peVar2 = param_2->getLayer( 0 );

        //0098fd3c c7 40 38        MOV        dword ptr[EAX + param_2 + 0x38], 0x1
        //0098fd43 89 58 3c        MOV        dword ptr[EAX + param_2 + 0x3c], EBX
        peVar2->pLayerTextures[1].Hashcode = 1ull << 32;

        return 0x2d;
    }

    case 0x1c6cff57183157b8:
    {
        MaterialShaderParameterArray* peVar3 = ( MaterialShaderParameterArray* )param_2;

        if ( param_2->Flags.bUsingSpecular ) {
            float* peVar4 = ( float* )peVar3->getParameterData( 0x1b );

            bool cVar2 = IsFloatAlmostZero( peVar4 );
            if ( cVar2 != '\0' ) {
                param_2->Flags.bUsingSpecular = '\0';
            }
        }

        if ( param_2->Flags.bUsingReflection ) {
            float* peVar4 = ( float* )peVar3->getParameterData( 0x3e );

            bool cVar2 = IsFloatAlmostZero( peVar4 );
            if ( ( cVar2 != '\0' ) || ( cVar2 = IsFloatAlmostZero( peVar4 + 1 ), cVar2 != '\0' ) ) {
                MaterialLayer* peVar2 = param_2->getLayer( 0 );
                peVar2->pLayerTextures[3].Hashcode = 0ull;

                param_2->Flags.bUsingReflection = '\0';
                param_2->Flags.UnknownBytes2[6] = '\0';
                if ( param_2->Flags.bUsingGlossMap ) {
                    MaterialLayer* peVar2 = param_2->getLayer( 0 );
                    peVar2->pLayerTextures[2].Hashcode = 0ull;
                    param_2->Flags.bUsingGlossMap = '\0';
                }
            }
        }
        break;
    }

    case 0x1c6cee57241375b8:
    {
        if ( !param_2->Flags.bUsingReflection )
        {
            break;
        }

        MaterialShaderParameterArray* peVar5 = ( MaterialShaderParameterArray* )( param_2 );
        float* peVar4 = ( float* )peVar5->getParameterData( 0x3e );
        bool cVar2 = IsFloatAlmostZero( peVar4 );
        if ( ( cVar2 ) || ( cVar2 = IsFloatAlmostZero( peVar4 + 1 ), cVar2 ) ) {
            param_2->Flags.bUsingReflection = '\0';

            MaterialLayer* peVar2 = param_2->getLayer( 0 );
            peVar2->pLayerTextures[4].Hashcode = 0ull;

            if ( param_2->Flags.bUsingGlossMap ) {
                MaterialLayer* peVar2 = param_2->getLayer( 0 );
                peVar2->pLayerTextures[3].Hashcode = 0ull;
                param_2->Flags.bUsingGlossMap = '\0';
            }
        }
        break;
    }
    default:
        OTDU_UNIMPLEMENTED;
        return 0x2d;
    };

    iVar6 = 5;
    if ( AlphaTest != '\0' ) {
        iVar6 = local_4;
    }
    if ( BlendOP != '\x06' ) {
        iVar6 = 0x1c;
    }
    if ( DepthWrite == '\x02' ) {
        return 0x1d;
    }
    if ( iVar6 == 0x2d ) {
        return 4;
    }

    return iVar6;
}

uint16_t Material::getOTNumber()
{
    MaterialParameter* peVar1 = nullptr;
    uint16_t iVar2 = 0x2d;
    uint64_t uVar3 = 0;

    if ( NumParams != 0 ) {
        peVar1 = getParameterByIndex( 0 );
        if ( peVar1->Type == 2 ) {
            iVar2 = getOTForParameter( peVar1 );
        }
        if ( peVar1->Type == 1 ) {
            if ( AlphaTest == '\0' ) {
                iVar2 = 0xc;
            }
            if ( BlendOP == '\x06' ) {
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
    if ( AlphaTest == '\0' ) {
        iVar2 = 9;
    }
    if ( ( BlendOP == '\x06' ) || ( iVar2 = 0x26, DepthWrite != '\x02' ) ) {
        if ( iVar2 == 0x2d ) {
            iVar2 = 8;
        }
    } else {
        iVar2 = 0x28;
    }
LAB_00990061:
    uVar3 = __allshl( iVar2, 0, 1 );
    if ( ( uVar3 & 0x82000005fff ) != 0 ) {
        StencilEnable = '\x01';
        StencilFail = '\0';
        StencilZFail = '\0';
        StencilPass = '\x02';
        StencilFunc = '\a';
        StencilRef = 0;
        StencilMask = 0xffffffff;
        StencilWriteMask = 0xffffffff;
    }
    return iVar2;
}

MaterialParameter* Material::getParameterByIndex( const uint32_t index )
{
    if ( NumParams == 0 ) {
        return nullptr;
    }

    MaterialParameter* pParameters = ( MaterialParameter* )( this + 0xd0 );
    return &pParameters[index];
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

void* MaterialShaderParameterArray::getParameterData( const uint32_t type )
{
    uint32_t parameterIndex = 0u;
    getParameter( type, &parameterIndex );
    return getParameterDataByIndex( parameterIndex );
}

void* MaterialShaderParameterArray::getParameterDataByIndex( uint32_t param_2 )
{
    int iVar2;
    int* iVar3 = nullptr;
    int* piVar4 = nullptr;

    uint32_t uVar1 = NumParameters;
    if ( uVar1 <= param_2 ) {
        return 0;
    }
    if ( uVar1 - 1 < uVar1 ) {
        OTDU_UNIMPLEMENTED;
        // iVar3 = ( uVar1 + 4 ) * 0x10 + this;
    } else {
        iVar3 = 0;
    }
    iVar3 = iVar3 + 0x10;
    if ( param_2 == 0 ) {
        return iVar3;
    }
    piVar4 = ( int* )( this + 0x58 );
    do {
        iVar2 = *piVar4;
        param_2 = param_2 - 1;
        piVar4 = piVar4 + 4;
        iVar3 = iVar2 + iVar3;
    } while ( param_2 != 0 );
    return iVar3;
}

MaterialLayer* MaterialParameter::getLayer( const uint32_t index )
{
    // TODO: This is a direct translation from ASM (and is quite frankly horrible to read). Refactor.
    if ( Type == 1 && index < NumLayers) { // Terrain (or multilayered material?)
        OTDU_UNIMPLEMENTED;
        return nullptr;
        //return ( MaterialTerrainParameter* )( this )->pLayers[index];
    } else if ( Type == 2 && index == 0 ) { // Regular
        return ( MaterialLayer* )( this + 0x44 );
    }

    return nullptr;
}

MaterialRegister::MaterialRegister()
    : numRegisteredMaterial( 0u )
{

}

void MaterialRegister::pushMaterial( RenderFile::Section* pFile )
{
    OTDU_ASSERT( pFile );

    Material* pMaterial = ( Material* )( pFile + 1 );
    pMaterial->OT = pMaterial->getOTNumber();
    MaterialParameter* pParameters = *( MaterialParameter** )( ( uint8_t* )pMaterial + 0xd0 );
    pParameters = ( MaterialParameter* )(( int8_t* )pParameters + 0x10 ); // Skip header

    gShaderRegister.retrieveShadersForMaterial( pMaterial, 0 );

    if ( pMaterial->NumParams != 0 && pParameters->Flags.bAffectedByFog ) {
        pMaterial->FXFlags2 |= 1;
        numRegisteredMaterial++;
        return;
    }

    numRegisteredMaterial++;
}

void MaterialRegister::popMaterial()
{
    numRegisteredMaterial--;
}
