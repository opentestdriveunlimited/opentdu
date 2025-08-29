#include "shared.h"
#include "ed_store.h"

#include "core/hash/crc32.h"

static uint8_t gUseHalfPrecisionIntegers = 0xff; // DAT_00f56a70

edStore::edStore()
    : ID( 0 )
    , pBuffer( nullptr )
    , isExternalBuffer( 0 )
    , bufferSize( 0 )
    , bufferSizeWithPadding( 0 )
    , bufferOffset( 0 )
    , CRC( 0 )
    , state( 0 )
{
    // FUN_0062449
    TestDriveCRC32( nullptr, 0 ); // Init CRC32 LUT
    if (gUseHalfPrecisionIntegers == 0xff) {
        gUseHalfPrecisionIntegers = true;
    }
}

edStore::~edStore()
{
    // FUN_00622ff0
    if (isExternalBuffer == 0 && pBuffer != nullptr) {
        TestDrive::Free(pBuffer);
    }
    pBuffer = nullptr;
}

void edStore::bindExternalBuffer(void *pExtBuffer, uint32_t bufferLength)
{
    // FUN_00623180
    clear();
    pBuffer = pExtBuffer;
    bufferSizeWithPadding = bufferLength;
    bufferSize = bufferLength;
    isExternalBuffer = 1;
    prepareBuffer();
}

void edStore::clear()
{
    // FUN_00623020
    if (isExternalBuffer == 0 && pBuffer != nullptr) {
        TestDrive::Free(pBuffer);
    }
    pBuffer = nullptr;
    bufferSize = 0;

    if (isExternalBuffer == 0) {
        bufferSizeWithPadding = 0;
    }
    bufferOffset = 0;
    CRC = 0;
    state = 0;
}

int32_t edStore::reserve_buffer(int32_t sizeToReserve)
{
    // FUN_00623060
    if (isExternalBuffer != 0) {
        if (bufferSizeWithPadding <= bufferOffset + sizeToReserve) {
            OTDU_LOG_WARN("**WARNING** edStore::reserve_buffer  m_external_buffer too small (%d bytes)\n", this->bufferSizeWithPadding);
            state = 0xff;
            return -1;
        }
        bufferSize = bufferSize + sizeToReserve;
        return 0;
    }
    if (pBuffer == nullptr) {
        void* puVar1 = TestDrive::Alloc(sizeToReserve);
        pBuffer = puVar1;
        if (puVar1 != nullptr) {
            bufferSizeWithPadding = sizeToReserve;
            bufferSize = sizeToReserve;
            return 0;
        }
        bufferSizeWithPadding = 0;
        bufferSize = 0;
        OTDU_LOG_WARN("**WARNING** edStore::reserve_buffer m_buffer == NULL\n");
        state = 0xff;
        return -1;
    }

    uint32_t iVar2 = bufferSize + sizeToReserve;
    if (bufferSizeWithPadding < iVar2) {
        void* puVar1 = realloc(pBuffer,bufferSizeWithPadding + 0x200 + sizeToReserve);
        pBuffer = puVar1;
        if (puVar1 == nullptr) {
            bufferSizeWithPadding = 0;
            bufferSize = 0;
            OTDU_LOG_WARN("**WARNING** edStore::reserve_buffer m_buffer == NULL\n");
            state = 0xff;
            return -1;
        }
        iVar2 = bufferSize + sizeToReserve;
        bufferSizeWithPadding = iVar2 + 0x200;
    }
    bufferSize = iVar2;
    return 0;
}

void edStore::writeInteger(int32_t param_2)
{
    // FUN_006234b0
    int32_t iVar1 = 0;
    if (state != 0xff) {
        iVar1 = reserve_buffer(4);
        if (iVar1 == 0) {
            packAndWriteToBuffer(param_2, (int32_t*)((uint8_t*)pBuffer + bufferOffset));
            bufferOffset = bufferOffset + 4;
        }
    }
}

void edStore::prepareBuffer()
{
    // FUN_006236e0
    bufferOffset = 0;
    state = 0;

    uint16_t* local_4 = (uint16_t*)this;
    if (pBuffer != nullptr) {
        local_4 = (uint16_t*)pBuffer;
    }

    bufferOffset = 2;

    if (gUseHalfPrecisionIntegers == '\x01') {
        CRC = (uint16_t)(*local_4) >> 8 | (uint16_t)(*local_4) << 8;
    } else {
        CRC = (uint16_t)(*local_4);
    }
}

void edStore::packAndWriteToBuffer(int32_t in, int32_t *pOut)
{
    // FUN_00622df0
    if (gUseHalfPrecisionIntegers == '\x01') {
        in = (in & 0xff0000 | in >> 0x10) >> 8 | (in & 0xff00 | in << 0x10) << 8;
    }
    if (pOut != nullptr) {
        *pOut = in;
    }
}
