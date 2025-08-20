#pragma once

class RenderScene;
class RenderObjectBase;

class FrameGraph {
public:
    FrameGraph();
    ~FrameGraph();

    void pushObject(RenderObjectBase* pObject, bool bPushBack);
    void insertAfterObject(RenderObjectBase* pObject, RenderObjectBase* pObjToInsertAfter);
    void removeObject(RenderObjectBase* pObject);

private:
    std::list<RenderObjectBase*> renderNodes;

private:
    void pushBackObject(RenderObjectBase* pObject);
};
