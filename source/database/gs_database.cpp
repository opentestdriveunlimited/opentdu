#include "shared.h"
#include "gs_database.h"

#include "config/gs_config.h"
#include "player_data/gs_playerdata.h"
#include "filesystem/gs_file.h"
#include "filesystem/bank.h"
#include "flash/flash_player.h"
#include "flash/mng_flash_localize.h"

#include <locale>
#include <codecvt>

GSDatabase* gpDatabase = nullptr;

GSDatabase::GSDatabase()
    : NbChallengeNames( 0xa0 )
    , NbChallengeDesc( 0x38 )
    , bInitialized( false )
{
    gpDatabase = this;
}

GSDatabase::~GSDatabase()
{

}

bool GSDatabase::initialize( TestDriveGameInstance* )
{
    engineDatabase.bInitialized = false;

    const char* pLanguage = gpPlayerData->getLanguage();
    bool bDatabaseLoadResult = reloadDatabases(pLanguage, false);

    return bDatabaseLoadResult;
}

void GSDatabase::terminate()
{
    cars.clear();
    releaseDatabases();
}

void GSDatabase::reset()
{
    const char* pLanguage = gpPlayerData->getLanguage();
    reloadDatabases(pLanguage, false);
}

void GSDatabase::pause()
{
    releaseDatabases();
}

bool GSDatabase::reloadDatabases(const char *pLanguage, const bool bForceReload)
{
    OTDU_ASSERT(pLanguage != nullptr);
    
    pActiveLanguage[0] = pLanguage[0];
    pActiveLanguage[1] = pLanguage[1];
    pActiveLanguage[2] = '\0';
    activeLocale = PlayerDataLanguage(pActiveLanguage);
    OTDU_LOG_DEBUG("Active lanuage : '%s'\n", pActiveLanguage);

    const char* pResourcePath = gpConfig->getResRootPath();
    gameDatabase.Filepath = pResourcePath;
    gameDatabase.Filepath += "DataBase";

    if (bInitialized && !bForceReload) {
        return true;
    }

    OTDU_LOG_INFO("Loading Databases.\n");
    gpFile->initCarPacks(false);

    if (strstr(pLanguage, "gb") == 0) {
        gpPlayerData->setDefaultLanguage("us", false);
    }

    bool bLoadResult = loadDatabases( gpPlayerData->getLanguage() );
    return bLoadResult;
}

char *GSDatabase::getStringByHashcode(const uint32_t dbIndex, const uint64_t hashcode)
{
    OTDU_ASSERT( dbIndex < kNumDatabases );
    const DBEntryList& dbEntry = gameDatabase.Databases[dbIndex];

    for (const DBEntry& entry : dbEntry.List) {
        if (entry.Hashcode == hashcode) {
            return &dbEntry.pData[entry.Offset];
        }
    }

    return nullptr;
}

void GSDatabase::releaseDatabases()
{
    for (int32_t i = 0; i < kNumDatabases; i++) {
        gameDatabase.Databases[i].List.clear();
        gameDatabase.Databases[i].pData = nullptr;
    }

    bInitialized = false;
    activeLocale = eLocale::L_Unknown;
}

struct DBToLoadDesc {
    const char* pName;
    const uint32_t Index;
    const uint32_t Stride;
};

static constexpr DBToLoadDesc kDatabasesToLoad[19] = {
    { "INVALID",                0xffffffff, 0 },
    { "TDU_Menus",              0,          0x18 },
    { "TDU_Rims",               2,          0x60 },
    { "TDU_CarRims",            3,          0x10 },
    { "TDU_CarPhysicsData",     1,          0x1f8 },
    { "TDU_Interior",           4,          0x38 },
    { "TDU_CarColors",          5,          0xa8 },
    { "TDU_Brands",             6,          0x30 },
    { "TDU_Houses",             7,          0x38 },
    { "TDU_CarShops",           8,          0x98 },
    { "TDU_Clothes",            9,          0x58 },
    { "TDU_Hair",               10,         0x38 },
    { "TDU_Tutorials",          0xb,        0x28 },
    { "TDU_AfterMarketPacks",   0xc,        0x38 },
    { "TDU_CarPacks",           0xd,        0x98 },
    { "TDU_Bots",               0xe,        0x30 },
    { "TDU_Achievements",       0xf,        0x40 },
    { "TDU_PNJ",                0x10,       0x70 },
    { "TDU_SubTitles",          0x11,       0x20 },
};

