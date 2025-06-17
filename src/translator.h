#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <unordered_map>
#include <string>
#include <tchar.h>

#define _TR(str) Translator::instance().tr(_T(#str))

using std::wstring;
using std::unordered_map;

typedef unordered_map<wstring, wstring> LocaleMap;
typedef unordered_map<wstring, LocaleMap> TranslationsMap;


class Translator
{
public:
    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;
    static Translator& instance();

    void init(unsigned long langId, int resourceId);
    const wchar_t* tr(const wchar_t*) const;

private:
    Translator();
    ~Translator();

    void parseTranslations();

    TranslationsMap translMap;
    wstring        translations,
                   error_substr;
    wstring langName;
    bool    is_translations_valid;

    enum TokenType : unsigned char {
        TOKEN_BEGIN_DOCUMENT = 0,
        TOKEN_END_DOCUMENT,
        TOKEN_BEGIN_STRING_ID,
        TOKEN_END_STRING_ID,
        TOKEN_BEGIN_LOCALE,
        TOKEN_END_LOCALE,
        TOKEN_BEGIN_VALUE,
        TOKEN_END_VALUE
    };
};

#endif // TRANSLATOR_H
