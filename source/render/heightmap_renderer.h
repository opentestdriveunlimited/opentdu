#pragma once

struct GPUVertexLayout;
struct GPUBuffer;

class HeightmapRenderer {
public:
    HeightmapRenderer();
    ~HeightmapRenderer();

    void destroy();
    bool initialize();

private:
    GPUVertexLayout* pVertexLayout;
    GPUBuffer* pVertexBuffer;
};

class HeightmapTileRenderer {
public:
    HeightmapTileRenderer();
    ~HeightmapTileRenderer();

    void destroy();
    bool initialize();

private:
    struct DrawCommand {
        int16_t PrimitiveCount;
        int16_t StartIndex;
        int16_t MinVertexIndex;
        int16_t NumVertices;
    };

    static constexpr uint32_t kMaxNumDrawCommand = 0x7d;

private:
    GPUBuffer* pVertexBuffer;
    GPUBuffer* pIndexBuffer;

    std::array<DrawCommand, kMaxNumDrawCommand> drawCommands;
    uint32_t primitiveCount;

private:
    void uploadIndexBuffer();
};

extern HeightmapTileRenderer gHeightmapTileRenderer;
extern HeightmapRenderer gHeightmapRenderer;
