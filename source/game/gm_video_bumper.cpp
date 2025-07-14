#include "shared.h"
#include "gm_video_bumper.h"

#include "config/gs_config.h"

GMVideoBumper::GMVideoBumper()
    : movieManager( nullptr )
{

}

GMVideoBumper::~GMVideoBumper()
{

}

void GMVideoBumper::tick(float deltaTime)
{
    /*int iVar1;
    undefined4 uVar2;
    uint local_8;
    int local_4;

    if ( gpConfig->bEnableMovie ) {
        LoadMovie( movieManager, gActiveVideoName );
    }

    DAT_00fa4942._2_1_ = 0;
    UNK_0129cfa8._8_1_ = 0;
    if ( DAT_010e7d3d != '\0' ) {
        iVar1 = FUN_0099a950( gpFLashPage, &gGSFlash );
        if ( iVar1 != 0 ) {
            FUN_0099b3f0( gpFLashPage, &gGSFlash );
        }
    }
    uVar2 = FUN_0099a490( &local_8, &local_4, "CONFIGPC", &gGSFlash );
    if ( ( char )uVar2 != '\0' ) {
        iVar1 = FUN_0099a780( &gGSFlash, local_8, local_4 );
        if ( iVar1 != 0 ) {
            *( uint* )( iVar1 + 0xac4 ) = *( uint* )( iVar1 + 0xac4 ) | 2;
        }
    }*/
}
