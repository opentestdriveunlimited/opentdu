#include "shared.h"
#include "gs_audio.h"

GSAudio* gpAudio = nullptr;

GSAudio::GSAudio()
    : NbSpeakers( 5 )
    , OutputSpatialFactor( 0.0f )
    , OutputDopplerFactor( 0.0f )
{
    gpAudio = this;
}

GSAudio::~GSAudio()
{

}

bool GSAudio::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSAudio::tick()
{

}

void GSAudio::terminate()
{

}

void GSAudio::addMixer( TDUMixer* mixer )
{

}
