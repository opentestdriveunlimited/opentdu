#pragma once

class edStore {
public:
    edStore();
    ~edStore();

    void bindExternalBuffer(void* pExtBuffer, uint32_t bufferLength);
    void clear();

    int32_t reserve_buffer(int32_t sizeToReserve);
    void writeInteger(int32_t param_2);

private:
    uint32_t ID;
    uint32_t bufferSize;
    uint32_t bufferSizeWithPadding;
    uint32_t bufferOffset;
    uint32_t isExternalBuffer;
    uint16_t CRC;
    uint8_t state;
    void* pBuffer;

private:
    void prepareBuffer();

    void packAndWriteToBuffer(int32_t in, int32_t* pOut);
};