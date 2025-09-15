#include "shared.h"
#include "geometry_buffer.h"

#include "gs_render.h"

static constexpr uint32_t kMaxNumPendingGeoBufferUpload = 1024;

static GeometryBuffer* gpPendingGeometryBufferUploads[kMaxNumPendingGeoBufferUpload]; // DAT_00fae058
static uint32_t gNumPendingGeometryBufferUploads = 0; // DAT_00faf058

void* GeometryBuffer::lock(const uint32_t startOffset, const uint32_t length)
{
    void* pDataPointer = static_cast<uint8_t*>( pCPUBuffer ) + startOffset;
    LockStart = Min(LockStart, startOffset);
    bLocked = true;

    uint32_t endOffset = startOffset + length;
    LockEnd = Max(LockEnd, endOffset);

    return pDataPointer;
}

void GeometryBuffer::uploadVerticesToGPU()
{
    // FUN_005f51d0    
    if (bLocked) {
        uint32_t uVar5 = LockEnd;
        void* piVar3 = pGPUBuffer;
        uint32_t offsetToLock = LockStart;

        if (pCPUBufferCopy != nullptr && gNumPendingGeometryBufferUploads < kMaxNumPendingGeoBufferUpload) {
            gpPendingGeometryBufferUploads[gNumPendingGeometryBufferUploads] = this;
            gNumPendingGeometryBufferUploads++;
        }
        uVar5 -= offsetToLock;

        bool bDiscardContent = (Flags & 1) != 0;
        void* local_4 = gpRender->getRenderDevice()->lockBuffer( pGPUBuffer, offsetToLock, uVar5, bDiscardContent );
        if (local_4 != nullptr) {
            uint8_t* pvVar3 = pCPUBuffer;
            uint8_t* puVar8 = pvVar3 + offsetToLock;        
            memcpy( local_4, puVar8, uVar5 );

            gpRender->getRenderDevice()->unlockBuffer( pGPUBuffer );
        }

        Flags &= 0xfd;
        LockStart = 0xffffffff;
        LockEnd = 0;
        bLocked = false;
        return;
    }
    
    uint8_t bVar2 = Flags;
    if ((Flags & 2) != 0) {
        void* pvVar3 = pCPUBuffer;
        pCPUBuffer = pCPUBufferCopy;
        void* pvVar4 = pCPUBufferCopy;
        pCPUBufferCopy = pvVar3;
        if (pvVar4 != (void *)0x0) {
            pGPUBufferCopy = pGPUBuffer;
            pGPUBuffer = pvVar4;
        }
        Flags |= 2;
    }
    Flags &= 0xfd;
}

void GeometryBuffer::uploadIndicesToGPU()
{
    // FUN_005f50f0
    if (bLocked) {
        uint32_t uVar5 = LockEnd;
        void* piVar3 = pGPUBuffer;
        uint32_t offsetToLock = LockStart;

        if (pCPUBufferCopy != nullptr && gNumPendingGeometryBufferUploads < kMaxNumPendingGeoBufferUpload) {
            gpPendingGeometryBufferUploads[gNumPendingGeometryBufferUploads] = this;
            gNumPendingGeometryBufferUploads++;
        }
        uVar5 -= offsetToLock;

        bool bDiscardContent = (Flags & 1) != 0;
        void* local_4 = gpRender->getRenderDevice()->lockBuffer( pGPUBuffer, offsetToLock, uVar5, bDiscardContent );
        if (local_4 != nullptr) {
            uint8_t* pvVar3 = pCPUBuffer;
            uint8_t* puVar8 = pvVar3 + offsetToLock;        
            memcpy( local_4, puVar8, uVar5 );

            gpRender->getRenderDevice()->unlockBuffer( pGPUBuffer );
        }

        Flags &= 0xfd;
        LockStart = 0xffffffff;
        LockEnd = 0;
        bLocked = false;
        return;
    }

    uint8_t bVar2 = Flags;
    if ((Flags & 2) != 0) {
        void* pvVar3 = pCPUBuffer;
        pCPUBuffer = pCPUBufferCopy;
        void* pvVar4 = pCPUBufferCopy;
        pCPUBufferCopy = pvVar3;
        if (pvVar4 != (void *)0x0) {
            pGPUBufferCopy = pGPUBuffer;
            pGPUBuffer = pvVar4;
        }
        Flags |= 2;
    }
    Flags &= 0xfd;
}
