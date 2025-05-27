#include "shared.h"
#include "gs_database.h"

#include "config/gs_config.h"
#include "player_data/gs_playerdata.h"

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
    bool bDatabaseLoadResult  = reloadDatabases(pLanguage, false);

    return bDatabaseLoadResult;
}

void GSDatabase::tick()
{

}

void GSDatabase::terminate()
{

}

bool GSDatabase::reloadDatabases(const char *pLanguage, const bool bForceReload)
{
    activeLocale = PlayerDataLanguage(pLanguage);

    const char* pResourcePath = gpConfig->getResRootPath();
    gameDatabase.Filepath = pResourcePath;
    gameDatabase.Filepath += "DataBase";

    return true;
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
