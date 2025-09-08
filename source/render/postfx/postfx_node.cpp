#include "shared.h"
#include "postfx_node.h"

#include "render/gs_render.h"
#include "render/gs_render_helper.h"
#include "render/shaders/shader_register.h"

#include "postfx_stack.h"
#include "postfx_instance.h"

PostFXNode::PostFXNode(uint32_t param_2, uint32_t param_3)
    : RenderObjectBase()
    , pPostFX( nullptr )
    , state( 0 )
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
    return (param_2 < numOutput) ? pRenderTargets[param_2] : nullptr;
}

PostFX* PostFXNode::getInputNode(uint32_t param_2)
{
    return (param_2 < numInput) ? inputs[param_2].pPostFX->getEffect() : nullptr;
}

uint32_t PostFXNode::getInputLinkIndex(uint32_t param_2)
{
    return (param_2 < numInput) ? inputs[param_2].LinkIndex : 0xffffffff;
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
    OTDU_ASSERT( param_2 );
    pRenderTargets[param_3] = param_2;
}

bool PostFXNode::prepare()
{
    // FUN_0050c030
    state = 0;
    for (uint32_t uVar2 = 0; uVar2 < numInput; uVar2++) {
        if (inputs[uVar2].pPostFX == nullptr) {
            return false;
        }
        inputs[uVar2].pPostFX->prepare();
    }

    return true;
}

bool PostFXNode::execute()
{
    // FUN_0050c070
    if (pPostFX == nullptr) {
        return false;
    }

    if (state != 0) {
        return state == 1;
    }

    for (uint32_t uVar4 = 0; uVar4 < numInput; uVar4++) {
        if (inputs[uVar4].pPostFX == nullptr) {
            return false;
        }

        bool bVar1 = inputs[uVar4].pPostFX->execute();
        if (!bVar1) {
            return false;
        }
    }

    bool bVar2 = pPostFX->execute();
    state = 2 - ((bVar2) ? 1 : 0);

    return state == 1;
}

PostFX *PostFXNode::getEffect()
{
    return pPostFX;
}
