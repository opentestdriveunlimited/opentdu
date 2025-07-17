#include "shared.h"
#include "gs_radio.h"

GSRadio* gpRadio = nullptr;

GSRadio::GSRadio()
    : GameSystem()
{
    gpRadio = this;
}

GSRadio::~GSRadio()
{

}

bool GSRadio::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSRadio::terminate()
{

}
