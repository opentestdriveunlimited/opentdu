#include "shared.h"
#include "gs_subtitles.h"

GSSubtitles* gpSubtitles = nullptr;

GSSubtitles::GSSubtitles()
    : GameSystem()
{
    gpSubtitles = this;
}

GSSubtitles::~GSSubtitles()
{

}

bool GSSubtitles::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSSubtitles::terminate()
{

}
