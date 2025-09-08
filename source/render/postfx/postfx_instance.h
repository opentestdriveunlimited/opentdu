#pragma once

struct GPUShader;
class RenderTarget;

#include "postfx_node.h"

class PostFX {
public:
    PostFX(PostFXNode* param_1);
    ~PostFX();

    virtual bool initialize();
    virtual void releaseResources();
    virtual bool execute();

    virtual uint64_t getVertexShaderHashcode() const;
    virtual uint64_t getPixelShaderHashcode() const;
    
    RenderTarget* getOutput(const uint32_t index);

protected:
    PostFXNode* pOwner;

    const GPUShader* pVertexShader;
    const GPUShader* pPixelShader;

    uint8_t bInitialized : 1;
    
    std::array<RenderTarget*, PostFXNode::kMaxNumLink> pRenderTargets;
};
