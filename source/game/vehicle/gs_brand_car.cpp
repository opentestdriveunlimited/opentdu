#include "shared.h"
#include "gs_brand_car.h"

GSBrandCar* gpBrandCar = nullptr;

GSBrandCar::GSBrandCar()
    : GameSystem()
{
    gpBrandCar = this;
}

GSBrandCar::~GSBrandCar()
{

}

bool GSBrandCar::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSBrandCar::terminate()
{

}
