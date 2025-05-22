#include "shared.h"
#include "gs_render.h"

#if OTDU_VULKAN
#include "render/vulkan/render_device.h"
#endif

#include "config/gs_config.h"
#include "system/gs_system.h"

#include "Eigen/src/Core/MathFunctions.h"

GSRender* gpRender = nullptr;

GSRender::GSRender()
    : pRenderDevice( nullptr )
    , renderWidth( 0 )
    , renderHeight( 0 )
    , frameIndex( 0 )
    , aspectRatio( 0.0f )
    , invAspectRatio( 0.0f )
    , bHDREnabled( false )
    , bFrameRecordingInProgress( false )
    , bFovVsLodOn( true )
    , bInitialized( false )
    , mainRT( nullptr )
    , scnDown4( nullptr )
    , sunRenderTargets{ nullptr }
    , noiseComposite( nullptr )
    , noiseAssembleF( nullptr )
    , noiseAssembleS( nullptr )
    , oceanNMap( nullptr )
    , projectionMatrix( Eigen::Matrix4f::Identity() )
    , glassDirt( 0.0f, 0.0f, 0.0f, 0.0f )
    , unknownVector1( 0.484375f, 0.4921875f, 0.5195313f, 1.0f )
    , unknownVector2( 0.5f, 0.5f, 0.5f, 1.0f )
    , globalLODFactor( 1.0f )
    , fovVsLodFactor( 1.0f )
    , fovVsLodFactorStart( 0.4f )
    , fovVsLodFactorEnd( 0.1f )
    , lodFactor( 0.1f )
    , lineMipBias( 0.1f )
    , vegetationUVAFactor( 0.1f )
    , vegetationFactor( 0.015f )
    , vegetationPeriod( 0.001f )
    , treeWindSpeedLimit( 200.5f )
    , vegetationWindSpeedLimit( 100.0f )
    , hmapBeginFadeDist( 100.0f )
    , hmapFadeLength( 3000.0f )
    , nearPlane( 0.8f )
    , farPlane( 40000.0f )
    , halfFarPlane( 40000.0f )
{

}

GSRender::~GSRender()
{

}

bool GSRender::initialize( TestDriveGameInstance* )
{
    pRenderDevice = new RenderDevice();
    pRenderDevice->initialize();

    if ( gpConfig->bWindowed || gpConfig->FullscreenMode == eFullscreenMode::FM_Desktop ) {
        renderWidth = gpConfig->WindowWidth;
        renderHeight = gpConfig->WindowHeight;
    } else {
        renderWidth = gpConfig->Width;
        renderHeight = gpConfig->Height;
    }

    // Note: The original game didn't work this way (but it's probably more convenient than making
    // fullscreen exclusive only...) 
    gpSystem->resizeGameWindow( renderWidth, renderHeight, gpConfig->FullscreenMode, gpConfig->bWindowed );

    // Update aspect ratio flag based on the actual render resolution (this will effectively
    // override the configuration parsed from disk).
    aspectRatio = ( float )renderWidth / ( float )renderHeight;
    invAspectRatio = 1.0f / aspectRatio;

    gpConfig->setScreenRatio( abs( aspectRatio - 1.777778f ) <= abs( aspectRatio - 1.333333f ) );

    allocateRenderTargets();

    //projectionMatrix << xScale, 0, 0, 0,
    //    0, yScale, 0, 0,
    //    0, 0, -( zFar + zNear ) / ( zFar - zNear ), -1,
    //    0, 0, -2 * zNear * zFar / ( zFar - zNear ), 0;

    Eigen::Vector4<float> vector1( 0.0f, 0.0f, 200.0f, 1.0f );
    Eigen::Vector4<float> vector2( 0.0f, 0.0f, 0.0f, 1.0f );
    Eigen::Vector4<float> vector3( 0.0f, 1.0f, 0.0f, 1.0f );

    projectionMatrix( 1, 0 ) = 1.570796f; // 90.0 deg
    projectionMatrix( 1, 1 ) = aspectRatio;

#define EIGEN_PI_4 EIGEN_PI / 4

    projectionMatrix( 1, 2 ) = cosf( static_cast< float >( EIGEN_PI_4 ) );
    projectionMatrix( 1, 3 ) = sinf( static_cast< float >( EIGEN_PI_4 ) );

    vector2 -= vector1;
    projectionMatrix( 2, 0 ) = projectionMatrix( 1, 3 ) / projectionMatrix( 1, 2 );

    bInitialized = true;

    return true;
}

