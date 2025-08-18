#pragma once

class RenderTarget;

#include "render/postfx/postfx_node.h"
#include "render/postfx/postfx_instance.h"
#include "render/gs_render_helper.h"

class PostFXNodeBlit;

class PostFXBlit : public PostFX {
public:
    PostFXBlit(PostFXNodeBlit* pOwner);
    ~PostFXBlit();

    virtual bool execute() override;
};

class PostFXNodeBlit : public PostFXNode {
public:
    PostFXNodeBlit();
    ~PostFXNodeBlit();

    virtual uint32_t getType() const override;

    eBlendOP getBlendOP() const { return blendOP; }
    eBlendMode getBlendSrc() const { return blendSrc; }
    eBlendMode getBlendDst() const { return blendDst; }

    void setBlendOP( eBlendOP op ) { blendOP = op; }

private:
    eBlendOP blendOP;
    eBlendMode blendSrc;
    eBlendMode blendDst;
};
