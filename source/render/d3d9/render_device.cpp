#include "shared.h"

#if OTDU_D3D9
#include <d3d9.h>

#include "render_device.h"
#include "render/gs_render_helper.h"
#include "render/2db.h"

#include "render/shaders/shader_register.h"
#include "render/postfx/postfx_stack.h"

#include "system/gs_system.h"

#define OTDU_D3D9_SAFE_RELEASE( resource )\
if ( resource != nullptr ) {\
    resource->Release();\
    resource = nullptr;\
}\

static constexpr const D3DFORMAT kViewFormatPixelFormatLUT[eViewFormat::VF_Count] = {
    D3DFORMAT::D3DFMT_R8G8B8,
    D3DFORMAT::D3DFMT_A8R8G8B8,
    D3DFORMAT::D3DFMT_X8R8G8B8,
    D3DFORMAT::D3DFMT_A8R8G8B8,
    D3DFORMAT::D3DFMT_R5G6B5,
    D3DFORMAT::D3DFMT_A1R5G5B5,
    D3DFORMAT::D3DFMT_A8,
    D3DFORMAT::D3DFMT_A8R3G3B2,
    D3DFORMAT::D3DFMT_A2R10G10B10,
    D3DFORMAT::D3DFMT_G16R16,
    D3DFORMAT::D3DFMT_A16B16G16R16,
    D3DFORMAT::D3DFMT_L8,
    D3DFORMAT::D3DFMT_L16,
    D3DFORMAT::D3DFMT_V8U8,
    D3DFORMAT::D3DFMT_Q8W8V8U8,
    D3DFORMAT::D3DFMT_V16U16,
    D3DFORMAT::D3DFMT_Q16W16V16U16,
    D3DFORMAT::D3DFMT_R16F,
    D3DFORMAT::D3DFMT_G16R16F,
    D3DFORMAT::D3DFMT_A16B16G16R16F,
    D3DFORMAT::D3DFMT_R32F,
    D3DFORMAT::D3DFMT_G32R32F,
    D3DFORMAT::D3DFMT_A32B32G32R32F,
    D3DFORMAT::D3DFMT_DXT1,
    D3DFORMAT::D3DFMT_DXT2,
    D3DFORMAT::D3DFMT_DXT3,
    D3DFORMAT::D3DFMT_DXT4,
    D3DFORMAT::D3DFMT_DXT5,

    D3DFORMAT::D3DFMT_R8G8B8,
    D3DFORMAT::D3DFMT_A8R8G8B8,
    D3DFORMAT::D3DFMT_X8R8G8B8,
    D3DFORMAT::D3DFMT_A8R8G8B8,
    D3DFORMAT::D3DFMT_R5G6B5,
    D3DFORMAT::D3DFMT_A1R5G5B5,
    D3DFORMAT::D3DFMT_A8,
    D3DFORMAT::D3DFMT_A8R3G3B2,
    D3DFORMAT::D3DFMT_A2R10G10B10,
    D3DFORMAT::D3DFMT_G16R16,
    D3DFORMAT::D3DFMT_A16B16G16R16,
    D3DFORMAT::D3DFMT_L8,
    D3DFORMAT::D3DFMT_L16,
    D3DFORMAT::D3DFMT_V8U8,
    D3DFORMAT::D3DFMT_Q8W8V8U8,
    D3DFORMAT::D3DFMT_V16U16,
    D3DFORMAT::D3DFMT_Q16W16V16U16,
    D3DFORMAT::D3DFMT_R16F,
    D3DFORMAT::D3DFMT_G16R16F,
    D3DFORMAT::D3DFMT_A16B16G16R16F,
    D3DFORMAT::D3DFMT_R32F,
    D3DFORMAT::D3DFMT_G32R32F,
    D3DFORMAT::D3DFMT_A32B32G32R32F,
    D3DFORMAT::D3DFMT_DXT1,
    D3DFORMAT::D3DFMT_DXT2,
    D3DFORMAT::D3DFMT_DXT3,
    D3DFORMAT::D3DFMT_DXT4,
    D3DFORMAT::D3DFMT_DXT5,

    D3DFORMAT::D3DFMT_UNKNOWN,
    D3DFORMAT::D3DFMT_UNKNOWN,
    D3DFORMAT::D3DFMT_UNKNOWN,
};

static constexpr D3DFORMAT kBackbufferFormats[4] = {
    D3DFMT_A2B10G10R10,
    D3DFMT_X8R8G8B8,
    D3DFMT_X1R5G5B5,
    D3DFMT_R5G6B5
};

static constexpr D3DMULTISAMPLE_TYPE ToD3DMSAA( const eAntiAliasingMethod antiAliasing )
{
    switch ( antiAliasing ) {
    case eAntiAliasingMethod::AAM_MSAA_X2: return D3DMULTISAMPLE_2_SAMPLES;
    case eAntiAliasingMethod::AAM_MSAA_X4: return D3DMULTISAMPLE_4_SAMPLES;
    default: return D3DMULTISAMPLE_NONE;
    }
}

static constexpr D3DMULTISAMPLE_TYPE kMSAATypes[3] = {
    D3DMULTISAMPLE_NONE,
    D3DMULTISAMPLE_2_SAMPLES,
    D3DMULTISAMPLE_4_SAMPLES
};

static constexpr const int32_t UINT_ARRAY_ARRAY_00eedac0[34] = {
    0x0, 0x2, 0x1, 0x10,
    0x2, 0x4, 0x3, 0x4,
    0x4, 0x4, 0x5, 0x4,
    0x6, 0x2, 0x7, 0x11,
    0x8, 0x11, 0x9, 0xE,
    0xA, 0xC, 0xB, 0x10,
    0xC, 0x1, 0xD, 0x6,
    0xE, 0xF, 0xF, 0x0,
    0x10, 0x11
};

static uint32_t gRenderTargetFormatCapsLUT[eViewFormat::VF_Count];
static uint32_t gTextureFormatCapsLUT[eViewFormat::VF_Count];
static uint32_t gDeviceTier = 1; // DAT_00f47218 // TODO: Not really sure what's used for yet
static uint32_t gPixelShaderSupport = 4; // DAT_00f4721c

static uint8_t gDeviceIndividualCaps[3][4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }; // DAT_00fadb44
static uint8_t uint8_t_00fadaa0 = 0;
static void* PTR_00faf3e0 = nullptr;

static bool RetrieveAdapterInfos( IDirect3D9* pIDirect3D9, const uint32_t adapterIndex, GPUAdapterDesc* pOutDesc )
{
    D3DADAPTER_IDENTIFIER9 local_458;
    HRESULT HVar1 = pIDirect3D9->GetAdapterIdentifier( adapterIndex, 2, &local_458 );
    if ( HVar1 < 0 ) {
        return false;
    }

    pOutDesc->AdapterName = local_458.Driver;
    pOutDesc->DeviceDesc = local_458.Description;
    pOutDesc->DeviceID = local_458.DeviceId;
    pOutDesc->Revision = local_458.Revision;
    pOutDesc->SubSysId = local_458.SubSysId;
    pOutDesc->VendorId = local_458.VendorId;

    return true;
}

