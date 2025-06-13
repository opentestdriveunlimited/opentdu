#pragma once

#include "render_file.h"

class Render3DG : public RenderFile {
public:
    Render3DG();
    ~Render3DG();

    virtual bool parseSection(RenderFile::Section* pSection) override;

private:
    RenderFile::Section* pGeometryArray;
};
