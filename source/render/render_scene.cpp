#include "shared.h"
#include "render_scene.h"

RenderScene::RenderScene()
    : RenderObjectBase()
    , frustum()
    , fogDesc()
    , pActiveCamera( nullptr )
    , pActiveViewport( nullptr )
    , flags( 0u )
    , unknownMask( 0xffffffffffffffff )
    , activeVertexShaderIndex( 0u )
    , activePixelShaderIndex( 0u )
    , bOrthoProjection( false )
{

}

RenderScene::~RenderScene()
{
    
}

void RenderScene::enqueueDynamicDrawList(DrawList *pDrawList)
{
    drawCommands.DynamicDrawLists.push_back(pDrawList);
}
