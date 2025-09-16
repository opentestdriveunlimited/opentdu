#pragma once

#include "gs_render_helper.h"
#include "render_file.h"

struct GeometryBuffer {
    x86Pointer_t pGPUBuffer;
    x86Pointer_t pCPUBuffer;
    uint32_t     ByteSize;
    uint32_t     LockStart;
    uint32_t     LockEnd;
    x86Pointer_t pCPUBufferCopy;
    uint16_t     DataOffset;
    uint8_t      bLocked;
    uint8_t      Flags;
    x86Pointer_t pGPUBufferCopy;
    int8_t       __PADDING__[0x10];
    
    void* lock(const uint32_t startOffset, const uint32_t length);
    // NOTE: This could be merged into a single function (only the D3D9 buffer type differs)
    void uploadVerticesToGPU();
    void uploadIndicesToGPU();
};
OTDU_SIZE_MUST_MATCH(GeometryBuffer, 0x30);

struct GeomtryBufferWithHeader {
    RenderFile::Section Section;
    GeometryBuffer Buffer;
};
OTDU_SIZE_MUST_MATCH(GeomtryBufferWithHeader, 0x40);
