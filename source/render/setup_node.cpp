#include "shared.h"
#include "setup_node.h"

static uint32_t gNumLights = 0; // DAT_00facb20
static Light* DAT_00facb18 = nullptr; // DAT_00facb18

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
        
        gNumLights++;
        DAT_00facb18 = lights.back();
    }
}

void LightSetupNode::removeLight(Light *light)
{
    // FUN_0050b7a0
    auto it = std::find_if(lights.begin(), lights.end(), [=](Light* x) { return x == light; });
    if (it != lights.end()) {
        lights.erase(it);

        gNumLights--;
        DAT_00facb18 = lights.back();
    }
}

bool LightSetupNode::isEmpty() const
{
    return lights.empty();
}

SetupGraph::SetupGraph()
    : flags(0)
{

}

SetupGraph::~SetupGraph()
{

}

void SetupGraph::removeLightNodes()
{
    // FUN_0050de60
    if ((flags & 1) != 0) {
        return;
    }

    // This is basically FUN_0050dda0 (without the custom container the game uses)
    std::remove_if(nodes.begin(), nodes.end(), [](SetupNode* x) { return x->getType() == 0x0; });
}

void SetupGraph::removeFrustumNodes()
{
    // FUN_0050de60
    if (((flags >> 2) & 1) != 0) {
        return;
    }

    // This is basically FUN_0050dda0 (without the custom container the game uses)
    std::remove_if(nodes.begin(), nodes.end(), [](SetupNode* x) { return x->getType() == 0x02; });
}

bool SetupGraph::addNode(SetupNode *node)
{
    // FUN_0050dec0
    nodes.push_back(node);

    uint8_t nodeType = static_cast<uint8_t>(node->getType());
    flags |= 1 << (nodeType & 0x1f);

    return true;
}
