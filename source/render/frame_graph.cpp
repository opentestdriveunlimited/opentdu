#include "shared.h"
#include "frame_graph.h"

#include "render/render_scene.h"

FrameGraph::FrameGraph()
{
}

FrameGraph::~FrameGraph()
{
}

void FrameGraph::pushObject(RenderScene *param_2, bool param_3)
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

void FrameGraph::pushBackObject(RenderObjectBase *param_2)
{
    // FUN_004047a0
    renderNodes.push_back(param_2);
}
