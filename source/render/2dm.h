#pragma once

#include "render_file.h"

struct Material;
class Render2DB;
class RenderUVA;

class Render2DM : public RenderFile {
public:
    Render2DM();
    ~Render2DM();

    bool initialize( void* pStream );
    virtual bool parseSection(RenderFile::Section* pSection) override;

    void bindBitmapReference( const Render2DB* param_2 );
    void bindUVAnimationReference( const RenderUVA* param_2 );

    Material* create( void* pContent, uint64_t param_2, int32_t param_3, uint32_t param_4, uint32_t param_5, uint32_t param_6 );

    RenderFile::Section* getMaterial( uint64_t param_2 );

    static uint32_t CalcSize(int32_t param_1, uint32_t param_2)
    {
        return param_1 * 0x10 + 0x2d3 + param_2 & 0xfffffff0;
    }

private:
    struct HashTableEntry {
        uint64_t                Hashcode;
        RenderFile::Section*    pMaterial;
    };

private:
    RenderFile::Section* pMatArray;
    RenderFile::Section* pHashcodes;
    RenderFile::Section* pParameters;
    RenderFile::Section* pLayers;
};
