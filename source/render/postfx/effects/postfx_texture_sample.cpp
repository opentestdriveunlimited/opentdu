#include "shared.h"
#include "postfx_texture_sample.h"

PostFXTexSample::PostFXTexSample(PostFXNodeTexSample* pOwner)
    : PostFX(pOwner)
{

}

PostFXTexSample::~PostFXTexSample()
{
    
}

bool PostFXTexSample::initialize()
{
    PostFXNodeTexSample* pNode = (PostFXNodeTexSample*)pOwner;
    pRenderTargets[0] = pNode->getTextureToSample();
    return pRenderTargets[0] != nullptr;
}

bool PostFXTexSample::execute()
{
    bool bVar1 = PostFX::execute();
    if (!bVar1) {
        return false;
    }

    return initialize();
}

PostFXNodeTexSample::PostFXNodeTexSample()
    : PostFXNode(0, 1)
    , pTexture(nullptr)
{
    // FUN_0050c1f0
    pPostFX = new (TestDrive::Alloc(sizeof(PostFXTexSample))) PostFXTexSample(this);
}

PostFXNodeTexSample::~PostFXNodeTexSample()
{
    TestDrive::Free(pPostFX);
}

uint32_t PostFXNodeTexSample::getType() const
{
    return 0x0;
}

void PostFXNodeTexSample::setTextureToSample(RenderTarget* param_1)
{
    pTexture = param_1;
}

RenderTarget* PostFXNodeTexSample::getTextureToSample()
{
    return pTexture;
}
