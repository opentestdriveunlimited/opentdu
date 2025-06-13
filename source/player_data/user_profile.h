#pragma once

class UserProfileRegister {
public:
    inline uint64_t getActiveProfileHashcode() const { return activeProfileHashcode; }

public:
    UserProfileRegister();
    ~UserProfileRegister();

    bool initialize(uint64_t hashcode);

    void deserialize();
    std::string getProfileName(uint64_t hashcode) const;
    int32_t getProfileIndex(uint64_t hashcode) const;

private:
    std::string saveRoot;
    std::string activeProfileName;
    int32_t activeProfileIndex;
    uint64_t activeProfileHashcode;
    std::string pathToProfile;

    uint64_t listHashcode;

    std::vector<std::string> profileNames;
};

extern UserProfileRegister gUserProfileRegister;
