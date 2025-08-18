#pragma once

class RenderTarget;

#include "render/postfx/postfx_node.h"
#include "render/postfx/postfx_instance.h"

class PostFXNodeDownscale;

class PostFXDownscale : public PostFX {
public:
    PostFXDownscale(PostFXNodeDownscale* pOwner);
    ~PostFXDownscale();

    virtual bool execute() override;    
    virtual uint64_t getPixelShaderHashcode() const override;
};

class PostFXNodeDownscale : public PostFXNode {
public:
    PostFXNodeDownscale();
    ~PostFXNodeDownscale();

    virtual uint32_t getType() const override;
};
