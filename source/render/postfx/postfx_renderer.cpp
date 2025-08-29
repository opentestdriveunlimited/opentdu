#include "shared.h"
#include "postfx_renderer.h"

#include "render/material.h"
#include "render/gs_render_helper.h"
#include "render/gs_render.h"
#include "render/render_scene.h"
#include "render/shaders/shader_register.h"

PostFXRenderer gPostFXRenderer = {};

PostFXRenderer::PostFXRenderer()
    : pMaterial( nullptr )
    , p2DMBuffer( nullptr )
    , render2DM{}
    , pNextParameterAvailable( nullptr )
    , pActiveVertexShader( nullptr )
    , pActivePixelShader( nullptr )
    , pDefaultVertexShader( nullptr )
    , pDefaultPixelShader( nullptr )
    , pScreenQuadVertexBuffer( nullptr )
    , renderPassDimensions{ 0.0f, 0.0f, 0.0f, 0.0f }
{

}

PostFXRenderer::~PostFXRenderer()
{
    if (gpRender != nullptr && gpRender->getRenderDevice() != nullptr)
    {
        releaseResources(gpRender->getRenderDevice());
    }
}

void PostFXRenderer::createResources(RenderDevice *pRenderDevice)
{
    // FUN_006048f0
    GPUBufferDesc desc;
    desc.BufferType = GPUBufferDesc::Type::BT_VertexBuffer;
    desc.Size = sizeof( float ) * 8;
    desc.Stride = sizeof( float ) * 2;

    pScreenQuadVertexBuffer = pRenderDevice->createBuffer( &desc );
    OTDU_ASSERT_FATAL( pScreenQuadVertexBuffer );

    void* pBufferContent = pRenderDevice->lockBuffer( pScreenQuadVertexBuffer, 0, sizeof( float ) * 8 );
    OTDU_ASSERT( pBufferContent );

    float* pScreenQuadVertices = (float*)pBufferContent;
    *pScreenQuadVertices = -1.0;
    pScreenQuadVertices[1] = -1.0;
    pScreenQuadVertices[2] = -1.0;
    pScreenQuadVertices[3] = 1.0;
    pScreenQuadVertices[4] = 1.0;
    pScreenQuadVertices[5] = -1.0;
    pScreenQuadVertices[6] = 1.0;
    pScreenQuadVertices[7] = 1.0;

    pRenderDevice->unlockBuffer( pScreenQuadVertexBuffer );

    if ( p2DMBuffer == nullptr ) {
        uint32_t uVar3 = Render2DM::CalcSize( 2, 0x20 );
        uVar3 = uVar3 + 0xf & 0xfffffff0;
        p2DMBuffer = TestDrive::Alloc( uVar3 );
    }
    pMaterial = Render2DM::Create( p2DMBuffer, 0xfffffffffffff000, 0, 2, 0x20, 0 );
    
    MaterialShaderParameterArray* peVar5 = ( MaterialShaderParameterArray* )pMaterial->getParameterByIndex( 0 );
    if ( peVar5->NumParameters != 0 ) {
        MaterialShaderParameter* peVar6 = ( MaterialShaderParameter* )peVar5->getParameterDataByIndex( 0 );
        peVar6->Size = 0x10;
        peVar6->bBindToVS = 1;
        peVar6->Format = 2;
        peVar6->Type = 0x7e;

        peVar6 = ( MaterialShaderParameter* )peVar5->getParameterDataByIndex( 1 );
        peVar6->bBindToVS = 1;
        peVar6->Size = 0x10;
        peVar6->Format = 2;
        peVar6->Type = 0x96;
    }

    render2DM.initialize( p2DMBuffer );
    pActiveVertexShader = pMaterial->pVertexShaders[0];
    pActivePixelShader = pMaterial->pPixelShaders[0];

    void* puVar7 = nullptr;

    if ( peVar5->NumParameters != 0 ) {
        uint32_t uVar3 = peVar5->NumParameters;
        if ( uVar3 - 1 < uVar3 ) {
            // NOTE Had to hand translate this (as Ghidra output was non sense)
            //00604AA6 | 8D41 05 | lea eax, dword ptr ds : [ecx + 5] |
            //00604AA9 | C1E0 04 | shl eax, 4 |
            //00604AAC | 03C7 | add eax, edi |
            //00604AAE | 83C0 10 | add eax, 10 |
            uint32_t eax = ( uVar3 - 1 ) + 5;
            eax <<= 4;
            uint8_t* edi = ( uint8_t* )peVar5;
            edi += eax;
            edi += 0x10;

            puVar7 = edi;
        } else {
            puVar7 = (void*)0x10; // TODO: I assume this could be int based on param type?
        }
    }

    pNextParameterAvailable = puVar7;
    bindRasterState( nullptr );

    pDefaultVertexShader = gShaderRegister.getShader( pRenderDevice, eShaderType::ST_Vertex, 0xfffffffffffff000 );
    pDefaultPixelShader = gShaderRegister.getShader( pRenderDevice, eShaderType::ST_Pixel, 0x0ffffffffffff000 );
}