void GSRender::tick()
{

}

void GSRender::terminate()
{

}

void GSRender::beginFrame()
{
    bFrameRecordingInProgress = true;
}

void GSRender::endFrame()
{
    bFrameRecordingInProgress = false;
    frameIndex++;
}

void GSRender::allocateRenderTargets()
{
    eViewFormat viewFormat = ( bHDREnabled ) ? VF_A16B16G16R16 : VF_X8R8G8B8;

    int32_t width = ( int32_t )( renderWidth + ( ( renderWidth >> 0x1f ) & 3 ) ) >> 2;
    int32_t height = ( int32_t )( renderHeight + ( ( renderHeight >> 0x1f ) & 3 ) ) >> 2;

    // TODO: MSAA support
    GPUTextureDesc mainRTDesc( renderWidth, renderHeight, 1, 1, viewFormat, RTF_ReadOnly, "MainRT" );
    mainRT = pRenderDevice->createTexture( &mainRTDesc );

    GPUTextureDesc scnDown4Desc( width, height, 1, 1, viewFormat, RTF_ReadOnly, "ScnDown4" );
    scnDown4 = pRenderDevice->createTexture( &scnDown4Desc );

    allocateAtmosphereResources();
}

void GSRender::allocateAtmosphereResources()
{
    static constexpr const char* kResourcesName[kNumSunRT] = {
        "SUN_DS_64x64",
        "SUN_DS_16x16",
        "SUN_DS_4x4",
        "SUN_REAL_LUM",
        "SUN_EYE_LUM",
        "SUN_EYE_LUM_BK",
        "SUN_DAZZLE_COEFF",
        "SUN_DAZZLE_COEFF_BK"
    };

    GPUTextureDesc rtDesc( 64, 64, 1, 1, VF_X8R8G8B8, RTF_ReadOnly, kResourcesName[0] );
    sunRenderTargets[0] = pRenderDevice->createTexture( &rtDesc );

    uint32_t dimension = 16;
    for ( int32_t i = 1; i < 8; i++ ) {
        rtDesc.Fill( dimension, dimension, 1, 1, VF_X8R8G8B8, RTF_ReadOnly, kResourcesName[i]);
        sunRenderTargets[i] = pRenderDevice->createTexture( &rtDesc );
        
        dimension >>= 2;
        if ( dimension < 2 ) {
            dimension = 1;
        }
    }

    rtDesc.Fill( 32, 32, 1, 1,   VF_X8R8G8B8, RTF_ReadOnly, "NOISE_COMPOSITE" );
    noiseComposite = pRenderDevice->createTexture( &rtDesc );

    rtDesc.Fill( 256, 256, 1, 1, VF_X8R8G8B8, RTF_ReadOnly, "NOISE_ASSEMBLE_F" );
    noiseAssembleF = pRenderDevice->createTexture( &rtDesc );

    rtDesc.Fill( 256, 256, 1, 1, VF_X8R8G8B8, RTF_ReadOnly, "NOISE_ASSEMBLE_S" );
    noiseAssembleS = pRenderDevice->createTexture( &rtDesc );

    rtDesc.Fill( 128, 128, 1, 1, VF_X8R8G8B8, RTF_ReadOnly, "OceanNMap" );
    oceanNMap = pRenderDevice->createTexture( &rtDesc );
}

void GPUTextureDesc::Fill( uint32_t width,
                             uint32_t height,
                             uint32_t depth,
                             uint32_t numMips,
                             eViewFormat format,
                             uint32_t flags,
                             const char* pResourceName )
{
    Width = width;
    Height = height;
    ArrayLength = depth;
    MipCount = numMips;
    Format = format;
    Flags = flags;
    Size = CalculateResourceSize( width, height, depth, numMips, format, flags );
    Hashcode = pResourceName ? GetIdentifier64bit( pResourceName ) : 0ull;
}
