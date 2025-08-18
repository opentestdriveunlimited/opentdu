#include "shared.h"
#include "postfx_instance.h"

#include "render/gs_render.h"
#include "render/gs_render_helper.h"
#include "render/shaders/shader_register.h"

#include "postfx_stack.h"

PostFX::PostFX(PostFXNode* param_1)
    : pOwner( param_1 )
    , pVertexShader( nullptr )
    , pPixelShader( nullptr )
    , bInitialized( false )
{
    // FUN_005f7410
    gPostFXStack.registerEffect(this);

}


PostFX::~PostFX()
{
    releaseResources();
}

bool PostFX::initialize()
{
    // FUN_005f71e0
    OTDU_ASSERT( pOwner );

    uint32_t numOutputs = pOwner->getNumOutput();
    for (uint32_t uVar5 = 0; uVar5 < numOutputs; uVar5++) {
        pRenderTargets[uVar5] = pOwner->getOuput(uVar5);
    }

    pVertexShader = nullptr;
    pPixelShader = nullptr;

    if (getVertexShaderHashcode() != 0ull) {
        pVertexShader = gShaderRegister.getShader(gpRender->getRenderDevice(), eShaderType::ST_Vertex, getVertexShaderHashcode());
        if (pVertexShader == nullptr) {
            return false;
        }
    }
    
    if (getPixelShaderHashcode() != 0ull) {
        pPixelShader = gShaderRegister.getShader(gpRender->getRenderDevice(), eShaderType::ST_Pixel, getPixelShaderHashcode());
        if (pPixelShader == nullptr) {
            return false;
        }
    }

    return true;
}

bool PostFX::execute()
{
    // FUN_005f60a0
    if (!bInitialized) {
        bInitialized = initialize();
    }

    return bInitialized;
}

void PostFX::releaseResources()
{
    if (pVertexShader != nullptr && getVertexShaderHashcode() != 0ull) {
        if (getVertexShaderHashcode()) {
            gShaderRegister.invalidateShader(eShaderType::ST_Vertex, getVertexShaderHashcode());    
        }
        pVertexShader = nullptr;
    }

    if (pPixelShader != nullptr) {
        if (getPixelShaderHashcode() != 0ull) {
            gShaderRegister.invalidateShader(eShaderType::ST_Pixel, getPixelShaderHashcode());
        }
        pPixelShader = nullptr;
    }

    bInitialized = false;
}

uint64_t PostFX::getVertexShaderHashcode() const
{
    return 0ull;
}

uint64_t PostFX::getPixelShaderHashcode() const
{
    return 0ull;
}

