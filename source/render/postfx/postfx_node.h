#pragma once

struct GPUShader;
class PostFX;
class RenderTarget;

#include "render/render_object_base.h"

class PostFXNode : public RenderObjectBase {
public:
    // Represents one input (or output) entry for this PostFX instance
    struct Link {
        PostFXNode* pPostFX    = nullptr;
        uint32_t    LinkIndex  = 0xffffffff;
    };

    static constexpr uint32_t kMaxNumLink = 4;

public:
    PostFXNode(uint32_t param_2, uint32_t param_3);
    ~PostFXNode();

    virtual uint32_t getType() const = 0;

    uint32_t getNumOutput();
    RenderTarget* getOuput(uint32_t param_2);

    PostFXNode* getInputNode(uint32_t param_2);
    uint32_t getInputLinkIndex(uint32_t param_2);

    uint32_t getUnknownMask() const;
    uint32_t getFlags() const;

    bool connect(PostFXNode* pInput, uint32_t outputSlotID, uint32_t inputSlotID);

    void setOutput(RenderTarget* param_2, uint32_t param_3);

protected:
    PostFX* pPostFX;

    std::array<Link, kMaxNumLink> outputs;
    std::array<Link, kMaxNumLink> inputs;

    std::array<RenderTarget*, kMaxNumLink> pRenderTargets;

    uint32_t numOutput;
    uint32_t numInput;
    uint32_t flags;
    uint32_t unknownMask;

    uint8_t bInitialized : 1;

    std::string name;
};
