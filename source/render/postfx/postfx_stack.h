#pragma once

class PostFX;

#include "render/gs_render_helper.h"

class PostFXStack {
public:
    PostFXStack();
    ~PostFXStack();

    void registerEffect(PostFX* param_2);
    bool releaseResources();

    void beginPass();
    void endPass();

private:
    std::vector<PostFX*> effects;

    std::array<RenderTarget*, kMaxSimultaneousRT> renderTargets;
    uint32_t isMSAAEnabled;
};

extern PostFXStack gPostFXStack;
