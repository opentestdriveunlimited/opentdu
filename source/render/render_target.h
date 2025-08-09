#pragma once

#include "gs_render_helper.h"

struct GPUTexture;
class RenderDevice;
class Render2DB;

// TODO: Should be renamed "PooledRenderTarget"
class RenderTarget 
{
public:
    RenderTarget* pNextRT; // Used for linked list traversal

public:
    RenderTarget();
    ~RenderTarget();

    void destroy( RenderDevice* pDevice );
    void bind2DB( Render2DB* param_1 );

    bool create();
    bool create(float width, float height, const eViewFormat viewFormat, const eAntiAliasingMethod aaMethod, const uint32_t texFlags);
    bool createFrom2DB( Render2DB* param_2, uint32_t uStack_4 );
    bool createWithCopy( RenderTarget* param_2, const uint32_t param_3 );
    bool createFromBackbuffer();

    eViewFormat getFormat() const;
    void setFlagsUnknown(const uint32_t param_1);

    uint32_t getWidth();
    uint32_t getHeight();

    eAntiAliasingMethod getAntiAliasingMethod() const;

    static RenderTarget* GetBackBuffer();

    Render2DB* getBound2DB();
    GPUTexture* getTextureColor();

private:
    float width;
    float height;

    // Only valid if the rendertarget has been created with flag 0x4 set
    float widthNoPadding;
    float heightNoPadding;

    eViewFormat format;
    eAntiAliasingMethod antiAliasing;
    Render2DB* p2DB;
    uint32_t flags;
    uint32_t flagsUnknown;
    eShaderType boundStage;

    GPUTexture* pTexture;
    GPUTexture* pDepthBuffer; // TODO: Is it a D3D9 or engine quirk?

    RenderTarget* pParentRT; // Used by RT Copy func

    uint8_t bOwnTexture : 1;
    uint8_t bOwnZBuffer : 1;
};

static constexpr uint32_t kMaxNumAttachments = 4; // Based on D3D9 limits (could be raised in the future)

struct FramebufferAttachments {
    RenderTarget* pAttachments[kMaxNumAttachments] = { nullptr, nullptr, nullptr, nullptr };
};

class RenderTargetPool {
public:
    RenderTargetPool();
    ~RenderTargetPool();

    RenderTarget* allocateUninitialized();
    RenderTarget* allocate(const float width, const float height, const eViewFormat format, const uint32_t flags);
    RenderTarget* allocateFromOriginal(RenderTarget* param_1, const uint32_t flags);
    RenderTarget* allocateFrom2DB(Render2DB* param_1, const uint32_t flags);

    void removeFromPool( RenderTarget* param_1 );

private:
    RenderTarget*   pListHead; // DAT_00fe7a90
    uint32_t        numPooledRT; // DAT_00fe7a94

private:
    RenderTarget* createAndAddToPool();
};
 
RenderTarget* CreateRenderTarget(const uint32_t width, const uint32_t height, const eViewFormat format, const eAntiAliasingMethod aaMethod, const uint32_t flags );
RenderTarget* CopyRenderTarget(RenderTarget* pSource, const uint32_t flags);
bool CreateBackbufferRenderTarget(); // FUN_005f15f0
void ReleaseBackbufferRenderTarget(); // FUN_005f1650
RenderTarget* FUN_0050aff0(RenderTarget *param_1, float param_2, float param_3, eViewFormat param_4, uint32_t param_5, uint32_t flags);
RenderTarget* CreateRenderTargetFrom2DB(Render2DB* param_1, uint32_t param_2);