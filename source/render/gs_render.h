#pragma once

#include "game/gs_base.h"

#include "world/world_listener.h"
#include "gs_render_helper.h"
#include "light.h"
#include "render_scene.h"
#include "2db.h"

#include "camera/camera_base.h"
#include "camera/camera_game.h"

class RenderTarget;
class RenderScene;
struct FramebufferAttachments;
class PostFXNodeTexSample;
class PostFXNodeDownscale;
class PostFXNodeBlit;
class FrameGraph;

struct RenderPass {
    RenderScene* pScene;
    Camera* pCamera;
    Viewport* pViewport;
    FramebufferAttachments* pFramebufferAttachments;
    LightSetupNode* pLightSetupNode;
    uint32_t Flags;
    int32_t ShaderIndex;
    std::string Name;
    RenderScene* pSceneCopy;
    uint8_t bIs3DScene : 1;
    uint8_t bEnabled : 1;

    //RenderPass();
};

enum eRenderPass : uint32_t {
    RP_NoiseCompS                       = 0x0,
    RP_NoiseAssS                        = 0x1,
    RP_NoiseCompF                       = 0x2,
    RP_NoiseAssF                        = 0x3,
    RP_OceanNMap                        = 0x4,
    RP_AvtLRFront0                      = 0x5,
    RP_AvtLRFront1                      = 0x6,
    RP_AvtLRFront2                      = 0x7,
    RP_AvtLRFront3                      = 0x8,
    RP_AvtLRFront4                      = 0x9,
    RP_AvtLRFront5                      = 0xa,
    RP_AvtLRFront6                      = 0xb,
    RP_AvtLRFront7                      = 0xc,
    RP_AvtLRFront8                      = 0xd,
    RP_AvtLRBack0                       = 0xe,
    RP_AvtLRBack1                       = 0xf,
    RP_AvtLRBack2                       = 0x10,
    RP_AvtLRBack3                       = 0x11,
    RP_AvtLRBack4                       = 0x12,
    RP_AvtLRBack5                       = 0x13,
    RP_AvtLRBack6                       = 0x14,
    RP_AvtLRBack7                       = 0x15,
    RP_AvtLRBack8                       = 0x16,
    RP_HUDMap                           = 0x17,
    RP_ShadowClear                      = 0x18,
    RP_Shadow                           = 0x19,
    RP_ShadowCockpitClear               = 0x1a,
    RP_ShadowCockpit                    = 0x1b,
    RP_EnvMapBack                       = 0x1c,
    RP_EnvMap                           = 0x1d,
    RP_EnvMapMini                       = 0x1e,
    RP_SunComputeEyeLum                 = 0x1f,
    RP_SunComputeDazzleCoeff            = 0x20,
    RP_Occlusion                        = 0x21,
    RP_Back                             = 0x22,
    RP_Back2D                           = 0x23,
    RP_PreNear                          = 0x24,
    RP_ClearZStencil                    = 0x25,
    RP_ReflectPrePass                   = 0x26,
    RP_Reflect                          = 0x27,
    RP_NearOpaque                       = 0x28,
    RP_NearNoFog                        = 0x29,
    RP_VegetationPrePass                = 0x2a,
    RP_Near                             = 0x2b,
    RP_NearOpaqueCarAlpha               = 0x2c,
    RP_NearCarAlpha                     = 0x2d,
    RP_NearCompo                        = 0x2e,
    RP_NearAfterCarAlpha                = 0x2f,
    RP_Sun                              = 0x30,
    RP_FakeDOFPrePass                   = 0x31,
    RP_FakeDOF                          = 0x32,
    RP_CarPlayerPrePass                 = 0x33,
    RP_CarPlayer                        = 0x34,
    RP_CarPlayerPrePassCarAlpha         = 0x35,
    RP_CarPlayerCarAlpha                = 0x36,
    RP_CarCompo                         = 0x37,
    RP_OcclusionCockpit                 = 0x38,
    RP_SunMarker                        = 0x39,
    RP_SunComputeRealLuminosity         = 0x3a,
    RP_Particles                        = 0x3b,
    RP_Cockpit                          = 0x3c,
    RP_CockpitAvtPrePass                = 0x3d,
    RP_CockpitAvatar                    = 0x3e,
    RP_PostCockpit                      = 0x3f,
    RP_Helmet                           = 0x40,
    RP_CockpitCarAlpha                  = 0x41,
    RP_CockpitAvtCarAlphaPrePass        = 0x42,
    RP_CockpitAvatarCarAlpha            = 0x43,
    RP_HelmetCarAlpha                   = 0x44,
    RP_CockpitCompo                     = 0x45,
    RP_SunHalo                          = 0x46,
    RP_SunDazzle                        = 0x47,
    RP_SpotMap2D                        = 0x48,
    RP_Before2D                         = 0x49,
    RP_2D                               = 0x4a,
    RP_After2D                          = 0x4b,
};

