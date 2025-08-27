#include "shared.h"
#include "game_intro_tutorial.h"

GameIntroTutorial::GameIntroTutorial()
    : tutorialStep( 0 )
{

}

GameIntroTutorial::~GameIntroTutorial()
{

}

bool GameIntroTutorial::isActive() const
{
    // FUN_00406310
    return tutorialStep != 0xd && tutorialStep != 0x0;
}