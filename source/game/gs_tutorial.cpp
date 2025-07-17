#include "shared.h"
#include "gs_tutorial.h"

GSTutorial* gpTutorial = nullptr;

GSTutorial::GSTutorial()
    : GameSystem()
{
    gpTutorial = this;
}

GSTutorial::~GSTutorial()
{

}

bool GSTutorial::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSTutorial::terminate()
{

}
