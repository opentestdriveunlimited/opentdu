#pragma once

#if OTDU_OPENGL
struct GPUBuffer
{
    uint32_t NativeBuffer = 0u;
};

struct GPUTexture
{
    uint32_t NativeTextureFormat = 0u;
    uint32_t NativeTexture = 0u;
};

class RenderDevice {
public:
    RenderDevice();
    ~RenderDevice();

    void initialize();

    GPUTexture*   createTexture( const struct GPUTextureDesc* desc );
    GPUBuffer*    createBuffer( const struct GPUBufferDesc* desc );

    void* lockBuffer( GPUBuffer* pBuffer, const uint32_t offset, const uint32_t size );
    void unlockBuffer( GPUBuffer* pBuffer );

    void generateMipchain( GPUTexture* pTexture );

    void destroyTexture( GPUTexture* pTexture );
    void destroyBuffer( GPUBuffer* pBuffer );

private:
    uint32_t frameIndex;
};
#endif
