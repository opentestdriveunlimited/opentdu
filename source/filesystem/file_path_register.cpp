#include "shared.h"
#include "file_path_register.h"

FilePathRegister gFilePathRegister = {};

FilePathRegister::FilePathRegister()
{
    
}

FilePathRegister::~FilePathRegister()
{

}

void FilePathRegister::registerPath(const std::string &param_1)
{
    bool bVar1 = false;
    auto it = param_1.find( ':' );
    if (it != std::string::npos) {
        auto pcVar2 = param_1.begin() + it;
        if ((pcVar2[1] != '\\') || (bVar1 = true, pcVar2[2] != '\0')) {
            bVar1 = false;
        }
    }

    for (PathEntry& entry : entries) {
        if (entry.Path == param_1) {
            if (bVar1) {
                entry.Path.clear();
                entry.Priority = 0;
                return;
            }

            PathEntry newEntry;
            newEntry.Path = param_1;
            newEntry.Priority = entries.size();
            entries.push_back( newEntry );
            return;
        }
    }
}

void FilePathRegister::setDefaultPath(const std::string &param_1)
{
    defaultPath.clear();
    for (const char& cVar1 : param_1) {
        char cVar2 = cVar1;
        if (('`' < cVar1) && (cVar1 < '{')) {
            cVar2 = cVar1 + -0x20;
        }
        defaultPath += cVar2;

        if (cVar2 == '>') {
            break;
        }
    }
}

size_t FilePathRegister::getFiler(const char *param_1)
{
    return strlen(param_1) - defaultPath.length();
    // char cVar1;
    // char *pcVar2;
    // char *pcVar3;
    // char *pcVar4;

    // pcVar2 = const_cast<char*>(defaultPath.c_str());
    // pcVar4 = pcVar2;
    // do {
    //     cVar1 = *pcVar4;
    //     pcVar4[param_1 - (int)pcVar2] = cVar1;
    //     pcVar4 = pcVar4 + 1;
    //     pcVar3 = pcVar2;
    // } while (cVar1 != '\0');
    // do {
    //     cVar1 = *pcVar3;
    //     pcVar3 = pcVar3 + 1;
    // } while (cVar1 != '\0');    
    // return pcVar3 + (-1 - (int)pcVar2);
}
