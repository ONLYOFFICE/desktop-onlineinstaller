#ifndef UIUTILS_H
#define UIUTILS_H

#ifdef _WIN32
# include <Windows.h>
# include <gdiplus.h>
#else
# include <gio/gio.h>
#endif
#include "uidefines.h"


namespace UIUtils
{
#ifdef _WIN32
    enum WinVer : BYTE {
        Undef, WinXP, WinVista, Win7, Win8, Win8_1, Win10, Win11, WinFuture
    };
    WinVer DECL_VISUALUI winVersion() noexcept;
    std::wstring DECL_VISUALUI currentUserSID();
    DWORD DECL_VISUALUI regQueryDwordValue(HKEY rootKey, LPCWSTR subkey, LPCWSTR value);
    double DECL_VISUALUI screenDpiAtPoint(const POINT &pt);
    double DECL_VISUALUI screenDpiAtRect(const RECT &rc);
    void DECL_VISUALUI loadImageResource(Gdiplus::Bitmap* &hBmp, int id, LPCWSTR type);
    void DECL_VISUALUI loadEmfResource(Gdiplus::Metafile* &hBmp, int id, LPCWSTR type);
    void DECL_VISUALUI loadStringResource(tstring &str, int id);
#else
    enum class DesktopEnv : unsigned char {
        UNDEF, UNITY, GNOME, KDE, XFCE, CINNAMON, OTHER
    };
    enum class WindowServer : unsigned char {
        UNDEF, X11, WAYLAND, OTHER
    };
    DesktopEnv DECL_VISUALUI desktopEnv();
    WindowServer DECL_VISUALUI windowServer();
    void DECL_VISUALUI loadStringResource(tstring &str, GResource *res, const char *resourcePath);
#endif
    bool isAllocOnHeap(void *addr);
};

namespace UILocalization
{
#ifdef _WIN32
bool DECL_VISUALUI isRtlLanguage(unsigned long lcid);
#else
bool DECL_VISUALUI isRtlLanguage(const char *locale);
#endif
};

namespace UIUnicode
{
#ifdef _WIN32
std::wstring DECL_VISUALUI utf8ToWStr(const std::string &str);
#endif
size_t DECL_VISUALUI charLenAt(const tstring &str, size_t pos) noexcept;
size_t DECL_VISUALUI charLenBefore(const tstring &str, size_t pos) noexcept;
size_t DECL_VISUALUI charPrevPos(const tstring &str, size_t pos) noexcept;
size_t DECL_VISUALUI charNextPos(const tstring &str, size_t pos) noexcept;
};

#endif // UIUTILS_H
