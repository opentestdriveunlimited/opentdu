#include "shared.h"
#include "gs_render.h"

#include "config/gs_config.h"
#include "system/gs_system.h"
#include "player_data/gs_playerdata.h"
#include "world/weather/weather_config.h"
#include "world/weather/gs_weather.h"
#include "render_target.h"

#include "shaders/shader_register.h"
#include "postfx/postfx_stack.h"

#include "Eigen/src/Core/MathFunctions.h"

GSRender* gpRender = nullptr;
eViewFormat gDepthStencilFormat = eViewFormat::VF_D24S8F; // DAT_00fac8e4
GPUTexture* gpMainDepthBuffer = nullptr; // DAT_00f47920

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
    , mainRTwM()
    , scnDown4()
    , mainRT()
    , pMainRTwM( nullptr )
    , pMainRT( nullptr )
    , pScnDown4RT( nullptr )
    , pUnknownRT( nullptr )
    , pUnknownRT2( nullptr )
    , pMainRTFramebuffer( nullptr )
    , pMainRTwMFramebuffer( nullptr )
    , pBackbufferFramebuffer( nullptr )
    , pUnknownRTFramebuffer( nullptr )
    , pUnknownRT2Framebuffer( nullptr )
    , pSunFramebuffer( nullptr )
    , shaderUniforms{ Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero(), Eigen::Vector4f::Zero() }
    , projectionMatrix( Eigen::Matrix4f::Identity() )
    , defaultAmbientLight()
    , defaultDirectionalLight()
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
#if OTDU_WIN32
    pRenderDevice->bindWindowHandle( gpSystem->getWindowHandle() );
#endif
    pRenderDevice->resize( renderWidth, renderHeight );
    pRenderDevice->initialize();

    // Update aspect ratio flag based on the actual render resolution (this will effectively
    // override the configuration parsed from disk).
    aspectRatio = ( float )renderWidth / ( float )renderHeight;
    invAspectRatio = 1.0f / aspectRatio;

    gpConfig->setScreenRatio( abs( aspectRatio - 1.777778f ) <= abs( aspectRatio - 1.333333f ) );

    activeAA = gpPlayerData->getAAMethod();
    bHDREnabled = gpPlayerData->isHDREnabled();
    bWasHDREnabled = bHDREnabled;

    allocateDeviceResources();

    projectionMatrix( 1, 0 ) = 1.570796f; // 90.0 deg
    projectionMatrix( 1, 1 ) = aspectRatio;

#define EIGEN_PI_4 EIGEN_PI / 4

    projectionMatrix( 1, 2 ) = cosf( static_cast< float >( EIGEN_PI_4 ) );
    projectionMatrix( 1, 3 ) = sinf( static_cast< float >( EIGEN_PI_4 ) );

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
    mainRTwM.destroy();
    scnDown4.destroy();
    mainRT.destroy();

    pMainRTwM->destroy(pRenderDevice);
    pMainRT->destroy(pRenderDevice);
    pScnDown4RT->destroy(pRenderDevice);
    pUnknownRT->destroy(pRenderDevice);
    pUnknownRT2->destroy(pRenderDevice);
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

void GSRender::setLODQuality(const uint32_t qualityIndex)
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
    OTDU_IMPLEMENTATION_SKIPPED("FUN_005066e0");
}

bool GSRender::initializeShaderCache()
{
    gShaderRegister.releaseCachedShaderInstances();
    /* DAT_00fadbe0 = 0;
       DAT_00fadbec = 0;*/
    gShaderRegister.registerMasterTable();
    gPostFXStack.releaseResources();
    return true;
}

bool GSRender::allocateDeviceResources()
{
    // FUN_00995520
    bool bVar1 = CreateBackbufferRenderTarget();

    bool bVar13 = allocateRenderTargets();
    if (!bVar13) {
        OTDU_LOG_ERROR("Failed to allocate global render targets!\n");
        return false;
    }

    bVar13 = allocateAtmosphereResources();
    if (!bVar13) {
        OTDU_LOG_ERROR("Failed to allocate sun/ocean render targets!\n");
        return false;
    }

    bVar13 = setupSunPostFXStack();
    if (!bVar13) {
        OTDU_LOG_ERROR("Failed to setup sun PostFX stack!\n");
        return false;
    }


    Eigen::Vector4<float> vector1( 0.0f, 0.0f, 200.0f, 1.0f );
    Eigen::Vector4<float> vector2( 0.0f, 0.0f, 0.0f, 1.0f );
    Eigen::Vector4<float> vector3( 0.0f, 1.0f, 0.0f, 1.0f );

    setupScenes(nearPlane, farPlane);

    return bVar1;
}

