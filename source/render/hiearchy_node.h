#pragma once

class Instance;

struct HiearchyNode {
    uint32_t        Flags;
    Instance*       pInstance;
    uint16_t        NumChildren;
    uint8_t         __PADDING__[6];
    HiearchyNode*   pParent;
    HiearchyNode*   pNext;
    HiearchyNode*   pPrevious;
    uint8_t         __PADDING2__[4];
    Eigen::Matrix4f TransformMatrix;
};

struct HiearchyNodeInstance {
    Eigen::Matrix4f InstanceMatrix;
    HiearchyNode*   pNode;
    Instance*       pInstance;
};