RenderDevice::RenderDevice()
    : activeWindow( 0 )
    , displayInfos{}
    , presentParameters{}
    , frameIndex( 0 )
    , flags( 0 )
    , bSceneEncodingInProgress( false )
    , pDevice( nullptr )
    , pIDirect3D9( nullptr )
    , pBoundVertexShader( nullptr )
    , pBoundPixelShader( nullptr )
    , pBoundIndexBuffer( nullptr )
    , pHmapVertexBuffer( nullptr )
    , pHmapIndexBuffer( nullptr )
    , pLightVertexBuffer( nullptr )
    , PTR_00fe8a14( nullptr )
    , PTR_00fe8a18( nullptr )
    , PTR_00fe8a1c( nullptr )
    , PTR_00fe8a20( nullptr )
    , DAT_00fe8a28( 0 )
    , postPresentCallback( nullptr )
{
    memset( pBoundRenderTargets, 0, sizeof( IDirect3DSurface9* ) * kMaxSimultaneousRT );
    memset( pBoundTextures, 0, sizeof( IDirect3DBaseTexture9* ) * kMaxBoundTextures );
    memset( boundSamplers, 0, sizeof( GPUSampler ) * kMaxSamplerCount );
    memset( boundRenderStates, 0xff, kNumCachedRasterState * sizeof( DWORD ) );
    memset( boundVertexStreams, 0x0, kMaxNumCachedVertexStream * sizeof( BoundVertexStream ) );
    memset( boundVertexStreamFrequency, 0x0, kMaxNumCachedVertexStream * sizeof( uint32_t ) );

    // DAT_00eeb540
    rasterStates[D3DRS_ZENABLE] = 0;
    rasterStates[D3DRS_ZFUNC] = 4;
    rasterStates[D3DRS_ZWRITEENABLE] = 1;
    rasterStates[D3DRS_FILLMODE] = 3;
    rasterStates[D3DRS_CULLMODE] = 3;
    rasterStates[D3DRS_ALPHABLENDENABLE] = 0;
    rasterStates[D3DRS_SEPARATEALPHABLENDENABLE] = 0;
    rasterStates[D3DRS_BLENDFACTOR] = 0xffffffff;
    rasterStates[D3DRS_SRCBLEND] = 2;
    rasterStates[D3DRS_DESTBLEND] = 1;
    rasterStates[D3DRS_BLENDOP] = 1;
    rasterStates[D3DRS_SRCBLENDALPHA] = 2;
    rasterStates[D3DRS_DESTBLENDALPHA] = 1;
    rasterStates[D3DRS_BLENDOPALPHA] = 1;
    rasterStates[D3DRS_ALPHATESTENABLE] = 0;
    rasterStates[D3DRS_ALPHAREF] = 0;
    rasterStates[D3DRS_ALPHAFUNC] = 8;
    rasterStates[D3DRS_STENCILENABLE] = 0;
    rasterStates[D3DRS_TWOSIDEDSTENCILMODE] = 0;
    rasterStates[D3DRS_STENCILFAIL] = 1;
    rasterStates[D3DRS_STENCILZFAIL] = 1;
    rasterStates[D3DRS_STENCILPASS] = 1;
    rasterStates[D3DRS_STENCILFUNC] = 8;
    rasterStates[D3DRS_CCW_STENCILFAIL] = 1;
    rasterStates[D3DRS_CCW_STENCILZFAIL] = 1;
    rasterStates[D3DRS_CCW_STENCILPASS] = 1;
    rasterStates[D3DRS_CCW_STENCILFUNC] = 8;
    rasterStates[D3DRS_STENCILREF] = 0;
    rasterStates[D3DRS_STENCILMASK] = 0xffffffff;
    rasterStates[D3DRS_STENCILWRITEMASK] = 0xffffffff;
    rasterStates[D3DRS_CLIPPLANEENABLE] = 0;
    rasterStates[D3DRS_POINTSIZE] = 0x3F800000;
    rasterStates[D3DRS_POINTSIZE_MIN] = 0x3F800000;
    rasterStates[D3DRS_POINTSPRITEENABLE] = 0;
    rasterStates[D3DRS_POINTSCALEENABLE] = 0;
    rasterStates[D3DRS_POINTSCALE_A] = 0x3F800000;
    rasterStates[D3DRS_POINTSCALE_B] = 0;
    rasterStates[D3DRS_POINTSCALE_C] = 0;
    rasterStates[D3DRS_POINTSIZE_MAX] = 0x3F800000;
    rasterStates[D3DRS_MULTISAMPLEANTIALIAS] = 1;
    rasterStates[D3DRS_MULTISAMPLEMASK] = 0xffffffff;
    rasterStates[D3DRS_SCISSORTESTENABLE] = 0;
    rasterStates[D3DRS_SLOPESCALEDEPTHBIAS] = 0;
    rasterStates[D3DRS_DEPTHBIAS] = 0;
    rasterStates[D3DRS_COLORWRITEENABLE] = 0xf;
    rasterStates[D3DRS_TWEENFACTOR] = 0;
    rasterStates[D3DRS_COLORWRITEENABLE1] = 0xf;
    rasterStates[D3DRS_COLORWRITEENABLE2] = 0xf;
    rasterStates[D3DRS_COLORWRITEENABLE3] = 0xf;
    rasterStates[D3DRS_SRGBWRITEENABLE] = 0;
    rasterStates[D3DRS_MINTESSELLATIONLEVEL] = 0x3F800000;
    rasterStates[D3DRS_MAXTESSELLATIONLEVEL] = 0x3F800000;
    rasterStates[D3DRS_WRAP0] = 0;
    rasterStates[D3DRS_WRAP1] = 0;
    rasterStates[D3DRS_WRAP2] = 0;
    rasterStates[D3DRS_WRAP3] = 0;
    rasterStates[D3DRS_WRAP4] = 0;
    rasterStates[D3DRS_WRAP5] = 0;
    rasterStates[D3DRS_WRAP6] = 0;
    rasterStates[D3DRS_WRAP7] = 0;
    rasterStates[D3DRS_WRAP8] = 0;
    rasterStates[D3DRS_WRAP9] = 0;
    rasterStates[D3DRS_WRAP10] = 0;
    rasterStates[D3DRS_WRAP11] = 0;
    rasterStates[D3DRS_WRAP12] = 0;
    rasterStates[D3DRS_WRAP13] = 0;
    rasterStates[D3DRS_WRAP14] = 0;
    rasterStates[D3DRS_WRAP15] = 0;
    rasterStates[D3DRS_PATCHEDGESTYLE] = 0;
    rasterStates[D3DRS_POSITIONDEGREE] = 3;
    rasterStates[D3DRS_NORMALDEGREE] = 1;
    rasterStates[D3DRS_ANTIALIASEDLINEENABLE] = 0;
    rasterStates[D3DRS_ADAPTIVETESS_X] = 0;
    rasterStates[D3DRS_ADAPTIVETESS_Y] = 0;
    rasterStates[D3DRS_ADAPTIVETESS_Z] = 0x3F800000;
    rasterStates[D3DRS_ADAPTIVETESS_W] = 0;
    rasterStates[D3DRS_DITHERENABLE] = 0;
    rasterStates[D3DRS_SHADEMODE] = 3;
    rasterStates[D3DRS_LASTPIXEL] = 1;
    rasterStates[D3DRS_AMBIENT] = 0;
    rasterStates[D3DRS_SPECULARENABLE] = 0;
    rasterStates[D3DRS_CLIPPING] = 1;
    rasterStates[D3DRS_LIGHTING] = 1;
    rasterStates[D3DRS_NORMALIZENORMALS] = 0;
    rasterStates[D3DRS_TEXTUREFACTOR] = 0xffffffff;
    rasterStates[D3DRS_INDEXEDVERTEXBLENDENABLE] = 0;
    rasterStates[D3DRS_VERTEXBLEND] = 0;
    rasterStates[D3DRS_FOGENABLE] = 0;
    rasterStates[D3DRS_FOGCOLOR] = 0;
    rasterStates[D3DRS_FOGTABLEMODE] = 0;
    rasterStates[D3DRS_FOGSTART] = 0;
    rasterStates[D3DRS_FOGEND] = 0x3F800000;
    rasterStates[D3DRS_FOGDENSITY] = 0x3F800000;
    rasterStates[D3DRS_RANGEFOGENABLE] = 0;
    rasterStates[D3DRS_FOGVERTEXMODE] = 0;
    rasterStates[D3DRS_COLORVERTEX] = 1;
    rasterStates[D3DRS_LOCALVIEWER] = 1;
    rasterStates[D3DRS_DIFFUSEMATERIALSOURCE] = 1;
    rasterStates[D3DRS_SPECULARMATERIALSOURCE] = 2;
    rasterStates[D3DRS_AMBIENTMATERIALSOURCE] = 0;
    rasterStates[D3DRS_EMISSIVEMATERIALSOURCE] = 0;
    rasterStates[D3DRS_DEBUGMONITORTOKEN] = 0;
    rasterStates[D3DRS_FORCE_DWORD] = 0; // Safe guard
}

RenderDevice::~RenderDevice()
{

}

void RenderDevice::initialize()
{
    bool bVar1 = createInstance();
    OTDU_ASSERT_FATAL( bVar1 );

    displayInfos.selectAdapter( pIDirect3D9 );
    OTDU_LOG_DEBUG( "Selected adapter %u\n", displayInfos.AdapterIndex );

    presentParameters = { 0 };
    displayInfos.fillPresentInfos( pIDirect3D9, activeWindow, &presentParameters );

    if ( ( flags >> 0x10 & 1 ) == 0 ) {
        if ( ( flags >> 0x11 & 1 ) != 0 ) {
            D3DADAPTER_IDENTIFIER9 DStack_1458;
            UINT UVar1 = pIDirect3D9->GetAdapterCount();
            for ( UINT Adapter = 0; Adapter < UVar1; Adapter++ ) {
                HRESULT HVar2 = pIDirect3D9->GetAdapterIdentifier( Adapter, 0, &DStack_1458 );
                if ( -1 < HVar2 ) {
                    if ( strcmp( DStack_1458.Driver, "NVIDIA NVPerfHUD" ) == 0 ) {
                        displayInfos.AdapterIndex = Adapter;
                        displayInfos.DeviceType = D3DDEVTYPE_REF;
                        break;
                    }
                }
            }
        }
    } else {
        displayInfos.DeviceType = D3DDEVTYPE_REF;
        displayInfos.BehaviorFlags = 0x20;
    }

    bVar1 = createDevice( &displayInfos, &presentParameters );
    OTDU_ASSERT_FATAL( bVar1 );
}

GPUShader* RenderDevice::createShader( eShaderType type, const void* pBytecode )
{
    GPUShader* pShader = new GPUShader();
    switch ( type ) {
    case eShaderType::ST_Pixel:
    {
        IDirect3DPixelShader9* pShaderInstance = nullptr;
        pDevice->CreatePixelShader( ( DWORD* )pBytecode, &pShaderInstance );
        pShader->pPixelShader = pShaderInstance;
        break;
    }
    case eShaderType::ST_Vertex:
    {
        IDirect3DVertexShader9* pShaderInstance = nullptr;
        pDevice->CreateVertexShader( ( DWORD* )pBytecode, &pShaderInstance );
        pShader->pVertexShader = pShaderInstance;
        break;
    }
    default:
        OTDU_LOG_ERROR( "Shader type is not supported!\n" );
        OTDU_ASSERT_FATAL( false );
        break;
    }

    return pShader;
}

static IDirect3DVolumeTexture9* D3D9CreateTexture3D( IDirect3DDevice9* pDevice, Texture* param_1 )
{
    IDirect3DVolumeTexture9* pTexture = nullptr;
    HRESULT HVar6 = pDevice->CreateVolumeTexture( param_1->Width, param_1->Height, param_1->Depth,
                                  param_1->NumMipmap, 0, kViewFormatPixelFormatLUT[param_1->Format],
                                  D3DPOOL_MANAGED, &pTexture, nullptr );
    OTDU_ASSERT( HVar6 == S_OK );
    param_1->bUploaded = false;
    return pTexture;
}

static IDirect3DCubeTexture9* D3D9CreateTextureCube( IDirect3DDevice9* pDevice, Texture* param_1 )
{
    IDirect3DCubeTexture9* pTexture = nullptr;
    HRESULT HVar6 = pDevice->CreateCubeTexture( param_1->Width, 
                                                param_1->NumMipmap, 0, kViewFormatPixelFormatLUT[param_1->Format], 
                                                D3DPOOL_MANAGED, &pTexture, nullptr );

    OTDU_ASSERT( HVar6 == S_OK );
    param_1->bUploaded = false;
    return pTexture;
}

static IDirect3DTexture9* D3D9CreateTexture2D( IDirect3DDevice9* pDevice, Texture* param_1 )
{
    //uint uVar1;
    //IDirect3DTexture9* pIVar2;
    //int iVar3;
    //edRenderBitmap* peVar4;
    //int* piVar5;
    //HRESULT HVar6;
    //uint Width;
    //bool bVar7;
    //IDirect3DTexture9* local_24;
    //uint NumLevels;
    //uint Height;
    //uint local_18;
    //uint local_14;
    //uint local_10;
    //DWORD local_c;
    //edRenderBitmap* ViewFormat;
    //D3DPOOL local_4;

    //peVar4 = param_1;
    //Height = ( uint )param_1->Height;
    //NumLevels = ( uint )param_1->NumMipmap;
    //ViewFormat = ( edRenderBitmap* )kFormatLUT[param_1->Format].Format;
    //uVar1 = param_1->Flags;
    //Width = ( uint )param_1->Width;

    DWORD Usage = 0;
    D3DPOOL Pool = D3DPOOL_MANAGED;
    IDirect3DTexture9* local_24 = nullptr;
    Pool = D3DPOOL_MANAGED;
    if ( ( param_1->Flags & 0x1000 ) != 0 ) {
        Pool = D3DPOOL_SYSTEMMEM;
    }
    if ( ( param_1->Flags & 0x400 ) != 0 ) {
        Pool = D3DPOOL_MANAGED;
    }
    bool bVar7 = ( param_1->Flags & 0x200 ) != 0;
    if ( bVar7 ) {
        Pool = D3DPOOL_DEFAULT;
    }
    Usage = ( DWORD )bVar7;
    if ( ( param_1->Flags & 0x100 ) != 0 ) {
        Usage = 0x200;
        Pool = D3DPOOL_DEFAULT;
    }
    if ( ( param_1->Flags & 0x4000 ) != 0 ) {
        Usage = Usage | 0x400;
    }
 
    //if ( ( Pool == D3DPOOL_MANAGED ) &&
    //     ( piVar5 = FUN_005f2c30( 0xfaf540, ( int* )&local_18 ), piVar5 != ( int* )0x0 ) ) {
    //    pIVar2 = ( IDirect3DTexture9* )piVar5[6];
    //    param_1->bUploaded = '\0';
    //    return pIVar2;
    //}

    HRESULT HVar6 = pDevice->CreateTexture( param_1->Width, param_1->Height, param_1->NumMipmap, Usage, kViewFormatPixelFormatLUT[param_1->Format], Pool,
          &local_24, ( HANDLE* )0x0 );
    //if ( ( -1 < HVar6 ) && ( iVar3 = DAT_00fc5544, Pool == D3DPOOL_MANAGED ) ) {
    //    for ( ; iVar3 != 0; iVar3 = *( int* )( iVar3 + 4 ) ) { }
    //    param_1 = ( edRenderBitmap* )local_24;
    //    FUN_005f2bc0( 0xfaf540, &local_18, ( IDirect3DTexture9* )&param_1 );
    //}
    param_1->bUploaded = '\0';
    return local_24;
}

