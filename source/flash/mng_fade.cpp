#include "shared.h"
#include "mng_fade.h"

#include "render/draw_list.h"
#include "gs_timer.h"
#include "gs_flash.h"

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
    delete pDrawListTransition;
    delete pDrawListLogo;
}

bool MngFade::initialize(TestDriveGameInstance *)
{                  
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

void MngFade::tick(float systemDeltaTime, float)
{
    if (systemDeltaTime <= 0.0 && gGSTimer.GameSpeed <= 0.0) {
        systemDeltaTime = gGSTimer.SystemDeltaTime;
    }
    systemDeltaTime = Min(systemDeltaTime, 0.1f);

    pDrawListTransition->reset();
    bool bVar2 = false;
    uint8_t alpha = 0x0;
    float fVar7 = 0.0f;
    if ((flags & 1) != 0) {
        float fVar6 = (elapsedTime / transitionDuration) * 255.0f;
        elapsedTime += systemDeltaTime;
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

    fVar7 = 0.0;
    if ((flags >> 1 & 1) == 0) {
        fVar7 = -1.0;
    }

    if (bVar2) {
        encodeTransitionDrawList(alpha, fVar7);
    }

    if (bRenderLogo) {
        pDrawListLogo->reset();
        encodeLogoDrawList();
        
        if (!bWaitForPopup) {
            FlashPlayer* pPlayer = gpFlash->getFlashPlayer( "GENERAL" );
            OTDU_ASSERT( pPlayer );

            if (gpActiveGameMode != nullptr && !gbLoadingInProgress) {
                OTDU_UNIMPLEMENTED; // TODO:
            }
        }
    }
}

void MngFade::terminate()
{
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
    OTDU_ASSERT(p2DMBuffer);
    float fVar2 = gGSTimer.SystemTotalTime - logoElapsedTime;
    constexpr float FLOAT_00fabf34 = 0.05f;
    if ((fVar2 < 0.0) || (FLOAT_00fabf34 <= fVar2)) {
        logoElapsedTime = gGSTimer.SystemTotalTime;
    }
    pDrawListLogo->setActiveMaterial( pMaterial );
    pDrawListLogo->beginPrimitive(ePrimitiveType::PT_TriangleStrip, 4);
   
    pDrawListLogo->commitPrimitive();

    OTDU_UNIMPLEMENTED; // TODO: Figure out where the stack integers come from
    // pDrawListLogo->setTexCoords( 0, { local_38, local_34 } );
    // pDrawListLogo->pushVertex(local_40,1.0 - local_3c,-2.0);
    
    // pDrawListLogo->setTexCoords( 0, { local_28, local_24 } );
    // pDrawListLogo->pushVertex(local_30,1.0 - local_2c,-2.0);
    
    // pDrawListLogo->setTexCoords( 0, { local_8, local_4 } );
    // pDrawListLogo->pushVertex(local_30,local_10,1.0 - local_c,-2.0);
    
    // pDrawListLogo->setTexCoords( 0, { local_18, local_14 } );
    // pDrawListLogo->pushVertex(local_20,1.0 - local_1c,-2.0);
    
    pDrawListLogo->commitPrimitive();
}