class GSRender : public GameSystem, WorldListener {
public:
    const char* getName() const override { return "Service : Render"; }
    inline RenderDevice* getRenderDevice() const { return pRenderDevice; }

    inline float getDefaultNearPlane() const { return nearPlane; }
    inline float getDefaultFarPlane() const { return farPlane; }
    inline float getDefaultInteriorNearPlane() const { return interiorNearPlane; }
    inline float getDefaultInteriorFarPlane() const { return interiorFarPlane; }

    template<eRenderPass TPass>
    inline RenderPass& getRenderPass() { return renderPasses.at(TPass); }
    inline FrameGraph* getFrameGraph() { return frameGraph; }

public:
    GSRender();
    ~GSRender();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    void beginFrame();
    void endFrame();

    void setLODQuality( const uint32_t qualityIndex );

    virtual void onWeatherConfigChange(WeatherConfig* param_1 ) override;

    void flushDrawCommands(bool param_1);

private:
    static constexpr int32_t kNumSunRT = 8;
    static constexpr int32_t kNumSunDownscaleRT = 3;
    static constexpr int32_t kNumAvtScenes = 9;

private:
    class RenderDevice* pRenderDevice;

    std::vector<RenderPass> renderPasses;

    eAntiAliasingMethod activeAA;
    uint32_t activeLODQuality;

    Viewport defaultViewport;
    Viewport clearColorViewport;
    Viewport clearViewport;
    Viewport clearColorDepthViewport;
    Viewport clearColorStencilViewport;
    Viewport clearDepthViewport;
    Viewport viewportBack;
    Viewport viewportHUDMap;
    Viewport viewportNoise;
    Viewport viewportShadowClear;
    Viewport viewportShadow;
    Viewport viewportEnvmapBack;
    Viewport viewportEnvmapNear;
    Viewport viewportNear;
    Viewport viewportParticles;
    Viewport viewportSun;
    Viewport viewportInterior;
    Viewport viewportAvatar;
    Viewport viewportHelmet;
    Viewport viewport2D;
    Viewport viewportLRFront;
    Viewport viewportLRBack;
    Viewport viewportCockpitAvatar;

    uint32_t renderWidth;
    uint32_t renderHeight;
    uint32_t frameIndex;
    uint32_t activeRenderLayers;

    float aspectRatio;
    float invAspectRatio;
    float time;

    uint8_t bHDREnabled : 1;
    uint8_t bWasHDREnabled : 1;
    uint8_t bFrameRecordingInProgress : 1;
    uint8_t bFovVsLodOn : 1;
    uint8_t bInitialized : 1;

    RuntimeRender2DB mainRTwM;
    RuntimeRender2DB scnDown4;
    RuntimeRender2DB mainRT;
    RuntimeRender2DB noiseComposite;
    RuntimeRender2DB noiseAssembleF;
    RuntimeRender2DB noiseAssembleS;
    RuntimeRender2DB oceanNMap;

