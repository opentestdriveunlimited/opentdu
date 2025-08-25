#include "shared.h"
#include "postfx_stack.h"

#include "render/gs_render.h"
#include "postfx_instance.h"

PostFXStack gPostFXStack = {};

// Arbitrary limit from the original game.
static constexpr uint32_t kMaxNumEffect = 64;

PostFXStack::PostFXStack()
    : isMSAAEnabled( 0 )
{
    renderTargets.fill( nullptr );
}

PostFXStack::~PostFXStack()
{
    releaseResources();
}

void PostFXStack::registerEffect(PostFX* param_2)
{
    OTDU_ASSERT( effects.size() < kMaxNumEffect );
    effects.push_back(param_2);
}

bool PostFXStack::releaseResources()
{
    // FUN_005f5d60
    for ( PostFX* pEffect : effects ) {
        pEffect->releaseResources();
    }

    return true;
}

void PostFXStack::beginPass()
{
    // FUN_005f5cc0
    for (uint32_t i = 0; i < kMaxSimultaneousRT; i++) {
        renderTargets[i] = gpRender->getRenderDevice()->getBoundRenderTargetAtIndex(i);
    }

    isMSAAEnabled = gpRender->getRenderDevice()->isFramebufferUsingMSAA();
}

void PostFXStack::endPass()
{
    // FUN_005f5b70
    if (gpRender->getRenderDevice()->isFramebufferUsingMSAA() != isMSAAEnabled) {
        gpRender->getRenderDevice()->setMSAAState(isMSAAEnabled);
    }
}
