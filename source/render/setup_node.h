#pragma once

struct Light;

class SetupNode {
public:
    SetupNode();
    virtual ~SetupNode() {}

    virtual void execute() = 0;
    virtual uint32_t getType() const = 0;

protected:
    uint32_t flags;
};

class LightSetupNode : public SetupNode {
public:
    LightSetupNode();
    ~LightSetupNode();

    void execute() override;
    uint32_t getType() const override;

    void addLight(Light* light);

private:
    std::vector<Light*> lights;
};

class SetupGraph {
public:
    SetupGraph();
    ~SetupGraph();

    bool addNode(SetupNode* node);

private:
    uint32_t flags;
    std::vector<SetupNode*> nodes;
};