bool GSDatabase::loadDatabases(const char *pLanguage)
{
    BankInstance local_498;
    BankInstance local_4c4;

    const char* pResRoot = gpConfig->getResRootPath();

    std::string local_200 = pResRoot;
    local_200 += "DataBase/DB_";
    local_200 += pLanguage;
    local_200 += ".bnk";
    void* local_470 = nullptr;
    uint32_t local_474 = 0u;
    bool bLoadResult = gpFile->loadFile( local_200.c_str(), &local_470, &local_474 );
    OTDU_ASSERT( bLoadResult );

    std::string local_202 = pResRoot;
    local_202 += "DataBase/DB.bnk";
    void* local_49c = nullptr;
    uint32_t local_4a0 = 0u;
    bLoadResult = gpFile->loadFile( local_202.c_str(), &local_49c, &local_4a0 );
    OTDU_ASSERT( bLoadResult );

    if (local_49c != nullptr && local_470 != nullptr) {
        // Load localized DBs
        int32_t iVar4 = local_498.load((const uint8_t*)local_470, local_474, true);
        if (iVar4 == 0) {
            ParsedBankEntry local_44c;
            int32_t numElements = local_498.getNumElements();

            for (int32_t i = 0; i < numElements; i++) {
                iVar4 = local_498.getEntry( i, local_44c );
                if ((iVar4 == 0) && (local_44c.Type == 7)) {
                    if (local_44c.SubType >= kNumDatabases) {
                        continue;
                    }
                    iVar4 = kDatabasesToLoad[local_44c.SubType].Index;

                    localizedDbInstallRequests[iVar4].pBuffer = (char16_t*)local_44c.pData;
                    localizedDbInstallRequests[iVar4].BufferSize = local_44c.Size;
                }
            }
        }

        // Load common dbs
        iVar4 = local_4c4.load((const uint8_t*)local_49c, local_4a0, true);
        if (iVar4 == 0) {
            ParsedBankEntry local_44c;
            int32_t numElements = local_4c4.getNumElements();

            char** pDatabaseItems[19] = {
                nullptr,
                (char**)&gameDatabase.DBMenu,
                (char**)&gameDatabase.DBRim,
                (char**)&gameDatabase.DBCarRims,
                (char**)&gameDatabase.DBCarData,
                (char**)&gameDatabase.DBInterior,
                (char**)&gameDatabase.DBColors,
                (char**)&gameDatabase.DBBrand,
                (char**)&gameDatabase.DBHouse,
                (char**)&gameDatabase.DBCarShop,
                (char**)&gameDatabase.DBClothe,
                (char**)&gameDatabase.DBHair,
                (char**)&gameDatabase.DBTutorial,
                (char**)&gameDatabase.DBPack,
                (char**)&gameDatabase.DBCarPack,
                (char**)&gameDatabase.DBBot,
                (char**)&gameDatabase.DBAchivement,
                (char**)&gameDatabase.DBPNJ,
                (char**)&gameDatabase.DBSubtitle
            };

            for (int32_t i = 0; i < numElements; i++) {
                iVar4 = local_4c4.getEntry( i, local_44c );
                if (iVar4 == 0) {
                    void* pBankData = local_44c.pData;
                    uint32_t bankSize = local_44c.Size;
                    
                    uint32_t remainingBytes = 0;
                    Bank::DecryptTEA(pBankData, bankSize, &remainingBytes);
                    if (local_44c.Type == 6) {
                        if (local_44c.SubType < 18) {
                            OTDU_LOG_DEBUG("Opening %s...\n", kDatabasesToLoad[local_44c.SubType].pName);
                            dbInstallRequests[kDatabasesToLoad[local_44c.SubType].Index].BufferSize = bankSize;
                            dbInstallRequests[kDatabasesToLoad[local_44c.SubType].Index].pBuffer = (char*)pBankData;
                            if (!initializeDatabase( 
                                kDatabasesToLoad[local_44c.SubType].pName, 
                                kDatabasesToLoad[local_44c.SubType].Index, 
                                kDatabasesToLoad[local_44c.SubType].Stride,
                                pDatabaseItems[local_44c.SubType]
                            )) {
                                OTDU_LOG_ERROR("Failed to open and load database %s!\n", kDatabasesToLoad[local_44c.SubType].pName);
                                local_44c.Clear();
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }

  return true;
}

inline bool IsCarriageReturn(bool param_1,char16_t *param_2)
{
    char16_t uVar1;
    char16_t uVar2;

    if (param_1) {
        uVar2 = *param_2;
        uVar1 = param_2[1];
    } else {
        uVar2 = *param_2;
        uVar1 = param_2[1];
    }

    return (((uVar2 == L'\\') || (uVar2 == L'/')) && (uVar1 == L'n'));
}

int64_t __allmul(uint32_t param_1,int param_2, uint32_t param_3,int param_4)
{
  if (param_4 == 0 && param_2 == 0) {
    return (uint64_t)param_1 * (uint64_t)param_3;
  }
  return ((int)((uint64_t)param_1 * (uint64_t)param_3 >> 0x20) +
            param_2 * param_3 + param_1 * param_4,
            (int)((uint64_t)param_1 * (uint64_t)param_3));
}

int64_t GetNextUint64InString(char16_t *param_1,int param_2)
{
  int64_t lVar1;
  char16_t *pcVar2;
  char16_t cVar3;
  char16_t bVar4;
  int64_t lVar5;
  int64_t lVar6;
  int iVar7;
  uint32_t local_c;
  int local_4;
  
  cVar3 = *param_1;
  bVar4 = 0;
  pcVar2 = param_1;
  if (cVar3 < '0') {
    do {
      if (cVar3 < ':') goto LAB_009ae3d7;
      cVar3 = pcVar2[param_2];
      pcVar2 = pcVar2 + param_2;
    } while (cVar3 < '0');
  }
  else {
LAB_009ae3d7:
    if (*pcVar2 < '0') goto LAB_009ae3f5;
  }
  cVar3 = *pcVar2;
  do {
    if ('9' < cVar3) break;
    cVar3 = pcVar2[param_2];
    pcVar2 = pcVar2 + param_2;
    bVar4 = bVar4 + 1;
  } while ('/' < cVar3);
LAB_009ae3f5:
  lVar6 = 1;
  lVar1 = 0;
  if (bVar4 == 0) {
    return 0;
  }
  local_c = (uint32_t)bVar4;
  pcVar2 = param_1 + local_c * param_2;
  do {
    local_4 = (int)((uint64_t)lVar1 >> 0x20);
    pcVar2 = pcVar2 + -param_2;
    cVar3 = *pcVar2;
    if (('/' < cVar3) && (cVar3 < ':')) {
      if (cVar3 == '-') {
        lVar1 = -lVar1;
      }
      else if (cVar3 != '+') {
        iVar7 = (int)((uint64_t)lVar6 >> 0x20);
        lVar5 = __allmul((int)cVar3 - 0x30U,(int)((int)cVar3 - 0x30U) >> 0x1f,(uint32_t)lVar6,iVar7);
        lVar1 = lVar5 + lVar1;
        lVar6 = __allmul((uint32_t)lVar6,iVar7,10,0);
      }
    }
    local_c = local_c - 1;
  } while (local_c != 0);
  return lVar1;
}

static bool FillList(CommonDBEntry& database, DBEntryList& entryList)
{
    if (database.Content.NumEntries == 0) {
        return true;
    }

    if (database.Content.pList != nullptr && database.Content.NumItems != 0) {
        entryList.Name = database.Name;
        entryList.pData = (char*)TestDrive::Alloc( database.Content.DBSize );

        if (entryList.pData != nullptr) {
            entryList.List.resize( database.Content.NumItems );
            
            char16_t* pcVar18 = database.Content.pList;
            int32_t charIndex = 0;
            while (pcVar18[charIndex] != '{' && pcVar18[charIndex] != '\0') {
                charIndex++;
            }

            int32_t itemIndex = 0;
            while (pcVar18[charIndex] != '\0') {
                if (itemIndex == entryList.List.size()) {
                    break;
                }

                if (pcVar18[charIndex] == '{') {
                    entryList.List[itemIndex].Offset = charIndex;
                    while (pcVar18[charIndex] != '}' && pcVar18[charIndex] != '\0') {
                        charIndex++;
                    } 

                    if (pcVar18[charIndex] == '}') {
                        charIndex += 2;
                    }

                    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> converter;
                    std::basic_string<char16_t> hashString;
                    while ((pcVar18[charIndex] != '\r' && (pcVar18[charIndex] != '\0'))) {
                        hashString += pcVar18[charIndex];
                        charIndex++;
                    }

                    entryList.List[itemIndex].Hashcode = std::stoull(converter.to_bytes(hashString));
                    
                    while ((pcVar18[charIndex] != '\n' && (pcVar18[charIndex] != '\0'))) {
                        charIndex++;
                    }

                    itemIndex++;
                }
                charIndex++;
            }

            return true;
        }
    }

    return false;
}

int StrStr2Int(char *param_1)
{
  bool bVar1;
  bool bVar2;
  int iVar3;
  char cVar4;
  
  bVar1 = false;
  iVar3 = 0;
  if (*param_1 == '-') {
    bVar2 = true;
  }
  else {
    bVar2 = false;
    if (*param_1 != '+') goto LAB_009ae366;
  }
  bVar1 = bVar2;
  param_1 = param_1 + 1;
LAB_009ae366:
  cVar4 = *param_1;
  do {
    param_1 = param_1 + 1;
    iVar3 = cVar4 + -0x30 + iVar3 * 10;
    cVar4 = *param_1;
  } while (cVar4 != ';');
  if (bVar1) {
    iVar3 = -iVar3;
  }
  return iVar3;
}

float StringToFloat(char *param_1)
{
  char *pcVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  
  fVar12 = 0.0;
  fVar9 = 1.0;
  fVar13 = 0.0;
  fVar14 = 1.0;
  if (*param_1 == '-') {
    fVar14 = -1.0;
  }
  else if (*param_1 != '+') goto LAB_009ae798;
  param_1 = param_1 + 1;
LAB_009ae798:
  iVar7 = 0;
  cVar2 = *param_1;
  while ((iVar8 = iVar7, '/' < cVar2 && (cVar2 < ':'))) {
    iVar8 = iVar7 + 1;
    iVar7 = iVar7 + 1;
    cVar2 = param_1[iVar8];
  }
  for (; (((cVar2 = param_1[iVar8], '/' < cVar2 && (cVar2 < ':')) || (cVar2 == '.')) ||
         (cVar2 == ',')); iVar8 = iVar8 + 1) {
  }
  iVar6 = iVar7 + -1;
  if (3 < iVar7) {
    do {
      pcVar1 = param_1 + iVar6;
      iVar3 = iVar6 + -1;
      iVar4 = iVar6 + -2;
      iVar5 = iVar6 + -3;
      fVar10 = fVar9 * 10.0f * 10.0f;
      fVar11 = fVar10 * 10.0f;
      iVar6 = iVar6 + -4;
      fVar13 = (float)(param_1[iVar5] + -0x30) * fVar11 +
               (float)(param_1[iVar4] + -0x30) * fVar10 +
               (float)(param_1[iVar3] + -0x30) * fVar9 * 10.0f +
               (float)(*pcVar1 + -0x30) * fVar9 + fVar13;
      fVar9 = fVar11 * 10.0f;
    } while (2 < iVar6);
  }
  for (; -1 < iVar6; iVar6 = iVar6 + -1) {
    fVar13 = (float)(param_1[iVar6] + -0x30) * fVar9 + fVar13;
    fVar9 = fVar9 * 10.0f;
  }
  iVar7 = iVar7 + 1;
  fVar9 = 0.1f;
  if (3 < iVar8 - iVar7) {
    do {
      pcVar1 = param_1 + iVar7;
      iVar6 = iVar7 + 1;
      iVar3 = iVar7 + 2;
      iVar4 = iVar7 + 3;
      fVar10 = fVar9 * 0.1f * 0.1f;
      fVar11 = fVar10 * 0.1f;
      iVar7 = iVar7 + 4;
      fVar12 = (float)(param_1[iVar4] + -0x30) * fVar11 +
               (float)(param_1[iVar3] + -0x30) * fVar10 +
               (float)(param_1[iVar6] + -0x30) * fVar9 * 0.1f +
               (float)(*pcVar1 + -0x30) * fVar9 + fVar12;
      fVar9 = fVar11 * 0.1f;
    } while (iVar7 < iVar8 + -3);
  }
  for (; iVar7 < iVar8; iVar7 = iVar7 + 1) {
    fVar12 = (float)(param_1[iVar7] + -0x30) * fVar9 + fVar12;
    fVar9 = fVar9 * 0.1f;
  }
  return ((fVar12 + fVar13) * fVar14);
}

uint64_t StrToInt64(char *param_1)
{
  char cVar1;
  uint64_t lVar2;
  
  lVar2 = 0;
  if ((*param_1 == '-') || (*param_1 == '+')) {
    param_1 = param_1 + 1;
  }
  cVar1 = *param_1;
  do {
    lVar2 = __allmul((uint32_t)lVar2,(int)((uint64_t)lVar2 >> 0x20),10,0);
    lVar2 = lVar2 + (cVar1 + -0x30);
    cVar1 = param_1[1];
    param_1 = param_1 + 1;
  } while (cVar1 != ';');
  return lVar2;
}

inline void SkipLineCount(int param_1,char **param_2)
{
  char cVar1;
  char *pcVar2;
  
  cVar1 = **param_2;
  pcVar2 = *param_2;
  while ((cVar1 != '\n' && (cVar1 != '\0'))) {
    cVar1 = pcVar2[param_1];
    pcVar2 = pcVar2 + param_1;
  }
  *param_2 = pcVar2 + param_1;
  return;
}

bool IsValidReference(DBEntryList *param_1,uint64_t param_2)
{
    return true;

    // int32_t iVar4 = 0;
    // for (DBEntry& entry : param_1->List) {
    //     if (entry.Hashcode == /***/param_2) {
    //         //*param_2 = param_1->pData[entry.Offset];
    //         return true;
    //     }
    // }

    // OTDU_LOG_ERROR("Failed to find reference to %llu (not found in localized list)\n", param_2);
    // return false;
}

static bool ParseDB(CommonDBEntry* param_1,char *param_2,int stride,DBEntryList *param_5)
{
    bool bVar1;
    bool bVar2;
    bool bVar3;
    int iVar5;
    char *_Str2;
    char *pcVar7;
    char cVar8;
    const char *pcVar10;
    char *puVar11 = param_2;
    uint64_t uVar12;
    int local_104;
    float fVar13;

    _Str2 = (param_1->Content).pDB;
    bVar1 = false;
    local_104 = 0;
    do {
        cVar8 = *_Str2;
        while (((cVar8 != ' ' && (cVar8 != '\n')) && (cVar8 != '\0'))) {
            pcVar10 = _Str2 + 1;
            _Str2 = _Str2 + 1;
            cVar8 = *pcVar10;
        }
        _Str2 = _Str2 + 1;
        iVar5 = strncmp("items:",_Str2,6);
    } while (iVar5 != 0);

    SkipLineCount(1, &_Str2);

    for (int32_t i = 0; i < param_1->Content.NumData; i++) {
        // Parse each entry based on the data layout (previously parsed)
         for (char cVar8 : param_1->ParsedContent) {
            switch(cVar8) {
            case 'b':
                iVar5 = StrStr2Int(_Str2);
                *(int *)puVar11 = iVar5;
                cVar8 = *_Str2;
                while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                }
                _Str2 = _Str2 + 1;
                puVar11 += 4;
                break;
            case 'c':
                iVar5 = sscanf(_Str2,"%c",puVar11);
                if (iVar5 != 0) {
                    cVar8 = *_Str2;
                    while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                    }
                    puVar11++;
                    goto LAB_009aee2c;
                }
                bVar1 = true;
                break;
            case 'd':
            case 'i':
            case 't':
                bVar3 = false;
                iVar5 = 0;
                if (*_Str2 == '-') {
                    bVar2 = true;
        LAB_009aecae:
                    bVar3 = bVar2;
                    pcVar7 = _Str2 + 1;
                }
                else {
                    pcVar7 = _Str2;
                    bVar2 = false;
                    if (*_Str2 == '+') goto LAB_009aecae;
                }
                cVar8 = *pcVar7;
                do {
                    pcVar7 = pcVar7 + 1;
                    iVar5 = cVar8 + -0x30 + iVar5 * 10;
                    cVar8 = *pcVar7;
                } while (cVar8 != ';');
                if (bVar3) {
                    iVar5 = -iVar5;
                }
                *(int *)puVar11 = iVar5;
                cVar8 = *_Str2;
                while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                }
                _Str2 = _Str2 + 1;
                puVar11 += 4;
                break;
            case 'f':
                fVar13 = StringToFloat(_Str2);
                *(float *)puVar11 = fVar13;
                cVar8 = *_Str2;
                while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                }
                _Str2 = _Str2 + 1;
                puVar11 += 4;
                break;
            case 'h':
            case 'u':
                uVar12 = StrToInt64(_Str2);
                *(uint64_t*)puVar11 = uVar12;
                cVar8 = IsValidReference(param_5, uVar12);
                if (cVar8 == '\0') {
                    bVar1 = true;
                }
                puVar11 = puVar11 + 8;
                cVar8 = *_Str2;
                while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                }
                goto LAB_009aee2c;
            case 'l':
            case 'r':
            case 'x':
                *(uint64_t*)puVar11 = StrToInt64(_Str2);
                puVar11 += 8;
                cVar8 = *_Str2;
                while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                }
                goto LAB_009aee2c;
            case 'p':
                fVar13 = StringToFloat(_Str2);
                *(float *)puVar11 = fVar13;
                puVar11 += 4;
                cVar8 = *_Str2;
                while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                }
        LAB_009aee2c:
                _Str2 = _Str2 + 1;
                break;
            case 's':
                iVar5 = sscanf(_Str2,"%[^;];",puVar11);
                if (iVar5 != 0) {
                    cVar8 = *_Str2;
                    while ((cVar8 != ';' && (cVar8 != '\0'))) {
                    pcVar7 = _Str2 + 1;
                    _Str2 = _Str2 + 1;
                    cVar8 = *pcVar7;
                    }
                    puVar11 += 3;
                    goto LAB_009aee2c;
                }
                bVar1 = true;
                break;

            default:
                OTDU_FATAL_ERROR("Unknown database data type '%c'\n", cVar8);
                break;
            }
        }
    }

  return true;
}

