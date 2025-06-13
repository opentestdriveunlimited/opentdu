#pragma once

struct Material;

class Render2DM;
class Render2DB;
struct Texture;

struct FlashSound {
    int32_t CharID = -1;
    void* pSound = nullptr;
};

struct FlashMaterial {
    int32_t ID;

    union {
        uint32_t Hash[8];
        char     Bytes[32];
    };

    Material* pMaterial;
    Texture* pTexture;
};

struct Flash2DM {
    void*       pBuffer = nullptr;
    Render2DM*  p2DM = nullptr;
};

struct FlashMovie {
    std::string Filename;
    int32_t GameMode;
    int32_t MaxNumVars;
    int32_t NumDrawListStrips;
    int32_t NumDrawListVertices;

    FlashMovie()
        : Filename("")
        , GameMode(-1)
        , MaxNumVars(0)
        , NumDrawListStrips(0)
        , NumDrawListVertices(0)
    {
        
    }
};
