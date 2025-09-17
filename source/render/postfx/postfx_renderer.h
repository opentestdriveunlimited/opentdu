#pragma once

struct GPUShader;
struct GPUBuffer;
struct Material;
class RenderDevice;

#include "render/2dm.h"

class PostFXRenderer {
public:
    PostFXRenderer();
    ~PostFXRenderer();

    bool initialize();

    bool createResources( RenderDevice* pRenderDevice );
    void releaseResources( RenderDevice* pRenderDevice );

    void bindShaders( const GPUShader* pVertexShader = nullptr, const GPUShader* pPixelShader = nullptr );
    void setRenderPassDimensions( float width, float height );

    void drawFullscreenQuad( bool bClearColor, bool bClearDepth, bool bClearStencil, uint32_t stencilMask );

    void bindRasterState( Material* pMaterial );
    void bindDefaultBlendState( Material* param_1 );

private:
    Material* pMaterial;

    void* p2DMBuffer;
    Render2DM render2DM;

    void* pNextParameterAvailable;

    const GPUShader* pActiveVertexShader;
    const GPUShader* pActivePixelShader;

    const GPUShader* pDefaultVertexShader;
    const GPUShader* pDefaultPixelShader;

    GPUBuffer* pScreenQuadVertexBuffer;

    float renderPassDimensions[4];

    GPUTexture* pDepthStencilRT;
    GPUVertexLayout* pVertexLayout;
    GPUVertexLayout* pVertexLayout2;
};

extern PostFXRenderer gPostFXRenderer; // DAT_00f47800

void FUN_005f7060(uint32_t width, uint32_t height, float *pOutput);