void PostFXRenderer::releaseResources(RenderDevice *pRenderDevice)
{
    // FUN_00604b90
    if ( p2DMBuffer != nullptr ) {
        pMaterial = nullptr;
        render2DM.destroy();
        
        TestDrive::Free( p2DMBuffer );
    }

    pNextParameterAvailable = nullptr;

    pDefaultVertexShader= nullptr;
    pDefaultPixelShader = nullptr;
    
    pActiveVertexShader = nullptr;
    pActivePixelShader = nullptr;
        
    if (pScreenQuadVertexBuffer != nullptr) {
        pRenderDevice->destroyBuffer( pScreenQuadVertexBuffer );
        pScreenQuadVertexBuffer = nullptr;
    }
}

void PostFXRenderer::bindShaders(const GPUShader *pVertexShader, const GPUShader *pPixelShader)
{
    // FUN_006048c0
    if (pMaterial != nullptr) {
        if (pVertexShader == nullptr) {
            pVertexShader = pActiveVertexShader;
        }

        if (pPixelShader == nullptr) {
            pPixelShader = pActivePixelShader;
        }

        pMaterial->pVertexShaders[0] = pVertexShader;
        pMaterial->pPixelShaders[0] = pPixelShader;
    }
}

void PostFXRenderer::setRenderPassDimensions(float width, float height)
{
    // FUN_00604df0
    renderPassDimensions[0] = width;
    renderPassDimensions[1] = height;
    renderPassDimensions[2] = 0.0f;
    renderPassDimensions[3] = 0.0f;
}

void PostFXRenderer::drawFullscreenQuad(bool bClearColor, bool bClearDepth, bool bClearStencil, uint32_t stencilMask)
{
    // FUN_00604c00
    if (stencilMask == 0xffffffff) {
        pMaterial->StencilEnable = false;
    } else {
        pMaterial->StencilEnable = '\x01';
        pMaterial->StencilFail = '\0';
        pMaterial->StencilZFail = '\0';
        pMaterial->StencilPass = '\0';
        pMaterial->StencilFunc = '\x02';
        pMaterial->StencilRef = 0xffffffff;
        pMaterial->StencilMask = stencilMask;
        pMaterial->StencilWriteMask = 0;
    }
    
    bool bVar1 = false;
    if (gpActiveRenderScene != nullptr && ((gpActiveRenderScene->getFlags() >> 3 & 1) != 0)) {
        gpActiveRenderScene->setFlags(gpActiveRenderScene->getFlags() & 0xfffffff7);
        bVar1 = true;
    }

    gpRender->getRenderDevice()->bindMaterial(pMaterial);

    if (bVar1) {
        gpActiveRenderScene->setFlags(gpActiveRenderScene->getFlags() | 8);
    }

    if (bClearColor) {
        gpRender->getRenderDevice()->clearFramebuffer(bClearColor, bClearDepth, bClearStencil);
    }
    
    gpRender->getRenderDevice()->beginRenderPass();
    gpRender->getRenderDevice()->bindVertexBuffer(pScreenQuadVertexBuffer, 0, 8);
    gpRender->getRenderDevice()->draw(4, 1);
}

