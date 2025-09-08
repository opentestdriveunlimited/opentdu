#include "shared.h"
#include "postfx_downscale.h"

#include "render/postfx/postfx_renderer.h"
#include "render/render_target.h"
#include "render/gs_render.h"
#include "render/gs_render_helper.h"

PostFXDownscale::PostFXDownscale(PostFXNodeDownscale *pOwner)
    : PostFX(pOwner)
{

}

PostFXDownscale::~PostFXDownscale()
{

}

bool PostFXDownscale::execute()
{
    bool bVar1 = PostFX::execute();
    if (!bVar1) {
        return false;
    }
    
    RenderTarget* peVar9 = nullptr;
    PostFX* peVar5 = pOwner->getInputNode(0);
    if (peVar5 != nullptr) {
        uint32_t uVar6 = pOwner->getInputLinkIndex(0);
        peVar9 = peVar5->getOutput(uVar6);
    }

    gPostFXRenderer.bindShaders( nullptr, pPixelShader );

    uint32_t uVar6 = pRenderTargets[0]->getHeight();
    uint32_t uVar7 = pRenderTargets[0]->getWidth();
    gPostFXRenderer.setRenderPassDimensions( (float)uVar7, (float)uVar6 );

    uVar6 = peVar9->getHeight();
    uVar7 = peVar9->getWidth();

    float pShaderConstants[0x10];
    FUN_005f7060(uVar7,uVar6,pShaderConstants);

    uint32_t reg_idx = 0xf; // TODO: Retrieve this from metadata // (pPixelShader->PSFloatConstantMap).pConstants[0x254];
    gpRender->getRenderDevice()->setFloatConstants( eShaderType::ST_Pixel, pShaderConstants, reg_idx, 0x10 );
    gpRender->getRenderDevice()->bindRenderTargetAndSetViewport( pRenderTargets[0], 0 );

    Texture* peVar8 = peVar9->getBound2DB()->getFirstBitmap();
    gpRender->getRenderDevice()->bindTexture(peVar8, 0);

    uVar6 = pOwner->getUnknownMask();
    if (uVar6 != 0xffffffff) {
        uVar7 = 1 << ((uint8_t)uVar6 & 0x1f);
    } else {
        uVar7 = 0xffffffff;
    }

    gPostFXRenderer.drawFullscreenQuad((uVar6 != 0xffffffff),false,false,uVar7);
    gPostFXRenderer.bindShaders();

    if ((pOwner->getFlags() & 1) != 0) {
        pRenderTargets[0]->resolveMSAA(0);
    }

    return true;
}

uint64_t PostFXDownscale::getPixelShaderHashcode() const
{
    return 0x6c6cdc43c72bc9da;
}

PostFXNodeDownscale::PostFXNodeDownscale()
    : PostFXNode(1, 1)
{
    // FUN_0050d120
    pPostFX = new (TestDrive::Alloc(sizeof(PostFXDownscale))) PostFXDownscale(this);
}

PostFXNodeDownscale::~PostFXNodeDownscale()
{
    TestDrive::Free(pPostFX);
}

uint32_t PostFXNodeDownscale::getType() const
{
    return 0x1b;
}
