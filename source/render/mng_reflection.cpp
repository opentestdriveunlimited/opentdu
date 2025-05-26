#include "shared.h"
#include "mng_reflection.h"

MngReflection* gpMngReflection = nullptr;

MngReflection::MngReflection()
    : Manager()
{
    OTDU_UNIMPLEMENTED;
    OTDU_ASSERT(gpMngReflection == nullptr);
    gpMngReflection = this;
}

MngReflection::~MngReflection()
{
    gpMngReflection = nullptr;
}

bool MngReflection::initialize(TestDriveGameInstance *)
{
    return false;
}

void MngReflection::tick(float, float)
{
}

void MngReflection::draw()
{
}

void MngReflection::terminate()
{
}
