#include "shared.h"
#include "mng_hud.h"

MngHud* gpMngHud = nullptr;

MngHud::MngHud()
    : Manager()
{

}

MngHud::~MngHud()
{

}

bool MngHud::initialize( TestDriveGameInstance* )
{
    return false;
}

void MngHud::tick( float, float )
{

}

void MngHud::terminate()
{

}

void MngHud::updateSettings()
{
    OTDU_UNIMPLEMENTED;

    //undefined* uVar1;
    //MngBase_vtable* pMVar1;
    //MngBase_vtable* pMVar2;
    //MngBase_vtable* pMVar3;
    //MngBase_vtable* pMVar4;
    //MngBase_vtable* pMVar5;

    //uVar1 = &UNK_00fa88f1.field_0x7;
    //FUN_0099cc40( 0xfa88f8, &LAB_006e7910 );
    //*( undefined* )( ( int )&param_1[0xc98].lpVtbl + 3 ) = 0;
    //FUN_0099cc40( uVar1, &LAB_006e7910 );
    //FUN_0099cc40( uVar1, &LAB_006e77c0 );
    //*( undefined* )( ( int )&param_1[0xc98].lpVtbl + 2 ) = 0;
    //if ( DAT_0114c174 == 0 ) {
    //    pMVar1 = param_1[0x5f].lpVtbl;
    //    pMVar2 = param_1[0x61].lpVtbl;
    //    pMVar3 = param_1[0x5e].lpVtbl;
    //    pMVar4 = param_1[0x5d].lpVtbl;
    //    pMVar5 = param_1[0x60].lpVtbl;
    //    param_1[0x4d].lpVtbl = param_1[0x5c].lpVtbl;
    //} else {
    //    pMVar1 = param_1[0x59].lpVtbl;
    //    pMVar2 = param_1[0x5b].lpVtbl;
    //    pMVar3 = param_1[0x58].lpVtbl;
    //    pMVar4 = param_1[0x57].lpVtbl;
    //    pMVar5 = param_1[0x5a].lpVtbl;
    //    param_1[0x4d].lpVtbl = param_1[0x56].lpVtbl;
    //}
    //param_1[0x4e].lpVtbl = pMVar4;
    //param_1[0x4f].lpVtbl = pMVar3;
    //param_1[0x50].lpVtbl = pMVar1;
    //param_1[0x51].lpVtbl = pMVar5;
    //param_1[0x52].lpVtbl = pMVar2;
    //pMVar1 = param_1[0x66].lpVtbl;
    //if ( pMVar1 != ( MngBase_vtable* )0x0 ) {
    //    if ( DAT_0114c174 == 1 ) {
    //        FUN_00440b00( pMVar1, "/:SpeedUnit" );
    //        return;
    //    }
    //    FUN_00440b00( pMVar1, "/:SpeedUnit" );
    //}
}
