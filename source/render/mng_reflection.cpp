#include "shared.h"
#include "mng_reflection.h"

#include "gs_render.h"
#include "render_scene.h"
#include "camera/camera_game.h"

MngReflection* gpMngReflection = nullptr;

ColorRGBA MngReflection::ReflectionBackgroundColor = ColorRGBA( 0xffffffffu );

MngReflection::MngReflection()
    : Manager()
    , pScene( nullptr )
    , pCamera( nullptr )
    , planeNormal( kWorldUpVector )
    , clearFlagsBackup( 0 )
    , activeClipPlane( 0 )
    , bEnableClip( true )
    , bInitialized( false )
{
    OTDU_ASSERT(gpMngReflection == nullptr);
    gpMngReflection = this;
}

MngReflection::~MngReflection()
{
    gpMngReflection = nullptr;
}

bool MngReflection::initialize(TestDriveGameInstance *)
{
    RenderPass& reflectionRenderPass = gpRender->getRenderPassByIndex( 0x27 );

    pScene = reflectionRenderPass.pScene;
    pCamera = reflectionRenderPass.pCamera;

    pScene->setUnknownMask( 0xffefbfffffffff7f );
    
    return true;
}

void MngReflection::draw()
{
}

void MngReflection::terminate()
{
    instances.clear();
    hiearchyNodes.clear();
    
    RenderPass& renderPass = gpRender->getRenderPassByIndex( 0x27 );
    renderPass.bEnabled = false;

    pScene = nullptr;
    pCamera = nullptr;
}

void MngReflection::reset()
{
    RenderPass& renderPass = gpRender->getRenderPassByIndex( 0x21 );
    renderPass.pViewport->ClearColor = ReflectionBackgroundColor;
    clearFlagsBackup = ~(renderPass.pViewport->Flags & 1);
    renderPass.pViewport->Flags |= 1;
}

void MngReflection::pause()
{
    if (clearFlagsBackup != 0) {
        RenderPass& renderPass = gpRender->getRenderPassByIndex( 0x21 );
        renderPass.pViewport->Flags &= 0xfffffffe;
    }
}

void MngReflection::removeNode(HiearchyNode *pNode)
{
    for (uint32_t i = 0; i < hiearchyNodes.size(); i++) {
        if (hiearchyNodes[i] == pNode) {
            hiearchyNodes.erase(hiearchyNodes.begin() + i);
            return;
        }
    }

    OTDU_LOG_ERROR( "Tried to remove unregistered node %p\n", pNode);
    OTDU_ASSERT_FATAL( false );
}
