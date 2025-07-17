#include "shared.h"
#include "gs_pc_options.h"

GSPCOptions* gpPCOptions = nullptr;

GSPCOptions::GSPCOptions()
    : GameSystem()
{
    gpPCOptions = this;
}

GSPCOptions::~GSPCOptions()
{

}

bool GSPCOptions::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSPCOptions::terminate()
{

}
