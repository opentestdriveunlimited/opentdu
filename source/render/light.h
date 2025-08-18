#pragma once

enum class eLightType {
    LT_Invalid = 0,	
    LT_Ambient	= 1,	
    LT_Directional = 2,	
    LT_Point = 3,	
    LT_Spot = 4,	
    LT_Count = 4	
};

struct Light {
    eLightType Type;

    Light(eLightType lightType)
        : Type(lightType)
    {
        
    }
};

struct DirectionalLight : public Light {
     DirectionalLight()
        : Light(eLightType::LT_Directional)
        , Direction( Eigen::Vector3f::Zero() )
    {
        
    }

    Eigen::Vector4f Diffuse;
    Eigen::Vector4f Ambient;
    Eigen::Vector4f Specular;
    Eigen::Vector3f Direction;
};

struct AmbientLight : public Light {
     AmbientLight()
        : Light(eLightType::LT_Ambient)
    {
        
    }

    Eigen::Vector4f Color;
};
