#pragma once

class RenderFile {
public:
    struct Section {
        uint32_t Type;
        uint16_t VersionMajor;
        uint16_t VersionMinor;
        uint32_t Size;
        uint32_t DataSize;
    };

    struct Header {
        uint16_t VersionMajor;
        uint16_t VersionMinor;
        uint32_t Flags;
        uint32_t Size;
        uint32_t Hashcode;
    };

public:
    inline Header* getHeader() { return pHeader; }

public:
    RenderFile();
    ~RenderFile();

    virtual bool parseSection( Section* pSection ) { return true; }
    
    bool parseHeader( void* pFileStream );
    bool parseFile();
    bool readSection( Section* pSection );

protected:
    Header* pHeader;
    Section* pSections;
};
