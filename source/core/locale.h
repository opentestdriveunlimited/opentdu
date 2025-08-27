#pragma once

#include "shared.h"

enum eLocale : uint32_t 
{
    L_Japanese = 0,
    L_English = 1,
    L_French = 2,
    L_Spanish = 3,
    L_German = 4,
    L_Italian = 5,
    L_Dutch = 6,
    L_Portuguese = 7,
    L_Chinese = 8,
    L_Korean = 9,
    L_Russian = 10,
    L_Polish = 11,

    L_Count,

    L_Unknown = 0xffffffff,
};

struct PlayerDataLanguage 
{
    union 
    {
        uint16_t ID = 0;
        char Str[2];
    };

    constexpr PlayerDataLanguage( const uint16_t langID ) : ID( langID ) {}
    constexpr PlayerDataLanguage( const char* langStr ) { Str[0] = langStr[0]; Str[1] = langStr[1]; }

    constexpr operator uint16_t() const
    {
        return ID;
    }

    constexpr bool operator == ( PlayerDataLanguage r )
    {
        return ID == r.ID;
    }

    constexpr bool operator == ( const char* r )
    {
        return Str[0] == r[0] && Str[1] == r[1];
    }

    constexpr operator eLocale() const 
    {
        switch(ID)
        {
            case 0x6a61:                return eLocale::L_Japanese;
            case 0x6762: case 0x7573:   return eLocale::L_English;
            case 0x6672:                return eLocale::L_French;
            case 0x7370:                return eLocale::L_Spanish;
            case 0x6765:                return eLocale::L_German;
            case 0x6974:                return eLocale::L_Italian;
            case 0x6368:                return eLocale::L_Chinese;
            case 0x6b6f:                return eLocale::L_Korean;
            case 0x7275:                return eLocale::L_Russian;
            case 0x706f:                return eLocale::L_Polish;
            case 0x7074:                return eLocale::L_Portuguese;
            case 0x6465:                return eLocale::L_Dutch;
            default:                    return eLocale::L_English;
        }
    }
};

static constexpr PlayerDataLanguage kLangUS( 0x7573 );
static constexpr PlayerDataLanguage kLangGB( 0x6762 );
static constexpr PlayerDataLanguage kLangFR( 0x6672 );
static constexpr PlayerDataLanguage kLangIT( 0x6974 );
static constexpr PlayerDataLanguage kLangGE( 0x6765 );
static constexpr PlayerDataLanguage kLangSP( 0x7370 );
static constexpr PlayerDataLanguage kLangKO( 0x6b6f );
static constexpr PlayerDataLanguage kLangJP( 0x6a61 );
static constexpr PlayerDataLanguage kLangCH( 0x6368 );
static constexpr PlayerDataLanguage kLangRU( 0x7275 );
static constexpr PlayerDataLanguage kLangPO( 0x706f );
static constexpr PlayerDataLanguage kLangPT( 0x7074 );
static constexpr PlayerDataLanguage kLangDE( 0x6465 );

#if OTDU_WIN32
static eLocale GetLocaleFromSystem()
{
    LCID LVar1 = GetUserDefaultLCID();
    switch ( PRIMARYLANGID( LVar1 ) ) {
    case LANG_CHINESE: // Chinese
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
        return L_Japanese;
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
        return L_Japanese;
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
