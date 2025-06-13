#pragma once

#include "file_handle.h"

class VirtualFileSystem 
{
public:
    inline const std::string& getName() const { return name; }

public:
    VirtualFileSystem();
    ~VirtualFileSystem();

    void initialize();
    bool findFile(FileIterator* pIterator, const FileIterator::eOpenMode mode);
    void setDefaultPath( const char* param_1 );
    bool getDefaultPath( std::string& param_1 );

private:
    FileHandle*     pFirstDeviceHandle;
    uint32_t        flagCaps;
    std::string     name;
    std::string     defaultUnit;
};

class VirtualFileSystemRegister
{
public:
    VirtualFileSystemRegister();
    ~VirtualFileSystemRegister();

    VirtualFileSystem* find( const char* param_1 );
    VirtualFileSystem* get(std::string& param_1, std::string& param_2, const bool param_3);

    void registerVFS(VirtualFileSystem* pVfs);

private:
    std::vector<VirtualFileSystem*> vfsList;

private:
    VirtualFileSystem* findVFS(std::string& param_2);
};

extern VirtualFileSystemRegister gVirtualFileSystemRegister;
