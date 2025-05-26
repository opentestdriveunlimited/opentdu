#include "shared.h"
#include "gs_database.h"

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
    return true;
}

void GSDatabase::tick()
{

}

void GSDatabase::terminate()
{

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