bool GSDatabase::finalizeDatabaseLoad(const int32_t databaseIndex, const uint32_t stride, char** pDatabaseItems)
{
    OTDU_ASSERT(pDatabaseItems);

    CommonDBEntry& database = engineDatabase.DBList[databaseIndex];
    DBEntryList& entryList = gameDatabase.Databases[databaseIndex];

    if (*pDatabaseItems == nullptr) {
        *pDatabaseItems = (char*)TestDrive::Alloc( database.Content.NumData * stride);
    }

    database.Content.Stride = stride;
    database.Content.pDB2 = *pDatabaseItems;

    return ParseDB(&database,database.Content.pDB2,stride,&entryList);
}

bool GSDatabase::initializeDatabase(const char* pName, const uint32_t index, const uint32_t stride, char** pDatabaseItems)
{
    bool bVar2 = openDatabase( pName, index );
    if (bVar2) {
        CommonDBEntry& database = engineDatabase.DBList[index];
        DBEntryList& entryList = gameDatabase.Databases[index];
        
        FillList(database, entryList);

        finalizeDatabaseLoad(index, stride, pDatabaseItems);

        engineDatabase.DBList[index].Content.pDB = nullptr;
        engineDatabase.DBList[index].Content.pList = nullptr;
        return true;
    }

    return false;
}

