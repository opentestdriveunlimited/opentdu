#include "shared.h"
#include "gs_car_display.h"

GSCarDisplay* gpCarDisplay = nullptr;

GSCarDisplay::GSCarDisplay()
    : GameSystem()
{
    gpCarDisplay = this;
}

GSCarDisplay::~GSCarDisplay()
{
    gpCarDisplay = nullptr;
}

bool GSCarDisplay::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSCarDisplay::terminate()
{

}
