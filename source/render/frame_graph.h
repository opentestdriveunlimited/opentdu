#pragma once

class RenderScene;
class RenderObjectBase;

class FrameGraph {
public:
    FrameGraph();
    ~FrameGraph();

    void pushObject(RenderScene* pObject, bool bPushBack);
    void insertAfterObject(RenderObjectBase* pObject, RenderObjectBase* pObjToInsertAfter);

private:
    std::list<RenderObjectBase*> renderNodes;

private:
    void pushBackObject(RenderObjectBase* pObject);
};
