#pragma once

struct ParticleInstance;

class CinematicEntity {
public:
    CinematicEntity();
    ~CinematicEntity();

protected:
    std::string cutsceneName;
    uint32_t cutsceneIndex;
    ParticleInstance* pParticleInstance;

    uint8_t bInitialized : 1;
    uint8_t bHighPriorityInit : 1;
    uint8_t bNeedRegisterEntity : 1;
};