uint32_t FUN_00451d50(char16_t *param_1)
{
  char16_t uVar1 = *(char16_t*)param_1;
  uint32_t uVar2;

  if (uVar1 < 0x80) {
    return 1;
  }
  if (uVar1 < 0x800) {
    return 2;
  }
  if ((uVar1 < 0xd800) || (uVar2 = 1, 0xdfff < uVar1)) {
    uVar2 = 3;
  }
  return uVar2;
}

static bool ParseList(CommonDBEntry& param_1)
{
    char16_t* pcVar6 = param_1.Content.pList;
    size_t local_24 = param_1.Content.DBSize;
    if ((pcVar6 == nullptr) || (local_24 == 0)) {
        return false;
    }

    bool local_2f = false;
    int32_t iVar12 = 0;
    if ((*pcVar6 == -1) && (pcVar6[1] == -2)) {
        iVar12 = 2;
        param_1.Content.Attributes |= 1;
        local_2f = true;
        pcVar6 = pcVar6 + 2;
    } else {
        param_1.Content.Attributes &= 0xfffffffe;
        local_2f = false;
        iVar12 = 1;
    }

    char16_t* pcVar8 = nullptr;
    char16_t cVar2 = *pcVar6;
    while ((cVar2 != '/' && (cVar2 != '\0'))) {
        pcVar8 = pcVar6 + iVar12;
        pcVar6 = pcVar6 + iVar12;
        cVar2 = *pcVar8;
    }
    cVar2 = pcVar6[iVar12];
    while ((pcVar6 = pcVar6 + iVar12, cVar2 != ' ' && (cVar2 != '\0'))) {
        cVar2 = pcVar6[iVar12];
    }
    pcVar6 = pcVar6 + iVar12;

    char16_t local_20 [32];
    pcVar8 = local_20;
    cVar2 = *pcVar6;
    while ((cVar2 != '.' && (cVar2 != '\0'))) {
        pcVar6 = pcVar6 + iVar12;
        *pcVar8 = cVar2;
        pcVar8 = pcVar8 + 1;
        cVar2 = *pcVar6;
    }
    pcVar6 = pcVar6 + iVar12;
    *pcVar8 = '\0';
    cVar2 = *pcVar6;
    while (((cVar2 != ' ' && (cVar2 != '\n')) && (cVar2 != '\0'))) {
        pcVar8 = pcVar6 + iVar12;
        pcVar6 = pcVar6 + iVar12;
        cVar2 = *pcVar8;
    }
    char16_t* local_28 = pcVar6 + iVar12;
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> converter;
    auto wideName = converter.from_bytes(param_1.Name);
    if (wideName != local_20) {
        return false;
    }

    SkipLineCount(iVar12,(char **)&local_28);

    size_t uVar11 = local_28 - param_1.Content.pList;
    char16_t* puVar9 = local_28;
    char local_2e = '{';
    bool bVar3 = false;
    bool bVar4 = false;
    bool bVar5 = false;
    uint32_t local_2c = 0u;
    uint32_t uVar10 = 0u;
    do {
        if (uVar11 < local_24) {
            if (((char)*puVar9 == local_2e) &&
                ((local_2f == false || (*(char *)((size_t)puVar9 + 1) == '\0')))) {
                if (bVar3) {
                    local_2c = local_2c + 1;
                    bVar3 = false;
                    local_2e = '{';
            LAB_009aeacc:
                    uVar10 = uVar10 + 1;
                }
                else {
                    bVar3 = true;
                    local_2e = '}'; 
                }
            }
            else if (bVar3) {
                bVar5 = IsCarriageReturn(local_2f,puVar9);
                if (bVar5) {
                    uVar10 = uVar10 + 2;
                    puVar9 = (char16_t *)((size_t)puVar9 + iVar12);
                    uVar11 = uVar11 + iVar12;
                } else {
                    if (local_2f == false) goto LAB_009aeacc;
                    int32_t iVar7 = FUN_00451d50(puVar9);
                    uVar10 = uVar10 + iVar7;
                    puVar9 = puVar9;
                }
            }
        } else {
            bVar4 = true;
        }
        puVar9 = (char16_t *)((size_t)puVar9 + iVar12);
        uVar11 = uVar11 + iVar12;
        if (bVar4) {
            param_1.Content.DBSize = uVar10;
            param_1.Content.NumItems = local_2c;
            return true;
        }
    } while( true );

    return false;
}

