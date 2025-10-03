#ifndef DRAWNINGENGINE_H
#define DRAWNINGENGINE_H

#include <Windows.h>
#include <gdiplus.h>
#include <string>


class UIDrawningSurface;
class UIDrawingEngine
{
public:
    UIDrawingEngine(const UIDrawingEngine&) = delete;
    UIDrawingEngine& operator=(const UIDrawingEngine&) = delete;
    static UIDrawingEngine *instance();


    UIDrawningSurface *surface();
    void Begin(UIDrawningSurface*, HWND, RECT *rc);
    void FillBackground() const;
    // void DrawRoundedRect();
    void DrawBorder() const;
    void DrawTopBorder(int, COLORREF) const;
    void DrawIcon(HICON hIcon) const;
    void DrawEmfIcon(Gdiplus::Metafile *hEmf) const;
    void DrawImage(Gdiplus::Bitmap *hBmp) const;
    void DrawStockCloseIcon();
    void DrawStockMinimizeIcon();
    void DrawStockMaximizeIcon();
    void DrawStockRestoreIcon();
    void DrawCheckBox(const std::wstring &text, HFONT hFont, bool checked = false);
    void DrawRadioButton(const std::wstring &text, HFONT hFont, bool checked = false);
    void DrawProgressBar(int progress, int pulse_pos);
    void DrawText(const RECT &rc, const std::wstring &text, HFONT hFont, bool multiline = false) const;
    void End();

    // void LayeredBegin(UIDrawningSurface*, HWND, RECT *rc);
    // void LayeredDrawRoundedRect() const;
    void LayeredDrawText(RECT &rc, const std::wstring &text, HFONT hFont, bool rtl = false) const;
    // void LayeredDrawShadow(int shadowWidth, int rad);
    // void LayeredUpdate(BYTE alpha);
    // void LayeredEnd();

private:
    UIDrawingEngine();
    ~UIDrawingEngine();

    UIDrawningSurface *m_ds;
    RECT *m_rc;
    PAINTSTRUCT *m_ps;
    HWND m_hwnd;
    HDC  m_hdc;
    HDC  m_memDC;
    HBITMAP m_memBmp;
    HBITMAP m_oldBmp;
    Gdiplus::Graphics *m_graphics;
};

#endif // DRAWNINGENGINE_H