GPUTexture* RenderDevice::createTexture( Texture* pTexture )
{
    //IDirect3DTexture9* pIVar2;
    //IDirect3DCubeTexture9* pIVar3;

    uint16_t uVar1 = pTexture->Width;
    if ( pTexture->Width < pTexture->Height ) {
        uVar1 = pTexture->Height;
    }
    uint8_t iVar4 = 0;
    for ( ; uVar1 != 0; uVar1 = uVar1 >> 1 ) {
        iVar4 = iVar4 + 1;
    }
    if ( iVar4 < pTexture->NumMipmap ) {
        pTexture->NumMipmap = iVar4;
    }

    OTDU_UNIMPLEMENTED;
    return nullptr;
    //if ( ( pTexture->Flags >> 0xb & 1 ) == 0 ) {
    //    if ( ( pTexture->Flags >> 6 & 1 ) == 0 ) {
    //        pIVar2 = CreateTexture2DD3D9( param_1 );
    //        pTexture->pTexture = pIVar2;
    //        return pTexture->pTexture;
    //    }
    //    pIVar3 = CreateTextureCubeD3D9( param_1 );
    //    pTexture->pTexture = pIVar3;
    //    return pTexture->pTexture;
    //}
    //pIVar2 = CreateTexture3DD3D9( param_1 );
    //pTexture->pTexture = pIVar2;
    //return pTexture->pTexture;
}

GPUBuffer* RenderDevice::createBuffer( const struct GPUBufferDesc* desc )
{
    OTDU_UNIMPLEMENTED;
    return nullptr;
}

void* RenderDevice::lockBuffer(GPUBuffer *pBuffer, const uint32_t offset, const uint32_t size)
{
    OTDU_UNIMPLEMENTED;
    return nullptr;
}

void RenderDevice::unlockBuffer(GPUBuffer * pBuffer)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::generateMipchain(GPUTexture *pTexture)
{
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::destroyShader( GPUShader* pShader )
{
    // Both inherits from IUnknown so it should be fine...
    IUnknown* pShaderGeneric = pShader->pVertexShader;
    pShaderGeneric->Release();
}

void RenderDevice::destroyTexture(GPUTexture *pTexture)
{
    OTDU_D3D9_SAFE_RELEASE( pTexture->pImpl );
}

void RenderDevice::destroyBuffer(GPUBuffer *pBuffer)
{
    OTDU_D3D9_SAFE_RELEASE( pBuffer->pVertexBuffer );
}

void RenderDevice::setGammaRamp( float param_1 )
{
    // NOTE: Function has been modified (since decompilation is using MSVC _CIpow())
    // As a result, stack values are now explicitly passed as parameters
    D3DGAMMARAMP local_600;
    WORD WVar1 = static_cast<WORD>( param_1 * 65535.0f );
    int32_t iVar2 = 0;
    do {
        //WVar1 = _CIpow();
        local_600.red[iVar2] = WVar1;
        local_600.green[iVar2] = WVar1;
        local_600.blue[iVar2] = WVar1;
        iVar2 = iVar2 + 1;
    } while ( iVar2 < 0x100 );

    pDevice->SetGammaRamp( 0, 0, &local_600 );
}

bool RenderDevice::retrieveAdapterInfos( const uint32_t adapterIndex, GPUAdapterDesc* pOutDesc )
{
    return RetrieveAdapterInfos( pIDirect3D9, adapterIndex, pOutDesc );
}

void RenderDevice::resize( uint32_t width, uint32_t height )
{
    displayInfos.BackbufferWidth = width;
    displayInfos.BackbufferHeight = height;
}

int32_t RenderDevice::beginFrame()
{
    if ( pDevice == ( IDirect3DDevice9* )0x0 ) {
        return 2;
    }
    HRESULT HVar2 = pDevice->TestCooperativeLevel();
    if ( -1 < HVar2 ) {
LAB_00514375:
        pBoundVertexLayout = nullptr;
        if ( !bSceneEncodingInProgress ) {
            pDevice->BeginScene();
            bSceneEncodingInProgress = true;
        }
        return 0;
    }
    if ( HVar2 == -0x7789f798 ) {
        return 1;
    }
    if ( HVar2 == -0x7789f797 ) {
        OTDU_LOG_WARN( "[edRender]: Reset device()\n" );
        freeDeviceResources();
        D3DPRESENT_PARAMETERS params = { 0 };
        displayInfos.fillPresentInfos( pIDirect3D9, activeWindow, &params );

        HVar2 = pDevice->Reset( &presentParameters );
        if ( HVar2 < 0 ) {
            return 1;
        }

        bool bVar1 = allocateDeviceResources();
        if ( bVar1 ) {
            if ( postPresentCallback != nullptr ) {
                postPresentCallback();
            }
            goto LAB_00514375;
        }
    }
    return 2;
}

void RenderDevice::bindTexture( Texture* pTexture, uint32_t index )
{
    if ( pTexture == nullptr ) {
        pDevice->SetTexture( index, nullptr );
        pBoundTextures[index] = nullptr;
        return;
    }

    if ( pTexture->pTexture == nullptr ) {
        OTDU_LOG_WARN( "Texture instance not created yet (might hitch)\n" );

    }
    
    OTDU_UNIMPLEMENTED;
}

void RenderDevice::uploadMipmap( Texture* pTexture, uint32_t mipIndex )
{
    OTDU_UNIMPLEMENTED;
    /*HRESULT HVar1;
    uint uVar2;
    undefined4* puVar3;
    undefined4* puVar4;
    int unaff_retaddr;
    D3DLOCKED_RECT* locked_rect;
    undefined4* puVar5;
    UINT UStack_c;
    D3DLOCKED_RECT local_8;

    puVar5 = ( undefined4* )0x0;
    locked_rect = &local_8;
    HVar1 = ( *param_1->lpVtbl->LockRect )( param_1, param_2, locked_rect, ( RECT* )0x0, 0 );
    if ( -1 < HVar1 ) {
        if ( locked_rect == ( D3DLOCKED_RECT* )local_8.pBits ) {
            for ( uVar2 = ( uint )( ( int )local_8.pBits * unaff_retaddr ) >> 2; uVar2 != 0; uVar2 = uVar2 - 1 ) {
                *puVar5 = *( undefined4* )local_8.Pitch;
                local_8.Pitch = ( INT )( local_8.Pitch + 4 );
                puVar5 = puVar5 + 1;
            }
            for ( uVar2 = ( int )local_8.pBits * unaff_retaddr & 3; uVar2 != 0; uVar2 = uVar2 - 1 ) {
                *( undefined1* )puVar5 = *( undefined1* )local_8.Pitch;
                local_8.Pitch = ( INT )( local_8.Pitch + 1 );
                puVar5 = ( undefined4* )( ( int )puVar5 + 1 );
            }
            ( *param_1->lpVtbl->UnlockRect )( param_1, UStack_c );
            return;
        }
        if ( unaff_retaddr != 0 ) {
            do {
                puVar3 = ( undefined4* )local_8.Pitch;
                puVar4 = puVar5;
                for ( uVar2 = ( uint )local_8.pBits >> 2; uVar2 != 0; uVar2 = uVar2 - 1 ) {
                    *puVar4 = *puVar3;
                    puVar3 = puVar3 + 1;
                    puVar4 = puVar4 + 1;
                }
                for ( uVar2 = ( uint )local_8.pBits & 3; uVar2 != 0; uVar2 = uVar2 - 1 ) {
                    *( undefined1* )puVar4 = *( undefined1* )puVar3;
                    puVar3 = ( undefined4* )( ( int )puVar3 + 1 );
                    puVar4 = ( undefined4* )( ( int )puVar4 + 1 );
                }
                puVar5 = ( undefined4* )( ( int )puVar5 + ( int )locked_rect );
                local_8.Pitch = local_8.Pitch + ( int )local_8.pBits;
                unaff_retaddr = unaff_retaddr + -1;
                param_1 = param_3;
            } while ( unaff_retaddr != 0 );
        }
        ( *param_1->lpVtbl->UnlockRect )( param_1, UStack_c );
    }
    return;*/
}

bool RenderDevice::createInstance()
{
    pIDirect3D9 = Direct3DCreate9( 0x20 );
    OTDU_ASSERT_FATAL( pIDirect3D9 != nullptr );

    return true;
}

bool RenderDevice::createDevice( GPUDisplayInfos* pDisplayInfos, D3DPRESENT_PARAMETERS* pPresentParams )
{
    if ( pIDirect3D9 != nullptr ) {
        bool bVar1;

        HRESULT HVar3 = pIDirect3D9->CreateDevice( 
            pDisplayInfos->AdapterIndex, 
            displayInfos.DeviceType, 
            activeWindow, 
            pDisplayInfos->BehaviorFlags,
            &presentParameters, 
            &pDevice
        );

        if ( -1 < HVar3 ) {
            if ( displayInfos.bWindowed ) {
                IDirect3DSurface9* pBackBuffer = NULL;
                D3DSURFACE_DESC backBufferDesc;
                if ( SUCCEEDED( pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer ) ) ) {
                    if ( SUCCEEDED( pBackBuffer->GetDesc( &backBufferDesc ) ) ) {
                        displayInfos.BackbufferFormat = backBufferDesc.Format;
                        pBackBuffer->Release();
                    }
                }
            }
#if 0
            // TODO: For some reason they do this twice???
            // Are they trying to test device caps/sys support?
            // Or is it a silly copy/paste mistake?
            pDevice->Reset( &presentParameters );
            pDevice->Release();
            HVar3 = pIDirect3D9->CreateDevice(
                pDisplayInfos->AdapterIndex,
                displayInfos.DeviceType,
                activeWindow,
                pDisplayInfos->BehaviorFlags,
                &presentParameters,
                &pDevice
            );
#endif
            if ( -1 < HVar3 ) {
                pDevice->Reset( &presentParameters );
#ifndef OTDU_DEVBUILD
                if ( !pDisplayInfos->bAllowGPUCapture ) {
                    D3DPERF_SetOptions( 1 );
                }
#endif
                bVar1 = queryDeviceCaps();
                if ( bVar1 ) {
                    bVar1 = initializeShaderCache();
                    // FUN_005f30e0 // TODO: Seems to be used for text draw/debug. 
                    // Probably useless since this is super Win/Dx9 specific and better alternatives exist nowadays
                    if ( bVar1 ) {
                        bVar1 = allocateDeviceResources();
                        return bVar1;
                    }
                }
                return bVar1;
            }
        }
    }

    return pDevice != nullptr;
}

