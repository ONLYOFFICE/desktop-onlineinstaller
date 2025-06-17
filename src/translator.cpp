
#define WIN32_LEAN_AND_MEAN
#include "translator.h"
#include "resource.h"
#include "utils.h"
#include <Windows.h>
#include <objidl.h>
#include <codecvt>
#include <algorithm>
#include <sstream>


wstring getPrimaryLang(const wstring &lang, bool withScript = false)
{
    if (lang.empty()) {
        NS_Logger::WriteLog(_T("An error occurred:\n") + wstring(_T(__FUNCTION__)));
        return L"en";
    }
    std::wistringstream iss(lang);
    wstring primlang, script;
    std::getline(iss, primlang, L'_');
    if (primlang.length() == 2 || primlang.length() == 3) {
        if (!withScript)
            return primlang;
        std::getline(iss, script, L'_');
        return (script.length() == 4) ? primlang + L"_" + script : primlang;
    }
    NS_Logger::WriteLog(_T("An error occurred:\n") + wstring(_T(__FUNCTION__)));
    return L"en";
}

wstring StrToWStr(const string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

static IStream* LoadResourceToStream(int resourceId)
{
    IStream *pStream = nullptr;
    HMODULE hInst = GetModuleHandle(nullptr);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(resourceId), RT_RCDATA)) {
        DWORD dataSize = SizeofResource(hInst, hRes);
        if (dataSize > 0) {
            if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
                if (LPVOID pData = LockResource(hResData)) {
                    if (HGLOBAL hGlobal = GlobalAlloc(GHND, dataSize)) {
                        if (LPVOID pBuffer = GlobalLock(hGlobal)) {
                            memcpy(pBuffer, pData, dataSize);
                            GlobalUnlock(hGlobal);
                            HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
                            if (FAILED(hr)) {
                                GlobalFree(hGlobal);
                                pStream = nullptr;
                            }
                        } else {
                            GlobalFree(hGlobal);
                        }
                    }
                }
                FreeResource(hResData);
            }
        }
    }
    return pStream;
}

Translator::Translator() :
    langName(_T("en")),
    is_translations_valid(false)
{

}

Translator& Translator::instance()
{
    static Translator inst;
    return inst;
}

void Translator::init(unsigned long langId, int resourceId)
{
    TCHAR _langName[LOCALE_NAME_MAX_LENGTH] = {0};
    if (GetLocaleInfo(langId, LOCALE_SNAME, _langName, LOCALE_NAME_MAX_LENGTH) > 0) {
        langName = _langName;
        std::replace(langName.begin(), langName.end(), L'-', L'_');
    } else
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);

    NS_Logger::WriteLog(_T("\nCurrent locale:\n") + langName);

    is_translations_valid = false;
    if (IStream *pStream = LoadResourceToStream(resourceId)) {
        const char ISL_MAGIC[] = "ISL";
        ULONG bytesRead = 0;
        HRESULT hr = S_OK;
        char magic[sizeof(ISL_MAGIC)] = { 0 };
        hr = pStream->Read(magic, sizeof(magic), &bytesRead);
        if (FAILED(hr) || bytesRead != sizeof(magic) || strncmp(magic, ISL_MAGIC, sizeof(magic) - 1) != 0) {
            pStream->Release();
            return;
        }
        WORD stringsMapSize = 0;
        hr = pStream->Read(&stringsMapSize, sizeof(stringsMapSize), &bytesRead);
        if (FAILED(hr) || bytesRead != sizeof(stringsMapSize)) {
            pStream->Release();
            return;
        }
        for (WORD i = 0; i < stringsMapSize; i++) {
            uint8_t stringIdLen = 0;
            hr = pStream->Read(&stringIdLen, sizeof(stringIdLen), &bytesRead);
            if (FAILED(hr) || bytesRead != sizeof(stringIdLen)) {
                pStream->Release();
                return;
            }
            std::string stringId(stringIdLen, '\0');
            hr = pStream->Read(&stringId[0], stringIdLen, &bytesRead);
            if (FAILED(hr) || bytesRead != stringIdLen) {
                pStream->Release();
                return;
            }
            WORD localeMapSize = 0;
            hr = pStream->Read(&localeMapSize, sizeof(localeMapSize), &bytesRead);
            if (FAILED(hr) || bytesRead != sizeof(localeMapSize)) {
                pStream->Release();
                return;
            }
            LocaleMap localeMap;
            for (WORD j = 0; j < localeMapSize; j++) {
                uint8_t localeLen = 0;
                hr = pStream->Read(&localeLen, sizeof(localeLen), &bytesRead);
                if (FAILED(hr) || bytesRead != sizeof(localeLen)) {
                    pStream->Release();
                    return;
                }
                std::string localeName(localeLen, '\0');
                hr = pStream->Read(&localeName[0], localeLen, &bytesRead);
                if (FAILED(hr) || bytesRead != localeLen) {
                    pStream->Release();
                    return;
                }
                WORD translationLen = 0;
                hr = pStream->Read(&translationLen, sizeof(translationLen), &bytesRead);
                if (FAILED(hr) || bytesRead != sizeof(translationLen)) {
                    pStream->Release();
                    return;
                }
                std::string translationString(translationLen, '\0');
                hr = pStream->Read(&translationString[0], translationLen, &bytesRead);
                if (FAILED(hr) || bytesRead != translationLen) {
                    pStream->Release();
                    return;
                }
                localeMap[StrToWStr(localeName)] = StrToWStr(translationString);
            }
            translMap[StrToWStr(stringId)] = localeMap;
        }
        pStream->Release();
        is_translations_valid = true;
    }
}

Translator::~Translator()
{

}

const wchar_t* Translator::tr(const wchar_t *str) const
{
    if (is_translations_valid) {
        auto it = translMap.find(str);
        if (it != translMap.end()) {
            const LocaleMap &lcmap = it->second;
            auto lc_it = lcmap.find(langName);
            if (lc_it == lcmap.end()) {
                wstring primaryLangAndScript = getPrimaryLang(langName, true);
                if ((lc_it = lcmap.find(primaryLangAndScript)) == lcmap.end()) {
                    wstring primaryLang = getPrimaryLang(langName);
                    if ((lc_it = lcmap.find(primaryLang)) == lcmap.end()) {
                        lc_it = lcmap.find(L"en");
                    }
                }
            }
            if (lc_it != lcmap.end())
                return lc_it->second.c_str();
        }
    }
    return str;
}
