#include "shared.h"
#include "postfx_node.h"

#include "render/gs_render.h"
#include "render/gs_render_helper.h"
#include "render/shaders/shader_register.h"

#include "postfx_stack.h"

PostFXNode::PostFXNode(uint32_t param_2, uint32_t param_3)
    : RenderObjectBase()
    , pPostFX( nullptr )
    , bInitialized(false )
    , numOutput(param_3)
    , numInput(param_2)
    , flags(1)
    , unknownMask(0xffffffff)
{
    objectType = 1;
}

PostFXNode::~PostFXNode()
{
    
}

uint32_t PostFXNode::getNumOutput()
{
    return numOutput;
}

RenderTarget* PostFXNode::getOuput(uint32_t param_2)
{
    return (numOutput <= param_2 ) ? nullptr : pRenderTargets[param_2];
}

PostFXNode* PostFXNode::getInputNode(uint32_t param_2)
{
    return (numInput <= param_2 ) ? nullptr : inputs[param_2].pPostFX;
}

uint32_t PostFXNode::getInputLinkIndex(uint32_t param_2)
{
    return (numInput <= param_2 ) ? 0xffffffff : inputs[param_2].LinkIndex;
}

uint32_t PostFXNode::getUnknownMask() const
{
    return unknownMask;
}

uint32_t PostFXNode::getFlags() const
{
    return flags;
}

bool PostFXNode::connect(PostFXNode *pInput, uint32_t outputSlotID, uint32_t inputSlotID)
{
    // FUN_0050bf10
    if (pInput != nullptr && inputSlotID < numInput && outputSlotID < pInput->getNumOutput()) {
        inputs[inputSlotID].pPostFX = pInput;
        inputs[inputSlotID].LinkIndex = outputSlotID;
     
        pInput->outputs[outputSlotID].pPostFX = this;
        pInput->outputs[outputSlotID].LinkIndex = inputSlotID;

        return true;
    }

    return false;
}

void PostFXNode::setOutput(RenderTarget *param_2, uint32_t param_3)
{
    OTDU_ASSERT( param_3 < kMaxNumLink );
    pRenderTargets[param_3] = param_2;
}
