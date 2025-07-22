#include "shared.h"
#include "postfx_stack.h"

#include "render/gs_render.h"

PostFXStack gPostFXStack = {};

PostFXStack::PostFXStack()
{

}

PostFXStack::~PostFXStack()
{
    releaseResources();
}

bool PostFXStack::releaseResources()
{
    // FUN_005f5d60
    for ( PostFX* pEffect : effects ) {
        pEffect->releaseResources();
    }

    return true;
}

PostFX::PostFX()
    : pVertexShader( nullptr )
    , pPixelShader( nullptr )
    , bInitialized( false )
{

}


PostFX::~PostFX()
{
    releaseResources();
}

void PostFX::releaseResources()
{
    if ( pVertexShader != nullptr ) {
        gpRender->getRenderDevice()->destroyShader( pVertexShader );
        pVertexShader = nullptr;
    }

    if ( pPixelShader != nullptr ) {
        gpRender->getRenderDevice()->destroyShader( pPixelShader );
        pPixelShader = nullptr;
    }

    bInitialized = false;
}
