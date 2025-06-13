#pragma once

#include "core/locale.h"
#include "game/gs_base.h"
#include "game/vehicle/car_desc.h"

#include "db_achievement.h"
#include "db_aftermarket_pack.h"
#include "db_bot.h"
#include "db_brand.h"
#include "db_car_color.h"
#include "db_car_pack.h"
#include "db_car_rim.h"
#include "db_car_shop.h"
#include "db_car_physics_data.h"
#include "db_clothe.h"
#include "db_hair.h"
#include "db_house.h"
#include "db_interior.h"
#include "db_menu.h"
#include "db_pnj.h"
#include "db_rim.h"
#include "db_subtitle.h"
#include "db_tutorial.h"

static constexpr int32_t kNumDatabases = 18;

struct DBEntry
{
    uint64_t Hashcode;
    uint32_t Offset;
};

struct DBEntryList
{
    std::string Name;
    std::vector<DBEntry> List;
    char* pData;
};

struct GameDatabase
{
    DBEntryMenu* DBMenu = nullptr;
    DBEntryCarPhysicsData* DBCarData = nullptr;
    DBEntryRim* DBRim = nullptr;
    DBEntryCarRim* DBCarRims = nullptr;
    DBEntryInterior* DBInterior = nullptr;
    DBEntryCarColor* DBColors = nullptr;
    DBEntryBrand* DBBrand = nullptr;
    DBEntryCarShop* DBCarShop = nullptr;
    DBEntryHouse* DBHouse = nullptr;
    DBEntryHair* DBHair = nullptr;
    DBEntryClothe* DBClothe = nullptr;
    DBEntryTutorial* DBTutorial = nullptr;
    DBEntryAftermarketPack* DBPack = nullptr;
    DBEntryCarPack* DBCarPack = nullptr;
    DBEntryBot* DBBot = nullptr;
    DBEntryAchievement* DBAchivement = nullptr;
    DBEntryPNJ* DBPNJ = nullptr;
    DBEntrySubtitle* DBSubtitle = nullptr;

    std::array<DBEntryList, kNumDatabases> Databases;
    std::string Filepath = "";
};

struct CommonDBContent
{
    char * pDB = nullptr;
    int32_t NumData = 0;
    uint32_t NumFields = 0;
    char16_t* pList = nullptr;
    uint32_t NumEntries = 0;
    uint32_t NumItems = 0;
    size_t DBSize = 0ull;
    uint64_t Hashcode = 0ull;
    int32_t Version = 0;
    int32_t Release = 0;
    char* pDB2 = nullptr;
    int32_t Stride = 0;
    uint32_t Attributes = 0;
};

struct CommonDBEntry
{
    std::vector<uint64_t> References;
    std::string Name = "";
    std::string Filepath = "";
    std::string ParsedContent = "";
    char Language[4] = { '\0', '\0', '\0', '\0' };
    CommonDBContent Content;
};

struct CommonDB
{
    std::array<CommonDBEntry, kNumDatabases> DBList;
    uint8_t bInitialized : 1;
};

template<typename TCharType>
struct DBInstallRequest 
{
    TCharType*   pBuffer = nullptr;
    size_t       BufferSize = 0ull;
};

class GSDatabase : public GameSystem {
public:
    int32_t NbChallengeNames;
    int32_t NbChallengeDesc;

public:
    const char* getName() const override { return "DataBase"; }
    inline const size_t getNumCars() const { return cars.size(); }
    inline const std::vector<CarConfig>& getCarList() const { return cars; }

public:
    GSDatabase();
    ~GSDatabase();

    bool initialize( TestDriveGameInstance* ) override;
    void terminate() override;
    void reset() override;
    void pause() override;

    bool reloadDatabases( const char* pLanguage, const bool bForceReload = false );
    char* getStringByHashcode(const uint32_t dbIndex, const uint64_t hashcode);
    void releaseDatabases();

private:
    CommonDB engineDatabase;
    GameDatabase gameDatabase;

    std::array<DBInstallRequest<char>, kNumDatabases>     dbInstallRequests;
    std::array<DBInstallRequest<char16_t>, kNumDatabases> localizedDbInstallRequests;

    std::vector<CarConfig> cars;
    char pActiveLanguage[3];
    eLocale activeLocale;

    uint8_t bInitialized : 1;

private:
    bool loadDatabases( const char* pLanguage );
    bool initializeDatabase( const char* pName, const uint32_t index, const uint32_t maxNumElements, char** pDatabaseItems);
    bool openDatabase( const char* pName, const uint32_t index );
    bool finalizeDatabaseLoad(const int32_t databaseIndex, const uint32_t stride, char** pDatabaseItems);
};

extern GSDatabase* gpDatabase;
