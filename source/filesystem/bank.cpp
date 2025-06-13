#include "shared.h"
#include "bank.h"

#include "core/crypto/tea.h"
#include "core/hash/crc32.h"

#include "filesystem/gs_file.h"

static constexpr const uint32_t gCRC32BankLUT[256] = {
    0,             0x77073096,    0xEE0E612C,    0x990951BA,
    0x76DC419,     0x706AF48F,    0xE963A535,    0x9E6495A3,
    0xEDB8832,     0x79DCB8A4,    0xE0D5E91E,    0x97D2D988,
    0x9B64C2B,     0x7EB17CBD,    0xE7B82D07,    0x90BF1D91,
    0x1DB71064,    0x6AB020F2,    0xF3B97148,    0x84BE41DE,
    0x1ADAD47D,    0x6DDDE4EB,    0xF4D4B551,    0x83D385C7,
    0x136C9856,    0x646BA8C0,    0xFD62F97A,    0x8A65C9EC,
    0x14015C4F,    0x63066CD9,    0xFA0F3D63,    0x8D080DF5,
    0x3B6E20C8,    0x4C69105E,    0xD56041E4,    0xA2677172,
    0x3C03E4D1,    0x4B04D447,    0xD20D85FD,    0xA50AB56B,
    0x35B5A8FA,    0x42B2986C,    0xDBBBC9D6,    0xACBCF940,
    0x32D86CE3,    0x45DF5C75,    0xDCD60DCF,    0xABD13D59,
    0x26D930AC,    0x51DE003A,    0xC8D75180,    0xBFD06116,
    0x21B4F4B5,    0x56B3C423,    0xCFBA9599,    0xB8BDA50F,
    0x2802B89E,    0x5F058808,    0xC60CD9B2,    0xB10BE924,
    0x2F6F7C87,    0x58684C11,    0xC1611DAB,    0xB6662D3D,
    0x76DC4190,    0x1DB7106,     0x98D220BC,    0xEFD5102A,
    0x71B18589,    0x6B6B51F,     0x9FBFE4A5,    0xE8B8D433,
    0x7807C9A2,    0xF00F934,     0x9609A88E,    0xE10E9818,
    0x7F6A0DBB,    0x86D3D2D,     0x91646C97,    0xE6635C01,
    0x6B6B51F4,    0x1C6C6162,    0x856530D8,    0xF262004E,
    0x6C0695ED,    0x1B01A57B,    0x8208F4C1,    0xF50FC457,
    0x65B0D9C6,    0x12B7E950,    0x8BBEB8EA,    0xFCB9887C,
    0x62DD1DDF,    0x15DA2D49,    0x8CD37CF3,    0xFBD44C65,
    0x4DB26158,    0x3AB551CE,    0xA3BC0074,    0xD4BB30E2,
    0x4ADFA541,    0x3DD895D7,    0xA4D1C46D,    0xD3D6F4FB,
    0x4369E96A,    0x346ED9FC,    0xAD678846,    0xDA60B8D0,
    0x44042D73,    0x33031DE5,    0xAA0A4C5F,    0xDD0D7CC9,
    0x5005713C,    0x270241AA,    0xBE0B1010,    0xC90C2086,
    0x5768B525,    0x206F85B3,    0xB966D409,    0xCE61E49F,
    0x5EDEF90E,    0x29D9C998,    0xB0D09822,    0xC7D7A8B4,
    0x59B33D17,    0x2EB40D81,    0xB7BD5C3B,    0xC0BA6CAD,
    0xEDB88320,    0x9ABFB3B6,    0x3B6E20C,     0x74B1D29A,
    0xEAD54739,    0x9DD277AF,    0x4DB2615,     0x73DC1683,
    0xE3630B12,    0x94643B84,    0xD6D6A3E,     0x7A6A5AA8,
    0xE40ECF0B,    0x9309FF9D,    0xA00AE27,     0x7D079EB1,
    0xF00F9344,    0x8708A3D2,    0x1E01F268,    0x6906C2FE,
    0xF762575D,    0x806567CB,    0x196C3671,    0x6E6B06E7,
    0xFED41B76,    0x89D32BE0,    0x10DA7A5A,    0x67DD4ACC,
    0xF9B9DF6F,    0x8EBEEFF9,    0x17B7BE43,    0x60B08ED5,
    0xD6D6A3E8,    0xA1D1937E,    0x38D8C2C4,    0x4FDFF252,
    0xD1BB67F1,    0xA6BC5767,    0x3FB506DD,    0x48B2364B,
    0xD80D2BDA,    0xAF0A1B4C,    0x36034AF6,    0x41047A60,
    0xDF60EFC3,    0xA867DF55,    0x316E8EEF,    0x4669BE79,
    0xCB61B38C,    0xBC66831A,    0x256FD2A0,    0x5268E236,
    0xCC0C7795,    0xBB0B4703,    0x220216B9,    0x5505262F,
    0xC5BA3BBE,    0xB2BD0B28,    0x2BB45A92,    0x5CB36A04,
    0xC2D7FFA7,    0xB5D0CF31,    0x2CD99E8B,    0x5BDEAE1D,
    0x9B64C2B0,    0xEC63F226,    0x756AA39C,    0x26D930A,
    0x9C0906A9,    0xEB0E363F,    0x72076785,    0x5005713,
    0x95BF4A82,    0xE2B87A14,    0x7BB12BAE,    0xCB61B38,
    0x92D28E9B,    0xE5D5BE0D,    0x7CDCEFB7,    0xBDBDF21,
    0x86D3D2D4,    0xF1D4E242,    0x68DDB3F8,    0x1FDA836E,
    0x81BE16CD,    0xF6B9265B,    0x6FB077E1,    0x18B74777,
    0x88085AE6,    0xFF0F6A70,    0x66063BCA,    0x11010B5C,
    0x8F659EFF,    0xF862AE69,    0x616BFFD3,    0x166CCF45,
    0xA00AE278,    0xD70DD2EE,    0x4E048354,    0x3903B3C2,
    0xA7672661,    0xD06016F7,    0x4969474D,    0x3E6E77DB,
    0xAED16A4A,    0xD9D65ADC,    0x40DF0B66,    0x37D83BF0,
    0xA9BCAE53,    0xDEBB9EC5,    0x47B2CF7F,    0x30B5FFE9,
    0xBDBDF21C,    0xCABAC28A,    0x53B39330,    0x24B4A3A6,
    0xBAD03605,    0xCDD70693,    0x54DE5729,    0x23D967BF,
    0xB3667A2E,    0xC4614AB8,    0x5D681B02,    0x2A6F2B94,
    0xB40BBE37,    0xC30C8EA1,    0x5A05DF1B,    0x2D02EF8D
};