void PostFXRenderer::bindRasterState(Material *param_2)
{
    // FUN_00604790
    if ( pMaterial != nullptr ) {
        if ( param_2 == nullptr ) {
            pMaterial->DepthWrite = '\x02';
            pMaterial->SrcBlend = '\x02';
            pMaterial->FillMode = '\0';
            pMaterial->AlphaRef = '\0';
            pMaterial->StencilEnable = '\0';
            pMaterial->DepthTest = '\x03';
            pMaterial->BlendOP = '\x06';
            pMaterial->DstBlend = '\x01';
            pMaterial->BackfaceCullingMode = '\x01';
            pMaterial->AlphaTest = '\x01';
            return;
        }
        pMaterial->FillMode = param_2->FillMode;
        pMaterial->DepthTest = param_2->DepthTest;
        pMaterial->DepthWrite = param_2->DepthWrite;
        pMaterial->BackfaceCullingMode = param_2->BackfaceCullingMode;
        pMaterial->AlphaTest = param_2->AlphaTest;
        pMaterial->AlphaRef = param_2->AlphaRef;
        pMaterial->BlendOP = param_2->BlendOP;
        pMaterial->SrcBlend = param_2->SrcBlend;
        pMaterial->DstBlend = param_2->DstBlend;
        pMaterial->StencilEnable = param_2->StencilEnable;
        pMaterial->StencilFail = param_2->StencilFail;
        pMaterial->StencilZFail = param_2->StencilZFail;
        pMaterial->StencilPass = param_2->StencilPass;
        pMaterial->StencilFunc = param_2->StencilFunc;
        pMaterial->StencilRef = param_2->StencilRef;
        pMaterial->StencilMask = param_2->StencilMask;
        pMaterial->StencilWriteMask = param_2->StencilWriteMask;
        pMaterial->StencilFailCW = param_2->StencilFailCW;
        pMaterial->StencilZFailCW = param_2->StencilZFailCW;
        pMaterial->StencilPassCW = param_2->StencilPassCW;
        pMaterial->StencilFuncCW = param_2->StencilFuncCW;
        pMaterial->StencilRefCW = param_2->StencilRefCW;
        pMaterial->StencilMaskCW = param_2->StencilMaskCW;
        pMaterial->StenwilWriteMaskCW = param_2->StenwilWriteMaskCW;
    }
}

void PostFXRenderer::bindDefaultBlendState(Material *param_1)
{
    if (param_1 == nullptr) {
        return;
    }
   
    param_1->DepthWrite = '\x02';
    param_1->SrcBlend = '\x02';
    param_1->FillMode = '\0';
    param_1->DepthTest = '\x03';
    param_1->BlendOP = '\x06';
    param_1->DstBlend = '\x01';
    param_1->BackfaceCullingMode = '\x01';
    param_1->AlphaTest = '\x01';
    param_1->AlphaRef = '\0';
    param_1->StencilEnable = '\0';
}

void FUN_005f7060(uint32_t width, uint32_t height, float *pOutput)
{
    // Looks like some kind of matrix for downscaling?
    float fVar1;
    float fVar2;
    int iVar3;
    float fVar4;

    fVar1 = (float)width;
    if (width < 0) {
        fVar1 = fVar1 + 4.2949673e+09f;
    }
    fVar1 = 1.0f / fVar1;
    fVar2 = (float)height;
    if (height < 0) {
        fVar2 = fVar2 + 4.2949673e+09f;
    }

    iVar3 = 0;
    do {
        *pOutput = fVar1 * -1.5f;
        fVar4 = ((float)iVar3 - 1.5f) * (1.0f / fVar2);
        pOutput[1] = fVar4;
        pOutput[4] = fVar1 * -0.5f;
        pOutput[5] = fVar4;
        pOutput[8] = fVar1 * 0.5f;
        pOutput[9] = fVar4;
        pOutput[0xc] = fVar1 * 1.5f;
        pOutput[0xd] = fVar4;
        iVar3 = iVar3 + 1;
        pOutput = pOutput + 0x10;
    } while (iVar3 < 4);
}
