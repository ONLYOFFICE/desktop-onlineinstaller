#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <Windows.h>
#include <gdiplus.h>


namespace Utils
{
    enum WinVer : BYTE {
        Undef, WinXP, WinVista, Win7, Win8, Win8_1, Win10, Win11
    };
    WinVer getWinVersion();
    // bool isColorDark(COLORREF color);
    COLORREF getColorizationColor(bool isActive = true, COLORREF topColor = 0x00ffffff);
    void loadImageResource(Gdiplus::Bitmap* &hBmp, int id, LPCWSTR type);
    void loadEmfResource(Gdiplus::Metafile* &hBmp, int id, LPCWSTR type);
};

#endif // BASEUTILS_H
