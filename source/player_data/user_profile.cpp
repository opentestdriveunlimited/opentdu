#include "shared.h"
#include "user_profile.h"

#include "core/hash/crc32.h"
#include "filesystem/gs_file.h"

#include <codecvt>

UserProfileRegister gUserProfileRegister = {};

UserProfileRegister::UserProfileRegister()
    : saveRoot( "./" )
    , activeProfileName( "" )
    , activeProfileIndex( 0 )
    , activeProfileHashcode( 0 )
    , pathToProfile( "" )
    , listHashcode( 0ull )
    , unknownDword( 0 )
    , unknownDword2( 0 )
{

}

UserProfileRegister::~UserProfileRegister()
{

}

bool UserProfileRegister::initialize(uint64_t hashcode)
{
    activeProfileHashcode = hashcode;
    activeProfileIndex = getProfileIndex(hashcode);
    activeProfileName.clear();
    
    activeProfileName = profileNames[activeProfileIndex];
    
    OTDU_LOG_DEBUG("Active profile set to '%s' (profile index %u hashcode %llu)\n", activeProfileName.c_str(), activeProfileIndex, activeProfileHashcode);

    return true;
}

void UserProfileRegister::deserialize()
{
    std::wstring saveLocation = gpFile->getSaveLocation( eContextSource::CS_Savegame );
    if (saveLocation.empty()) {
        OTDU_LOG_ERROR("Failed to retrieve save location; profile deserialization will be skipped\n");
        return;
    }
    saveLocation += L"/ProfileList.dat";

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    
    std::string narrowFilename = converter.to_bytes(saveLocation);
    
    OTDU_LOG_DEBUG("Deserializing profile list '%s'...\n", narrowFilename.c_str());

    std::ifstream profileListStream(narrowFilename.c_str(), std::ios::in | std::ios::binary);

    uint64_t activeProfileCRC = 0ull;
    if (profileListStream.good()) {
        profileListStream.seekg(0, std::ios::end);
        size_t fileSize = profileListStream.tellg();
        profileListStream.seekg(0);
    
        uint16_t word = 0;
        for (size_t i = 0; i < fileSize; ) {
            profileListStream.read((char*)&word, sizeof(uint16_t));
            i += sizeof(uint16_t);

            if (word == 0xffff) {
                profileListStream.read((char*)&activeProfileCRC, sizeof(uint64_t));
                break;
            } else {
                std::string profileName;
                profileName.resize( word );
                
                profileListStream.read(profileName.data(), sizeof(char) * word);
                OTDU_LOG_DEBUG("\t-Found profile '%s' (index %u)\n", profileName.c_str(), profileNames.size());

                profileNames.push_back(profileName);
            }

            i += word;
        }
    }
    profileListStream.close();

    if (profileNames.size() == 0) {
        listHashcode = 0ull;
        return;
    }

    int32_t deserializedProfileIndex = getProfileIndex(activeProfileCRC);
    if (deserializedProfileIndex < 0) {
        if (profileNames.size() > 0) {
            listHashcode = TestDriveCRC32(profileNames[0]);
        }
    } else {
        listHashcode = activeProfileCRC;
    }

    initialize(listHashcode);
}

std::string UserProfileRegister::getProfileName(uint64_t hashcode) const
{
    int32_t profileIndex = getProfileIndex(hashcode);
    return (-1 < profileIndex) ? profileNames[profileIndex] : "";
}

int32_t UserProfileRegister::getProfileIndex(uint64_t hashcode) const
{
    for (int32_t i = 0; i < profileNames.size(); i++) {
        uint32_t profileHash = TestDriveCRC32(profileNames[i]);
        if (profileHash == hashcode) {
            return i;
        }
    }

    return -1;
}

void UserProfileRegister::FUN_0060a160()
{
    // FUN_0060a160
    unknownDword = 5;
}

uint32_t UserProfileRegister::FUN_00419de0() const
{
    // FUN_00419de0
    return unknownDword2;
}
