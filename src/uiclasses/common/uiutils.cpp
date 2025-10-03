#include "uiutils.h"
#ifdef _WIN32
# include <string>
# include <sddl.h>
# define BIT123_LAYOUTRTL 0x08000000
# ifndef LOCALE_IREADINGLAYOUT
#  define LOCALE_IREADINGLAYOUT 0x70
# endif
# ifndef MDT_EFFECTIVE_DPI
#  define MDT_EFFECTIVE_DPI 0
# endif
#else
# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <cstring>
# include <gdk/gdk.h>
# include <pango/pango.h>
#endif


#ifdef _WIN32
static IStream* LoadResourceToStream(int id, LPCWSTR type) {
    IStream *pStream = nullptr;
    HMODULE hInst = GetModuleHandle(NULL);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(id), type)) {
        if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
            if (LPVOID pData = LockResource(hResData)) {
                DWORD dataSize = SizeofResource(hInst, hRes);
                if (dataSize > 0) {
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
            }
            FreeResource(hResData);
        }
    }
    return pStream;
}

static double ScreenDpiFromMonitor(HMONITOR hMon)
{
    if (HMODULE module = LoadLibrary(L"shcore")) {
        HRESULT(WINAPI *_GetDpiForMonitor)(HMONITOR, UINT, UINT*, UINT*) = NULL;
        *(FARPROC*)&_GetDpiForMonitor = GetProcAddress(module, "GetDpiForMonitor");
        UINT x = 0, y = 0;
        if (_GetDpiForMonitor && _GetDpiForMonitor(hMon, MDT_EFFECTIVE_DPI, &x, &y) == S_OK) {
            FreeLibrary(module);
            return (double)x/96;
        }
        FreeLibrary(module);
    }
    HDC hdc = GetDC(NULL);
    double dpi = (double)GetDeviceCaps(hdc, LOGPIXELSX)/96;
    ReleaseDC(NULL, hdc);
    return dpi;
}

UIUtils::WinVer UIUtils::winVersion() noexcept
{
    static WinVer winVer = WinVer::Undef;
    if (winVer != WinVer::Undef)
        return winVer;

    if (HMODULE module = GetModuleHandleA("ntdll")) {
        NTSTATUS(WINAPI *_RtlGetVersion)(LPOSVERSIONINFOEXW);
        *(FARPROC*)&_RtlGetVersion = GetProcAddress(module, "RtlGetVersion");
        OSVERSIONINFOEXW os = {0};
        os.dwOSVersionInfoSize = sizeof(os);
        if (_RtlGetVersion && _RtlGetVersion(&os) == 0) {
            const DWORD major = os.dwMajorVersion;
            const DWORD minor = os.dwMinorVersion;
            const DWORD build = os.dwBuildNumber;
            winVer =
                major == 5L && (minor == 1L || minor == 2L) ? WinVer::WinXP :
                major == 6L && minor == 0L ? WinVer::WinVista :
                major == 6L && minor == 1L ? WinVer::Win7 :
                major == 6L && minor == 2L ? WinVer::Win8 :
                major == 6L && minor == 3L ? WinVer::Win8_1 :
                major == 10L && minor == 0L && build < 22000 ? WinVer::Win10 :
                major == 10L && minor == 0L && build >= 22000 ? WinVer::Win11 :
                major == 10L && minor > 0L ? WinVer::WinFuture :
                major > 10L ? WinVer::WinFuture : WinVer::Undef;
        }
    }
    return winVer;
}

std::wstring UIUtils::currentUserSID()
{
    static std::wstring user_sid;
    if (user_sid.empty()) {
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            DWORD tokenLen = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenLen);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenLen)) {
                    if (GetTokenInformation(hToken, TokenUser, pTokenUser, tokenLen, &tokenLen)) {
                        LPWSTR sid = NULL;
                        if (ConvertSidToStringSid(pTokenUser->User.Sid, &sid)) {
                            user_sid = sid;
                            LocalFree(sid);
                        }
                    }
                    free(pTokenUser);
                }
            }
            CloseHandle(hToken);
        }
    }
    return user_sid;
}

DWORD UIUtils::regQueryDwordValue(HKEY rootKey, LPCWSTR subkey, LPCWSTR value)
{
    HKEY hKey;
    DWORD dwValue = 0;
    if (RegOpenKeyEx(rootKey, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, value, nullptr, &dwType, (LPBYTE)&dwValue, &dwSize);
        RegCloseKey(hKey);
    }
    return dwValue;
}

double UIUtils::screenDpiAtPoint(const POINT &pt)
{
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    return hMon ? ScreenDpiFromMonitor(hMon) : 1.0;
}

