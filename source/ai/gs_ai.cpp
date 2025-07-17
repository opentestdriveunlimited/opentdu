#include "shared.h"
#include "gs_ai.h"

GSAI* gpAI = nullptr;

GSAI::GSAI()
    : GameSystem()
{
    OTDU_ASSERT( gpAI == nullptr );
    gpAI = this;
}

GSAI::~GSAI()
{

}

bool GSAI::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSAI::terminate()
{

}