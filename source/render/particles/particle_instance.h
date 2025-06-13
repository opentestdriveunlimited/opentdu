#pragma once

struct ParticleInstance;

enum class eParticleSimulationType {
    PST_Default = 0,
    PST_Sparkle = 1
};

enum class eParticleType : uint32_t {
    PT_Burn = 0,
    PT_Smoke = 1,
    PT_Dust = 2,
    PT_StartLine = 3,
    PT_Grass = 4,
    PT_Gravel = 5,
    PT_Sparkle = 6,
    PT_Wood = 7,
    PT_Leaf = 8,
    PT_Leaf2 = 9,
    PT_CarBody = 10,
    PT_Glass = 11,
    PT_Plastic = 12,
    PT_Stone = 13,
    PT_Metal = 14,
    
    PT_Invalid = 0xffffffff
};

enum class eParticleState {
    PS_Dead = 0,
    PS_Alive = 1,
    PS_Killed = 2,
};

struct Particle
{
    std::vector<ParticleInstance*> Instances;
    std::string Name;
    uint64_t Hashcode;
    eParticleType Type;
    eParticleSimulationType SimuType;
};

struct ParticleInstance
{
    Particle*       pParticleInstanciated;
    eParticleState  State;
    Eigen::Matrix4f Matrix;
    Eigen::Vector4f Color;
    float           Scale;
    float           Density;
    float           Speed;
};
