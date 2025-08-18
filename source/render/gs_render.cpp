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

#include "postfx/effects/postfx_texture_sample.h"
#include "postfx/effects/postfx_downscale.h"
#include "postfx/effects/postfx_blit.h"

#include "render/frame_graph.h"

#include "Eigen/src/Core/MathFunctions.h"

GSRender* gpRender = nullptr;
eViewFormat gDepthStencilFormat = eViewFormat::VF_D24S8F; // DAT_00fac8e4
GPUTexture* gpMainDepthBuffer = nullptr; // DAT_00f47920
RenderScene* gpActiveRenderScene = nullptr;

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
    , noiseComposite()
    , noiseAssembleF()
    , noiseAssembleS()
    , oceanNMap()
    , pMainRTwM( nullptr )
    , pMainRT( nullptr )
    , pScnDown4RT( nullptr )
    , pUnknownRT( nullptr )
    , pUnknownRT2( nullptr )
    , sun2DBs{ RuntimeRender2DB(), RuntimeRender2DB(), RuntimeRender2DB(), RuntimeRender2DB(), RuntimeRender2DB(), RuntimeRender2DB(), RuntimeRender2DB(), RuntimeRender2DB() }
    , pSunRTs{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
    , pNoiseCompositeRT( nullptr )
    , pNoiseAssembleFRT( nullptr )
    , pNoiseAssembleSRT( nullptr )
    , pOceanNMapRT( nullptr )
    , pMainRTFramebuffer( nullptr )
    , pMainRTwMFramebuffer( nullptr )
    , pBackbufferFramebuffer( nullptr )
    , pUnknownRTFramebuffer( nullptr )
    , pUnknownRT2Framebuffer( nullptr )
    , pSunFramebuffer( nullptr )
    , pSunRealLumFramebuffer( nullptr )
    , pSunEyeLumFramebuffer( nullptr )
    , pSunDazzleCoeffFramebuffer( nullptr )
    , pNoiseCompositeFramebuffer( nullptr )
    , pNoiseAssembleFFramebuffer( nullptr )
    , pNoiseAssembleSFramebuffer( nullptr )
    , pOceanNMapFramebuffer( nullptr )
    , pPostFXMainHDR( nullptr )
    , pPostFXMainHDRDownscale( nullptr )
    , pSunAvgLum( nullptr )
    , pSunAvgLumCopy( nullptr )
    , pSunDazzleLUT( nullptr )
    , pSunDazzleLUTCopy( nullptr )
    , pSunBillboard( nullptr )
    , pSunDownscaleChain{ nullptr, nullptr, nullptr }
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
    , sunDirection(0.0f, -0.4f, -1.0f)
    , frameGraph( new FrameGraph() )
{
    sunDirection.normalize();

    shaderUniforms[4] = { 0.1456f, -0.49f, -0.49f, -1.5f };
    shaderUniforms[5] = { 1.0f, 1.0f, 1.0f, 1.0f };

    gpRender = this;
}

GSRender::~GSRender()
{
    terminate();
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

    setupRenderPasses();
    allocateDeviceResources();

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
    noiseComposite.destroy();
    noiseAssembleF.destroy();
    noiseAssembleS.destroy();
    oceanNMap.destroy();

    pMainRTwM->destroy(pRenderDevice);
    pMainRT->destroy(pRenderDevice);
    pScnDown4RT->destroy(pRenderDevice);
    pUnknownRT->destroy(pRenderDevice);
    pUnknownRT2->destroy(pRenderDevice);

    for (uint32_t i = 0; i < kNumSunRT; i++) {
        sun2DBs[i].destroy();
    }
    
    for (uint32_t i = 0; i < kNumSunRT; i++) {
        pSunRTs[i]->destroy(pRenderDevice);
    }
    
    pNoiseCompositeRT->destroy(pRenderDevice);
    pNoiseAssembleFRT->destroy(pRenderDevice);
    pNoiseAssembleSRT->destroy(pRenderDevice);

    pOceanNMapRT->destroy(pRenderDevice);

    delete pMainRTFramebuffer;
    delete pMainRTwMFramebuffer;
    delete pBackbufferFramebuffer;
    delete pUnknownRTFramebuffer;
    delete pUnknownRT2Framebuffer;

    delete pSunFramebuffer;
    delete pSunRealLumFramebuffer;
    delete pSunEyeLumFramebuffer;
    delete pSunDazzleCoeffFramebuffer;

    delete pNoiseCompositeFramebuffer;
    delete pNoiseAssembleFFramebuffer;
    delete pNoiseAssembleSFramebuffer;
    
    delete pOceanNMapFramebuffer;

    TestDrive::Free( pPostFXMainHDR );
    TestDrive::Free( pPostFXMainHDRDownscale );
    
    TestDrive::Free( pSunAvgLum );
    TestDrive::Free( pSunAvgLumCopy );
    TestDrive::Free( pSunDazzleLUT );
    TestDrive::Free( pSunDazzleLUTCopy );
    TestDrive::Free( pSunBillboard );

    for (uint32_t i = 0; i < kNumSunRT; i++) {
        TestDrive::Free(pSunDownscaleChain);
    }

    delete frameGraph;
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

    float width = (float)renderWidth;
    if (width < 0) {
        width = width + 4.2949673e+09;
    }
    float height = (float)renderHeight;
    if (height < 0) {
        height = height + 4.2949673e+09;
    }

    // 2D Camera
    camera2D.setAspectRatio(width, height);
    camera2D.setDefaultMatrix2D();

    // Main Camera
    Eigen::Vector3f defaultPosition( 0.0f, 0.0f, 200.0f );
    Eigen::Vector3f defaultLookAt( 0.0f, 0.0f, 0.0f );
    Eigen::Vector3f defaultUp( 0.0f, 1.0f, 0.0f );

    cameraMain.setAspectRatio(width, height);
    cameraMain.setFOV(1.5707964f);
    cameraMain.updateMatrix(defaultPosition, defaultUp, defaultLookAt);

    // LRFront Camera
    defaultPosition = Eigen::Vector3f( 0.0f, 0.85f, 1.0f );
    defaultLookAt = Eigen::Vector3f( 0.0f, 0.85f, 0.0f );

    cameraLRFront.setAspectRatio(1.0f, 1.0f);
    cameraLRFront.updateMatrix(defaultPosition, defaultUp, defaultLookAt);

    // LRBack Camera
    defaultPosition = Eigen::Vector3f( 0.0f, 0.85f, -1.0f );

    cameraLRBack.setAspectRatio(1.0f, 1.0f);
    cameraLRBack.updateMatrix(defaultPosition, defaultUp, defaultLookAt);

    defaultViewport.Flags = defaultViewport.Flags & 0xfffffff8;

    uint32_t uVar19 = clearColorViewport.Flags;
    clearColorViewport.Flags = uVar19 | 1;
    clearColorViewport.Flags = (uVar19 & 0xfffffff9) | 1;
    clearColorViewport.ClearColor.R = 0;
    clearColorViewport.ClearColor.G = 0;
    clearColorViewport.ClearColor.B = 0;
    clearColorViewport.ClearColor.A = 0xff;

    uint32_t* puVar1 = &clearViewport.Flags;
    *puVar1 = *puVar1 | 7;
    
    puVar1 = &clearColorDepthViewport.Flags;
    *puVar1 = *puVar1 | 3;
    *puVar1 = *puVar1 & 0xfffffffb;
    clearColorDepthViewport.ClearColor.R = 0;
    clearColorDepthViewport.ClearColor.G = 0;
    clearColorDepthViewport.ClearColor.B = 0;
    clearColorDepthViewport.ClearColor.A = 0xff;
    
    puVar1 = &clearColorStencilViewport.Flags;
    *puVar1 = *puVar1 | 6;
    *puVar1 = *puVar1 & 0xfffffffe;
    clearColorStencilViewport.ClearColor.R = 0;
    clearColorStencilViewport.ClearColor.G = 0;
    clearColorStencilViewport.ClearColor.B = 0;
    clearColorStencilViewport.ClearColor.A = 0xff;

    puVar1 = &clearDepthViewport.Flags;
    *puVar1 = *puVar1 | 2;
    *puVar1 = *puVar1 & 0xfffffffa;
    clearDepthViewport.ClearColor.R = 0;
    clearDepthViewport.ClearColor.G = 0;
    clearDepthViewport.ClearColor.B = 0;
    clearDepthViewport.ClearColor.A = 0xff;
    
    puVar1 = &viewportBack.Flags;
    *puVar1 = *puVar1 | 7;
    viewportBack.ClearColor.R = 0;
    viewportBack.ClearColor.G = 0;
    viewportBack.ClearColor.B = 0;
    viewportBack.ClearColor.A = 0xff;
    
    viewportHUDMap.ClearColor.R = 0x1c;
    viewportHUDMap.ClearColor.G = 0x3c;
    viewportHUDMap.ClearColor.B = 0x5e;
    viewportHUDMap.ClearColor.A = 0xff;
    
    puVar1 = &viewportNoise.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;

    viewportShadowClear.ClearColor.R = 0xff;
    viewportShadowClear.ClearColor.G = 0xff;
    viewportShadowClear.ClearColor.B = 0xff;
    viewportShadowClear.ClearColor.A = 0xff;
    
    puVar1 = &viewportShadowClear.Flags;
    *puVar1 = *puVar1 | 7;

    viewportShadow.ClearColor.R = 0xff;
    viewportShadow.ClearColor.G = 0xff;
    viewportShadow.ClearColor.B = 0xff;
    viewportShadow.ClearColor.A = 0xff;
    
    puVar1 = &viewportShadow.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;
    
    viewportEnvmapBack.ClearColor.R = 0x2b;
    viewportEnvmapBack.ClearColor.G = 0x51;
    viewportEnvmapBack.ClearColor.B = 0x2b;
    viewportEnvmapBack.ClearColor.A = 0xff;
    
    puVar1 = &viewportEnvmapBack.Flags;
    *puVar1 = *puVar1 | 7;

    puVar1 = &viewportEnvmapNear.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;
    
    viewportNear.ClearColor.R = 0xff;
    viewportNear.ClearColor.G = 0x0;
    viewportNear.ClearColor.B = 0x0;
    viewportNear.ClearColor.A = 0xff;

    puVar1 = &viewportNear.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;

    puVar1 = &defaultViewport.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;

    puVar1 = &viewportParticles.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;

    viewportParticles.ClearColor.R = 0x0;
    viewportParticles.ClearColor.G = 0x0;
    viewportParticles.ClearColor.B = 0xff;
    viewportParticles.ClearColor.A = 0x0;
        
    uVar19 = viewportSun.Flags;
    viewportSun.Flags = uVar19 | 1;
    viewportSun.Flags = (uVar19 & 0xfffffff9) | 1;
    viewportSun.ClearColor.A = 0xff;
    viewportSun.ClearColor.R = 0;
    viewportSun.ClearColor.G = 0;
    viewportSun.ClearColor.B = 0;

    uVar19 = viewportInterior.Flags & 0xfffffffa;
    viewportInterior.Flags = uVar19;
    viewportInterior.Flags = uVar19 | 2;

    puVar1 = &viewportAvatar.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;

    puVar1 = &viewportHelmet.Flags;
    *puVar1 = *puVar1 & 0xfffffff8;

    puVar1 = &viewport2D.Flags;
    *puVar1 = *puVar1 & 0xfffffffe;

    viewportLRFront.ClearColor.Color = 0xff7f7f7f;
    viewportLRBack.ClearColor.Color = 0xff7f7f7f;

    ambientLightBack.Color = Eigen::Vector4f::Ones();
    directionalLightBack.Ambient = Eigen::Vector4f::Ones();
    directionalLightBack.Diffuse = Eigen::Vector4f::Ones();
    directionalLightBack.Specular = Eigen::Vector4f::Ones();
    directionalLightBack.Direction = sunDirection;

    lightsBack.addLight(&ambientLightBack);
    lightsBack.addLight(&directionalLightBack);

    lightsDefault.addLight(&defaultAmbientLight);
    lightsDefault.addLight(&defaultDirectionalLight);

    default2DAmbientLight.Color = Eigen::Vector4f::Ones();
    
    default2DDirectionalLight.Ambient.x() = 0.4f;
    default2DDirectionalLight.Ambient.y() = 0.4f;
    default2DDirectionalLight.Ambient.z() = 0.4f;
    default2DDirectionalLight.Ambient.w() = 1.0f;
    default2DDirectionalLight.Diffuse.x() = 0.8f;
    default2DDirectionalLight.Diffuse.y() = 0.8f;
    default2DDirectionalLight.Diffuse.z() = 0.8f;
    default2DDirectionalLight.Diffuse.w() = 1.0f;
    default2DDirectionalLight.Specular = Eigen::Vector4f::Ones();
    default2DDirectionalLight.Direction = sunDirection;

    lights2DDefault.addLight(&default2DAmbientLight);
    lights2DDefault.addLight(&default2DDirectionalLight);

    lightLRAmbient = Eigen::Vector4f( 0.484375f, 0.4921875f, 0.51953125f, 1.0f );
    lightLRDiffuse = Eigen::Vector4f( 0.5f, 0.5f, 0.5f, 1.0f );
    lightLRDirectionFront = Eigen::Vector3f( 0.0f, 0.0f, -1.0f );
    lightLRDirectionBack = Eigen::Vector3f( 0.0f, 0.0f, 1.0f );

    lightLRFront.Ambient = lightLRAmbient;
    lightLRFront.Diffuse = lightLRDiffuse;
    lightLRFront.Direction = lightLRDirectionFront;

    lightLRBack.Ambient = lightLRAmbient;
    lightLRBack.Diffuse = lightLRAmbient;
    lightLRBack.Direction = lightLRDirectionBack;

    lightsLRFront.addLight(&lightLRFront);
    lightsLRBack.addLight(&lightLRBack);
    
    for (RenderPass& ppeVar21 : renderPasses) {
        ppeVar21.pScene->create(
            ppeVar21.pCamera,
            ppeVar21.pViewport,
            ppeVar21.pFramebufferAttachments,
            ppeVar21.Flags,
            ppeVar21.Name.c_str(),
            true,
            0,
            nullptr
        );

        ppeVar21.pScene->setSceneType(!ppeVar21.bIs3DScene);

        if (ppeVar21.pLightSetupNode != nullptr) {
            ppeVar21.pScene->addLightSetup(ppeVar21.pLightSetupNode);
        }

        ppeVar21.pScene->setShaderIndexes(ppeVar21.ShaderIndex, ppeVar21.ShaderIndex);
    }
    
    for (RenderPass& ppeVar21 : renderPasses) {
        ppeVar21.pScene->setEnabled(ppeVar21.bEnabled);
    }

    for (RenderPass& ppeVar21 : renderPasses) {
        if (ppeVar21.pSceneCopy != nullptr) {
            ppeVar21.pScene->copyDrawCommands(ppeVar21.pSceneCopy);
        }
    }

    sceneOcclusion.setUnknownMask(                      0x00004d5500000820);
    sceneOcclusionCockpit.setUnknownMask(               0x00004d5500000820);
    sceneNear.setUnknownMask(                           0xffffb2aa000017df);
    sceneNearOpaque.setUnknownMask(                     0x00004d5500000820);
    sceneNearOpaqueCarAlpha.setUnknownMask(             0x00004d5500000820);
    sceneNearCarAlpha.setUnknownMask(                   0xffffb2aa000017df);
    sceneCockpit.setUnknownMask(                        0x37fffffbfffffffc);
    sceneReflectPrepass.setUnknownMask(                 0xc000000000000003);
    sceneOcclusionDOFPrepass.setUnknownMask(            0xc000000000000003);
    sceneCarPlayerPrepass.setUnknownMask(               0xc000000000000003);
    sceneCarPlayerPPCarAlpha.setUnknownMask(            0xc000000000000003);
    sceneNearPrePassVegetation.setUnknownMask(          0xc08000000000001b);    
    scenePreNear.setUnknownMask(                        0x0000400000000000);
    scenePostCockpit.setUnknownMask(                    0x37fffffbfffffffc);
    sceneCockpitAvatarPrepassCarAlpha.setUnknownMask(   0xc000000000000003);
    sceneCockpitAvatarPrepass.setUnknownMask(           0xc000000000000003);
    sceneCockpitCarAlpha.setUnknownMask(                0x37fffffbfffffffc);
    sceneCockpitAvatar.setUnknownMask(                  0xc800000400000003);
    sceneCockpitAvatarCarAlpha.setUnknownMask(          0xc800000400000003);

    // LAB_00996050
    sceneSunCore.getFrustumWrite().setPlanes(1.0f, 100.0f);
    sceneSunHalo.getFrustumWrite().setPlanes(1.0f, 100.0f);
    sceneSunMarker.getFrustumWrite().setPlanes(1.0f, 100.0f);

    // LAB_009961a0
    sceneOcclusionCockpit.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneCockpit.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneCockpitAvatar.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneCockpitAvatarPrepass.getFrustumWrite().setPlanes(0.1f, 3.0f);
    scenePostCockpit.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneHelmet.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneCockpitCarAlpha.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneCockpitAvatarCarAlpha.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneCockpitAvatarPrepassCarAlpha.getFrustumWrite().setPlanes(0.1f, 3.0f);
    sceneHelmetCarAlpha.getFrustumWrite().setPlanes(0.1f, 3.0f);

    // LAB_009962a0
    for (uint32_t i = 0; i < kNumAvtScenes; i++) {
        sceneAVTLRFront[i].getFrustumWrite().setOrthoDimensions(0.6f, 1.8f);
        sceneAVTLRBack[i].getFrustumWrite().setOrthoDimensions(0.6f, 1.8f);
    }

    RenderScene* local_a0[0xc] = {
        &scenePreNear,
        &sceneNearOpaque,
        &sceneNear,
        &sceneNearPrePassVegetation,
        &sceneParticles,
        &sceneCarPlayerPrepass,
        &sceneCarPlayer,
        &sceneNearOpaqueCarAlpha,
        &sceneNearCarAlpha,
        &sceneNearAfterCarAlpha,
        &sceneCarPlayerPPCarAlpha,
        &sceneCarPlayerCarAlpha
    };

    for (uint32_t uVar18 = 0; uVar18 < 0xc; uVar18++) {
        FogDesc& peVar5 = local_a0[uVar18]->getFogDescWrite();

        peVar5.DistanceStart = 0.0f;
        peVar5.Type = eFogType::FT_Linear;
        peVar5.Color.x() = 0.39215687f;
        peVar5.Color.y() = 0.5764706f;
        peVar5.Color.z() = 0.7882353f;
        peVar5.DistanceEnd = 12500.0f;
        peVar5.Intensity = 1.0f;
    }

    pSunDownscaleChain[2]->setName("SunDownSample"); // DAT_00f080d0
    pSunAvgLumCopy->setName("SunEyeLumBk");
    pSunDazzleLUTCopy->setName("SunDazzleBk");

    frameGraph->insertAfterObject(pSunDownscaleChain[2], renderPasses[eRenderPass::RP_EnvMapMini].pScene);
    frameGraph->insertAfterObject(pSunAvgLumCopy, pSunDownscaleChain[2]);
    frameGraph->insertAfterObject(pSunDazzleLUTCopy, pSunAvgLumCopy);
    frameGraph->insertAfterObject(pPostFXMainHDRDownscale, renderPasses[eRenderPass::RP_CarCompo].pScene);

    pPostFXMainHDRDownscale->setName("DownSample4X");
    pPostFXMainHDRDownscale->setEnabled(false);

    setupScenes(nearPlane, farPlane);

    return bVar1;
}

bool GSRender::setupSunPostFXStack()
{
    pPostFXMainHDR = new (TestDrive::Alloc(sizeof(PostFXNodeTexSample))) PostFXNodeTexSample();
    pPostFXMainHDRDownscale = new (TestDrive::Alloc(sizeof(PostFXNodeDownscale))) PostFXNodeDownscale();
    
    pSunAvgLum = new (TestDrive::Alloc(sizeof(PostFXNodeTexSample))) PostFXNodeTexSample();
    pSunAvgLumCopy = new (TestDrive::Alloc(sizeof(PostFXNodeBlit))) PostFXNodeBlit();

    pSunDazzleLUT = new (TestDrive::Alloc(sizeof(PostFXNodeTexSample))) PostFXNodeTexSample();
    pSunDazzleLUTCopy = new (TestDrive::Alloc(sizeof(PostFXNodeBlit))) PostFXNodeBlit();
        
    pSunBillboard = new (TestDrive::Alloc(sizeof(PostFXNodeTexSample))) PostFXNodeTexSample();

    for ( uint32_t i = 0; i < kNumSunDownscaleRT; i++ ) {
        pSunDownscaleChain[i] = new (TestDrive::Alloc(sizeof(PostFXNodeDownscale))) PostFXNodeDownscale();
    }
    
    pSunAvgLumCopy->connect(pSunAvgLum, 0, 0);
    pSunDazzleLUTCopy->connect(pSunDazzleLUT, 0, 0);

    pSunDownscaleChain[0]->connect(pSunBillboard, 0, 0);
    pSunDownscaleChain[1]->connect(pSunDownscaleChain[0], 0, 0);
    pSunDownscaleChain[2]->connect(pSunDownscaleChain[1], 0, 0);

    // FUN_00990180 (inlined)
    pPostFXMainHDR->setTextureToSample(pMainRT);
    pPostFXMainHDRDownscale->setEnabled(true);
    
    pPostFXMainHDRDownscale->connect(pPostFXMainHDR, 0, 0);
    pPostFXMainHDRDownscale->setOutput(pScnDown4RT, 0);

    pSunAvgLum->setTextureToSample(pSunRTs[4]);
    pSunDazzleLUT->setTextureToSample(pSunRTs[6]);

    pSunAvgLumCopy->setOutput(pSunRTs[5], 0);
    pSunDazzleLUTCopy->setOutput(pSunRTs[7], 0);

    pSunAvgLumCopy->setBlendOP(eBlendOP::BO_Disabled);
    pSunDazzleLUTCopy->setBlendOP(eBlendOP::BO_Disabled);
    
    pSunBillboard->setTextureToSample(pSunRTs[0]);
    
    pSunDownscaleChain[0]->setOutput(pSunRTs[1], 0);
    pSunDownscaleChain[1]->setOutput(pSunRTs[2], 0);
    pSunDownscaleChain[2]->setOutput(pSunRTs[3], 0);
    
    return true;
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
                                pSunFramebuffer->pAttachments[0] = pUnknownRT;
                                
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

void GSRender::setupRenderPasses()
{
    // FUN_00990370
    renderPasses = {
        /*          pScene | pCamera | pViewport | pFramebufferAttachments | pLights | Flags | ShaderID | Name | pCopy | bIs3DScene | bEnabled */
        RenderPass{ &sceneNoiseComposeSlow, &camera2D, &viewportNoise, pNoiseCompositeFramebuffer, nullptr, 0, 0, "NOISE_COMP_S", nullptr, false, true }, // DAT_00f08500
        RenderPass{ &sceneNoiseAssembleSlow, &camera2D, &viewportNoise, pNoiseAssembleSFramebuffer, nullptr, 0, 0, "NOISE_ASS_S", nullptr, false, true }, // DAT_00f084f4
        RenderPass{ &sceneNoiseComposeFast, &camera2D, &viewportNoise, pNoiseCompositeFramebuffer, nullptr, 0, 0, "NOISE_COMP_F", nullptr, false, true }, // DAT_00f084e4
        RenderPass{ &sceneNoiseAssembleFast, &camera2D, &viewportNoise, pNoiseAssembleFFramebuffer, nullptr, 0, 0, "NOISE_ASS_F", nullptr, false, true }, // DAT_00f084d8
        RenderPass{ &sceneOceanNmap, &camera2D, &viewportNoise, pOceanNMapFramebuffer, nullptr, 0, 0, "OCEAN_NMAP", nullptr, false, true }, // DAT_00f084cc
        RenderPass{ &sceneAVTLRFront[0], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[0], &lightsLRFront, 0, 0, "AVT LR FRONT 0", nullptr, false, false }, // DAT_00f084bc
        RenderPass{ &sceneAVTLRFront[1], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[1], &lightsLRFront, 0, 0, "AVT LR FRONT 1", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[2], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[2], &lightsLRFront, 0, 0, "AVT LR FRONT 2", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[3], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[3], &lightsLRFront, 0, 0, "AVT LR FRONT 3", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[4], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[4], &lightsLRFront, 0, 0, "AVT LR FRONT 4", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[5], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[5], &lightsLRFront, 0, 0, "AVT LR FRONT 5", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[6], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[6], &lightsLRFront, 0, 0, "AVT LR FRONT 6", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[7], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[7], &lightsLRFront, 0, 0, "AVT LR FRONT 7", nullptr, false, false },
        RenderPass{ &sceneAVTLRFront[8], &cameraLRFront, &viewportLRFront, pRTArrayAVTLRFront[8], &lightsLRFront, 0, 0, "AVT LR FRONT 8", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[0], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[0], &lightsLRBack, 0, 0, "AVT LR BACK 0", nullptr, false, false }, // DAT_00f0842c
        RenderPass{ &sceneAVTLRBack[1], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[1], &lightsLRBack, 0, 0, "AVT LR BACK 1", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[2], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[2], &lightsLRBack, 0, 0, "AVT LR BACK 2", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[3], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[3], &lightsLRBack, 0, 0, "AVT LR BACK 3", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[4], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[4], &lightsLRBack, 0, 0, "AVT LR BACK 4", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[5], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[5], &lightsLRBack, 0, 0, "AVT LR BACK 5", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[6], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[6], &lightsLRBack, 0, 0, "AVT LR BACK 6", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[7], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[7], &lightsLRBack, 0, 0, "AVT LR BACK 7", nullptr, false, false },
        RenderPass{ &sceneAVTLRBack[8], &cameraLRBack, &viewportLRBack, pRTArrayAVTLRBack[8], &lightsLRBack, 0, 0, "AVT LR BACK 8", nullptr, false, false },
        RenderPass{ &sceneHUDMap, &camera2D, &viewportHUDMap, pHUDMapFramebuffer, nullptr, 0, 0, "RT HUD MAP", nullptr, false, false },
        RenderPass{ &sceneShadowClear, &cameraShadow, &viewportShadowClear, pShadowFramebuffer, nullptr, 0x20, 0, "RT SHADOW CLR", nullptr, false, false },
        RenderPass{ &sceneShadow, &cameraShadow, &viewportShadow, pShadowFramebuffer, nullptr, 0x0, 0, "RT SHADOW", nullptr, false, false },
        RenderPass{ &sceneShadowCockpitClear, &cameraShadowCockpit, &viewportShadowClear, pShadowCockpitFramebuffer, nullptr, 0x20, 0, "RT SHADOW COCKPIT CLR", nullptr, false, false },
        RenderPass{ &sceneShadowCockpit, &cameraShadowCockpit, &viewportShadow, pShadowCockpitFramebuffer, nullptr, 0x0, 0, "RT SHADOW COCKPIT", nullptr, false, false },
        RenderPass{ &sceneEnvMapBack, &cameraEnvMapBack, &viewportEnvmapBack, pEnvmapBackFramebuffer, &lightsBack, 0x20, 0, "ENVMAP BACK", &sceneBack, true, false },
        RenderPass{ &sceneEnvMap, &cameraEnvMap, &viewportEnvmapNear, pEnvmapBackFramebuffer, &lightsEnvmap, 0x0, 0, "ENVMAP", nullptr, true, false },
        RenderPass{ &sceneEnvMapMini, &camera2D, &viewportEnvmapNear, pEnvmapMiniFramebuffer, nullptr, 0x0, 0, "ENVMAP MINI", nullptr, false, false },
        RenderPass{ &sceneSunComputeEyeLum, &camera2D, &viewportSun, pSunEyeLumFramebuffer, nullptr, 0x0, 0, "SUN_COMPUTE_EYE_LUM", nullptr, false, true },
        RenderPass{ &sceneSunComputeDazzleCoeff, &camera2D, &viewportSun, pSunDazzleCoeffFramebuffer, nullptr, 0x0, 0, "SUN_COMPUTE_DAZZLE_COEFF", nullptr, false, true },
        RenderPass{ &sceneOcclusion, &cameraMain, &viewportBack, pMainRTwMFramebuffer, nullptr, 0x60, 0, "OCCLUSION", nullptr, true, true },
        RenderPass{ &sceneBack, &cameraMain, &viewportBack, pMainRTwMFramebuffer, &lightsBack, 0x20, 0, "BACK", nullptr, true, true },
        RenderPass{ &sceneBack2D, &camera2D, &viewportNear, pMainRTwMFramebuffer, &lightsBack, 0x20, 0, "BACK2D", nullptr, false, true },
        RenderPass{ &scenePreNear, &camera2D, &clearColorStencilViewport, pMainRTwMFramebuffer, &lightsDefault, 0x20, 0, "PRE_NEAR", &sceneNear, true, false },
        RenderPass{ &sceneClearDepth, &cameraMain, &clearDepthViewport, pMainRTwMFramebuffer, nullptr, 0x20, 0, "CLEAR_ZSTENCIL", nullptr, true, true },
        RenderPass{ &sceneReflectPrepass, &cameraReflection, &viewportNear, pMainRTwMFramebuffer, &lightsDefault, 0x29, 0, "REFLECT PRE PASS", &sceneReflect, true, false },
        RenderPass{ &sceneReflect, &cameraReflection, &viewportNear, pMainRTwMFramebuffer, &lightsDefault, 0x21, 0, "REFLECT", nullptr, true, false },
        RenderPass{ &sceneNearOpaque, &cameraMain, &viewportNear, pMainRTwMFramebuffer, &lightsDefault, 0x20, 0, "NEAR_OPAQUE", &sceneNear, true, true },
        RenderPass{ &sceneNearNoFog, &cameraMain, &viewportNear, pMainRTwMFramebuffer, &lightsDefault, 0x20, 0, "NEAR_NO_FOG", nullptr, true, true },
        RenderPass{ &sceneNearPrePassVegetation, &cameraMain, &viewportNear, pMainRTwMFramebuffer, &lightsDefault, 0x28, 0, "VEGET PRE PASS", &sceneNear, true, true },
        RenderPass{ &sceneNear, &cameraMain, &viewportNear, pMainRTwMFramebuffer, &lightsDefault, 0x20, 0, "NEAR", nullptr, true, true },
        RenderPass{ &sceneNearOpaqueCarAlpha, &cameraMain, &clearColorViewport, pCarFramebuffer, &lightsDefault, 0x20, 0, "NEAR_OPAQ_CA", &sceneNearCarAlpha, true, false },
        RenderPass{ &sceneNearCarAlpha, &cameraMain, &defaultViewport, pCarFramebuffer, &lightsDefault, 0, 0, "NEAR_CA", nullptr, true, false },
        RenderPass{ &sceneNearCompositing, &cameraMain, &defaultViewport, pMainRTwMFramebuffer, nullptr, 0x30, 0, "NEAR_COMPO", nullptr, false, false },
        RenderPass{ &sceneNearAfterCarAlpha, &cameraMain, &defaultViewport, pMainRTwMFramebuffer, &lightsDefault, 0x20, 0, "NEAR_AFTER_CA", nullptr, true, true },
        RenderPass{ &sceneSunCore, &camera2D, &defaultViewport, pMainRTwMFramebuffer, nullptr, 0x20, 0, "SUN", nullptr, false, true },
        RenderPass{ &sceneOcclusionDOFPrepass, &cameraMain, &defaultViewport, pMainRTFramebuffer, &lightsDefault, 0x28, 0, "FAKEDOF PP", &sceneOcclusionDOF, true, false },
        RenderPass{ &sceneOcclusionDOF, &cameraMain, &defaultViewport, pMainRTFramebuffer, &lightsDefault, 0x20, 0, "FAKEDOF", nullptr, true, false },
        RenderPass{ &sceneCarPlayerPrepass, &cameraMain, &defaultViewport, pMainRTFramebuffer, &lightsDefault, 0x28, 0, "CARPLAYER PP", &sceneCarPlayer, true, true },
        RenderPass{ &sceneCarPlayer, &cameraMain, &defaultViewport, pMainRTFramebuffer, &lightsDefault, 0, 0, "CARPLAYER", nullptr, true, true },
        RenderPass{ &sceneCarPlayerPPCarAlpha, &cameraMain, &clearColorViewport, pCarFramebuffer, &lightsDefault, 0x28, 0, "CARPLAYER PP_CA", &sceneCarPlayerCarAlpha, true, false },
        RenderPass{ &sceneCarPlayerCarAlpha, &cameraMain, &defaultViewport, pCarFramebuffer, &lightsDefault, 0, 0, "CARPLAYER_CA", nullptr, true, false },
        RenderPass{ &sceneCarPlayerCompositing, &camera2D, &defaultViewport, pMainRTFramebuffer, nullptr, 0x10, 0, "CARCOMPO", nullptr, false, false },
        RenderPass{ &sceneOcclusionCockpit, &cameraMain, &defaultViewport, pSunFramebuffer, nullptr, 0x60, 0, "OCCLUSION_COCKPIT", &sceneCockpit, true, true },
        RenderPass{ &sceneSunMarker, &camera2D, &clearColorViewport, pSunFramebuffer, nullptr, 0, 0, "SUNMARKER", nullptr, false, true },
        RenderPass{ &sceneSunExtractMarker, &camera2D, &viewportSun, pSunRealLumFramebuffer, nullptr, 0, 0, "SUN_COMPUTE_REAL_LUM", nullptr, false, true },
        RenderPass{ &sceneParticles, &cameraMain, &defaultViewport, pMainRTFramebuffer, &lightsDefault, 0x30, 0, "PARTICLES", nullptr, true, true },
        RenderPass{ &sceneCockpit, &cameraMain, &viewportInterior, pMainRTFramebuffer, &lightsDefault, 0x20, 0, "COCKPIT", nullptr, true, true },
        RenderPass{ &sceneCockpitAvatarPrepass, &cameraMain, &viewportCockpitAvatar, pMainRTFramebuffer, &lightsDefault, 0x28, 0, "COCKPIT AVT PP", &sceneCockpit, true, true },
        RenderPass{ &sceneCockpitAvatar, &cameraMain, &viewportCockpitAvatar, pMainRTFramebuffer, &lightsDefault, 0x20, 0, "COCKPIT AVATAR", &sceneCockpit, true, true },
        RenderPass{ &scenePostCockpit, &cameraMain, &viewportCockpitAvatar, pMainRTFramebuffer, &lightsDefault, 0x20, 0, "POST COCKPIT", nullptr, true, false },
        RenderPass{ &sceneHelmet, &cameraHelmet, &viewportHelmet, pMainRTFramebuffer, &lightsDefault, 0, 0, "HELMET", nullptr, true, true },
        RenderPass{ &sceneCockpitCarAlpha, &cameraMain, &clearColorDepthViewport, pCarFramebuffer, &lightsDefault, 0x20, 0, "COCKPIT CA", nullptr, true, false },
        RenderPass{ &sceneCockpitAvatarPrepassCarAlpha, &cameraMain, &defaultViewport, pCarFramebuffer, &lightsDefault, 0x28, 0, "COCKPIT AVT PP CA", &sceneCockpitCarAlpha, true, false },
        RenderPass{ &sceneCockpitAvatarCarAlpha, &cameraMain, &defaultViewport, pCarFramebuffer, &lightsDefault, 0x20, 0, "COCKPIT AVATAR CA", &sceneCockpitCarAlpha, true, false },
        RenderPass{ &sceneHelmetCarAlpha, &cameraHelmet, &defaultViewport, pCarFramebuffer, &lightsDefault, 0, 0, "HELMET CA", nullptr, true, false },
        RenderPass{ &sceneCockpitCompositing, &camera2D, &defaultViewport, pMainRTFramebuffer, nullptr, 0x10, 0, "COCKPIT COMPO ", nullptr, false, false },
        RenderPass{ &sceneSunHalo, &camera2D, &defaultViewport, pBackbufferFramebuffer, nullptr, 0x20, 0, "SUN_HALO", nullptr, false, true },
        RenderPass{ &sceneSunDazzle, &camera2D, &viewport2D, pBackbufferFramebuffer, nullptr, 0x20, 0, "SUN_DAZZLE", nullptr, false, true },
        RenderPass{ &sceneSpotMap2D, &camera2D, &viewport2D, pBackbufferFramebuffer, &lights2DDefault, 0x20, 0, "SpotMap2D", nullptr, false, true },
        RenderPass{ &sceneBefore2D, &camera2D, &viewport2D, pBackbufferFramebuffer, &lights2DDefault, 0x20, 0, "Before2D", nullptr, false, true },
        RenderPass{ &scene2D, &camera2D, &viewport2D, pBackbufferFramebuffer, &lights2DDefault, 0x20, 0, "2D", nullptr, false, true },
        RenderPass{ &sceneAfter2D, &camera2D, &viewport2D, pBackbufferFramebuffer, &lights2DDefault, 0x20, 0, "After2D", nullptr, false, true }
    };
}
