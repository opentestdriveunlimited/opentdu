#pragma once

struct Material;
struct MaterialParameterFlags;

class ShaderRegister {
public:
    ShaderRegister();

    void retrieveShadersForMaterial( Material* param_1, const uint32_t index );

private:
    void fillParameterFlags( Material* param_1, MaterialParameterFlags* param_2 );
};

extern ShaderRegister gShaderRegister;
