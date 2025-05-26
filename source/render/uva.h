#pragma once

class RenderUVA {
public:
    struct Header {
        float Version;
        uint32_t CompilerVersion;
        uint64_t Hashcode;	
    };

    struct Data {
        uint32_t numPoints;
        float totalDuration;
        float totalDuration2;
    };

public:
    RenderUVA();
    ~RenderUVA();

    bool initialize( void* pBuffer, const size_t length );

private:
    Header* pHeaderSection;
    Data* pDataSection;
};