bool RenderDevice::queryDeviceCaps()
{
    char bVar1 = 0;
    char cVar4 = 0;
    uint32_t uVar5 = 0;
    uint32_t uVar6 = 0;
    int32_t iVar7 = 0;
    uint8_t* puVar9 = nullptr;
    D3DFORMAT DVar10 = D3DFORMAT::D3DFMT_UNKNOWN;

    uint32_t pixelShaderSupportTier = 0;
    HRESULT HVar2 = pDevice->GetDeviceCaps( &deviceCaps );
    if ( HVar2 < 0 ) {
        return false;
    }

    if ( deviceCaps.VertexShaderVersion == 0xfffe0101 ) {
        gDeviceTier = 0;
    } else {
        cVar4 = ( char )( deviceCaps.VertexShaderVersion >> 8 );
        if ( cVar4 == '\x02' ) {
            if ( ( ( ( byte )deviceCaps.VS20Caps.Caps & 1 ) == 0 ) ||
                 ( deviceCaps.VS20Caps.DynamicFlowControlDepth != 1 ) ) {
                gDeviceTier = 1;
            } else {
                gDeviceTier = 2;
            }
        } else {
            if ( cVar4 != '\x03' ) {
                return 0;
            }
            gDeviceTier = 3;
        }
    }
    if ( deviceCaps.PixelShaderVersion == 0xffff0101 ) {
        pixelShaderSupportTier = 0;
    } else if ( deviceCaps.PixelShaderVersion == 0xffff0102 ) {
        pixelShaderSupportTier = 1;
    } else if ( deviceCaps.PixelShaderVersion == 0xffff0103 ) {
        pixelShaderSupportTier = 2;
    } else if ( deviceCaps.PixelShaderVersion == 0xffff0104 ) {
        pixelShaderSupportTier = 3;
    } else {
        cVar4 = ( char )( deviceCaps.PixelShaderVersion >> 8 );
        if ( cVar4 == '\x02' ) {
            if ( deviceCaps.PS20Caps.NumInstructionSlots == 0x200 ) {
                if ( ( ( byte )deviceCaps.PS20Caps.Caps & 1 ) == 0 ) {
                    pixelShaderSupportTier = ( ( byte )~( byte )deviceCaps.PS20Caps.Caps & 1 | 2 ) * 2;
                } else {
                    pixelShaderSupportTier = 5;
                }
            } else {
                pixelShaderSupportTier = 4;
            }
        } else {
            if ( cVar4 != '\x03' ) {
                return 0;
            }
            pixelShaderSupportTier = 7;
        }
    }
    gDeviceIndividualCaps[1][0] = '\0';
    gDeviceIndividualCaps[1][1] = '\0';
    gDeviceIndividualCaps[1][2] = '\0';
    gDeviceIndividualCaps[1][3] = '\0';
    gDeviceIndividualCaps[0][0] = '\0';
    gDeviceIndividualCaps[0][1] = '\0';
    gDeviceIndividualCaps[0][2] = '\0';
    gDeviceIndividualCaps[0][3] = '\0';
    gDeviceIndividualCaps[2][0] = '\0';
    gDeviceIndividualCaps[2][1] = '\0';
    gDeviceIndividualCaps[2][2] = '\0';
    gDeviceIndividualCaps[2][3] = '\0';
    uVar5 = gDeviceTier + 1;
    uVar6 = uVar5 >> 2;
    puVar9 = (uint8_t*)&gDeviceIndividualCaps[0];
    gPixelShaderSupport = pixelShaderSupportTier;
    for ( ; uVar6 != 0; uVar6 = uVar6 - 1 ) {
        // 00512c57 f3 ab           STOSD.REP  ES:EDI=>gDeviceIndividualCaps
        puVar9[0] = 0x1;
        puVar9[1] = 0x1;
        puVar9[2] = 0x1;
        puVar9[3] = 0x1;
        puVar9 = puVar9 + 4;
    }
    for ( uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1 ) {
        *puVar9 = '\x01';
        puVar9 = puVar9 + 1;
    }
    uint8_t* pauVar8 = ( uint8_t* )gDeviceIndividualCaps;
    for ( uVar6 = (pixelShaderSupportTier + 1) >> 2; bVar1 = ( byte )deviceCaps.PS20Caps.Caps,
          pauVar8 = ( pauVar8 + 4 ), uVar6 != 0; uVar6 = uVar6 - 1 ) {
        pauVar8[0] = 0x1;
        pauVar8[1] = 0x1;
        pauVar8[2] = 0x1;
        pauVar8[3] = 0x1;
    }
    for ( uVar6 = pixelShaderSupportTier + 1 & 3; uVar6 != 0; uVar6 = uVar6 - 1 ) {
        pauVar8[0] = '\x01';
        pauVar8++;
    }
    if ( pixelShaderSupportTier != 6 ) {
        if ( pixelShaderSupportTier != 7 ) goto LAB_00512cb1;
        if ( ( bVar1 & 1 ) != 0 ) {
            gDeviceIndividualCaps[2][2] = '\0';
            goto LAB_00512cb1;
        }
        if ( ( ~bVar1 & 1 ) == 0 ) goto LAB_00512cb1;
    }
    gDeviceIndividualCaps[2][1] = '\0';
LAB_00512cb1:
    memset( gTextureFormatCapsLUT, 0, sizeof( uint32_t )* eViewFormat::VF_Count );
    memset( gRenderTargetFormatCapsLUT, 0, sizeof( uint32_t ) * eViewFormat::VF_Count );

    for ( uint32_t iVar7 = 0; iVar7 < eViewFormat::VF_Count; iVar7++ ) {
        D3DFORMAT CheckFormat = kViewFormatPixelFormatLUT[iVar7];
        if ( CheckFormat != D3DFMT_UNKNOWN ) {
            HVar2 = pIDirect3D9->CheckDeviceFormat( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                    displayInfos.BackbufferFormat, 1, D3DRTYPE_SURFACE, CheckFormat );
            if ( -1 < HVar2 ) {
                gRenderTargetFormatCapsLUT[iVar7] = gRenderTargetFormatCapsLUT[iVar7] | 1;

                HVar2 = pIDirect3D9->CheckDeviceFormat( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                        displayInfos.BackbufferFormat, 0x80001, D3DRTYPE_SURFACE, CheckFormat );
                if ( -1 < HVar2 ) {
                    gRenderTargetFormatCapsLUT[iVar7] = gRenderTargetFormatCapsLUT[iVar7] | 2;
                }

                HVar2 = pIDirect3D9->CheckDeviceMultiSampleType( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                                 CheckFormat, 0, D3DMULTISAMPLE_2_SAMPLES, ( DWORD* )0x0 );
                if ( -1 < HVar2 ) {
                    gRenderTargetFormatCapsLUT[iVar7] = gRenderTargetFormatCapsLUT[iVar7] | 4;
                }

                HVar2 = pIDirect3D9->CheckDeviceMultiSampleType( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                                 CheckFormat, 0, D3DMULTISAMPLE_4_SAMPLES, ( DWORD* )0x0 );
                if ( -1 < HVar2 ) {
                    gRenderTargetFormatCapsLUT[iVar7] = gRenderTargetFormatCapsLUT[iVar7] | 8;
                }
            }

            HVar2 = pIDirect3D9->CheckDeviceFormat( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                    displayInfos.BackbufferFormat, 0, D3DRTYPE_TEXTURE, CheckFormat );
            if ( -1 < HVar2 ) {
                gTextureFormatCapsLUT[iVar7] = gTextureFormatCapsLUT[iVar7] | 1;

                HVar2 = pIDirect3D9->CheckDeviceFormat( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                        displayInfos.BackbufferFormat, 1, D3DRTYPE_TEXTURE, CheckFormat );
                if ( -1 < HVar2 ) {
                    gTextureFormatCapsLUT[iVar7] = gTextureFormatCapsLUT[iVar7] | 2;
                }

                HVar2 = pIDirect3D9->CheckDeviceFormat( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                        displayInfos.BackbufferFormat, 0x20000, D3DRTYPE_TEXTURE, CheckFormat );
                if ( -1 < HVar2 ) {
                    gTextureFormatCapsLUT[iVar7] = gTextureFormatCapsLUT[iVar7] | 8;
                }

                HVar2 = pIDirect3D9->CheckDeviceFormat( displayInfos.AdapterIndex, displayInfos.DeviceType, 
                                                        displayInfos.BackbufferFormat, 0x80001, D3DRTYPE_TEXTURE, CheckFormat );
                if ( -1 < HVar2 ) {
                    gTextureFormatCapsLUT[iVar7] = gTextureFormatCapsLUT[iVar7] | 4;
                }
            }
        }
    }

    return true;
}

static uint32_t FUN_00606120( uint32_t param_1 )
{
    int iVar1;

    iVar1 = 0;
    while ( UINT_ARRAY_ARRAY_00eedac0[iVar1 * 2 + 1] != param_1 ) {
        iVar1 = iVar1 + 1;
        if ( 0x10 < iVar1 ) {
            return 0x11;
        }
    }
    iVar1 = UINT_ARRAY_ARRAY_00eedac0[iVar1 * 2];
    if ( iVar1 == 0x1a ) {
        return 0x11;
    }
    if ( ( iVar1 != 9 ) && ( iVar1 != 0x18 ) ) {
        if ( iVar1 == 0x16 ) {
            return 2;
        }
        return UINT_ARRAY_ARRAY_00eedac0[( ( -( iVar1 != 0x19 ) & 5 ) + 0x15 ) * 2 + 1];
    }
    return 2;
}

