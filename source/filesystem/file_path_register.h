#pragma once

class FilePathRegister
{
public:
    FilePathRegister();
    ~FilePathRegister();

    void registerPath(const std::string& param_1);
    void setDefaultPath(const std::string &param_1);

    size_t getFiler(const char *param_1);

private:
    struct PathEntry {
        std::string Path;
        size_t Priority;
    };

private:
    std::vector<PathEntry> entries;
    std::string defaultPath;
};

extern FilePathRegister gFilePathRegister;
