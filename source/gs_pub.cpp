#include "shared.h"
#include "gs_pub.h"

GSPub* gpPub = nullptr;

GSPub::GSPub()
    : GameSystem()
{
    gpPub = this;
}

GSPub::~GSPub()
{

}

bool GSPub::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSPub::terminate()
{

}
