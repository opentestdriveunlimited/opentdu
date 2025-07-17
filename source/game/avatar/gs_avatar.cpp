#include "shared.h"
#include "gs_avatar.h"

GSAvatar* gpAvatar = nullptr;

GSAvatar::GSAvatar()
    : GameSystem()
{
    gpAvatar = this;
}

GSAvatar::~GSAvatar()
{

}

bool GSAvatar::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSAvatar::terminate()
{

}
