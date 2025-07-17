#include "shared.h"
#include "gs_car_common.h"

GSCarCommon* gpCarCommon = nullptr;

GSCarCommon::GSCarCommon()
    : GameSystem()
{
    gpCarCommon = this;
}

GSCarCommon::~GSCarCommon()
{

}
