#pragma once

struct Timestamp {
    int32_t Years;
    int32_t Months;
    int32_t Days;
    int32_t Hours;
    int32_t Minutes;
    int32_t Seconds;

    inline void reset()
    {
        Years = 0;
        Months = 0;
        Days = 0;
        Hours = 0;
        Minutes = 0;
        Seconds = 0;
    }

    std::string toString();
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
    
    Timestamp   CreationDate;
    std::string Filename;
    std::string Date;

                ParsedBankEntry();

    void        Clear();
};

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
    uint32_t        SectionOffsetData;

    inline bool     isVersionValid() const { return Version == 0; }
};

struct BankSection
{
    int32_t         Offset;
    int32_t         Size;
    int32_t         Date;
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
    inline int32_t  getFirstEntry( ParsedBankEntry& outEntry ) { return getEntry( 0, outEntry ); }

public:
                    BankInstance();
                    ~BankInstance();

    int32_t         load( 
        const uint8_t* pBankMemory, 
        const uint32_t bankSize, 
        const bool param_4 
    );

    int32_t         getNumElements();
    int32_t         getEntry( int32_t entryIndex, ParsedBankEntry& outEntry );
    int32_t         resolveIndex( int32_t param_2, bool param_3 );

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

    void getFilename(
        int32_t fileIndex,
        std::string& filename
    );
};

class GSFile;

enum class eBankEntryType {
    BET_COMMON = 0xe,
};

enum class eBankEntrySubtype {
    BES_2DM = 0x2,
};

class Bank
{
public:
                    Bank();
                    ~Bank();

    void            loadContentAsync();
    void            loadBank( 
        GSFile* pGSFile, 
        const char* pBankPath
    );
    virtual void    loadContent() { OTDU_LOG_WARN( "loadContent() is unimplemented; bank resources won't be loaded!\n" ); OTDU_ASSERT(); }

    // For TEA encrypted resources (e.g. DBs). Done in place.
    static void     DecryptTEA(void* pBankData, const uint32_t bankSize, uint32_t* pOutDataPointer);

    void* getFirstEntry( const eBankEntryType type, const eBankEntrySubtype subType, uint32_t* pOutEntrySize );

protected:
    BankInstance    bank;
    ParsedBankEntry currentBankEntry;
    void*           pBankFile;
    uint32_t        bankFileSize;
    uint8_t         bLoaded : 1;
    uint8_t         bResolveFilename : 1;
};