bool GSRender::setupSunPostFXStack()
{

}

void GSRender::setupScenes(float nearZ, float farZ)
{
    scenePreNear.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNearOpaque.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNear.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNearNoFog.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneOcclusionDOFPrepass.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneOcclusionDOF.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNearPrePassVegetation.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneParticles.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneCarPlayerPrepass.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneCarPlayer.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNearOpaqueCarAlpha.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNearCarAlpha.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneNearAfterCarAlpha.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneCarPlayerPPCarAlpha.getFrustumWrite().setPlanes(nearZ, farZ);
    sceneCarPlayerCarAlpha.getFrustumWrite().setPlanes(nearZ, farZ);
}

bool GSRender::allocateRenderTargets()
{
    uint32_t height = renderHeight;
    uint32_t width = renderWidth;
    bool bSDR = bHDREnabled;
    uint32_t uVar6 = height >> 0x1f;
    uint32_t uVar3 = width >> 0x1f;

    eViewFormat viewFormat = (bSDR) ? VF_X8R8G8B8 : VF_A16B16G16R16F;
    uint32_t uVar7 = (bSDR) ? 0 : 6;
    bool bVar2 = false;
    bVar2 = mainRTwM.allocateAndCreate(width, height, 1, 1, viewFormat, 0x200, "MainRTwM");
    if (!bVar2) {
        return false;
    }
    uVar6 = (uint32_t)(height + (uVar6 & 3)) >> 2;
    uVar3 = (uint32_t)(width + (uVar3 & 3)) >> 2;
    bVar2 = scnDown4.allocateAndCreate(uVar3, uVar6, 1, 1, viewFormat, 0x200, "ScnDown4");

    if (bVar2) {
        bVar2 = mainRT.allocateAndCreate(width, height, 1, 1, viewFormat, 0x200, "MainRT");
        if (bVar2) {
            pMainRTwM = CreateRenderTarget(width, height, viewFormat, activeAA, uVar7);
            if (pMainRTwM != nullptr) {
                pMainRT = CopyRenderTarget(pMainRTwM, 8);
                if (pMainRT != nullptr) {
                    pUnknownRT = CopyRenderTarget(pMainRTwM, 8);
                    if (pUnknownRT != nullptr) {
                        pUnknownRT2 = CopyRenderTarget(pMainRTwM, 8);
                        if (pUnknownRT2 != nullptr) {
                            eViewFormat eVar6 = pMainRT->getFormat();
                            pScnDown4RT = FUN_0050aff0(pMainRT,(float)uVar3,(float)uVar6,viewFormat,0x200,1);
                            if (pScnDown4RT != nullptr) {
                                pMainRTwM->bind2DB(&mainRTwM);
                                pMainRT->bind2DB(&mainRT);
                                pUnknownRT->bind2DB(&mainRTwM);
                                pUnknownRT2->bind2DB(&mainRT);

                                RenderTarget* peVar3 = RenderTarget::GetBackBuffer();
                                peVar3->bind2DB(&mainRT);
                                pScnDown4RT->bind2DB(&scnDown4);

                                pMainRTFramebuffer = new FramebufferAttachments();
                                pMainRTFramebuffer->pAttachments[0] = pMainRTwM;

                                pMainRTwMFramebuffer = new FramebufferAttachments();
                                pMainRTwMFramebuffer->pAttachments[0] = pMainRT;

                                pBackbufferFramebuffer = new FramebufferAttachments();
                                pBackbufferFramebuffer->pAttachments[0] = RenderTarget::GetBackBuffer();

                                pUnknownRTFramebuffer = new FramebufferAttachments();
                                pUnknownRTFramebuffer->pAttachments[0] = pUnknownRT;
                                
                                pUnknownRT2Framebuffer = new FramebufferAttachments();
                                pUnknownRT2Framebuffer->pAttachments[0] = pUnknownRT2;

                                pSunFramebuffer = new FramebufferAttachments();
                                pUnknownRT2Framebuffer->pAttachments[0] = pUnknownRT;
                                
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool GSRender::allocateAtmosphereResources()
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

    bool bVar1 = sun2DBs[0].allocateAndCreate(0x40,0x40,1,1, VF_X8R8G8B8, 0x200, "SUN_DS_64x64");
    if (!bVar1) {
        return false;
    }
    
    uint32_t uVar7 = 0;
    eAntiAliasingMethod activeAA = pMainRT->getAntiAliasingMethod();
    pSunRTs[0] = CreateRenderTarget(0x40, 0x40, VF_X8R8G8B8, activeAA, uVar7);
    pSunRTs[0]->bind2DB( &sun2DBs[0] );
    
    uint32_t dimension = 16;
    for ( int32_t i = 1; i < kNumSunRT; i++ ) {
        bVar1 = sun2DBs[i].allocateAndCreate(dimension,dimension,1,1,VF_X8R8G8B8,0x200,kResourcesName[i]);
        if (!bVar1) {
            return false;
        }
        pSunRTs[i] = CreateRenderTargetFrom2DB( &sun2DBs[i], 2 );
        
        dimension >>= 2;
        if ( dimension < 2 ) {
            dimension = 1;
        }
    }

    pSunRealLumFramebuffer = new FramebufferAttachments();
    pSunRealLumFramebuffer->pAttachments[0] = pSunRTs[0];
    
    pSunEyeLumFramebuffer = new FramebufferAttachments();
    pSunRealLumFramebuffer->pAttachments[0] = pSunRTs[4];
    
    pSunDazzleCoeffFramebuffer = new FramebufferAttachments();
    pSunRealLumFramebuffer->pAttachments[0] = pSunRTs[6];
    
    bool bVar2 = noiseComposite.allocateAndCreate(0x20,0x20,1,1,VF_X8R8G8B8,0x200,"NOISE_COMPOSITE");
    if (bVar2) {
        pNoiseCompositeRT = CreateRenderTargetFrom2DB(&noiseComposite, 2);

        if (pNoiseCompositeRT != nullptr) {
            pNoiseCompositeFramebuffer = new FramebufferAttachments();
            pNoiseCompositeFramebuffer->pAttachments[0] = pNoiseCompositeRT;

            bVar2 = noiseAssembleF.allocateAndCreate(0x100,0x100,1,1,VF_X8R8G8B8,0x200,"NOISE_ASSEMBLE_F");
            if (bVar2) {
                bVar2 = noiseAssembleS.allocateAndCreate(0x100,0x100,1,1,VF_X8R8G8B8,0x200,"NOISE_ASSEMBLE_S");
                if (bVar2) {
                    pNoiseAssembleFRT = CreateRenderTargetFrom2DB(&noiseAssembleF, 2);
                    pNoiseAssembleSRT = CreateRenderTargetFrom2DB(&noiseAssembleS, 2);

                    if (pNoiseAssembleFRT != nullptr && pNoiseAssembleSRT != nullptr) {
                        pNoiseAssembleFFramebuffer = new FramebufferAttachments();
                        pNoiseAssembleFFramebuffer->pAttachments[0] = pNoiseAssembleFRT;
                        
                        pNoiseAssembleSFramebuffer = new FramebufferAttachments();
                        pNoiseAssembleSFramebuffer->pAttachments[0] = pNoiseAssembleSRT;

                        bVar2 = oceanNMap.allocateAndCreate(0x80,0x80,1,1,VF_X8R8G8B8,0xc200,"OceanNMap");
                        if (bVar2) {
                            pOceanNMapRT = CreateRenderTargetFrom2DB(&oceanNMap, 2);

                            if (pOceanNMapRT != nullptr) {
                                pOceanNMapFramebuffer = new FramebufferAttachments();
                                pOceanNMapFramebuffer->pAttachments[0] = pOceanNMapRT;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
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