static bool ParseDatabase(CommonDBEntry& database)
{
    char local_120[32];
    char auStack_100[256];

    char* local_12c = database.Content.pDB;
    if (local_12c == nullptr) {
        return false;
    }

    int32_t local_128 = 0;
    int32_t iVar4 = sscanf(local_12c,"// %[^.].db",local_120);
    if (iVar4 != 0) {
        if (database.Name != local_120) {
            OTDU_LOG_ERROR( "Error while parsing database '%s': name mismatch (name parsed '%s')\n", database.Name.c_str(), local_120);
            return false;
        }
        
        SkipLineCount(1,&local_12c);
        iVar4 = sscanf(local_12c, "// Version: %d,%d", &database.Content.Version, &database.Content.Release);
        if (iVar4 != 0) {
            SkipLineCount(1,&local_12c);
            iVar4 = sscanf(local_12c,"// Categories: %d", &database.Content.NumEntries);
            bool bVar3 = ParseList(database);
            if (bVar3) {
                SkipLineCount(1,&local_12c);
                iVar4 = sscanf(local_12c,"// Fields: %d",&database.Content.NumFields);
                if (iVar4 != 0) {
                    SkipLineCount(1,&local_12c);
                    iVar4 = sscanf(local_12c,"{%[^}]} %I64d",local_120,&database.Content.Hashcode);
                    if ((iVar4 != 0) && database.Name == local_120) {
                        SkipLineCount(1,&local_12c);
                        for (uint32_t i = 0; i < database.Content.NumFields; i++) {
                            char *pcVar5 = local_12c;
                            char cVar2 = '\0';
                            char *pcVar1 = nullptr;
                            int64_t pCVar8 = 0ull;
                            char pcVar6 = '\0';
                            iVar4 = sscanf(local_12c,"{%[^}]} %c",auStack_100,&pcVar6);

                            if (iVar4 == 0) {
                                database.Content.pDB = nullptr;
                                database.Content.pList = nullptr;
                                return false;
                            }
                            if (pcVar6 == 'r' || pcVar6 == 'l') {
                                cVar2 = *pcVar5;
                                while ((cVar2 != ' ' && (cVar2 != '\0'))) {
                                    pcVar1 = pcVar5 + 1;
                                    pcVar5 = pcVar5 + 1;
                                    cVar2 = *pcVar1;
                                }
                                cVar2 = pcVar5[1];
                                while ((pcVar5 = pcVar5 + 1, cVar2 != ' ' && (cVar2 != '\0'))) {
                                    cVar2 = pcVar5[1];
                                }
                                local_12c = pcVar5; // + 1;
                                iVar4 = sscanf(local_12c," %llu\r\n",&pCVar8);
                                if (iVar4 == 0) {
                                    database.Content.pDB = nullptr;
                                    database.Content.pList = nullptr;
                                    return false;
                                }
                                database.References.push_back(pCVar8);
                            }

                            SkipLineCount(1,&local_12c);

                            database.ParsedContent += pcVar6;
                        }
                        iVar4 = sscanf(local_12c,"// items: %d",&database.Content.NumData);
                        SkipLineCount(1,&local_12c);
                        
                        return iVar4 != 0;
                    }
                }
            }
        }
    }

    database.Content.pDB = nullptr;
    database.Content.pList = nullptr;
    return false;
}

