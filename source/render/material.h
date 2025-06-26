#pragma once

#include "render/render_file.h"

struct CachedShader;

enum eMaterialParameterType : uint32_t {
    MPT_Unknown = 0,
    MPT_Terrain = 1, // TODO: Might be more generic?
    MPT_Generic = 2
};

struct MaterialLayerTextureAttribute 
{
    uint32_t NumAttributes;
    uint8_t __PADDING__[0xc];
};

struct MaterialLayerTexture 
{
    uint64_t HashcodeUnknown;
    uint64_t Hashcode;
    RenderFile::Section* pTextureSection;
    uint8_t UnknownBytes[8];
    MaterialLayerTextureAttribute* pAttributes;
};

struct MaterialLayer 
{
    uint32_t Flags;
    uint32_t Flags2;
    uint32_t UVMapIndex;
    uint8_t NumTextures;
    uint8_t AlphaValue;
    uint16_t __PADDING__;
    MaterialLayerTexture pLayerTextures[8];

    uint8_t getNumUsedTextureSlots();
};

struct MaterialParameterFlags 
{
    uint8_t VersionMajor;
    uint8_t VersionMinor;
    uint8_t __PADDING__[2];
    uint8_t NumTextures;
    uint8_t bUseTextureMatrix;
    uint8_t Unknown;
    uint8_t NumLightmaps;
    uint8_t NumFXMap;
    uint8_t bUsingGlossMap;
    uint8_t bUsingOffsetMap;
    uint8_t bUsingTangentSpace;
    uint8_t bUsingReflection;
    uint8_t bUsingAmbient;
    uint8_t bUsingEmissive;
    uint8_t bUsingSpecular;
    uint8_t bUsingBumpMap;
    uint8_t bUsingSpriteRotation;
    uint8_t bUsingDepthSprite;
    uint8_t bUsingDiffuse;
    uint8_t NumUVMaps;
    uint8_t bUsingSkinning;
    uint8_t NumPrelight;
    uint8_t bUsingNormals;
    uint8_t bUsingTangent;
    uint8_t bUsingBinormal;
    uint8_t bNeedDepthPrepass;
    uint8_t UnknownBytes[5];
    uint8_t bUnknown2;
    uint8_t UnknownBytes2[4];
    uint8_t bAffectedByFog;
    uint8_t __PADDING2__[2];
    uint8_t pCustomFlags[8];
};
static_assert( sizeof( MaterialParameterFlags ) == 0x30, "Size mismatch! Material deserialization will break" );

struct MaterialParameter 
{
    eMaterialParameterType Type;
    uint32_t NumLayers;
    uint64_t Hashcode;
    MaterialParameterFlags Flags;

    MaterialLayer* getLayer( const uint32_t index );
};

struct MaterialShaderParameter
{
    uint32_t Type;
    uint32_t Format;
    uint32_t Size;
    uint32_t bBindToVS;
};

struct MaterialShaderParameterArray : public MaterialParameter 
{
    uint32_t NumParameters;
    RenderFile::Section* pParameters; // TODO: Check deserialization (as x86 pointer are 4 bytes we are reading padding)
    uint32_t __PADDING__;

    MaterialShaderParameter* getParameter( const uint32_t type, uint32_t* pOutParamIndex = nullptr );
    void* getParameterData( const uint32_t type );
    void* getParameterDataByIndex( uint32_t index );
};

struct MaterialTerrainParameter : public MaterialParameter {
    uint32_t                AlphaBlendModeTest[8]; // TODO: I assume this is used to control layer blending (but still need confirmation)
    RenderFile::Section*    pLayers[8];
};

struct Material 
{
    uint16_t OT;
    uint16_t NumParams;
    uint32_t FXFlags;
    uint32_t FXFlags2;
    uint32_t pOTNodePointer32;
    uint8_t DepthTest;
    uint8_t DepthWrite;
    uint8_t AlphaSource;
    uint8_t BackfaceCullingMode;
    uint8_t FillMode;
    uint8_t AlphaTest;
    uint8_t MatrixParam;
    uint8_t AlphaRef;
    uint8_t SrcBlend;
    uint8_t DstBlend;
    uint8_t BlendOP;
    uint8_t ColorDisable;
    uint32_t UsageFlags;
    uint8_t StencilEnable;
    uint8_t StencilFail;
    uint8_t StencilZFail;
    uint8_t StencilPass;
    uint8_t StencilFunc;
    uint32_t StencilRef;
    uint32_t StencilMask;
    uint32_t StencilWriteMask;
    uint8_t StencilFailCW;
    uint8_t StencilZFailCW;
    uint8_t StencilPassCW;
    uint8_t StencilFuncCW;
    uint32_t StencilRefCW;
    uint32_t StencilMaskCW;
    uint32_t StenwilWriteMaskCW;
    CachedShader* pVertexShaders[4];
    CachedShader* pPixelShaders[4];
    Eigen::Vector4f AmbientColor;
    Eigen::Vector4f DiffuseColor;
    Eigen::Vector4f SpecularColor;
    Eigen::Vector4f EmissiveColor;

    Material();
    ~Material();

    void setDefaultMaterial();
    uint16_t getOTNumber();
    MaterialParameter* getParameterByIndex( const uint32_t index );
    uint16_t getOTForParameter( MaterialParameter* param_1 );
};
static_assert( sizeof( Material ) == 0xd0, "Size mismatch; material deserialization will fail!!" );

class MaterialRegister {
public:
    MaterialRegister();

    void pushMaterial( RenderFile::Section* pFile );
    void popMaterial();
    
private:
    uint32_t numRegisteredMaterial;
};

extern MaterialRegister gMaterialRegister;
