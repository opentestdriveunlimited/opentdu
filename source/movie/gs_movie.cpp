#include "shared.h"
#include "gs_movie.h"

GSMovie* gpMovie = nullptr;

GSMovie::GSMovie()
    : GameSystem()
{
    gpMovie = this;
}

GSMovie::~GSMovie()
{

}

bool GSMovie::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSMovie::tick(float deltaTime)
{

}

void GSMovie::terminate()
{

}
