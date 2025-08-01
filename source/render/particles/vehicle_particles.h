#pragma once

struct ParticleInstance;
class VehicleBase;

class VehicleParticles {
public:
    VehicleParticles();
    ~VehicleParticles();

    void update(VehicleBase* param_2);
    void reset();
    void setState(bool param_2);

private:
    static constexpr const uint32_t kNumInstancePerType = 4;

private:
    std::array<ParticleInstance*, kNumInstancePerType> burnParticles; // 0x00f29d80
    std::array<ParticleInstance*, kNumInstancePerType> smokeParticles; // 0x00f29d74
    std::array<ParticleInstance*, kNumInstancePerType> dustParticles; // 0x00f29d5c
    std::array<ParticleInstance*, kNumInstancePerType> gravelParticles; // 0x00f29d44
    std::array<ParticleInstance*, kNumInstancePerType> grassParticles; // 0x00f29d50
    std::array<ParticleInstance*, kNumInstancePerType> startExhaustParticles; // 0x00f29d50
};
