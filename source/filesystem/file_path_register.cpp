#include "shared.h"
#include "file_path_register.h"

FilePathRegister* gpFilePathRegister = nullptr;

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

    OTDU_UNIMPLEMENTED; // TODO:
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
