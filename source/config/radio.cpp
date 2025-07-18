#include "shared.h"
#include "gs_config.h"

void GSConfig::registerRadioCommands()
{
    registerCommand( "ADDSTATION", [&]( const char* pArg ) {
        OTDU_UNIMPLEMENTED
    } );
}
