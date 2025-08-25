#pragma once

#include "game/mng_base.h"

#include "render/2dm.h"
#include "render/gs_render.h"
#include "render/draw_list.h"

class CarAlphaListener {
public:
    virtual void onAlphaChange() = 0;
};

struct AlphaElement
{
    CarAlphaListener* pListener = nullptr;
    Material* pMaterial = nullptr;
    float* pShaderConstantsVS = nullptr;
    float* pShaderConstantsPS = nullptr;
    float Alpha = 1.0f;
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 1.0f;
    float Height = 1.0f;
    uint32_t Mask = 0;
    void* pAlpha2DMBuffer = nullptr;
    Render2DM Alpha2DM;
    eRenderPass RenderPass = eRenderPass::RP_NearCarAlpha;
    DrawList ComposingDrawList;
};

class MngCarAlpha : public Manager {
public:
    const char* getName() const override { return "MngCarAlpha"; } // FUN_00432610

public:
    MngCarAlpha();
    ~MngCarAlpha();

    virtual bool initialize( TestDriveGameInstance* ) override;
    virtual void tick( float, float )  override;
    virtual void terminate()  override;

    bool bindRenderTargetToElements();
    bool initializeElement(uint32_t param_1);

    void registerMaterial(uint32_t param_2,Material *param_3,bool param_4);
    void freeElement(uint32_t param_1);

private:
    static constexpr const uint32_t kNumAlphaElements = 32;

private:
    uint8_t bEnabled : 1;
    uint32_t stencilMask;
    uint32_t numElements;

    std::array<AlphaElement, kNumAlphaElements> elements;
    std::function<bool(void)> bind2DBsCallback;

private:
    bool bindBitmapsToScene();
};

extern MngCarAlpha* gpMngCarAlpha;
