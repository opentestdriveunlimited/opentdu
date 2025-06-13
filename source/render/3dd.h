#pragma once

#include "render_file.h"

class Render3DD : public RenderFile {
public:
    Render3DD();
    ~Render3DD();

    virtual bool parseSection(RenderFile::Section* pSection) override;

private:
    RenderFile::Section* pMatBankArray;
    RenderFile::Section* pObjectArray;
    RenderFile::Section* pInstanceArray;
    RenderFile::Section* pHiearchyArray;
};