BYTE RenderDevice::FUN_00512e90( uint32_t param_1 )
{
    BYTE bVar1;

    bVar1 = 0;
    switch ( param_1 ) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
        return '\x01';
    case 5:
        bVar1 = ( byte )deviceCaps.DeclTypes & 1;
        return bVar1;
    case 8:
        return ( byte )( deviceCaps.DeclTypes >> 1 ) & 1;
    case 9:
        return ( byte )( deviceCaps.DeclTypes >> 2 ) & 1;
    case 10:
        return ( byte )( deviceCaps.DeclTypes >> 3 ) & 1;
    case 0xb:
        return ( byte )( deviceCaps.DeclTypes >> 4 ) & 1;
    case 0xc:
        return ( byte )( deviceCaps.DeclTypes >> 5 ) & 1;
    case 0xd:
        return ( byte )( deviceCaps.DeclTypes >> 6 ) & 1;
    case 0xe:
        return ( byte )( deviceCaps.DeclTypes >> 7 ) & 1;
    case 0xf:
        return ( byte )( deviceCaps.DeclTypes >> 8 ) & 1;
    case 0x10:
        bVar1 = ( byte )( deviceCaps.DeclTypes >> 9 ) & 1;
    }
    return bVar1;
}

bool RenderDevice::FUN_00606240( D3DVERTEXELEMENT9* param_1 )
{
    WORD WVar1;
    char cVar2;
    BYTE BVar3;
    byte bVar4;

    cVar2 = '\0';
    WVar1 = param_1->Stream;
    do {
        if ( WVar1 == 0xff ) {
            return ( bool )( cVar2 + '\x01' );
        }
        BVar3 = FUN_00512e90( param_1->Type );
        if ( BVar3 == '\0' ) {
            cVar2 = '\x01';
            do {
                bVar4 = FUN_00606120( param_1->Type );
                param_1->Type = bVar4;
                if ( bVar4 == 0x11 ) {
                    return false;
                }
                BVar3 = FUN_00512e90( bVar4 );
            } while ( BVar3 == '\0' );
        }
        param_1 = param_1 + 1;
        WVar1 = param_1->Stream;
    } while ( true );
}

bool RenderDevice::createDebugTextResources()
{
    HRESULT HVar2 = pDevice->CreateDepthStencilSurface( displayInfos.BackbufferWidth,
                                                        displayInfos.BackbufferHeight,
                                                        D3DFMT_D24S8,
                                                        D3DMULTISAMPLE_NONE,
                                                        0,
                                                        FALSE,
                                                        &frameResources.pDepthStencilRT,
                                                        nullptr
    );
    if ( HVar2 < 0 ) {
        return false;
    }

    // FUN_006048f0 (inlined)
    HVar2 = pDevice->CreateVertexBuffer( 32, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &frameResources.pTextVB,
                                         ( HANDLE* )0x0 );

    float* pfStack_2c = nullptr;
    D3DVERTEXELEMENT9 DStack_28[2] = { {0}, {0} };
    D3DVERTEXELEMENT9 DStack_18[2] = { {0}, {0} };

    if ( -1 < HVar2 ) {
        HVar2 = frameResources.pTextVB->Lock(0, 0, (void**)&pfStack_2c, 0);
        if ( -1 < HVar2 ) {
            *pfStack_2c = -1.0;
            pfStack_2c[1] = -1.0;
            pfStack_2c[2] = -1.0;
            pfStack_2c[3] = 1.0;
            pfStack_2c[4] = 1.0;
            pfStack_2c[5] = -1.0;
            pfStack_2c[6] = 1.0;
            pfStack_2c[7] = 1.0;
            HVar2 = frameResources.pTextVB->Unlock();
            if ( -1 < HVar2 ) {
                DStack_28[0].Stream = 0;
                DStack_28[0].Offset = 0;
                DStack_28[0].Type = '\x01';
                DStack_28[0].Method = '\0';
                DStack_28[0].Usage = '\0';
                DStack_28[0].UsageIndex = '\0';
                DStack_28[1].Stream = 0xff;
                DStack_28[1].Offset = 0;
                DStack_28[1].Type = '\x11';
                DStack_28[1].Method = '\0';
                DStack_28[1].Usage = '\0';
                DStack_28[1].UsageIndex = '\0';
                HVar2 = pDevice->CreateVertexDeclaration( DStack_28, &frameResources.pTextVertexLayout[0] );
                if ( -1 < HVar2 ) {
                    if ( frameResources.pText2DMBuffer == nullptr ) {
                        uint32_t uVar3 = Render2DM::CalcSize( 2, 0x20 );
                        uVar3 = uVar3 + 0xf & 0xfffffff0;
                        frameResources.pText2DMBuffer = TestDrive::Alloc( uVar3 );
                    }
                    frameResources.pMaterial = frameResources.text2DM.create( frameResources.pText2DMBuffer, 0xfffffffffffff000, 0, 2, 0x20, 0 );
                    
                    MaterialShaderParameterArray* peVar5 = ( MaterialShaderParameterArray* )frameResources.pMaterial->getParameterByIndex( 0 );
                    if ( peVar5->NumParameters != 0 ) {
                        MaterialShaderParameter* peVar6 = ( MaterialShaderParameter* )peVar5->getParameterDataByIndex( 0 );
                        peVar6->Size = 0x10;
                        peVar6->bBindToVS = 1;
                        peVar6->Format = 2;
                        peVar6->Type = 0x7e;

                        peVar6 = ( MaterialShaderParameter* )peVar5->getParameterDataByIndex( 1 );
                        peVar6->bBindToVS = 1;
                        peVar6->Size = 0x10;
                        peVar6->Format = 2;
                        peVar6->Type = 0x96;
                    }

                    frameResources.text2DM.initialize( frameResources.pText2DMBuffer );
                    frameResources.pVertexShader = frameResources.pMaterial->pVertexShaders[0];
                    frameResources.pPixelShader = frameResources.pMaterial->pVertexShaders[1];

                    void* puVar7 = nullptr;

                    if ( peVar5->NumParameters != 0 ) {
                        uint32_t uVar3 = peVar5->NumParameters;
                        if ( uVar3 - 1 < uVar3 ) {
                            // NOTE Had to hand translate this (as Ghidra output was non sense)
                            //00604AA6 | 8D41 05 | lea eax, dword ptr ds : [ecx + 5] |
                            //00604AA9 | C1E0 04 | shl eax, 4 |
                            //00604AAC | 03C7 | add eax, edi |
                            //00604AAE | 83C0 10 | add eax, 10 |
                            uint32_t eax = ( uVar3 - 1 ) + 5;
                            eax <<= 4;
                            uint8_t* edi = ( uint8_t* )peVar5;
                            edi += eax;
                            edi += 0x10;

                            puVar7 = edi;
                        } else {
                            puVar7 = (void*)0x10; // TODO: I assume this could be int based on param type?
                        }
                    }

                    frameResources.pNextParameterAvailable = puVar7;
                    frameResources.bindMaterial( nullptr );
                    DStack_18[0].Stream = 0;
                    DStack_18[0].Offset = 0;
                    DStack_18[0].Type = '\x01';
                    DStack_18[0].Method = '\0';
                    DStack_18[0].Usage = '\0';
                    DStack_18[0].UsageIndex = '\0';
                    DStack_18[1].Stream = 0;
                    DStack_18[1].Offset = 8;
                    DStack_18[1].Type = '\x01';
                    DStack_18[1].Method = '\0';
                    DStack_18[1].Usage = '\x05';
                    DStack_18[1].UsageIndex = '\0';
                    HVar2 = pDevice->CreateVertexDeclaration( DStack_18, &frameResources.pTextVertexLayout[1] );

                    frameResources.pTextVS = gShaderRegister.getShader( this, eShaderType::ST_Vertex, 0xfffffffffffff000 );
                    frameResources.pTextPS = gShaderRegister.getShader( this, eShaderType::ST_Pixel, 0x0ffffffffffff000 );

                    return frameResources.pTextVS != nullptr && frameResources.pTextPS != nullptr;
                }
            }
        }
    }
    return false;
}

bool RenderDevice::initializeShaderCache()
{
    resetShaderCache();
    /* DAT_00fadbe0 = 0;
       DAT_00fadbec = 0;*/
    gShaderRegister.registerMasterTable();
    gPostFXStack.releaseResources();
    return true;
}

bool RenderDevice::resetShaderCache()
{
    // FUN_005163d0
    unbindShaders();
    gShaderRegister.releaseCachedShaderInstances();
    return true;
}

bool RenderDevice::resetCachedStates()
{
    // FUN_005f15b0
    unbindRasterStates();
    unbindVertexBuffers();
    
    pDevice->SetIndices( nullptr );
    pBoundIndexBuffer = nullptr;

    unbindSamplers();
    unbindTextures();
    unbindShaders();
    unbindRenderTargets();
    return true;
}

void RenderDevice::unbindVertexBuffers()
{
    UINT StreamNumber;

    StreamNumber = 0;
    do {
        boundVertexStreams[StreamNumber].pBuffer = nullptr;
        boundVertexStreams[StreamNumber].Offset = 0;
        boundVertexStreams[StreamNumber].Stride = 0;

        boundVertexStreamFrequency[StreamNumber] = 1;

        pDevice->SetStreamSource( StreamNumber, nullptr, 0, 0 );
        pDevice->SetStreamSourceFreq( StreamNumber, 1 );

        StreamNumber++;
    } while ( StreamNumber < 0x10 );

    pBoundVertexLayout = nullptr;
}

void RenderDevice::unbindRasterStates()
{
    if ( PTR_00faf3e0 == 0x0 ) { // TODO: Figure out this
        for ( auto& rasterState : rasterStates ) {
            pDevice->SetRenderState( rasterState.first, rasterState.second );
            boundRasterStates[rasterState.first] = rasterState.second;
        }
        return;
    }

    // Doesn't seem to be in use (PTR_00faf3e0 is always null at runtime)
    OTDU_UNIMPLEMENTED;

    memset( boundRenderStates, 0xff, kNumCachedRasterState * sizeof( DWORD ) );

    for ( auto& rasterState : rasterStates ) {
        boundRasterStates[rasterState.first] = rasterState.second;
    }

    // ( **( code** )( pRVar4->StateType + D3DRS_DESTBLEND ) )( pRVar4 ); // TODO: Figure out this
}

void RenderDevice::unbindShaders()
{
    HRESULT HVar1;

    if ( pBoundVertexShader != nullptr ) {
        HVar1 = pDevice->SetVertexShader( nullptr );
        // TODO: For some weird reason this is explicitly duplicated. 
        // Were they trying to hide a threading issue? Or WA some driver bug?
#if 0
        if ( HVar1 != 0 ) {
            ( *( pD3DDevice )->lpVtbl->SetVertexShader )( pD3DDevice, ( IDirect3DVertexShader9* )0x0 );
        }
#endif
        pBoundVertexShader = 0;
    }


    if ( pBoundVertexShader != nullptr ) {
        HVar1 = pDevice->SetPixelShader( nullptr );
#if 0
        if ( HVar1 != 0 ) {
            ( *( pD3DDevice )->lpVtbl->SetPixelShader )( pD3DDevice, ( IDirect3DPixelShader9* )0x0 );
        }
#endif
        pBoundVertexShader = 0;
    }
}