static uint32_t CaclulateBankCRC( const uint8_t* pBankMemory, uint32_t numBytes )
{
    uint32_t param_3 = 0xffffffff;
    if (numBytes != 0) {
        do {
            param_3 = param_3 >> 8 ^ gCRC32BankLUT[( *pBankMemory ^ param_3 ) & 0xff];
            pBankMemory++;
            numBytes--;
        } while (numBytes != 0);
    }
    return param_3;
}

BankInstance::BankInstance()
    : pMemory( nullptr )
    , size( 0u )
    , pHeader( nullptr )
    , pDirectory( nullptr )
    , pEntry( nullptr )
    , pType( nullptr )
    , pIndex( nullptr )
    , pIndexReverse( nullptr )
    , bCheckCRC( true )
{
    
}

BankInstance::~BankInstance()
{

}

const void* BankInstance::getSectionHeaderAddress( const uint8_t* pBankMemory, const uint32_t sectionOffset )
{
    if (sectionOffset < 8) {
        return nullptr;
    }

    return pBankMemory + (sectionOffset - 8);
}

int32_t BankInstance::getSectionAddress( const void* pBankMemory,
    const uint8_t* pEndOfBankMemory,
    const uint32_t sectionOffset,
    void** ppSectionAddress,
    const bool bValidate )
{
    const void* pSectionHeader = getSectionHeaderAddress( (const uint8_t*)pBankMemory, sectionOffset );
    if ( pSectionHeader != nullptr ) {
        uint32_t uVar3 = validateChecksum( (const uint8_t*)pSectionHeader, pEndOfBankMemory, bValidate );
        if (uVar3 != 0) {
            return uVar3;
        }

        *ppSectionAddress = (void*)( (const uint8_t*)pSectionHeader + 8 );
    }

    return 0;
}

