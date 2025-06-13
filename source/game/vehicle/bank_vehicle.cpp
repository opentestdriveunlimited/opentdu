#include "shared.h"
#include "bank_vehicle.h"

BankVehicle::BankVehicle()
    : Bank()
    , StreamedResource()
    , resourceState( eState::BVS_None )
    , name( "" )
    , bLoadAsync( false )
    , bIsDefaultRes( false )
    , file3DGPath( "" )
    , pPhysicsBinary( nullptr )
    , pTorqueBinary( nullptr )
    , pPhysFactory( nullptr )
{
    
}

BankVehicle::~BankVehicle()
{
}
