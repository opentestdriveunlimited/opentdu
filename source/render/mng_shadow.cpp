#include "shared.h"
#include "mng_shadow.h"

MngShadow* gpMngShadow = nullptr;

MngShadow::MngShadow()
    : Manager()
{
    OTDU_UNIMPLEMENTED;
    OTDU_ASSERT(gpMngShadow == nullptr);
    gpMngShadow = this;
}

MngShadow::~MngShadow()
{
    gpMngShadow = nullptr;
}

bool MngShadow::initialize(TestDriveGameInstance *)
{
    return false;
}

void MngShadow::tick(float, float)
{
}

void MngShadow::draw()
{
}

void MngShadow::terminate()
{
}
