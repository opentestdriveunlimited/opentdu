#pragma once

#include "game/gs_base.h"
#include "gs_render_helper.h"

#include <Eigen/Dense>

struct RenderTarget;

class GSRender : public GameSystem {
public:
    const char* getName() const override { return "Service : Render"; }
    inline RenderDevice* getRenderDevice() const { return pRenderDevice; }

public:
    GSRender();
    ~GSRender();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

    void beginFrame();
    void endFrame();

private:
    static constexpr int32_t kNumSunRT = 8;

private:
    class RenderDevice* pRenderDevice;

    uint32_t renderWidth;
    uint32_t renderHeight;
    uint32_t frameIndex;

    float aspectRatio;
    float invAspectRatio;

    uint8_t bHDREnabled : 1;
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

    Eigen::Matrix4f projectionMatrix;

    // Glass dirt (implemented but disabled in vanilla TDU)
    // x: Unknown y: Dirt Value z: Dirt Alpha Coef w: Unused
    Eigen::Vector4f glassDirt;

    // Unknown usage; added for the sake of completeness (might get removed
    // in the future if proved useless).
    Eigen::Vector4f unknownVector1;
    Eigen::Vector4f unknownVector2;

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
    float halfFarPlane; // TODO: Not sure what's the point of this (yet)

private:
    void allocateRenderTargets();
    void allocateAtmosphereResources();
};

extern GSRender* gpRender;