double UIUtils::screenDpiAtRect(const RECT &rc)
{
    HMONITOR hMon = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    return hMon ? ScreenDpiFromMonitor(hMon) : 1.0;
}

void UIUtils::loadImageResource(Gdiplus::Bitmap *&hBmp, int id, LPCWSTR type)
{
    if (IStream *pStream = LoadResourceToStream(id, type)) {
        hBmp = new Gdiplus::Bitmap(pStream);
        pStream->Release();
    }
}

void UIUtils::loadEmfResource(Gdiplus::Metafile* &hBmp, int id, LPCWSTR type)
{
    if (IStream *pStream = LoadResourceToStream(id, type)) {
        hBmp = new Gdiplus::Metafile(pStream);
        pStream->Release();
    }
}

void UIUtils::loadStringResource(tstring &str, int id)
{
    if (IStream *pStream = LoadResourceToStream(id, L"TEXT")) {
        STATSTG statstg;
        if (SUCCEEDED(pStream->Stat(&statstg, STATFLAG_NONAME)) && statstg.cbSize.QuadPart > 0) {
            ULONG size = static_cast<ULONG>(statstg.cbSize.QuadPart);
            std::string buf(size + 1, '\0');
            ULONG bytesRead = 0;
            if (SUCCEEDED(pStream->Read(&buf[0], size, &bytesRead)) && bytesRead == size) {
                str = UIUnicode::utf8ToWStr(buf);
            }
        }
        pStream->Release();
    } else {
        str.clear();
    }
}
#else
UIUtils::DesktopEnv UIUtils::desktopEnv()
{
    static DesktopEnv desktop_env = DesktopEnv::UNDEF;
    if (desktop_env == DesktopEnv::UNDEF) {
        if (const char *env = getenv("XDG_CURRENT_DESKTOP")) {
            if (strstr(env, "Unity")) {
                const char *session = getenv("DESKTOP_SESSION");
                desktop_env = (session && strstr(session, "gnome-fallback")) ? DesktopEnv::GNOME : DesktopEnv::UNITY;
            } else
            if (strstr(env, "GNOME"))
                desktop_env = DesktopEnv::GNOME;
            else
            if (strstr(env, "KDE"))
                desktop_env = DesktopEnv::KDE;
            else
            if (strstr(env, "XFCE"))
                desktop_env = DesktopEnv::XFCE;
            else
            if (strstr(env, "Cinnamon"))
                desktop_env = DesktopEnv::CINNAMON;
            else
                desktop_env = DesktopEnv::OTHER;
        }
    }
    return desktop_env;
}

UIUtils::WindowServer UIUtils::windowServer()
{
    static WindowServer win_server = WindowServer::UNDEF;
    if (win_server == WindowServer::UNDEF) {
        if (const char *session_type = getenv("XDG_SESSION_TYPE")) {
            if (strcmp(session_type, "wayland") == 0)
                win_server = WindowServer::WAYLAND;
            else
            if (strcmp(session_type, "x11") == 0)
                win_server = WindowServer::X11;
            else
                win_server = WindowServer::OTHER;
        }
    }
    return win_server;
}

void UIUtils::loadStringResource(tstring &str, GResource *res, const char *resourcePath)
{
    if (res && resourcePath) {
        if (GBytes *bytes = g_resource_lookup_data(res, resourcePath, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL)) {
            gsize dataSize = 0;
            const char *pData = (const char*)g_bytes_get_data(bytes, &dataSize);
            if (dataSize > 0) {
                str.assign(pData, dataSize);
            }
            g_bytes_unref(bytes);
        }
    }
}
#endif

// CRITICAL: Checks whether `addr` points to a block allocated on the process heap.
// Used before freeing memory to avoid accessing invalid or foreign memory regions.
// An incorrect implementation or misuse of this function
// will result in application crashes or memory corruption.
bool UIUtils::isAllocOnHeap(void *addr) {
#ifdef _WIN32
    if (HANDLE procHeap = GetProcessHeap()) {
        if (HeapLock(procHeap)) {
            bool res = false;
            PROCESS_HEAP_ENTRY entry = {0};
            while (HeapWalk(procHeap, &entry)) {
                if ((entry.wFlags & PROCESS_HEAP_REGION) && addr >= (void*)entry.Region.lpFirstBlock && addr <= (void*)entry.Region.lpLastBlock) {
                    res = true;
                    break;
                }
            }
            if (!HeapUnlock(procHeap))
                res = false;
            return res;
        }
    }
#else
    if (FILE *maps = fopen("/proc/self/maps", "r")) {
        char *line = NULL;
        size_t line_size = 0;
        uintptr_t start_address = 0, end_address = 0;
        int name_start = 0, name_end = 0;
        while (getline(&line, &line_size, maps) > 0) {
            if (sscanf(line, "%lx-%lx %*s %*lx %*u:%*u %*lu %n%*[^\n]%n", &start_address, &end_address, &name_start, &name_end) == 2) {
                if (name_end > name_start) {
                    line[name_end] = '\0';
                    if (strcmp(&line[name_start], "[heap]") == 0 && (uintptr_t)addr >= start_address && (uintptr_t)addr <= end_address) {
                        free(line);
                        fclose(maps);
                        return true;
                    }
                }
            }
        }
        free(line);
        fclose(maps);
    }
#endif
    return false;
}

