#include "shared.h"
#include "postfx_stack.h"

#include "postfx_instance.h"

PostFXStack gPostFXStack = {};

// Arbitrary limit from the original game.
static constexpr uint32_t kMaxNumEffect = 64;

PostFXStack::PostFXStack()
{

}

PostFXStack::~PostFXStack()
{
    releaseResources();
}

void PostFXStack::registerEffect(PostFX* param_2)
{
    OTDU_ASSERT( effects.size() < kMaxNumEffect );
    effects.push_back(param_2);
}

bool PostFXStack::releaseResources()
{
    // FUN_005f5d60
    for ( PostFX* pEffect : effects ) {
        pEffect->releaseResources();
    }

    return true;
}
