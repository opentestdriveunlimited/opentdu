#pragma once

#include "render_file.h"

struct Material;

class Render2DM : public RenderFile {
public:
    Render2DM();
    ~Render2DM();

    virtual bool parseSection(RenderFile::Section* pSection) override;

    Material* create( void* pContent, uint64_t param_2, int32_t param_3, uint32_t param_4, uint32_t param_5, uint32_t param_6 );
    
private:
    RenderFile::Section* pMatArray;
    RenderFile::Section* pHashcodes;
    RenderFile::Section* pParameters;
    RenderFile::Section* pLayers;
};