#ifdef _WIN32
bool UILocalization::isRtlLanguage(unsigned long lcid)
{
    if (UIUtils::winVersion() >= UIUtils::WinVer::Win7) {
        DWORD layout = 0;
        if (GetLocaleInfo(lcid, LOCALE_IREADINGLAYOUT | LOCALE_RETURN_NUMBER, (LPWSTR)&layout, sizeof(layout)/sizeof(WCHAR)) > 0)
            return layout == 1;
    } else {
        LOCALESIGNATURE lsig;
        if (GetLocaleInfo(lcid, LOCALE_FONTSIGNATURE, (LPWSTR)&lsig, sizeof(lsig)/sizeof(WCHAR)) > 0)
            return (lsig.lsUsb[3] & BIT123_LAYOUTRTL) != 0;
    }
    return false;
}
#else
bool UILocalization::isRtlLanguage(const char *locale)
{
    PangoLanguage *lang = pango_language_from_string(locale);
    const char *sample = pango_language_get_sample_string(lang);
    if (sample && *sample) {
        gunichar uch = g_utf8_get_char(sample);
        PangoDirection dir = pango_unichar_direction(uch);
        return dir == PANGO_DIRECTION_RTL;
    }
    return false;
}
#endif

#ifdef _WIN32
std::wstring UIUnicode::utf8ToWStr(const std::string &str)
{
    if (!str.empty()) {
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), nullptr, 0);
        if (size > 0) {
            std::wstring result(size, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], size);
            return result;
        }
    }
    return {};
}
#endif

// NOTE:
//  On Windows: 'pos' is an index in wchar_t units (UTF-16 code units).
//               Surrogate pairs take 2 wchar_t.
//  On Linux:   'pos' is a byte offset in UTF-8.
//               Multi-byte sequences take 2–4 bytes.

// Returns the length of the character starting at position 'pos'.
// NOTE: See above for meaning of 'pos' on Windows vs Linux.
size_t UIUnicode::charLenAt(const tstring &str, size_t pos) noexcept
{
    const size_t n = str.size();
    if (pos >= n) return 0;
#ifdef _WIN32
    wchar_t ch = str[pos];
    return (IS_HIGH_SURROGATE(ch) && (pos + 1) < n && IS_LOW_SURROGATE(str[pos + 1])) ? 2 : 1;
#else
    const char *cur = str.c_str() + pos;
    return g_utf8_next_char(cur) - cur;
#endif
}

// Returns the length of the character immediately before position 'pos'.
// NOTE: See above for meaning of 'pos' on Windows vs Linux.
size_t UIUnicode::charLenBefore(const tstring &str, size_t pos) noexcept
{
    const size_t n = str.size();
    if (pos == 0) return 0;
    if (pos > n) pos = n;
#ifdef _WIN32
    wchar_t ch = str[pos - 1];
    return (IS_LOW_SURROGATE(ch) && pos >= 2 && IS_HIGH_SURROGATE(str[pos - 2])) ? 2 : 1;
#else
    const char *cur = str.c_str() + pos;
    return cur - g_utf8_prev_char(cur);
#endif
}

// Returns the position of the previous character relative to 'pos'.
// NOTE: See above for meaning of 'pos' on Windows vs Linux.
size_t UIUnicode::charPrevPos(const tstring &str, size_t pos) noexcept
{
    const size_t n = str.size();
    if (pos == 0) return 0;
    if (pos > n) pos = n;
    return pos - charLenBefore(str, pos);
}

// Returns the position of the next character relative to 'pos'.
// NOTE: See above for meaning of 'pos' on Windows vs Linux.
size_t UIUnicode::charNextPos(const tstring &str, size_t pos) noexcept
{
    const size_t n = str.size();
    if (pos >= n) return n;
    return pos + charLenAt(str, pos);
}
