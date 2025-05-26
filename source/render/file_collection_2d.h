#pragma once

#include "2dm.h"
#include "2db.h"
#include "uva.h"

struct FileCollection2D
{
    std::vector<Render2DM> Render2DMs;
    std::vector<Render2DB> Render2DBs;
    std::vector<RenderUVA> UVAs;
};
