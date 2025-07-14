#pragma once

#include "game/gs_base.h"

#include "world/world_listener.h"
#include "gs_render_helper.h"
#include "viewport.h"
#include "light.h"
#include "render_scene.h"

struct RenderTarget;
class RenderScene;
class CameraGame;

struct RenderPass {
    RenderScene* pScene;
    CameraGame* pCamera;
    Viewport* pViewport;
    std::vector<RenderTarget*>* pFramebufferAttachments;
    std::vector<Light*>* pLights;
    uint32_t Flags;
    int32_t ShaderIndex;
    std::string Name;
    RenderScene* pSceneCopy;
    uint8_t bIs3DScene : 1;
    uint8_t bEnabled : 1;

    RenderPass();
};

class GSRender : public GameSystem, WorldListener {
public:
    const char* getName() const override { return "Service : Render"; }
    inline RenderDevice* getRenderDevice() const { return pRenderDevice; }

    inline float getDefaultNearPlane() const { return nearPlane; }
    inline float getDefaultFarPlane() const { return farPlane; }
    inline float getDefaultInteriorNearPlane() const { return interiorNearPlane; }
    inline float getDefaultInteriorFarPlane() const { return interiorFarPlane; }

    inline RenderPass& getRenderPassByIndex( const uint32_t index ) { return renderPasses.at(index); }

public:
    GSRender();
    ~GSRender();

    bool initialize( TestDriveGameInstance* ) override;
    void tick(float deltaTime) override;
    void terminate() override;

    void beginFrame();
    void endFrame();

    void setLODQuality( const int32_t qualityIndex );

    virtual void onWeatherConfigChange(WeatherConfig* param_1 ) override;

    void flushDrawCommands(bool param_1);

private:
    static constexpr int32_t kNumSunRT = 8;

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

    uint32_t renderWidth;
    uint32_t renderHeight;
    uint32_t frameIndex;

    float aspectRatio;
    float invAspectRatio;
    float time;

    uint8_t bHDREnabled : 1;
    uint8_t bWasHDREnabled : 1;
    uint8_t bFrameRecordingInProgress : 1;
    uint8_t bFovVsLodOn : 1;
    uint8_t bInitialized : 1;

    GPUTexture* mainRT; // SDR/HDR (+MSAA or regular)
    GPUTexture* scnDown4;

    GPUTexture* sunRenderTargets[kNumSunRT];

    GPUTexture* noiseComposite;
    GPUTexture* noiseAssembleF;
    GPUTexture* noiseAssembleS;
    GPUTexture* oceanNMap;

    // 0
    // 1 - Wind Params
    // 2 - Terrain Params
    Eigen::Vector4f shaderUniforms[8];
    Eigen::Matrix4f projectionMatrix;

    AmbientLight defaultAmbientLight;
    DirectionalLight defaultDirectionalLight;

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

    AmbientLight ambientLightBack;
    DirectionalLight directionalLightBack;
    std::vector<Light*> lightsBack;
    
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

private:
    void allocateRenderTargets();
    void allocateAtmosphereResources();

    void updateWeatherParams();
};

extern GSRender* gpRender;
