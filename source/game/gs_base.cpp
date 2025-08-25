#include "shared.h"
#include "gs_base.h"

GameSystem::GameSystem()
    : index( 0xffffffff )
    , bPaused( false )
{

}

void GameSystem::reset()
{
    OTDU_UNIMPLEMENTED;
}

bool GameSystem::isPaused() const
{
    return bPaused;
}

uint32_t GameSystem::getIndex() const
{
    return index;
}
