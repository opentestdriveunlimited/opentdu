#pragma once

#include "shared.h"

enum eLocale : uint32_t {
    L_Japenese = 0,
    L_English = 1,
    L_French = 2,
    L_Spanish = 3,
    L_German = 4,
    L_Italian = 5,
    L_Dutch = 6,
    L_Portuguese = 7,
    L_Chinese = 7,
    L_Korean = 9,
    L_Russian = 10,
    L_Polish = 11,

    L_Count,

    L_Unknown = 0xffffffff,
};

#if OTDU_WIN32
static eLocale GetLocaleFromSystem()
{
    LCID LVar1 = GetUserDefaultLCID();
    switch ( PRIMARYLANGID( LVar1 ) ) {
    case 4: // Chinese
        if ( SUBLANGID( LVar1 ) == 0x400 ) { // SUBLANGID
            return L_Chinese;
        }
        break;
    case LANG_GERMAN: // German
        return L_German;
    case LANG_SPANISH: // Spanish
        return L_Spanish;
    case LANG_FRENCH:
        return L_French;
    case LANG_ITALIAN:
        return L_Italian;
    case LANG_JAPANESE:
        return L_Japenese;
    case LANG_KOREAN:
        return L_Korean;
    case LANG_DUTCH:
        return L_Dutch;
    case LANG_POLISH:
        return L_Polish;
    case LANG_PORTUGUESE:
        return L_Portuguese;
    case LANG_RUSSIAN:
        return L_Russian;
    case LANG_ENGLISH: // English
    default:
        return L_English;
    }

    return L_Unknown;
}
#elif OTDU_MACOS
static eLocale GetLocaleFromSystem()
{
    setlocale(LC_ALL, "");
    const char* pLocale = setlocale(LC_ALL, NULL);

    if (strstr(pLocale, "zh_")) {
        return L_Chinese;
    } else if (strstr(pLocale, "de_")) {
        return L_German;
    } else if (strstr(pLocale, "es_")) {
        return L_Spanish;
    } else if (strstr(pLocale, "fr_")) {
        return L_French;
    } else if (strstr(pLocale, "it_")) {
        return L_Italian;
    } else if (strstr(pLocale, "ja_")) {
        return L_Japenese;
    } else if (strstr(pLocale, "ko_")) {
        return L_Korean;
    } else if (strstr(pLocale, "nl_")) {
        return L_Dutch;
    } else if (strstr(pLocale, "pl_")) {
        return L_Polish;
    } else if (strstr(pLocale, "pt_")) {
        return L_Portuguese;
    } else if (strstr(pLocale, "ru_")) {
        return L_Russian;
    } else if (strstr(pLocale, "en_")) {
        return L_English;
    }

    return L_Unknown;
}
#endif
