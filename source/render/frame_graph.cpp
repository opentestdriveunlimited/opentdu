#include "shared.h"
#include "frame_graph.h"

#include "render/render_scene.h"
#include "render/gs_render.h"
#include "render/gs_render_helper.h"

#include "render/postfx/postfx_stack.h"
#include "render/postfx/postfx_node.h"

uint32_t    DAT_00fe77c4 = 0;
uint32_t    DAT_00fe77c8 = 0;
uint32_t    DAT_00fe77cc = 0;

FrameGraph::FrameGraph()
    : bRemoveDynamicDrawCmds( false )
{
}

FrameGraph::~FrameGraph()
{
}

void FrameGraph::pushObject(RenderObjectBase *param_2, bool param_3)
{
    // FUN_0050b340
    if (param_3) {
        pushBackObject(param_2);
        return;
    }

    renderNodes.push_front(param_2);
}

void FrameGraph::insertAfterObject(RenderObjectBase *param_2, RenderObjectBase *param_3)
{
    // FUN_004046a0
    for (auto it = renderNodes.begin(); it != renderNodes.end(); it++) {
        RenderObjectBase* peVar3 = *it;
        if (peVar3 == param_3) {
            it++;
            renderNodes.insert(it, param_2); // FUN_004046e0 (inlined)
            return;
        }
    }
    OTDU_ASSERT(false);
}

void FrameGraph::removeObject(RenderObjectBase *pObject)
{
    std::remove_if(renderNodes.begin(), renderNodes.end(), [=](RenderObjectBase* x) { return x == pObject; });
}

void FrameGraph::submitDrawCommands(bool param_2)
{
    // FUN_0050ea50
    if (param_2) {
        uint32_t uVar5 = 0;
        while (!clearCommands.empty()) {
            ClearCommand& cmd = clearCommands.front();
            processClearCommand(cmd);
            clearCommands.pop();
        }
    }

    gpActiveRenderScene = nullptr;
    DAT_00fe77c4 = 0;
    DAT_00fe77c8 = 0;
    DAT_00fe77cc = 0;

    if (param_2) {
        uint32_t uVar5 = 0;
        for (RenderObjectBase* pNode : renderNodes) {
            uint32_t puVar2 = pNode->getObjectType();
            if (puVar2 == 0) {
                RenderScene* peVar1 = (RenderScene*)pNode;
                gpActiveRenderScene = peVar1;
                peVar1->getSetup().bind(0);
                peVar1->submitDrawCommands(uVar5);
                gpRender->FUN_00512420();
                peVar1->getSetup().unbind(0);
            } else if (puVar2 == 1) {
                PostFXNode* peVar1 = (PostFXNode*)pNode;

                gPostFXStack.beginPass();
                bool bVar3 = peVar1->prepare();
                if (bVar3) {
                    peVar1->execute();
                }
                gPostFXStack.endPass();
            }
            uVar5++;
        }
    }

    if (bRemoveDynamicDrawCmds) {
        for (RenderObjectBase* pNode : renderNodes) {
            uint32_t puVar2 = pNode->getObjectType();
            if (puVar2 == 0) {
                RenderScene* peVar1 = (RenderScene*)pNode;
                peVar1->removeDynamicDrawCommands();
            }
        }
    }
}

void FrameGraph::pushBackObject(RenderObjectBase *param_2)
{
    // FUN_004047a0
    renderNodes.push_back(param_2);
}

void FrameGraph::processClearCommand(ClearCommand &cmd)
{
    // FUN_005fcfc0
    RenderDevice* renderDevice = gpRender->getRenderDevice();
    OTDU_UNIMPLEMENTED;
}
