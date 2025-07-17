#include "shared.h"
#include "gs_world_streaming.h"

GSWorldStreaming* gpWorldStreaming = nullptr;

GSWorldStreaming::GSWorldStreaming()
    : GameSystem()
{
    OTDU_ASSERT( gpWorldStreaming == nullptr );
    gpWorldStreaming = this;
}

GSWorldStreaming::~GSWorldStreaming()
{

}

bool GSWorldStreaming::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSWorldStreaming::terminate()
{

}
