#include "shared.h"
#include "gs_render.h"

#if OTDU_VULKAN
#include "render/vulkan/render_device.h"
#endif

#include "config/gs_config.h"
#include "system/gs_system.h"
#include "player_data/gs_playerdata.h"
#include "world/weather/weather_config.h"
#include "world/weather/gs_weather.h"

#include "Eigen/src/Core/MathFunctions.h"

GSRender* gpRender = nullptr;

GSRender::GSRender()
    : GameSystem()
    , pRenderDevice( nullptr )
    , activeAA( eAntiAliasingMethod::AAM_Disabled )
    , activeLODQuality( 2u )
    , defaultViewport()
    , clearColorViewport()
    , clearViewport()
    , clearColorDepthViewport()
    , clearColorStencilViewport()
    , clearDepthViewport()
    , renderWidth( 0 )
    , renderHeight( 0 )
    , frameIndex( 0 )
    , activeRenderLayers( 0 )
    , aspectRatio( 0.0f )
    , invAspectRatio( 0.0f )
    , time( 0.0f )
    , bHDREnabled( false )
    , bWasHDREnabled( false )
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
    , shaderUniforms{ Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero() }
    , projectionMatrix( Eigen::Matrix4f::Identity() )
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
    , interiorNearPlane( 0.1f )
    , interiorFarPlane( 40.0f )
{
    renderPasses.resize( 75 );

    shaderUniforms[4] = { 0.1456f, -0.49f, -0.49f, -1.5f };
    shaderUniforms[5] = { 1.0f, 1.0f, 1.0f, 1.0f };

    gpRender = this;
}

GSRender::~GSRender()
{
    gpRender = nullptr;
}

bool GSRender::initialize( TestDriveGameInstance* )
{
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

    pRenderDevice = new RenderDevice();
    pRenderDevice->bindWindowHandle( gpSystem->getWindowHandle() );
    pRenderDevice->resize( renderWidth, renderHeight );
    pRenderDevice->initialize();

    // Update aspect ratio flag based on the actual render resolution (this will effectively
    // override the configuration parsed from disk).
    aspectRatio = ( float )renderWidth / ( float )renderHeight;
    invAspectRatio = 1.0f / aspectRatio;

    gpConfig->setScreenRatio( abs( aspectRatio - 1.777778f ) <= abs( aspectRatio - 1.333333f ) );

    activeAA = gpConfig->AntiAliasing;
    bHDREnabled = gpConfig->bEnableHDRI;
    bWasHDREnabled = bHDREnabled;

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

    gpWeather->registerListener( this );

    bInitialized = true;

    return true;
}

void GSRender::tick(float deltaTime)
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

void GSRender::setLODQuality(const int32_t qualityIndex)
{
    if (qualityIndex < 0 || 2 < qualityIndex) {
        return;
    }

    bool bDirtyLODQuality = activeLODQuality != qualityIndex;
    activeLODQuality = qualityIndex;

    switch (activeLODQuality) {
    case 0:
        globalLODFactor = 0.6f;
        interiorNearPlane = farPlane * 0.25f;
        break;
    case 1:
        globalLODFactor = 0.8f;
        interiorNearPlane = farPlane * 0.50f;
        break;
    case 2:
    default:
        globalLODFactor = 1.0f;
        interiorNearPlane = farPlane;
        break;
    };

    gpPlayerData->setLODQuality( activeLODQuality );

    OTDU_UNIMPLEMENTED;
    // if (bDirtyLODQuality) {
    //     FUN_00993b30(&this->field_0xa69c);
    // }
}

void GSRender::onWeatherConfigChange(WeatherConfig* param_1)
{
    defaultAmbientLight.Color = param_1->getGlobalAmbientColor();

    param_1->fillDirectionalLight(defaultDirectionalLight);

    param_1->fillFogDesc(sceneNear.getFogDescWrite());
    param_1->fillFogDesc(scenePreNear.getFogDescWrite());
    param_1->fillFogDesc(sceneNearOpaque.getFogDescWrite());
    param_1->fillFogDesc(sceneCarPlayerPrepass.getFogDescWrite());
    param_1->fillFogDesc(sceneNearPrePassVegetation.getFogDescWrite());
    param_1->fillFogDesc(sceneParticles.getFogDescWrite());
    param_1->fillFogDesc(sceneCarPlayer.getFogDescWrite());
    param_1->fillFogDesc(sceneNearOpaqueCarAlpha.getFogDescWrite());
    param_1->fillFogDesc(sceneCarPlayerCarAlpha.getFogDescWrite());
    param_1->fillFogDesc(sceneNearAfterCarAlpha.getFogDescWrite());
    param_1->fillFogDesc(sceneCarPlayerPPCarAlpha.getFogDescWrite());

    updateWeatherParams();

    shaderUniforms[2] = param_1->getTerrainUniformParams();
}

void GSRender::flushDrawCommands(bool param_1)
{
    // FUN_005066e0
    OTDU_UNIMPLEMENTED;
}

void GSRender::allocateRenderTargets()
{
    //eViewFormat viewFormat = ( bHDREnabled ) ? VF_A16B16G16R16 : VF_X8R8G8B8;

    //int32_t width = ( int32_t )( renderWidth + ( ( renderWidth >> 0x1f ) & 3 ) ) >> 2;
    //int32_t height = ( int32_t )( renderHeight + ( ( renderHeight >> 0x1f ) & 3 ) ) >> 2;

    //// TODO: MSAA support
    //GPUTextureDesc mainRTDesc( renderWidth, renderHeight, 1, 1, viewFormat, RTF_ReadOnly, "MainRT" );
    //mainRT = pRenderDevice->createTexture( &mainRTDesc );

    //GPUTextureDesc scnDown4Desc( width, height, 1, 1, viewFormat, RTF_ReadOnly, "ScnDown4" );
    //scnDown4 = pRenderDevice->createTexture( &scnDown4Desc );

    //allocateAtmosphereResources();
}

void GSRender::allocateAtmosphereResources()
{
    /*static constexpr const char* kResourcesName[kNumSunRT] = {
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
    oceanNMap = pRenderDevice->createTexture( &rtDesc );*/
}

void GSRender::updateWeatherParams()
{
    if (gpWeather->isConfigDirty()) {
        gpWeather->updateActiveConfig();
    }
}

RenderPass::RenderPass()
    : pScene( nullptr )
    , pCamera( nullptr )
    , pViewport( nullptr )
    , pFramebufferAttachments( nullptr )
    , pLights( nullptr )
    , Flags( 0u )
    , ShaderIndex( 0xffffffff )
    , Name( "" )
    , pSceneCopy( nullptr )
    , bIs3DScene( false )
    , bEnabled( false )
{

}