void RenderDevice::unbindRenderTargets()
{
    HRESULT HVar1;
    DWORD RenderTargetIndex;

    RenderTargetIndex = 1;
    if ( 1 < deviceCaps.NumSimultaneousRTs ) {
        do {
            HVar1 = pDevice->SetRenderTarget( RenderTargetIndex, nullptr );
            if ( -1 < HVar1 ) {
                pBoundRenderTargets[RenderTargetIndex] = nullptr;
            }
            RenderTargetIndex++;
        } while ( RenderTargetIndex < deviceCaps.NumSimultaneousRTs );
    }
}

void RenderDevice::unbindTextures()
{
    DWORD Stage;

    Stage = 0;
    do {
        pDevice->SetTexture( Stage, nullptr );
        pBoundTextures[Stage] = nullptr;
        Stage++;
    } while ( Stage < 0x10 );
}

void RenderDevice::unbindSamplers()
{
    for ( uint32_t i = 0; i < kMaxSamplerCount; i++ ) {
        GPUSampler& sampler = boundSamplers[i];
        sampler.AddressModes[0] = 1;
        sampler.AddressModes[1] = 1;
        sampler.AddressModes[2] = 1;
        sampler.MinFilter = 2;
        sampler.MagFilter = 2;
        sampler.MipFilter = 0;

        pDevice->SetSamplerState( i, D3DSAMP_ADDRESSU, 1 );
        pDevice->SetSamplerState( i, D3DSAMP_ADDRESSV, 1 );
        pDevice->SetSamplerState( i, D3DSAMP_ADDRESSW, 1 );
        pDevice->SetSamplerState( i, D3DSAMP_MINFILTER, 2 );
        pDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, 2 );
        pDevice->SetSamplerState( i, D3DSAMP_MIPFILTER, 0 );
    }
}

bool RenderDevice::resetPointSize()
{
    if ( ( ( uint8_t_00fadaa0 & 2 ) != 0 ) && ( boundRasterStates[D3DRS_POINTSIZE] != 0x54534e49 ) ) {
        pDevice->SetRenderState( D3DRS_POINTSIZE, 0x54534e49 );
        boundRasterStates[D3DRS_POINTSIZE] = 0x54534e49;
    }
    return true;
}

static bool BOOL_00faccc0 = false;

bool RenderDevice::flushPendingResourceCreation()
{
    if ( BOOL_00faccc0 ) 
    {
        // TODO: BOOL_00faccc0 is always false? If not, implement those funcs below
        OTDU_UNIMPLEMENTED;
        /* FUN_00512110( 0xfaccc8, &LAB_00512260 );
         FUN_00512110( 0xfad0d0, &LAB_005122f0 );
         FUN_00512110( 0xfad4d8, FUN_00512340 );*/
        BOOL_00faccc0 = false;
    }
    return true;
}

