#pragma once

struct GPUTexture;
struct GPUBuffer;

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool initialize();
    void pushCommand(int32_t x, int32_t y, const char *pText);
    void setColor(ColorRGBA& param_1);

    void submitCommands();

private:
    static constexpr uint32_t kMaxDrawCommands = 256;

    struct DrawCommand {
        int32_t PositionX;
        int32_t PositionY;
        ColorRGBA Color;
        char pText[512];
    };

private:
    std::array<DrawCommand, kMaxDrawCommands> drawCommands;
    uint32_t numDrawCommands;

    std::string fontName;
    GPUTexture* pGlyphAtlas;
    GPUBuffer* pGlyphVertexBuffer;
    void* pStateBlocks[2];
    uint32_t flags;
    uint32_t glyphDimension;
    uint32_t minGlyphDimension;
    ColorRGBA activeColor;
    float fontScale;
    uint32_t glyphAtlasWidth;
    uint32_t glyphAtlasHeight;

private:
    void submitDrawCommand(const float originX, const float originY, const ColorRGBA& color, const char *pText);
};

extern TextRenderer gTextRenderer;

static void DrawDebugText(uint32_t param_1, uint32_t param_2, const char* param_3);