    RenderTarget* pMainRTwM;
    RenderTarget* pMainRT;
    RenderTarget* pScnDown4RT;

    // Backbuffer sized RTs; no idea what they're used for (yet)
    RenderTarget* pUnknownRT;
    RenderTarget* pUnknownRT2;

    RuntimeRender2DB sun2DBs[kNumSunRT];
    RenderTarget* pSunRTs[kNumSunRT];

    RenderTarget* pNoiseCompositeRT;
    RenderTarget* pNoiseAssembleFRT;
    RenderTarget* pNoiseAssembleSRT;

    RenderTarget* pOceanNMapRT;

    FramebufferAttachments* pMainRTFramebuffer;
    FramebufferAttachments* pMainRTwMFramebuffer;
    FramebufferAttachments* pBackbufferFramebuffer;
    FramebufferAttachments* pUnknownRTFramebuffer;
    FramebufferAttachments* pUnknownRT2Framebuffer;

    FramebufferAttachments* pSunFramebuffer;
    FramebufferAttachments* pSunRealLumFramebuffer;
    FramebufferAttachments* pSunEyeLumFramebuffer;
    FramebufferAttachments* pSunDazzleCoeffFramebuffer;

    FramebufferAttachments* pNoiseCompositeFramebuffer;
    FramebufferAttachments* pNoiseAssembleFFramebuffer;
    FramebufferAttachments* pNoiseAssembleSFramebuffer;
    
    FramebufferAttachments* pOceanNMapFramebuffer;

    FramebufferAttachments* pRTArrayAVTLRFront[kNumAvtScenes];
    FramebufferAttachments* pRTArrayAVTLRBack[kNumAvtScenes];

    FramebufferAttachments* pHUDMapFramebuffer;
    FramebufferAttachments* pShadowFramebuffer;
    FramebufferAttachments* pShadowCockpitFramebuffer;
    FramebufferAttachments* pEnvmapBackFramebuffer;
    FramebufferAttachments* pEnvmapMiniFramebuffer;

    FramebufferAttachments* pCarFramebuffer;

    PostFXNodeTexSample*    pPostFXMainHDR;
    PostFXNodeDownscale*    pPostFXMainHDRDownscale;

    PostFXNodeTexSample*    pSunAvgLum;
    PostFXNodeBlit*         pSunAvgLumCopy;
    PostFXNodeTexSample*    pSunDazzleLUT;
    PostFXNodeBlit*         pSunDazzleLUTCopy;
    PostFXNodeTexSample*    pSunBillboard;
    PostFXNodeDownscale*    pSunDownscaleChain[kNumSunDownscaleRT];

    Camera camera2D;
    Camera cameraMain;

    CameraGame cameraLRFront;
    CameraGame cameraLRBack;

    CameraGame cameraShadow;
    CameraGame cameraShadowCockpit;
    Camera cameraEnvMapBack;
    Camera cameraEnvMap;

    Camera cameraReflection;
    Camera cameraHelmet;

    // 0
    // 1 - Wind Params
    // 2 - Terrain Params
    Eigen::Vector4f shaderUniforms[8];
    Eigen::Matrix4f projectionMatrix;

    AmbientLight defaultAmbientLight;
    DirectionalLight defaultDirectionalLight;
    LightSetupNode lightsDefault;

