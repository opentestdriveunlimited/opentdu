#include "shared.h"
#include "material.h"

Material::Material()
{
    setDefaultMaterial();
}

Material::~Material()
{

}

void Material::setDefaultMaterial()
{
    OT = 0;
    FXFlags = 0;
    FXFlags2 = 0;
    pOTNode = ( void* )0x0;
    AlphaSource = '\0';
    FillMode = '\0';
    MatrixParam = '\0';
    AlphaRef = '\0';
    ColorDisable = '\0';
    UsageFlags = 0;
    StencilEnable = '\0';
    StencilFail = '\0';
    StencilZFail = '\0';
    StencilRef = 0;
    StencilFailCW = '\0';
    StencilZFailCW = '\0';
    StencilRefCW = 0;
    BackfaceCullingMode = '\x03';
    SrcBlend = '\x02';
    BlendOP = '\x06';
    StencilPass = '\x02';
    StencilFunc = '\a';
    StencilPassCW = '\x02';
    StencilFuncCW = '\a';
    NumParams = 1;
    DepthTest = '\x01';
    DepthWrite = '\x01';
    AlphaTest = '\x01';
    DstBlend = '\x01';
    StencilMask = 0xffffffff;
    StencilWriteMask = 0xffffffff;
    StencilMaskCW = 0xffffffff;
    StenwilWriteMaskCW = 0xffffffff;
    pVertexShaders[0] = ( void* )0x0;
    pPixelShaders[0] = ( void* )0x0;
    pVertexShaders[1] = ( void* )0x0;
    pPixelShaders[1] = ( void* )0x0;
    pVertexShaders[2] = ( void* )0x0;
    pPixelShaders[2] = ( void* )0x0;
    pVertexShaders[3] = ( void* )0x0;
    pPixelShaders[3] = ( void* )0x0;
    pVertexShaders[4] = ( void* )0x0;
    pPixelShaders[4] = ( void* )0x0;
    pVertexShaders[5] = ( void* )0x0;
    pPixelShaders[5] = ( void* )0x0;
    pVertexShaders[6] = ( void* )0x0;
    pPixelShaders[6] = ( void* )0x0;
    pVertexShaders[7] = ( void* )0x0;
    pPixelShaders[7] = ( void* )0x0;
    AmbientColor = Eigen::Vector4<float>( 1.0f, 1.0f, 1.0f, 1.0f );
    DiffuseColor = Eigen::Vector4<float>( 1.0f, 1.0f, 1.0f, 1.0f );
    SpecularColor = Eigen::Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
    EmissiveColor = Eigen::Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
}