char* BankDirectoryRecurse(char *param_1,int *param_2,char **param_3)
{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  uint32_t uVar4;
  int iVar5;
  
  cVar1 = *param_1;
  pcVar2 = param_1 + 1;
  if (cVar1 < '\0') {
    iVar5 = -(int)cVar1;
    if (cVar1 == -0x80) {
      iVar5 = *pcVar2 + 0x80;
      pcVar2 = param_1 + 2;
    }
    uVar4 = 0;
    param_1 = (char *)0x0;
    do {
      cVar1 = *pcVar2;
      pcVar2 = pcVar2 + 1;
      uVar4 = uVar4 | ((int)cVar1 & 0x7fU) << (*param_1 & 0x1f);
      param_1 = param_1 + 7;
    } while (cVar1 < 0);
    pcVar2 = pcVar2 + iVar5;
    pcVar3 = pcVar2;
    do {
      if (uVar4 == 0) {
        return pcVar3;
      }
      uVar4 = uVar4 - 1;
      pcVar3 = BankDirectoryRecurse(pcVar3,param_2,param_3);
    } while (pcVar3 != (char *)0x0);
    for (; iVar5 != 0; iVar5 = iVar5 + -1) {
      *param_3 = *param_3 + -1;
      pcVar2 = pcVar2 + -1;
      **param_3 = *pcVar2;
    }
    *param_3 = *param_3 + -1;
    **param_3 = '\\';
  }
  else {
    iVar5 = (int)cVar1;
    if (iVar5 == 0x7f) {
      iVar5 = *pcVar2 + 0x7f;
      pcVar2 = param_1 + 2;
    }
    if (*param_2 != 0) {
      *param_2 = *param_2 + -1;
      return pcVar2 + iVar5;
    }
    pcVar2 = pcVar2 + iVar5;
    for (; iVar5 != 0; iVar5 = iVar5 + -1) {
      *param_3 = *param_3 + -1;
      pcVar2 = pcVar2 + -1;
      **param_3 = *pcVar2;
    }
    *param_3 = *param_3 + -1;
    **param_3 = '\\';
  }
  return (char *)0x0;
}

void DecodeFilePath(std::string& param_1,char *param_2)
{
  char *pcVar1;
  char *pcVar2;
  char cVar3;
  
  cVar3 = *param_2;
  pcVar1 = param_2;
  if (cVar3 != '\0') {
    do {
      param_2 = param_2 + 1;
      pcVar2 = pcVar1;
      if (cVar3 == '\\') {
        if (*pcVar1 == '.') {
          while ((pcVar2 = param_2, cVar3 = *pcVar2, cVar3 != '\0' && (cVar3 != '\\'))) {
            param_1 += cVar3;
            param_2 = pcVar2 + 1;
          }
          for (; (cVar3 = *pcVar1, param_2 = pcVar2, cVar3 != '\0' && (cVar3 != '\\'));
              pcVar1 = pcVar1 + 1) {
            param_1 += cVar3;
          }
        } else {
          for (; pcVar2 < param_2; pcVar2 = pcVar2 + 1) {
            param_1 += *pcVar2;
          }
        }
      }
      cVar3 = *param_2;
      pcVar1 = pcVar2;
    } while (cVar3 != '\0');
    for (; pcVar2 < param_2; pcVar2 = pcVar2 + 1) {
        param_1 += *pcVar2;
    }
  }
  param_1 += '\0';
}

