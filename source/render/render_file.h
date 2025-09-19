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
    inline Header* getHeader() const { return pHeader; }

public:
    RenderFile();
    virtual ~RenderFile();

    virtual void destroy();
    virtual bool parseSection( Section* pSection ) { return true; }
    virtual void unparseSection( Section* pSection ) {}

    void reset();
    void unparse();

    bool parseHeader( void* pFileStream );
    bool parseFile();
    bool readSection( Section* pSection );

protected:
    Header* pHeader;
    Section* pSections;

private:
    void unparseSubSections( Section* pSection );
};

OTDU_SIZE_MUST_MATCH(RenderFile::Header, 0x10);
OTDU_SIZE_MUST_MATCH(RenderFile::Section, 0x10);
