#include "shared.h"
#include "gs_details.h"

GSDetails* gpDetails = nullptr;

GSDetails::GSDetails()
    : GameSystem()
{
    OTDU_ASSERT( gpDetails == nullptr );
    gpDetails = this;
}

GSDetails::~GSDetails()
{

}

bool GSDetails::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSDetails::terminate()
{

}