void BankInstance::getFilename(int32_t param_1, std::string &param_2)
{
    char *local_20c [128];
    *local_20c[0] = '\0';

    char* pDirectoryContent = (char*)pDirectory;
    do {
        pDirectoryContent = BankDirectoryRecurse(pDirectoryContent,&param_1,local_20c);
        if (pDirectoryContent == (char *)0x0) break;
    } while (*pDirectoryContent != '\0');

    if (*local_20c[0] == '\\') {
        local_20c[0] = local_20c[0] + 1;
    }
       
    DecodeFilePath(param_2,local_20c[0]);
}

int32_t BankInstance::load( const uint8_t* pBankMemory, 
    const uint32_t bankSize, 
    const bool param_4 )
{
    pMemory = nullptr;
    size = 0u;
    pHeader = nullptr;

    int32_t uVar3 = validateEndianness(pBankMemory);
    if (uVar3 == 0) {
        const uint8_t* pEndOfBankMemory = pBankMemory + bankSize;
        uVar3 = validateChecksum( pBankMemory, pEndOfBankMemory, true );

        if (uVar3 == 0) {
            void* pHeaderRaw = (void*)( pBankMemory + 8 );
            
            pHeader = static_cast<BankHeader*>( pHeaderRaw );
            OTDU_ASSERT( pHeader );

            bool bVar1 = pHeader->isVersionValid();
            if (bVar1) {
                uVar3 = getSectionAddress( pHeaderRaw, pEndOfBankMemory, pHeader->SectionOffsetDirectory, &pDirectory, param_4 );
                if (uVar3 != 0)  {
                    return uVar3;
                }
                uVar3 = getSectionAddress( pHeaderRaw, pEndOfBankMemory, pHeader->SectionOffsetTable, (void**)&pEntry, param_4 );
                if (uVar3 != 0)  {
                    return uVar3;
                }
                uVar3 = getSectionAddress( pHeaderRaw, pEndOfBankMemory, pHeader->SectionOffsetTypes, (void**)&pType, param_4 );
                if (uVar3 != 0)  {
                    return uVar3;
                }
                uVar3 = getSectionAddress( pHeaderRaw, pEndOfBankMemory, pHeader->SectionOffsetIndexes, &pIndex, param_4 );
                if (uVar3 != 0)  {
                    return uVar3;
                }
                uVar3 = getSectionAddress( pHeaderRaw, pEndOfBankMemory, pHeader->SectionOffsetReversedIndexes, &pIndexReverse, param_4 );
                if (uVar3 != 0)  {
                    return uVar3;
                }

                pMemory = (void*)pBankMemory;
                size = bankSize;
                bCheckCRC = param_4;

                return 0;
            }
        }
    }

    return 5;
}

int32_t BankInstance::getNumElements()
{
    return pHeader != nullptr ? pHeader->NumEntries : 0;
}

void ParseFromPackedTimestamp(const uint32_t uVar1, const uint32_t param_2, Timestamp& outTimestamp)
{
    outTimestamp.reset();

    outTimestamp.Seconds = uVar1 & 0x3f;
    outTimestamp.Minutes = uVar1 >> 6 & 0x3f;
    outTimestamp.Hours = uVar1 >> 0xc & 0x1f;
    outTimestamp.Days = uVar1 >> 0x11 & 0x1f;
    outTimestamp.Years = (uVar1 >> 0x1a) + param_2;
    outTimestamp.Months = uVar1 >> 0x16 & 0xf;
}

