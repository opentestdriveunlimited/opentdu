#pragma once

class PostFX;

class PostFXStack {
public:
    PostFXStack();
    ~PostFXStack();

    void registerEffect(PostFX* param_2);
    bool releaseResources();

private:
    std::vector<PostFX*> effects;
};

extern PostFXStack gPostFXStack;
