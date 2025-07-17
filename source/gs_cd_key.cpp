#include "shared.h"
#include "gs_cd_key.h"

GSCDKey* gpCDKey = nullptr;

GSCDKey::GSCDKey()
    : GameSystem()
{
    gpCDKey = this;
}

GSCDKey::~GSCDKey()
{

}

bool GSCDKey::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSCDKey::terminate()
{

}
