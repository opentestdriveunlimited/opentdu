#pragma once

struct BankHeader
{
    uint8_t         Magic[4];
    uint32_t        Flags;
    uint32_t        Version;
    int16_t         Type;

    // This is used to specialize some bank logic; 
    // not sure if it's meant 
    // to represent a subtype or something else 
    // (e.g. compression method)
    int16_t         TypeUnknown;

    int32_t         Size;
    int32_t         SizeWithoutPadding;
    int32_t         SectionAlignment;
    int32_t         DataAlignment;
    int32_t         NumEntries;

    // Seems to match the time period the game 
    // was being developped...
    // This field isn't really used by the game 
    // client so it's hard to tell
    int32_t         CreationDate;

    int32_t         SectionOffsetTable;
    int32_t         SectionOffsetTypes;
    int32_t         SectionOffsetDirectory;
    int32_t         SectionOffsetIndexes;
    int32_t         SectionOffsetReversedIndexes;
    int32_t         SectionOffsetData;

    inline bool     isVersionValid() const { return Version == 0; }
};

struct BankSection
{
    int32_t         Offset;
    int32_t         Size;
    int32_t         Unknown;
    int32_t         CRC;
};

struct BankType
{
    int16_t        Type;
    int16_t        TypeUnknown;
};

class BankInstance
{
public:
                    BankInstance();
                    ~BankInstance();

    int32_t         load( 
        const uint8_t* pBankMemory, 
        const uint32_t bankSize, 
        const bool param_4 
    );

private:
    void*           pMemory;
    uint32_t        size;

    BankHeader*     pHeader;
    void*           pDirectory;
    BankSection*    pEntry;
    BankType*       pType;
    void*           pIndex;
    void*           pIndexReverse;

    uint8_t         bCheckCRC : 1;

private:
    int32_t         validateEndianness( 
        const uint8_t* pMemory 
    );

    int32_t         validateChecksum( 
        const uint8_t* pMemoryBegin, 
        const uint8_t* pMemoryEnd, 
        const bool bValidate 
    );

    const void*     getSectionHeaderAddress( 
        const uint8_t* pBankMemory, 
        const uint32_t sectionOffset 
    );
    int32_t         getSectionAddress( 
        const void* pBankMemory,
        const uint8_t* pEndOfBankMemory,
        const uint32_t sectionOffset,
        void** ppSectionAddress,
        const bool bValidate 
    );
};

struct ParsedBankEntry
{
    int32_t     Type;
    int32_t     SubType;
    int32_t     Size;
    int32_t     SizeWithPadding;
    int32_t     CRC;
    void*       pData;
    int32_t     IndexEntry;
    uint8_t     bMandatory : 1;
    uint8_t     bResolveFilename : 1;
    
    std::string Filename;
    std::string Date;

                ParsedBankEntry();

    void        Clear();
};

class GSFile;

class Bank
{
public:
                    Bank();
                    ~Bank();

    void            loadAsync();
    virtual void    load();
    void            loadBank( 
        GSFile* pGSFile, 
        const char* pBankPath, 
        const uint32_t bankSize 
    );

protected:
    BankInstance    bank;
    ParsedBankEntry currentBankEntry;
    void*           pBankFile;
    uint32_t        bankFileSize;
    uint8_t         bLoaded : 1;
    uint8_t         bResolveFilename : 1;
};
