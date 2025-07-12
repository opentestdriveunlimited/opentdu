#include "shared.h"
#include "gps_voice.h"

#include "config/gs_config.h"
#include "player_data/gs_playerdata.h"
#include "filesystem/gs_file.h"

static constexpr uint32_t kMaxNumSamples = 0x11;

GPSVoice::GPSVoice()
    : Bank()
    , audioState( eAudioState::GVAS_Unknown )
    , pMemoryPool( nullptr )
    , bEnabled( true )
{
    audioSamples.resize( kMaxNumSamples );
}

GPSVoice::~GPSVoice()
{
}

bool GPSVoice::initialize()
{
    if (audioState != eAudioState::GVAS_Unknown || pMemoryPool == nullptr) {
        return true;
    }

    const char* pPathRoot = gpConfig->getResRootPath();
    const char* pActiveLanguage = gpPlayerData->getLanguage();

    std::string bankPath = pPathRoot;
    bankPath += "Sound/Voices/";
    bankPath += pActiveLanguage;
    bankPath += "/GPS.bnk";

    if (!bLoaded) {
        bool bVar2 = loadBank(gpFile, bankPath.c_str());
        if (bVar2) {
            audioState = eAudioState::GVAS_WaitingForParsing;
        }
    }
}
