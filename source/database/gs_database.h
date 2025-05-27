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

static constexpr uint32_t kNumDatabases = 18;

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
    std::vector<DBEntryMenu> DBMenu;
    std::vector<DBEntryCarPhysicsData> DBCarData;
    std::vector<DBEntryRim> DBRim;
    std::vector<DBEntryCarRim> DBCarRims;
    std::vector<DBEntryInterior> DBInterior;
    std::vector<DBEntryCarColor> DBColors;
    std::vector<DBEntryBrand> DBBrand;
    std::vector<DBEntryCarShop> DBCarShop;
    std::vector<DBEntryHouse> DBHouse;
    std::vector<DBEntryHair> DBHair;
    std::vector<DBEntryClothe> DBClothe;
    std::vector<DBEntryTutorial> DBTutorial;
    std::vector<DBEntryAftermarketPack> DBPack;
    std::vector<DBEntryCarPack> DBCarPack;
    std::vector<DBEntryBot> DBBot;
    std::vector<DBEntryAchievement> DBAchivement;
    std::vector<DBEntryPNJ> DBPNJ;
    std::vector<DBEntrySubtitle> DBSubtitle;

    std::array<DBEntryList, kNumDatabases> Databases;
    std::string Filepath = "";
};

struct CommonDBContent
{
    char * pDB;
    int32_t NumData;
    uint32_t NumFields;
    char* pList;
    uint32_t NumEntries;
    uint32_t NumItems;
    uint32_t DBSize;
    uint64_t Hashcode;
    int32_t Version;
    int32_t Release;
    char* pDB2;
    int32_t Stride;
    uint32_t Attributes;
};

struct CommonDBEntry
{
    std::vector<uint64_t> References;
    std::string Name;
    std::string Filepath;
    std::string ParsedContent;
    char Language[4];
    CommonDBContent Content;
};

struct CommonDB
{
    std::array<CommonDBEntry, kNumDatabases> DBList;
    uint8_t bInitialized : 1;
};

struct DBInstallRequest 
{
    int8_t* pBuffer = nullptr;
    size_t  BufferSize = 0ull;
};

class GSDatabase : public GameSystem {
public:
    int32_t NbChallengeNames;
    int32_t NbChallengeDesc;

public:
    const char* getName() const override { return "DataBase"; }
    inline const uint32_t getNumCars() const { return cars.size(); }
    inline const std::vector<CarConfig>& getCarList() const { return cars; }

public:
    GSDatabase();
    ~GSDatabase();

    bool initialize( TestDriveGameInstance* ) override;
    void tick() override;
    void terminate() override;

    bool reloadDatabases( const char* pLanguage, const bool bForceReload = false );
    char* getStringByHashcode(const uint32_t dbIndex, const uint64_t hashcode);

private:
    CommonDB engineDatabase;
    GameDatabase gameDatabase;

    std::array<DBInstallRequest, kNumDatabases> dbInstallRequests;

    std::vector<CarConfig> cars;
    eLocale activeLocale;

    uint8_t bInitialized : 1;
};

extern GSDatabase* gpDatabase;