bool GSDatabase::openDatabase(const char *pName, const uint32_t index)
{
    CommonDBEntry& database = engineDatabase.DBList[index];
    database.Name = pName;
    database.Filepath = gameDatabase.Filepath + "/" + database.Name;
    database.Content.pDB = dbInstallRequests[index].pBuffer;
    database.Content.pList = localizedDbInstallRequests[index].pBuffer;
    database.Content.DBSize = localizedDbInstallRequests[index].BufferSize;
    database.Language[0] = pActiveLanguage[0];
    database.Language[1] = pActiveLanguage[1];
    database.Language[2] = pActiveLanguage[2];

    return ParseDatabase(database);
}

int32_t GSDatabase::setFlashLocalization(int32_t param_2, uint32_t param_3, FlashPlayer* param_4)
{
    uint32_t uVar3 = 0;
    int32_t local_4 = 0;
    int32_t iVar4 = 0;

    uVar3 = param_3 | 1;
    local_4 = 0;
    iVar4 = param_4->getVariableAsInt("db_hud");
    if (iVar4 == 1) {
        uVar3 = param_3 | 3;
    }
    iVar4 = param_4->getVariableAsInt("db_hud_cha");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 4;
    }
    iVar4 = param_4->getVariableAsInt("db_map");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 8;
    }
    iVar4 = param_4->getVariableAsInt("db_home");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 0x10;
    }
    iVar4 = param_4->getVariableAsInt("db_challenge");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 0x20;
    }
    iVar4 = param_4->getVariableAsInt("db_gme");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 0x40;
    }
    iVar4 = param_4->getVariableAsInt("db_shops");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 0x80;
    }
    iVar4 = param_4->getVariableAsInt("db_drive_in");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 0x100;
    }
    iVar4 = param_4->getVariableAsInt("db_ice");
    if (iVar4 == 1) {
        uVar3 = uVar3 | 0x200;
    }
    
    uint32_t numEntries = engineDatabase.DBList[param_2].Content.NumData;
    for (uint32_t i = 0; i < numEntries; i++) {
        const DBEntryMenu& pDVar5 = gameDatabase.DBMenu[i];
        
        if ((uVar3 & pDVar5.Flags) != 0) {
            if (pDVar5.Flags < 0) {
                if (gpMngFlashLocalize != nullptr) {
                    TestDriveString8 str;
                    str = pDVar5.Name;
                    iVar4 = gpMngFlashLocalize->FUN_00716f60(str.AsChar);
                    if (iVar4 == 0) {
                        TestDriveString8 str2;
                        str2 = pDVar5.FrontEndResourceName;
                        gpMngFlashLocalize->FUN_00717ff0(param_4, str.AsChar, str2.AsChar, &i);
                    }
                }
            } else if ( pDVar5.Name != 0ull && pDVar5.FrontEndResourceName != 0ull ) {
                param_4->setVariableValue(pDVar5.FrontEndResourceName, pDVar5.Name);
            }

            local_4++;
        }
    }

    const char* pLanguage = gpPlayerData->getLanguage();
    const PlayerDataLanguage langID(pLanguage);
    eLocale locale = langID;
    if (locale == eLocale::L_Spanish || locale == eLocale::L_Japenese || locale == eLocale::L_Korean || locale != eLocale::L_English) {
        uVar3 = 0;
    } else {
        uVar3 = 1;
    }
    gpPlayerData->setUnitSystem(uVar3);
    setLanguage(param_4);
    return local_4;
}

static constexpr const uint64_t kLanguageHashcodes[5] = {
    0x0000000035FE382,
    0x0000000036F25C2,
    0x0000000037E6802,
    0x0000000038DAA42,
    0x0000000039CEC82,
};

bool GSDatabase::setLanguage(FlashPlayer* param_1)
{
    PlayerDataLanguage langID(gpPlayerData->getLanguage());
    uint16_t langIndex = langID;
    uint64_t hashcode = langIndex <= 4 ? kLanguageHashcodes[langIndex] : kLanguageHashcodes[0];
    
    const char* pcVar1 = getStringByHashcode( 0u, hashcode );
    param_1->setVariableValue("/:WhatLanguage", pcVar1);
    return true;
}

bool GSDatabase::isInitialized() const
{
    return bInitialized;
}
