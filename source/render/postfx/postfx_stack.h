#pragma once

struct GPUShader;

class PostFX {
public:
    PostFX();
    ~PostFX();

    virtual void releaseResources();

protected:
    GPUShader* pVertexShader;
    GPUShader* pPixelShader;

    uint8_t bInitialized : 1;
};

class PostFXStack {
public:
    PostFXStack();
    ~PostFXStack();

    bool releaseResources();

private:
    std::vector<PostFX*> effects;
};

extern PostFXStack gPostFXStack;
