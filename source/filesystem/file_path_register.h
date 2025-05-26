#pragma once

class FilePathRegister
{
public:
    FilePathRegister();
    ~FilePathRegister();

    void registerPath(const std::string& param_1);
    void setDefaultPath(const std::string &param_1);

private:
    struct PathEntry {
        std::string Path;
        int32_t Priority;
    };

private:
    std::vector<PathEntry> entries;
    std::string defaultPath;
};

extern FilePathRegister* gpFilePathRegister;
