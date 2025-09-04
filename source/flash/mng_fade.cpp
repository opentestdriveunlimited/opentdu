#include "shared.h"
#include "mng_fade.h"

#include "render/draw_list.h"
#include "gs_timer.h"
#include "gs_flash.h"
#include "modal_message_box.h"
#include "render/gs_render.h"

#include "tdu_instance.h"

MngFade::MngFade()
    : Manager()
    , pDrawListTransition( new DrawList() )
    , transitionColor( 0u )
    , flags( 6 )
    , transitionDuration( 1.0f )
    , pCallback( nullptr )
    , pCallbackSource( nullptr )
    , pCallbackData( nullptr )
    , position( 0.0f, 0.0f )
    , width( 1.0f )
    , height( 1.0f )
    , elapsedTime( 0.0f )
    , logoElapsedTime( -1.0f )
    , pDrawListLogo( new DrawList() )
    , p2DMBuffer( nullptr )
    , pMaterial( nullptr )
    , bRenderLogo( false )
    , bWaitForPopup( false )
{

}

MngFade::~MngFade()
{

}

bool MngFade::initialize(TestDriveGameInstance *)
{
    // FUN_006cfb3          
    DrawStreams eStack_20;
    eStack_20.Tangent = '\0';
    eStack_20.Binormal = '\0';
    eStack_20.Index = '\0';
    eStack_20.Custom = '\0';
    eStack_20.Normals = '\0';
    eStack_20.Diffuse = '\0';
    eStack_20.Specular = '\0';
    eStack_20.UVMap = '\0';
    pDrawListTransition->initialize( 1, 4, 0, true, &eStack_20, "Trans_rlist2d", false );
    
    DrawStreams uStack_18;
    uStack_18.Normals = '\0';
    uStack_18.Diffuse = '\0';
    uStack_18.Specular = '\0';
    uStack_18.UVMap = '\0';
    uStack_18.UVMap = '\x01';
    pDrawListLogo->initialize( 1, 4, 0, true, &uStack_18, "Logo_rlist2d", false );

    return true;
}

void MngFade::tick(float deltaTime, float)
{
    // FUN_006cfc40
    if ((deltaTime <= 0.0f) && (gGSTimer.GameSpeed <= 0.0f)) {
        deltaTime = gGSTimer.SystemDeltaTime;
    }
    if (0.1f < deltaTime) {
        deltaTime = 0.1f;
    }
    if (pDrawListTransition == nullptr) {
        return;
    }

    pDrawListTransition->reset();
    bool bVar2 = false;
    uint8_t alpha = 0x0;
    float fVar7 = 0.0f;
    if ((flags & 1) != 0) {
        float fVar6 = (elapsedTime / transitionDuration) * 255.0f;
        elapsedTime += deltaTime;
        bVar2 = true;
        fVar7 = fVar6;
        if ((flags >> 2 & 1) != 0) {
            fVar7 = 255.0f - fVar6;
        }
        alpha = static_cast<uint8_t>(Clamp(static_cast<int32_t>(fVar7), 0, 255));
        
        if (255.0 < fVar6) {
            flags &= 0xfffffffe;        
            elapsedTime = 0.0f;

            if (pCallback != nullptr) {
                pCallback(pCallbackData, pCallbackSource);
            }
       
            if ((flags >> 2 & 1) != 0) {
                transitionColor[0] = '\0';
                transitionColor[1] = '\0';
                transitionColor[2] = '\0';
            }
        }
    }

    if ((flags >> 3 & 1) != 0) {
        alpha = 0xff;
    }

    fVar7 = 0.0f;
    if ((flags >> 1 & 1) == 0) {
        fVar7 = -1.0f;
    }

    if (bVar2) {
        encodeTransitionDrawList(alpha, fVar7);
    }

    if (pDrawListLogo != nullptr && bRenderLogo) {
        pDrawListLogo->reset();
        encodeLogoDrawList();
        
        if (!bWaitForPopup) {
            FlashPlayer* pPlayer = gpFlash->getFlashPlayer( "GENERAL" );
            OTDU_ASSERT( pPlayer );

            if (gpActiveGameMode != nullptr && !gbLoadingInProgress) {
                bWaitForPopup = gpFlashMessageBox->display(0x36c66a2, 4.0f, 0, nullptr, nullptr, false );
            }
        }

        if (!bRenderLogo && !gbLoadingInProgress) {
            return;
        }
    }

    if (bWaitForPopup) {
        FlashPlayer* peVar3 = gpFlash->getFlashPlayer( "GENERAL" );
        if (peVar3 != nullptr) {
            bWaitForPopup = false;
            gpFlashMessageBox->close();
        }
    }
}

void MngFade::terminate()
{
    // FUN_006cf8c0
    pDrawListLogo->destroy();
    if (pDrawListLogo != nullptr) {
        delete pDrawListLogo;
    }
    pDrawListLogo = nullptr;

    pDrawListTransition->destroy();
    if (pDrawListTransition != nullptr) {
        delete pDrawListTransition;
    }
    pDrawListTransition = nullptr;
}

void MngFade::draw()
{
    // FUN_006cfa90
    if (pDrawListTransition != nullptr) {
        RenderPass& pass = ((flags >> 1 & 1) == 0) 
            ? gpRender->getRenderPass<eRenderPass::RP_After2D>() 
            : gpRender->getRenderPass<eRenderPass::RP_Before2D>();

        pass.pScene->enqueueDynamicDrawList(pDrawListTransition);
    }

    if (pDrawListLogo != nullptr) {
        gpRender->getRenderPass<eRenderPass::RP_After2D>().pScene->enqueueDynamicDrawList(pDrawListLogo);
    }
}

void MngFade::pause()
{
    // FUN_006cfa50
    if (bWaitForPopup) {
        FlashPlayer* peVar1 = gpFlash->getFlashPlayer( "GENERAL" );
        if (peVar1 != nullptr) {
            bWaitForPopup = false;
            gpFlashMessageBox->close();
        }
    }
}

void MngFade::encodeTransitionDrawList(const uint8_t alpha, const float fVar7)
{
    pDrawListTransition->beginPrimitive(ePrimitiveType::PT_TriangleStrip, 4);

    ColorRGBA color = transitionColor;
    color.A = alpha;
    pDrawListTransition->setDiffuse(0, color);

    pDrawListTransition->pushVertex(position[0],            position[1],            fVar7);
    pDrawListTransition->pushVertex(position[0] + width,    position[1],            fVar7);
    pDrawListTransition->pushVertex(position[0],            position[1] + height,   fVar7);
    pDrawListTransition->pushVertex(position[0] + width,    position[1] + height,   fVar7);

    pDrawListTransition->commitPrimitive();
}

void MngFade::encodeLogoDrawList()
{
    // FUN_006cf730
    static constexpr float FLOAT_00fabf34 = 0.05f;
    // TODO: Looks like this is never called? (p2DMBuffer is always null at runtime)
    if (p2DMBuffer != nullptr) {
        OTDU_UNIMPLEMENTED;
    }
}
