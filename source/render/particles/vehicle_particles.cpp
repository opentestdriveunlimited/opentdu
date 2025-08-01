#include "shared.h"
#include "vehicle_particles.h"

VehicleParticles::VehicleParticles()
{
    reset();
}

VehicleParticles::~VehicleParticles()
{

}

void VehicleParticles::update(VehicleBase* param_2)
{
    OTDU_UNIMPLEMENTED;
}

void VehicleParticles::reset()
{
    for (ParticleInstance* pInstance : burnParticles) 
    {
        pInstance = nullptr;
    }
    
    for (ParticleInstance* pInstance : smokeParticles) 
    {
        pInstance = nullptr;
    }
    
    for (ParticleInstance* pInstance : dustParticles) 
    {
        pInstance = nullptr;
    }
    
    for (ParticleInstance* pInstance : gravelParticles) 
    {
        pInstance = nullptr;
    }
    
    for (ParticleInstance* pInstance : grassParticles) 
    {
        pInstance = nullptr;
    }

    for (ParticleInstance* pInstance : startExhaustParticles) 
    {
        pInstance = nullptr;
    }
}

void VehicleParticles::setState(bool param_2)
{
    OTDU_UNIMPLEMENTED;
}
