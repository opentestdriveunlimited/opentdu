#include "shared.h"
#include "postfx_blit.h"

#include "render/postfx/postfx_renderer.h"
#include "render/render_target.h"
#include "render/gs_render.h"
#include "render/gs_render_helper.h"
#include "render/material.h"

PostFXBlit::PostFXBlit(PostFXNodeBlit *pOwner)
    : PostFX(pOwner)
{

}

PostFXBlit::~PostFXBlit()
{

}

bool PostFXBlit::execute()
{
    bool bVar1 = PostFX::execute();
    if (!bVar1) {
        return false;
    }
    
    RenderTarget* peStack_e8 = nullptr;
    PostFX* peVar4 = pOwner->getInputNode(0);
    if (peVar4 != nullptr) {
        uint32_t uVar5 = pOwner->getInputLinkIndex(0);
        peStack_e8 = peVar4->getOutput(uVar5);
    }

    Render2DB* peVar7 = peStack_e8->getBound2DB();

    // To save time (and avoid potential issues); let's use built-in blit instead of a gfx based impl from the SM2.0 era...
#if 1
    gpRender->getRenderDevice()->blit(peVar7, pRenderTargets[0], true);
#else
    gPostFXRenderer.bindShaders( nullptr, nullptr );

    uint32_t uVar6 = pRenderTargets[0]->getHeight();
    uint32_t uVar7 = pRenderTargets[0]->getWidth();
    gPostFXRenderer.setRenderPassDimensions( (float)uVar7, (float)uVar6 );

    gpRender->getRenderDevice()->bindRenderTargetAndSetViewport( pRenderTargets[0], 0 );

    Material eStack_e0;
    gPostFXRenderer.bindDefaultBlendState( &eStack_e0 );

    PostFXNodeBlit* peVar3 = (PostFXNodeBlit*)pOwner;
    eStack_e0.BlendOP = peVar3->getBlendOP();
    eStack_e0.SrcBlend = peVar3->getBlendSrc();
    eStack_e0.DstBlend = peVar3->getBlendDst();

    gPostFXRenderer.bindRasterState( &eStack_e0 );

    gpRender->getRenderDevice()->bindTexture(peVar7->getFirstBitmap(), 0);
    
    uVar6 = pOwner->getUnknownMask();
    if (uVar6 != 0xffffffff) {
        uVar7 = 1 << ((uint8_t)uVar6 & 0x1f);
    } else {
        uVar7 = 0xffffffff;
    }

    gPostFXRenderer.drawFullscreenQuad((uVar6 != 0xffffffff),false,false,uVar7);
    gPostFXRenderer.bindShaders();

    gPostFXRenderer.bindRasterState( nullptr );
#endif
    if ((pOwner->getFlags() & 1) != 0) {
        pRenderTargets[0]->resolveMSAA(0);
    }

    return true;
}

PostFXNodeBlit::PostFXNodeBlit()
    : PostFXNode(1, 1)
    , blendOP(eBlendOP::BO_Add)
    , blendSrc(eBlendMode::BM_One)
    , blendDst(eBlendMode::BM_Zero)
{
    // FUN_005fa3d0
    pPostFX = new (TestDrive::Alloc(sizeof(PostFXBlit))) PostFXBlit(this);
}

PostFXNodeBlit::~PostFXNodeBlit()
{
    TestDrive::Free(pPostFX);
}

uint32_t PostFXNodeBlit::getType() const
{
    return 0x18;
}