int32_t BankInstance::getEntry(int32_t param_2, ParsedBankEntry &outEntry)
{
    outEntry.Clear();

    uint32_t uVar4 = 0;
    int32_t iVar5 = 0;
    if (pHeader == nullptr ) {
        iVar5 = 1;
    } else if ((param_2 < 0) || (pHeader->NumEntries <= param_2)) {
        iVar5 = 6;
    } else if (size < pHeader->SectionOffsetData) {
        iVar5 = 2;
    } else {
        if (outEntry.bResolveFilename && pDirectory != nullptr && pHeader->NumEntries != 0) {
            for (int32_t i = 0; i < pHeader->NumEntries; i++) {
                 int32_t iVar2 = resolveIndex(i, false);
                 if (iVar2 == param_2) {
                    getFilename(iVar5, outEntry.Filename);
                    break;
                 }
            }
        }
        
        if (pEntry != nullptr) {
            BankSection* pSection = &pEntry[param_2];

            ParseFromPackedTimestamp(pSection->Date, pHeader->CreationDate, outEntry.CreationDate);
            outEntry.Date = outEntry.CreationDate.toString();

            uVar4 = pSection->Size;
            outEntry.Size = uVar4;

            // TODO: How tf is that safe? Is it some bad decomp or bad code?
            // 00621ec9 8b 46 10        MOV        EAX,dword ptr [ESI + 0x10]
            // 00621ecc 2b 06           SUB        EAX,dword ptr [ESI]
            uint32_t uVar3 = (pSection + 1)->Offset - pSection->Offset;
            if (uVar3 < uVar4) {
                outEntry.SizeWithPadding = uVar3;
            } else {
                outEntry.SizeWithPadding = uVar4;
            }

            outEntry.pData = (uint8_t*)pMemory + pSection->Offset;
        }

        if (pType == nullptr) {
            outEntry.SubType = pHeader->TypeUnknown;
            uVar4 = pHeader->Type;
        } else {
            outEntry.SubType = pType[param_2].Type;
            uVar4 = pType[param_2].TypeUnknown;
        }

        outEntry.Type = uVar4;
        outEntry.IndexEntry = param_2;
        iVar5 = 0;
    }
    return iVar5;
}

int32_t BankInstance::resolveIndex(int32_t param_2, bool param_3)
{
  void *pvVar1 = param_3 ? pIndexReverse : pIndex;
  uint32_t uVar2 = pHeader->NumEntries;
  
  uint32_t* pIndexes = (uint32_t*)pvVar1;
  if (uVar2 < 0x100) {
    uVar2 = pIndexes[param_2];
  } else if (uVar2 < 0x10000) {
    uVar2 = pIndexes[param_2 * 2];
  } else {
    uVar2 = pIndexes[param_2 * 4];
  }
  return uVar2;
}

int32_t BankInstance::validateChecksum( const uint8_t* pMemoryBegin, 
    const uint8_t* pMemoryEnd, 
    const bool bValidate )
{
    const uint8_t *pcVar1 = pMemoryBegin + 8;
    const uint32_t dataSize = *(uint32_t*)pMemoryBegin;
    if ( pMemoryEnd < pcVar1 || (pMemoryEnd < ( pMemoryBegin + dataSize ) ) ) {
        return 2;
    }

    if ( bValidate ) {
        const uint32_t bankCRC = *(uint32_t*)(pMemoryBegin + 4);
        uint32_t memoryCRC = CaclulateBankCRC(pcVar1, dataSize);

        if ( bankCRC != memoryCRC ) {
            OTDU_LOG_WARN("CRC mismatch! (calculated %x found %x)\n", memoryCRC, bankCRC);
            return 3;
        }
    }

    return 0;
}

int32_t BankInstance::validateEndianness( const uint8_t* pMemory )
{
    if (!bCheckCRC)
    {
        // TODO: I assume we can ignore this check to avoid recalculating CRCs for new banks
        // (eg. for mods)
        return 3;
    }

    if (pMemory[0x9] == 'N' 
    &&  pMemory[0xa] == 'A' 
    &&  pMemory[0xb] == 'B' )
    {
        return 0;
    }
    else if (pMemory[0x9] == 'A' 
    &&  pMemory[0xa] == 'N' 
    &&  pMemory[0xb] == 'K' )
    {
        return 4;
    }

    return 3;
}

ParsedBankEntry::ParsedBankEntry()
{
    Clear();
}

void ParsedBankEntry::Clear()
{
    Type = 0;
    SubType = 0;
    Size = 0;
    SizeWithPadding = 0;
    pData = nullptr;
    IndexEntry = -1;
    CRC = 0;

    bMandatory = false;
    bResolveFilename = false;

    Filename.reserve( 512 );
    Date.reserve( 512 );
}

