#include "shared.h"
#include "setup_node.h"

// DAT_00fe7028
struct NodeCacheEntry
{
    int32_t NumNodes = -1;
    std::list<std::tuple<SetupNode*, uint32_t>> Nodes;
};

static uint32_t gNumLights = 0; // DAT_00facb20
static Light* DAT_00facb18 = nullptr; // DAT_00facb18
static NodeCacheEntry gNodeCache[4];
static int32_t gRenderSetupCacheFlags = 0; // DAT_00fe7008
Light* gBoundLights[kMaxNumLightPerScene] = { nullptr, nullptr, nullptr, nullptr }; // DAT_00facb04

ShaderParameter* gpActiveStaticParams = nullptr; // DAT_00facb68
size_t gActiveNumStaticParams = 0ull; // DAT_00facb6c

SetupNode::SetupNode()
    : flags( 0 )
{

}

uint32_t SetupNode::getFlags() const
{
    return flags;
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

void LightSetupNode::execute()
{
    // FUN_0050b8a0
    for (uint32_t i = 0; i < lights.size(); i++) {
        gBoundLights[i] = lights[i];
    }

    for (size_t j = lights.size(); j < kMaxNumLightPerScene; j++) {
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
    nodes.erase( std::remove_if( nodes.begin(), nodes.end(), []( SetupNode* x ) { return x->getType() == 0x0; } ), nodes.end() );
}

void SetupGraph::removeFrustumNodes()
{
    // FUN_0050de60
    if (((flags >> 2) & 1) != 0) {
        return;
    }

    // This is basically FUN_0050dda0 (without the custom container the game uses)
    nodes.erase( std::remove_if( nodes.begin(), nodes.end(), []( SetupNode* x ) { return x->getType() == 0x02; } ), nodes.end() );
}

bool SetupGraph::addNode(SetupNode *node)
{
    // FUN_0050dec0
    nodes.push_back(node);

    uint8_t nodeType = static_cast<uint8_t>(node->getType());
    flags |= 1 << (nodeType & 0x1f);

    return true;
}

void SetupGraph::bind(uint32_t param_1) const
{
    for (SetupNode* peVar1 : nodes) {
        if ((peVar1->getFlags() >> 8 & 1) != 0) {
            uint32_t iVar2 = peVar1->getType();
            gNodeCache[iVar2].NumNodes++;
            gNodeCache[iVar2].Nodes.push_back(std::make_tuple(peVar1, param_1));
            gRenderSetupCacheFlags |= 1 << (iVar2 & 0x1f);
        }
    }
}

void SetupGraph::Unbind(uint32_t param_1)
{
    // FUN_0050e640
    if (gNodeCache[0].NumNodes != -1 
        && std::get<1>(gNodeCache[0].Nodes.back()) == param_1) {
        gNodeCache[0].NumNodes--;
        gNodeCache[0].Nodes.pop_back();
        if (gNodeCache[0].NumNodes == -1) {
            gRenderSetupCacheFlags = gRenderSetupCacheFlags & 0xfffffffe;
        }
    }

    if (gNodeCache[1].NumNodes != -1 
        && std::get<1>(gNodeCache[1].Nodes.back()) == param_1) {
        gNodeCache[1].NumNodes--;
        gNodeCache[1].Nodes.pop_back();
        if (gNodeCache[1].NumNodes == -1) {
            gRenderSetupCacheFlags = gRenderSetupCacheFlags & 0xfffffffd;
        }
    }
    
    if (gNodeCache[2].NumNodes != -1 
        && std::get<1>(gNodeCache[2].Nodes.back()) == param_1) {
        gNodeCache[2].NumNodes--;
        gNodeCache[2].Nodes.pop_back();
        if (gNodeCache[2].NumNodes == -1) {
            gRenderSetupCacheFlags = gRenderSetupCacheFlags & 0xfffffffb;
        }
    }
}

bool SetupGraph::ExecuteCached(uint32_t param_1)
{
    // FUN_00513410
    uint32_t uVar1 = gNodeCache[param_1].NumNodes;
     if (uVar1 != -1 
        && std::get<0>(gNodeCache[param_1].Nodes.back()) != nullptr) {
        SetupNode* puVar2 = std::get<0>(gNodeCache[param_1].Nodes.back());
        if ((puVar2->getFlags() >> 9 & 1) != 0) {
            // TODO: This is supposed to call the first function in the vtable.
            // This function is not implemented in any child class so I have no idea if
            // it's used or not...
            OTDU_UNIMPLEMENTED;
        }

        puVar2->execute();
        return true;
    }

    return false;
}

ShaderParameterSetupNode::ShaderParameterSetupNode()
{
}

ShaderParameterSetupNode::~ShaderParameterSetupNode()
{
}

void ShaderParameterSetupNode::execute()
{
    // FUN_0050e49
    gpActiveStaticParams = parameters.data();
    gActiveNumStaticParams = parameters.size();
}

uint32_t ShaderParameterSetupNode::getType() const
{
    // FUN_00423f20
    return 1;
}
