#include "shared.h"
#include "gs_particles.h"

GSParticles* gpParticles = nullptr;

GSParticles::GSParticles()
    : GameSystem()
{
    OTDU_ASSERT(gpParticles == nullptr);
    gpParticles = this;
}

GSParticles::~GSParticles()
{
    gpParticles = nullptr;
}

bool GSParticles::initialize( TestDriveGameInstance* )
{
    return true;
}

void GSParticles::terminate()
{

}

void GSParticles::FUN_0080d330()
{
    OTDU_UNIMPLEMENTED;
}
