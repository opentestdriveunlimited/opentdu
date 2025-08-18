#pragma once

class RenderTarget;

#include "render/postfx/postfx_node.h"
#include "render/postfx/postfx_instance.h"

class PostFXNodeTexSample;

class PostFXTexSample : public PostFX {
public:
    PostFXTexSample(PostFXNodeTexSample* pOwner);
    ~PostFXTexSample();

    virtual bool initialize() override;
    virtual bool execute() override;
};

class PostFXNodeTexSample : public PostFXNode {
public:
    PostFXNodeTexSample();
    ~PostFXNodeTexSample();

    virtual uint32_t getType() const override;

    void setTextureToSample(RenderTarget* param_1);
    RenderTarget* getTextureToSample();

private:
    RenderTarget* pTexture;
};
