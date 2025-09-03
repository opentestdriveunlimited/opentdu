#include "shared.h"
#include "gs_pub.h"

#include "core/arg_parser.h"

static bool gNoPub = false; // DAT_0143f636
static CmdLineArg CmdLineArgNoPub( "nopub", []( const char* pArg ) { gNoPub = true; } );

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
