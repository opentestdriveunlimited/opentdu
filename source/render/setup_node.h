#pragma once

struct Light;
struct Material;

static constexpr uint32_t kMaxNumLightPerScene = 4;

class SetupNode {
public:
    SetupNode();
    virtual ~SetupNode() {}

    virtual void execute() = 0;
    virtual uint32_t getType() const = 0;

    uint32_t getFlags() const;

protected:
    uint32_t flags;
};

struct ShaderParameter {
    uint64_t ShaderHashcode = 0ull;
    Material* pMaterial = nullptr;
    uint32_t ParameterID = 0;
    void* pParameters = nullptr;
    int32_t NumParameters = 0;
    bool bBindToPixelShader = false;
};

class ShaderParameterSetupNode : public SetupNode {
public:
    ShaderParameterSetupNode();
    ~ShaderParameterSetupNode();

    void execute() override;
    uint32_t getType() const override;

private:
    std::vector<ShaderParameter> parameters;
};

class LightSetupNode : public SetupNode {
public:
    LightSetupNode();
    ~LightSetupNode();

    void execute() override;
    uint32_t getType() const override;

    void addLight(Light* light);
    void removeLight(Light* light);

    bool isEmpty() const;

private:
    std::vector<Light*> lights;
};

class SetupGraph {
public:
    SetupGraph();
    ~SetupGraph();

    void removeLightNodes();
    void removeFrustumNodes();

    bool addNode(SetupNode* node);
    void bind(uint32_t param_1) const;
    void unbind(uint32_t param_1);

    static bool ExecuteCached(uint32_t param_1);

private:
    uint32_t flags;
    std::vector<SetupNode*> nodes;
};

extern Light* gBoundLights[kMaxNumLightPerScene];
extern ShaderParameter* gpActiveStaticParams;
extern size_t gActiveNumStaticParams;
