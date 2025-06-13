#pragma once

#include "3dd.h"
#include "3dg.h"
#include "instance.h"
#include "hiearchy_node.h"

struct FileCollection3D
{
    std::vector<Render3DD>      Render3DDs;
    std::vector<Render3DG>      Render3DGs;
    std::vector<Instance>       Instances;
    std::vector<HiearchyNode>   HiearchyNodes;
};
