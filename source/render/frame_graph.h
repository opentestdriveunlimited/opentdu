#pragma once

class RenderScene;
class RenderObjectBase;
class RenderTarget;

#include "core/color.h"

struct ClearCommand
{
    RenderTarget* pRenderTarget = nullptr;
    ColorRGBA ClearColor = gColorBlack;
    int32_t X = 0;
    int32_t Y = 0;
    int32_t Width = 0;
    int32_t Height = 0;
};

class FrameGraph {
public:
    FrameGraph();
    ~FrameGraph();

    void pushObject(RenderObjectBase* pObject, bool bPushBack);
    void insertAfterObject(RenderObjectBase* pObject, RenderObjectBase* pObjToInsertAfter);
    void removeObject(RenderObjectBase* pObject);

    void submitDrawCommands(bool param_2);

private:
    std::list<RenderObjectBase*> renderNodes;
    std::queue<ClearCommand> clearCommands;

    uint8_t bRemoveDynamicDrawCmds : 1;

private:
    void pushBackObject(RenderObjectBase* pObject);
    void processClearCommand(ClearCommand& cmd);
};
