#include "shared.h"
#include "setup_node.h"

SetupNode::SetupNode()
    : flags( 0 )
{

}

LightSetupNode::LightSetupNode()
    : SetupNode()
{
    flags = 0x103;
}

LightSetupNode::~LightSetupNode()
{
    lights.clear();
}

static constexpr uint32_t kMaxNumLightPerScene = 4;
// TODO: Who owns this?
static Light* gBoundLights[kMaxNumLightPerScene] = { nullptr, nullptr, nullptr, nullptr };

void LightSetupNode::execute()
{
    // FUN_0050b8a0
    for (uint32_t i = 0; i < lights.size(); i++) {
        gBoundLights[i] = lights[i];
    }

    for (uint32_t j = lights.size(); j < kMaxNumLightPerScene; j++) {
        gBoundLights[j] = nullptr;
    }
}

uint32_t LightSetupNode::getType() const
{
    // FUN_0047a8a0
    return 0;
}

void LightSetupNode::addLight(Light *light)
{
    if (lights.size() < kMaxNumLightPerScene) {
        lights.push_back(light);
    }
}

SetupGraph::SetupGraph()
    : flags(0)
{

}

SetupGraph::~SetupGraph()
{

}

bool SetupGraph::addNode(SetupNode *node)
{
    // FUN_0050dec0
    nodes.push_back(node);

    uint8_t nodeType = static_cast<uint8_t>(node->getType());
    flags |= 1 << (nodeType & 0x1f);

    return true;
}