bool RenderDevice::allocateDeviceResources()
{
    bool bVar1 = resetCachedStates();
    if ( bVar1 ) {
        bVar1 = resetPointSize();
        if ( bVar1 ) {
            bVar1 = createDebugTextResources();
            if ( bVar1 ) {
                bVar1 = FUN_005f33d0();
                if ( bVar1 ) {
                    bVar1 = flushPendingResourceCreation();
                    if ( bVar1 ) {
                        bVar1 = createLightBuffer();
                        if ( bVar1 ) {
                            bVar1 = allocateHmapVertexBuffer();
                            if ( bVar1 ) {
                                bVar1 = vertexStreamCache.create( pDevice );
                                if ( bVar1 ) {
                                    bVar1 = FUN_005fd8f0();
                                    return bVar1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool RenderDevice::freeDeviceResources()
{
    FUN_005f36e0();
    releaseFrameResources();
    gPostFXStack.releaseResources();
    FUN_00512350();
    releaseLightBuffer();
    vertexStreamCache.release();
    releaseHmapVertexBuffer();
    FUN_005fd350();
    return true;
}

bool RenderDevice::allocateHmapVertexBuffer()
{
    // FUN_00600f40
    HRESULT HVar1;
    int iVar2;
    int iVar3;
    float* pfVar4;
    void* pvStack_4;

    if ( pHmapVertexDecl == ( IDirect3DVertexDeclaration9* )0x0 ) {
        static constexpr D3DVERTEXELEMENT9 kHMapVertexElements = { 0, 0, 1, 0, 5, 0 }; // DAT_00f4766c

        HVar1 = pDevice->CreateVertexDeclaration( &kHMapVertexElements, &pHmapVertexDecl );
        if ( HVar1 != 0 ) {
            return false;
        }
    }

    if ( pHmapVertexBuffer == ( IDirect3DVertexBuffer9* )0x0 ) {
        pDevice->CreateVertexBuffer( 528392, 8, 0, D3DPOOL_DEFAULT, &pHmapVertexBuffer, ( HANDLE* )0x0 );
    }
    pfVar4 = ( float* )0x0;
    pHmapVertexBuffer->Lock(0, 0, &pvStack_4, 0 );
    if ( pfVar4 != ( float* )0x0 ) {
        iVar3 = 0;
        do {
            iVar2 = 0;
            do {
                *pfVar4 = ( float )iVar2;
                pfVar4[1] = ( float )iVar3;
                pfVar4 = pfVar4 + 2;
                iVar2 = iVar2 + 1;
            } while ( iVar2 < 0x101 );
            iVar3 = iVar3 + 1;
        } while ( iVar3 < 0x101 );
        pHmapVertexBuffer->Unlock();
    }
    return true;
}

void RenderDevice::releaseHmapVertexBuffer()
{
    // FUN_006010e0
    OTDU_D3D9_SAFE_RELEASE( pHmapVertexDecl );
    OTDU_D3D9_SAFE_RELEASE( pHmapVertexBuffer );
}

bool RenderDevice::createLightBuffer()
{
    HRESULT HVar1;

    D3DVERTEXELEMENT9 D3DVERTEXELEMENT9_00f47700 = { 0, 0, 2, 0, 0, 0 };
    D3DVERTEXELEMENT9 D3DVERTEXELEMENT9_00f47738 = { 0, 0, 2, 0, 0, 0 };
    D3DVERTEXELEMENT9 D3DVERTEXELEMENT9_00f47778 = { 0, 0, 2, 0, 0, 0 };
    D3DVERTEXELEMENT9 D3DVERTEXELEMENT9_00f477b8 = { 0, 0, 2, 0, 0, 0 };

    FUN_00606240( &D3DVERTEXELEMENT9_00f47700 );
    FUN_00606240( &D3DVERTEXELEMENT9_00f47738 );
    FUN_00606240( &D3DVERTEXELEMENT9_00f47778 );
    FUN_00606240( &D3DVERTEXELEMENT9_00f477b8 );

    if ( PTR_00fe8a14 == ( IDirect3DVertexDeclaration9* )0x0 ) {
        HVar1 = pDevice->CreateVertexDeclaration( &D3DVERTEXELEMENT9_00f47700, &PTR_00fe8a14 );
        if ( HVar1 == 0 ) {
            HVar1 = pDevice->CreateVertexDeclaration( &D3DVERTEXELEMENT9_00f47738, &PTR_00fe8a18 );
            if ( HVar1 == 0 ) {
                HVar1 = pDevice->CreateVertexDeclaration( &D3DVERTEXELEMENT9_00f47778, &PTR_00fe8a1c );
                if ( HVar1 == 0 ) {
                    HVar1 = pDevice->CreateVertexDeclaration( &D3DVERTEXELEMENT9_00f477b8, &PTR_00fe8a20 );
                    if ( HVar1 == 0 ) goto LAB_006042c9;
                }
            }
        }
        return false;
    }
LAB_006042c9:
    if ( pLightVertexBuffer == ( IDirect3DVertexBuffer9* )0x0 ) {
        pDevice->CreateVertexBuffer( 0x1900, 0x208, 0, D3DPOOL_DEFAULT, &pLightVertexBuffer, ( HANDLE* )0x0 );
    }
    DAT_00fe8a28 = 0;
    return true;
}

void RenderDevice::releaseLightBuffer()
{
    OTDU_D3D9_SAFE_RELEASE( PTR_00fe8a14 );
    OTDU_D3D9_SAFE_RELEASE( PTR_00fe8a18 );
    OTDU_D3D9_SAFE_RELEASE( PTR_00fe8a1c );
    OTDU_D3D9_SAFE_RELEASE( PTR_00fe8a20 );
    OTDU_D3D9_SAFE_RELEASE( pLightVertexBuffer );
    DAT_00fe8a28 = 0;
}

bool RenderDevice::FUN_005f33d0()
{
    uint8_t uVar1;
    uint32_t uVar3;
    int8_t uStack_16d;
    IDirect3D9* pIStack_168;
    IDirect3DSurface9* pIStack_164;
    D3DDISPLAYMODE DStack_160;
    D3DSURFACE_DESC DStack_150;
    D3DCAPS9 DStack_130;
    
    HRESULT HVar2 = pDevice->CreateVertexBuffer( 96000, 0x208, 0, D3DPOOL_DEFAULT, &textRenderer.pVertexBuffer, ( HANDLE* )0x0 );
    if ( HVar2 < 0 ) {
        return false;
    }
    uStack_16d = 1;
    uVar1 = uStack_16d;
    uStack_16d = 1;
    pIStack_168 = ( IDirect3D9* )0x0;
    HVar2 = pDevice->GetDirect3D( &pIStack_168 );
 
    if ( -1 < HVar2 ) {
        pIStack_164 = ( IDirect3DSurface9* )0x0;
        pDevice->GetDeviceCaps( &DStack_130 );
        pDevice->GetDisplayMode( 0, &DStack_160 );
        HVar2 = pDevice->GetRenderTarget( 0, &pIStack_164 );
        uStack_16d = uVar1;
        if ( -1 < HVar2 ) {
            pIStack_164->GetDesc( &DStack_150 );
            uStack_16d = DStack_150.Type;
            D3DRESOURCETYPE type = pIStack_164->GetType();
            if ( type != 0 ) {
                uStack_16d = 0;
            }
            OTDU_D3D9_SAFE_RELEASE( pIStack_164 );
        }
        OTDU_D3D9_SAFE_RELEASE( pIStack_168 );
    }
    uVar3 = 0;
    do {
        pDevice->BeginStateBlock();
        pDevice->SetTexture( 0, textRenderer.pTexture );
        if ( ( textRenderer.BYTE_0x54 & 4 ) == 0 ) {
             pDevice->SetRenderState( D3DRS_ZENABLE, 0 );
        } else {
             pDevice->SetRenderState( D3DRS_ZENABLE, 1 );
        }
        if ( uStack_16d == 0 ) {
             pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 );
        } else {
             pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
             pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
             pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
        }
         pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, 1 );
         pDevice->SetRenderState( D3DRS_ALPHAREF, 8 );
         pDevice->SetRenderState( D3DRS_ALPHAFUNC, 7 );
         pDevice->SetRenderState( D3DRS_FILLMODE, 3 );
         pDevice->SetRenderState( D3DRS_CULLMODE, 3 );
         pDevice->SetRenderState( D3DRS_STENCILENABLE, 0 );
         pDevice->SetRenderState( D3DRS_CLIPPING, 1 );
         pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0 );
         pDevice->SetRenderState( D3DRS_VERTEXBLEND, 0 );
         pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, 0 );
         pDevice->SetRenderState( D3DRS_FOGENABLE, 0 );
         pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0xf );
         pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
         pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
         pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
         pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
         pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
         pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
         pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
         pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
         pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
         pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
         pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, 1 );
         pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, 1 );
         pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, 0 );
        if ( uVar3 == 0 ) {
             pDevice->EndStateBlock( &textRenderer.pStateBlock0 );
        } else {
             pDevice->EndStateBlock( &textRenderer.pStateBlock1 );
        }
        uVar3 = uVar3 + 1;
    } while ( uVar3 < 2 );
    return true;
}

void RenderDevice::releaseFrameResources()
{
    // FUN_00604dc0
    OTDU_D3D9_SAFE_RELEASE( frameResources.pDepthStencilRT );
    frameResources.release();
}

void RenderDevice::FUN_005f36e0()
{
    OTDU_D3D9_SAFE_RELEASE( textRenderer.pVertexBuffer );
    OTDU_D3D9_SAFE_RELEASE( textRenderer.pStateBlock0 );
    OTDU_D3D9_SAFE_RELEASE( textRenderer.pStateBlock1 );
}

void RenderDevice::FUN_00512350()
{
    if ( !BOOL_00faccc0 ) 
    {
        OTDU_UNIMPLEMENTED;
        /*  FUN_00512110( 0xfaccc8, &LAB_00512240 );
          FUN_00512110( 0xfad0d0, &LAB_005122d0 );
          FUN_00512110( 0xfad4d8, &LAB_00512330 );*/
        BOOL_00faccc0 = true;
    }
}

void RenderDevice::FUN_005fd350()
{
    OTDU_UNIMPLEMENTED;
}

bool RenderDevice::FUN_005fd8f0()
{
    OTDU_UNIMPLEMENTED;
    return true;
}

void GPUDisplayInfos::selectAdapter( IDirect3D9* pDirect3D9 )
{
    UINT UVar2 = pDirect3D9->GetAdapterCount();
    OTDU_LOG_DEBUG( "Enumerating adapters (%u found)...\n", UVar2 );

    uint32_t maxNumModeCount = 0;
    for ( UINT adapterIndex = 0; adapterIndex < UVar2; adapterIndex++ ) {
        GPUAdapterDesc adapterDesc;
        RetrieveAdapterInfos( pDirect3D9, adapterIndex, &adapterDesc );

        OTDU_LOG_DEBUG( "\t%u => '%s'\n", adapterIndex, adapterDesc.DeviceDesc.c_str());

        auto formats = getCompatBackbufferFormats( pDirect3D9, adapterIndex, false );

        uint32_t numModeCount = 0;
        for ( D3DFORMAT fmt : formats ) {
            numModeCount += pDirect3D9->GetAdapterModeCount( adapterIndex, fmt );
        }

        OTDU_LOG_DEBUG( "\t\tFound %u compatible modes\n", numModeCount );
        if ( numModeCount > maxNumModeCount ) {
            maxNumModeCount = numModeCount;
            AdapterIndex = adapterIndex;
        }
    }
}

std::vector<D3DFORMAT> GPUDisplayInfos::getCompatBackbufferFormats( IDirect3D9 * pDirect3D9, const uint32_t adapterIndex, const bool bWindowed /*= false */ )
{
    std::vector<D3DFORMAT> compatibleFormats;
    for ( uint32_t uVar2 = 0; uVar2 < 4; uVar2++ ) {
        D3DFORMAT backbufferFormat = kBackbufferFormats[uVar2];
        HRESULT HVar1 = pDirect3D9->CheckDeviceType( adapterIndex, DeviceType, backbufferFormat, backbufferFormat, bWindowed );
        if ( -1 < HVar1 ) {
            compatibleFormats.push_back( backbufferFormat );
        }
    }
    return compatibleFormats;
}

void GPUDisplayInfos::fillPresentInfos( IDirect3D9* param_1, HWND winHandle, D3DPRESENT_PARAMETERS* param_2 )
{
    HRESULT HVar3 = S_OK;
    
    if ( eVSyncMode::VSM_FourVBlank < VSync ) {
        VSync = eVSyncMode::VSM_FourVBlank;
    }

    UINT UVar5 = 0;
    switch ( VSync ) {
    case eVSyncMode::VSM_OneVBlank:
        UVar5 = 1;
        break;
    case eVSyncMode::VSM_TwoVBlank:
        UVar5 = 2;
        break;
    case eVSyncMode::VSM_ThreeVBank:
        UVar5 = 4;
        break;
    case eVSyncMode::VSM_FourVBlank:
        UVar5 = 8;
        break;
    };

    param_2->BackBufferWidth = BackbufferWidth;
    param_2->BackBufferHeight = BackbufferHeight;
    param_2->BackBufferFormat = bWindowed ? D3DFMT_UNKNOWN : BackbufferFormat;
    param_2->BackBufferCount = 1;

    eAntiAliasingMethod eVar1 = AntiAliasingMethod;
    if ( ( ( int32_t )eVar1 < 0 ) || ( 2 < ( int32_t )eVar1 ) ) {
        eVar1 = eAntiAliasingMethod::AAM_Disabled;
    }
    param_2->MultiSampleType = ToD3DMSAA(eVar1);
    param_2->MultiSampleQuality = 0;
    param_2->SwapEffect = D3DSWAPEFFECT_DISCARD;
    param_2->hDeviceWindow = winHandle;
    uint32_t uVar2 = 0;
    param_2->EnableAutoDepthStencil = TRUE;
    param_2->Windowed = bWindowed ? TRUE : FALSE;
    param_2->AutoDepthStencilFormat = D3DFMT_D16;
    param_2->Flags = 0;
    if ( param_2->Windowed == 0 ) {
        uVar2 = RefreshRate;
    } else {
        uVar2 = 0;
    }
    param_2->FullScreen_RefreshRateInHz = uVar2;
    param_2->PresentationInterval = UVar5;
    if ( ( param_2->MultiSampleType != D3DMULTISAMPLE_NONE ) &&
            ( HVar3 = param_1->CheckDeviceMultiSampleType(AdapterIndex, DeviceType, param_2->BackBufferFormat, param_2->Windowed, param_2->MultiSampleType, 0), HVar3 < 0 ) ) {
        AntiAliasingMethod = eAntiAliasingMethod::AAM_Disabled;
        param_2->MultiSampleType = D3DMULTISAMPLE_NONE;
    }
}

void RenderFrameResources::bindMaterial( Material* param_2 )
{
    // FUN_00604790
    if ( pMaterial != nullptr ) {
        if ( param_2 == nullptr ) {
            pMaterial->DepthWrite = '\x02';
            pMaterial->SrcBlend = '\x02';
            pMaterial->FillMode = '\0';
            pMaterial->AlphaRef = '\0';
            pMaterial->StencilEnable = '\0';
            pMaterial->DepthTest = '\x03';
            pMaterial->BlendOP = '\x06';
            pMaterial->DstBlend = '\x01';
            pMaterial->BackfaceCullingMode = '\x01';
            pMaterial->AlphaTest = '\x01';
            return;
        }
        pMaterial->FillMode = param_2->FillMode;
        pMaterial->DepthTest = param_2->DepthTest;
        pMaterial->DepthWrite = param_2->DepthWrite;
        pMaterial->BackfaceCullingMode = param_2->BackfaceCullingMode;
        pMaterial->AlphaTest = param_2->AlphaTest;
        pMaterial->AlphaRef = param_2->AlphaRef;
        pMaterial->BlendOP = param_2->BlendOP;
        pMaterial->SrcBlend = param_2->SrcBlend;
        pMaterial->DstBlend = param_2->DstBlend;
        pMaterial->StencilEnable = param_2->StencilEnable;
        pMaterial->StencilFail = param_2->StencilFail;
        pMaterial->StencilZFail = param_2->StencilZFail;
        pMaterial->StencilPass = param_2->StencilPass;
        pMaterial->StencilFunc = param_2->StencilFunc;
        pMaterial->StencilRef = param_2->StencilRef;
        pMaterial->StencilMask = param_2->StencilMask;
        pMaterial->StencilWriteMask = param_2->StencilWriteMask;
        pMaterial->StencilFailCW = param_2->StencilFailCW;
        pMaterial->StencilZFailCW = param_2->StencilZFailCW;
        pMaterial->StencilPassCW = param_2->StencilPassCW;
        pMaterial->StencilFuncCW = param_2->StencilFuncCW;
        pMaterial->StencilRefCW = param_2->StencilRefCW;
        pMaterial->StencilMaskCW = param_2->StencilMaskCW;
        pMaterial->StenwilWriteMaskCW = param_2->StenwilWriteMaskCW;
    }
}

void RenderFrameResources::release()
{
    // FUN_00604b90
    if ( pText2DMBuffer != nullptr ) {
        pMaterial = nullptr;
        text2DM.destroy();

        OTDU_D3D9_SAFE_RELEASE( pTextVertexLayout[1]);
        OTDU_D3D9_SAFE_RELEASE( pTextVertexLayout[0] );

        pTextPS = nullptr;
        pTextVS = nullptr;

        OTDU_D3D9_SAFE_RELEASE( pTextVB );
    }
}

bool VertexStreamCache::create( IDirect3DDevice9* pDevice )
{
    // FUN_00605af0
    FUN_00604e60( pDevice );
    uint32_t uVar1 = FUN_006059e0();
    HRESULT HVar1 = pDevice->CreateIndexBuffer( uVar1 * 2, 8, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIndexBuffer0, ( HANDLE* )0x0 );
    if ( -1 < HVar1 ) {
        OTDU_UNIMPLEMENTED;
    /*    HVar1 = pIndexBuffer0->Lock( 0, 0, &ppbData, 0 );
        if ( -1 < HVar1 ) {
            usage = 0;
            pCStack_10 = pOutBuffer->pDrawCommands;
            puVar5 = uint16_t_ARRAY_00eede20;
            do {
                uVar2 = FUN_006057e0( *( undefined4* )puVar5, ppbData );
                uVar2 = uVar2 & 0xffff;
                FUN_00604f60( uVar2, usage, ppbData );
                pCStack_10 = pCStack_10 + 5;
                puVar5 = puVar5 + 2;
                usage = usage + uVar2;
                psVar3 = ppbData + uVar2;
                ppbData = psVar3;
            } while ( ( int )puVar5 < 0xeede34 );
            iStack_8 = 0;
            psVar6 = &local_4->pDrawCommands[0x19].MinVertexIndex;
            do {
                uStack_c = 0;
                iVar4 = iStack_8;
                do {
                    if ( ( int )uStack_c < iVar4 ) {
                        ( ( CachedDrawCommand* )( psVar6 + -2 ) )->PrimitiveCount = 0;
                        *psVar6 = 0;
                        psVar6[1] = 0;
                        psVar6[-1] = -1;
                        psVar6[0x62] = 0;
                        psVar6[99] = 0xffff;
                        psVar6[100] = 0;
                        psVar6[0x65] = 0;
                        psVar6[0xc6] = 0;
                        psVar6[199] = 0xffff;
                        psVar6[200] = 0;
                        psVar6[0xc9] = 0;
                        psVar6[0x12a] = 0;
                        psVar6[299] = 0xffff;
                        psVar6[300] = 0;
                        psVar6[0x12d] = 0;
                    } else {
                        uVar7 = ( ushort ) * ( undefined4* )( uint16_t_ARRAY_00eede20 + iVar4 * 2 );
                        uVar8 = ( ushort ) * ( undefined4* )( uint16_t_ARRAY_00eede20 + uStack_c * 2 );
                        pCStack_10 = ( CachedDrawCommand* )FUN_00605360( uVar7, uVar8, psVar3, uStack_c );
                        FUN_00604f60( pCStack_10, usage, ppbData );
                        usage = usage + ( ( uint )pCStack_10 & 0xffff );
                        ppbData = ( short* )( ( int )ppbData + ( ( int )pCStack_10 * 2 & 0xffffU ) );
                        pCStack_10 = ( CachedDrawCommand* )FUN_006055a0( uVar7, uVar8, ppbData, ( uint )ppbData );
                        FUN_00604f60( pCStack_10, usage, ppbData );
                        usage = usage + ( ( uint )pCStack_10 & 0xffff );
                        ppbData = ( short* )( ( int )ppbData + ( ( int )pCStack_10 * 2 & 0xffffU ) );
                        pCStack_10 = ( CachedDrawCommand* )FUN_00604fd0( uVar7, uVar8, ppbData, ( uint )ppbData );
                        FUN_00604f60( pCStack_10, usage, ppbData );
                        usage = usage + ( ( uint )pCStack_10 & 0xffff );
                        ppbData = ( short* )( ( int )ppbData + ( ( int )pCStack_10 * 2 & 0xffffU ) );
                        uVar2 = FUN_00605190( uVar7, uVar8, ppbData, ( uint )ppbData );
                        FUN_00604f60( uVar2, usage, ppbData );
                        usage = usage + ( uVar2 & 0xffff );
                        psVar3 = ( short* )( ( int )ppbData + ( uVar2 * 2 & 0xffff ) );
                        iVar4 = iStack_8;
                        ppbData = psVar3;
                    }
                    uStack_c = uStack_c + 1;
                    psVar6 = psVar6 + 4;
                } while ( ( int )uStack_c < 5 );
                iStack_8 = iVar4 + 1;
            } while ( iStack_8 < 5 );
            pIndexBuffer0->Unlock();
            return true;
        }*/
    }
    return false;
}

void VertexStreamCache::release()
{
    OTDU_D3D9_SAFE_RELEASE( pIndexBuffer0 );
    OTDU_D3D9_SAFE_RELEASE( pIndexBuffer1 );
}

void VertexStreamCache::FUN_00604e60( IDirect3DDevice9* pDevice )
{
    DWORD_3f0 = 0x87d;

    HRESULT HVar3 = pDevice->CreateIndexBuffer( 0x2200, 8, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pIndexBuffer1,
                                                ( HANDLE* )0x0 );
    if ( -1 < HVar3 ) {
        uint32_t* puVar4 = nullptr;
        uint32_t* pVStack_4 = nullptr;
        HVar3 = pIndexBuffer1->Lock( 0, 0, (void**)&pVStack_4, 0 );
        if ( -1 < HVar3 ) {
            bool bVar1 = false;
            uint32_t uVar5 = 0;
            do {
                uint32_t uVar3 = 0;
                do {
                    *pVStack_4 = uVar5 + uVar3;
                    puVar4 = pVStack_4 + 1;
                    if ( bVar1 ) {
                        *puVar4 = *pVStack_4;
                        puVar4 = pVStack_4 + 2;
                        bVar1 = false;
                    }
                    *puVar4 = uVar5 + 0x808 + uVar3;
                    pVStack_4 = puVar4 + 1;
                    uVar3 = uVar3 + 8;
                } while ( uVar3 < 0x101 );
                *pVStack_4 = *puVar4;
                pVStack_4 = puVar4 + 2;
                uVar5 = uVar5 + 0x808;
                bVar1 = true;
            } while ( uVar5 < 0xf8f9 );
            pIndexBuffer1->Unlock();
        }
    }
}

static constexpr uint16_t uint16_t_ARRAY_00eede20[10] = {
    0x10,     0x0,     0x8,     0x0,
    0x4,     0x0,     0x2,     0x0,
    0x1,     0x0
};

int32_t FUN_006057e0( uint16_t param_1, int16_t* param_2 )
{
    OTDU_UNIMPLEMENTED;
    return 0;

    //int32_t iVar1;
    //uint64_t uVar2;
    //int32_t iVar3;
    //int16_t sVar4;
    //uint16_t uVar5;
    //int16_t* psVar6;
    //int32_t iVar7;
    //int16_t* psVar8;
    //int32_t iVar9;
    //int32_t iVar10;
    //int32_t iVar11;
    //int32_t iVar12;
    //uint32_t uVar13;
    //int32_t iVar14;
    //int32_t iVar15;
    //int32_t local_2c;
    //int32_t local_28;
    //int32_t local_24;
    //int32_t local_20;
    //uint32_t local_c;

    //psVar6 = param_2;
    //if ( param_1 != 1 ) {
    //    uVar2 = 0x10 / param_1;
    //    iVar7 = ( int32_t )uVar2;
    //    iVar10 = param_1 - 1;
    //    uVar5 = 1;
    //    if ( 1 < iVar10 ) {
    //        local_24 = iVar7 * 0x104;
    //        local_28 = iVar7 * 0x204;
    //        local_2c = iVar7 * 0x305;
    //        iVar15 = iVar7 * 0x203;
    //        iVar12 = iVar7 * 0x304;
    //        iVar9 = iVar7 * 0x102;
    //        local_c = 1;
    //        do {
    //            sVar4 = ( int16_t )uVar2;
    //            *param_2 = ( int16_t )iVar9 + sVar4;
    //            param_2[1] = ( int16_t )iVar9;
    //            uVar13 = 1;
    //            psVar8 = param_2 + 2;
    //            param_2 = psVar8;
    //            if ( 1 < iVar10 ) {
    //                param_2 = ( int16_t* )local_2c;
    //                iVar3 = iVar7 * 0x202;
    //                iVar11 = local_28;
    //                iVar14 = iVar15;
    //                iVar1 = ( int32_t )param_2;
    //                local_20 = iVar12;
    //                do {
    //                    param_2 = ( int16_t* )iVar1;
    //                    iVar1 = ( int32_t )param_2 + iVar3;
    //                    *psVar8 = ( int16_t )iVar11;
    //                    psVar8[1] = ( int16_t )iVar14;
    //                    psVar8[2] = ( int16_t )param_2;
    //                    psVar8[3] = ( int16_t )local_20;
    //                    local_20 = local_20 + iVar3;
    //                    uVar13 = uVar13 + 2;
    //                    iVar11 = iVar11 + iVar3;
    //                    psVar8 = psVar8 + 4;
    //                    iVar14 = iVar14 + iVar3;
    //                    param_2 = psVar8;
    //                } while ( ( int32_t )( uVar13 & 0xffff ) < iVar10 );
    //            }
    //            if ( ( int32_t )( local_c + 1 ) < iVar10 ) {
    //                *param_2 = ( ( int16_t )( uVar13 << 8 ) + uVar5 + ( int16_t )uVar13 ) * sVar4;
    //                param_2[1] = ( int16_t )local_24;
    //                param_2 = param_2 + 2;
    //            }
    //            uVar5 = uVar5 + 1;
    //            local_c = ( uint32_t )uVar5;
    //            local_2c = local_2c + iVar7;
    //            local_28 = local_28 + iVar7;
    //            local_24 = local_24 + iVar7;
    //            iVar9 = iVar9 + iVar7;
    //            iVar12 = iVar12 + iVar7;
    //            iVar15 = iVar15 + iVar7;
    //        } while ( ( int32_t )local_c < iVar10 );
    //    }
    //    return ( int32_t )param_2 - ( int32_t )psVar6 >> 1;
    //}
    //*param_2 = 0x10;
    //param_2[1] = 0;
    //param_2[2] = 0x1020;
    //param_2[3] = 0x1010;
    //return 4;
}

int32_t VertexStreamCache::FUN_006059e0()
{
    int32_t iVar6 = 0;
    OTDU_UNIMPLEMENTED;
    /*  uint32_t uVar2;
    uint32_t uVar3;
    uint32_t uVar4;
    uint32_t* puVar5;
    uint16_t* puVar7;
    uint16_t uVar8;
    uint16_t uVar9;
    int16_t* local_1804;
    int16_t local_1800[3070];
    uint32_t uVar1;

    uint32_t uStack_4 = 0x6059ea;
    iVar6 = 0;
    puVar7 = uint16_t_ARRAY_00eede20;

    for ( uint32_t i = 0; i < 10; i++ ) {
        uVar2 = FUN_006057e0( *puVar7, local_1800 );
        puVar7 = puVar7 + 2;
        iVar6 = iVar6 + ( uVar2 & 0xffff );
    }
    local_1804 = uint16_t_ARRAY_00eede20;
      for ( uint32_t i = 0; i < 10; i++ ) {
          uVar1 = *local_1804;
          puVar7 = uint16_t_ARRAY_00eede20;
          puVar5 = local_1804;
          for ( uint32_t j = 0; j < 10; j++ ) {
              uVar8 = uVar1;
              uVar9 = *puVar7;
              uVar2 = FUN_00605360( uVar8, uVar9, local_1800, ( uint )puVar5 );
              uVar3 = FUN_006055a0( uVar8, uVar9, local_1800, ( uint )local_1800 );
              uVar4 = FUN_00604fd0( uVar8, uVar9, local_1800, uVar3 );
              puVar5 = ( uint* )FUN_00605190( uVar8, uVar9, local_1800, uVar4 & 0xffff );
              puVar7 = puVar7 + 2;
              iVar6 = iVar6 + ( uVar2 & 0xffff ) + ( uVar3 & 0xffff ) + ( uVar4 & 0xffff ) +
                  ( ( uint )puVar5 & 0xffff );
          }
      }*/

    return iVar6;
}
#endif