    RenderScene sceneNear;
    RenderScene scenePreNear;
    RenderScene sceneNearOpaque;
    RenderScene sceneCarPlayerPrepass;
    RenderScene sceneNearPrePassVegetation;
    RenderScene sceneParticles;
    RenderScene sceneCarPlayer;
    RenderScene sceneNearOpaqueCarAlpha;
    RenderScene sceneCarPlayerCarAlpha;
    RenderScene sceneNearAfterCarAlpha;
    RenderScene sceneCarPlayerPPCarAlpha;
    RenderScene sceneNoiseComposeSlow;
    RenderScene sceneNearNoFog;
    RenderScene sceneOcclusionDOFPrepass;
    RenderScene sceneOcclusionDOF;
    RenderScene sceneNearCarAlpha;
    RenderScene sceneNoiseAssembleSlow;
    RenderScene sceneNoiseComposeFast;
    RenderScene sceneNoiseAssembleFast;
    RenderScene sceneOceanNmap;
    RenderScene sceneAVTLRFront[kNumAvtScenes];
    RenderScene sceneAVTLRBack[kNumAvtScenes];
    RenderScene sceneHUDMap;
    RenderScene sceneShadowClear;
    RenderScene sceneShadow;
    RenderScene sceneShadowCockpitClear;
    RenderScene sceneShadowCockpit;
    RenderScene sceneEnvMapBack;
    RenderScene sceneEnvMap;
    RenderScene sceneBack;
    RenderScene sceneEnvMapMini;
    RenderScene sceneSunComputeEyeLum;
    RenderScene sceneSunComputeDazzleCoeff;
    RenderScene sceneOcclusion;
    RenderScene sceneBack2D;
    RenderScene sceneClearDepth;
    RenderScene sceneReflectPrepass;
    RenderScene sceneReflect;
    RenderScene sceneNearCompositing;
    RenderScene sceneSunCore;
    RenderScene sceneCarPlayerCompositing;
    RenderScene sceneCockpit;
    RenderScene sceneOcclusionCockpit;
    RenderScene sceneCockpitAvatarPrepassCarAlpha;
    RenderScene sceneCockpitAvatarPrepass;
    RenderScene sceneCockpitCarAlpha;
    RenderScene scenePostCockpit;
    RenderScene sceneCockpitAvatar;
    RenderScene sceneCockpitAvatarCarAlpha;
    RenderScene sceneSunHalo;
    RenderScene sceneSunMarker;
    RenderScene sceneHelmet;
    RenderScene sceneHelmetCarAlpha;
    RenderScene sceneSunExtractMarker;
    RenderScene sceneCockpitCompositing;
    RenderScene sceneSunDazzle;
    RenderScene sceneSpotMap2D;
    RenderScene sceneBefore2D;
    RenderScene scene2D;
    RenderScene sceneAfter2D;

    AmbientLight ambientLightBack;
    DirectionalLight directionalLightBack;
    LightSetupNode lightsBack;
    
    AmbientLight default2DAmbientLight;
    DirectionalLight default2DDirectionalLight;
    LightSetupNode lights2DDefault;

    Eigen::Vector4f lightLRAmbient;
    Eigen::Vector4f lightLRDiffuse;
    Eigen::Vector3f lightLRDirectionFront;
    Eigen::Vector3f lightLRDirectionBack;
    
    DirectionalLight lightLRFront;
    DirectionalLight lightLRBack;
    LightSetupNode lightsLRFront;
    LightSetupNode lightsLRBack;
    
    LightSetupNode lightsEnvmap;

    float globalLODFactor;
    float fovVsLodFactor;
    float fovVsLodFactorStart;
    float fovVsLodFactorEnd;
    float lodFactor;
    float lineMipBias;

    float vegetationUVAFactor;
    float vegetationFactor;
    float vegetationPeriod;
    float treeWindSpeedLimit;
    float vegetationWindSpeedLimit;

    float hmapBeginFadeDist;
    float hmapFadeLength;
    
    float nearPlane;
    float farPlane;
    float interiorNearPlane;
    float interiorFarPlane;

    Eigen::Vector3f sunDirection;
    FrameGraph* frameGraph;

private:
    bool initializeShaderCache();
    bool allocateDeviceResources();

    bool setupSunPostFXStack();

    void setupScenes(float nearZ, float farZ);

    bool allocateRenderTargets();
    bool allocateAtmosphereResources();

    void updateWeatherParams();
    void setupRenderPasses();
};

extern GSRender* gpRender;