Bank::Bank()
    : bank()
    , currentBankEntry()
    , pBankFile( nullptr )
    , bankFileSize( 0u )
    , bLoaded( false )
    , bResolveFilename( false )
{

}

Bank::~Bank()
{
    
}

void Bank::loadContentAsync()
{
    bResolveFilename = true;
    if (bLoaded) {
        OTDU_LOG_WARN( "Bank is already loaded!\n");
        return;
    }

    if (pBankFile == nullptr) {
        OTDU_LOG_WARN( "Tried to load empty bank!\n");
        return;
    }

    int32_t iVar1 = bank.load( (const uint8_t*)pBankFile, bankFileSize, true );
    if (iVar1 == 0) {
        bResolveFilename = false;
        bLoaded = true;

        loadContent();
    }

    return;
}

bool Bank::loadBank( GSFile* pGSFile, const char* pBankPath )
{
    OTDU_ASSERT( pGSFile );
    bool bLoadingResult = pGSFile->loadFile( pBankPath, &pBankFile, &bankFileSize );

    if ( !bLoadingResult ) {
        OTDU_LOG_WARN( "Failed to load bank '%s'\n", pBankPath );
        return false;
    }

    return true;
}

static constexpr uint32_t kTEAKeysBank[4] = {
    0x4FE23C4A,
    0x80BAC211,
    0x6917BD3A,
    0xF0528EBD
};

void Bank::DecryptTEA(void *pBankData, const uint32_t bankSize, uint32_t *pOutDataPointer)
{
    OTDU_ASSERT(pBankData);
    OTDU_ASSERT(pOutDataPointer);
    
    uint32_t roundedSize =  (((bankSize - 8) + ((bankSize - 8) >> 0x1f & 7U)) >> 3);

    // Not sure wtf is going on here
    uint32_t* puVar1 = pOutDataPointer;
    *pOutDataPointer = roundedSize;
    //pOutDataPointer = (uint32_t*)roundedSize;

    uint32_t output[2] = { 0, 0 };
    uint32_t* pBankDataAsDword = (uint32_t*)pBankData;
    while ( *pOutDataPointer != 0) {
        TestDriveTEADecrypt( output, pBankDataAsDword + 2, kTEAKeysBank );

        pBankDataAsDword[0] ^= output[0];
        pBankDataAsDword[1] ^= output[1];

        *pOutDataPointer = *pOutDataPointer - 1;
        pBankDataAsDword += 2;
    }
    #if 0
    if (*puVar1 < bankSize) {
        uint32_t uVar3 = bankSize - *puVar1;
        uint8_t* pBankDataAsBytes = (uint8_t*)pBankDataAsDword;
        for (uint32_t uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
            *pBankDataAsBytes = 0;
            pBankDataAsBytes++;
        }
        for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
            *pBankDataAsBytes = 0;
            pBankDataAsBytes++;
        }
    }
    #endif
}

void* Bank::getFirstEntry( const int32_t type, const int32_t subType, uint32_t* pOutEntrySize )
{
    if ( !bLoaded ) {
        return nullptr;
    }

    currentBankEntry.Clear();
    currentBankEntry.bResolveFilename = bResolveFilename;

    void* pResourceContent = nullptr;
    bank.getFirstEntry( currentBankEntry );
    if ( currentBankEntry.IndexEntry != -1 ) {
        while ( ( currentBankEntry.Type != type || ( currentBankEntry.SubType != subType ) ) ) {
            bank.getFirstEntry( currentBankEntry );
            if ( currentBankEntry.IndexEntry == -1 ) {
                return nullptr;
            }
        }
        
        pResourceContent = currentBankEntry.pData;
        if ( pOutEntrySize != nullptr ) {
            *pOutEntrySize = currentBankEntry.Size;
        }
    }

    return pResourceContent;
}

std::string Timestamp::toString()
{
    std::string output;
    output += Days;
    output += "/";
    output += Months;
    output += "/";
    output += Years;
    output += " ";
    output += Hours;
    output += ":";
    output += Minutes;
    output += ":";
    output += Seconds;
    
    return output;
}
