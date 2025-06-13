#pragma once

struct RenderHashcode {
    uint64_t    Value;
    void*       pOffset;
};

struct LODDef {
    RenderHashcode* pHashcode;
    std::array<float, 4> Distances;
};

struct LOD {
    RenderHashcode* pHashcode;
    void* pUnknown;
    std::array<float, 4> Distances;
};
