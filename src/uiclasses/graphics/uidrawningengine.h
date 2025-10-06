#ifndef UIDRAWNINGENGINE_H
#define UIDRAWNINGENGINE_H

#include "uidefines.h"
#include "uiplatformtypes.h"
#ifdef _WIN32
# include <gdiplus.h>
  typedef HICON PlatformIcon;
#else
# include <uicommon.h>
  typedef GdkPixbuf* PlatformIcon;
#endif


struct _RsvgHandle;
class UIDrawningSurface;
class DECL_VISUALUI UIDrawingEngine
{
public:
    UIDrawingEngine(const UIDrawingEngine&) = delete;
    UIDrawingEngine& operator=(const UIDrawingEngine&) = delete;
    static UIDrawingEngine *instance();

    enum Corner : unsigned char {
        CornerLTop = 1,
        CornerRTop = 2,
        CornerLBottom = 4,
        CornerRBottom = 8,
        CornerAll  = CornerLTop | CornerRTop | CornerLBottom | CornerRBottom
    };

    UIDrawningSurface *surface();
    void DrawFlatRect(bool clearBkg = true) const noexcept;
    void DrawRoundedRect(unsigned char corner, int offset = 0, bool clearBkg = true) const noexcept;
    void DrawStockCloseIcon() const;
    void DrawStockMinimizeIcon() const;
    void DrawStockMaximizeIcon() const;
    void DrawStockRestoreIcon() const;
    void DrawCheckBox(const tstring &text, PlatformFont hFont, RECT &check_rc, bool checked = false) noexcept;
    void DrawRadioButton(const tstring &text, PlatformFont hFont, RECT &check_rc, bool checked = false) noexcept;
    void DrawToggleButton(const tstring &text, PlatformFont hFont, RECT &check_rc, bool checked = false) noexcept;
    void DrawProgressBar(int progress, int pulse_pos) const noexcept;
    void DrawString(const RECT &rc, const tstring &text, PlatformFont hFont, bool multiline = false, RECT *bounds = nullptr);
    void DrawIcon(PlatformIcon hIcon) const;
    void DrawShadow() const noexcept;
#ifdef _WIN32
    void Begin(UIDrawningSurface*, HWND, RECT *rc, double dpi = 1.0);
    void DrawTopBorder(int, COLORREF) const;
    void DrawEmfIcon(Gdiplus::Metafile *hEmf) noexcept;
    void DrawImage(Gdiplus::Bitmap *hBmp) const noexcept;
    void End() noexcept;

    void LayeredBegin(UIDrawningSurface*, HWND, RECT *rc, double dpi = 1.0);
    void LayeredChildBegin(UIDrawningSurface*, HWND, RECT *rc, double dpi = 1.0);
    void LayeredUpdate(BYTE alpha) const;
    void LayeredEnd() noexcept;
    void LayeredChildEnd() noexcept;
#else
    void Begin(UIDrawningSurface*, cairo_t*, Rect *rc) noexcept;
    void DrawSvgIcon(_RsvgHandle *hSvg) const noexcept;
    void End() noexcept;
#endif

private:
    UIDrawingEngine();
    UIDrawingEngine(bool rtl);
    ~UIDrawingEngine();

    UIDrawningSurface *m_ds, *m_tmp_ds;
    RECT *m_rc, *m_tmp_rc;
#ifdef _WIN32
    PAINTSTRUCT m_ps;
    HWND m_hwnd, m_tmp_hwnd;
    HDC  m_hdc, m_tmp_hdc;
    HDC  m_memDC;
    HBITMAP m_memBmp;
    HBITMAP m_oldBmp;
    Gdiplus::Bitmap   *m_bmp;
    Gdiplus::Graphics *m_graphics;
    Gdiplus::Matrix   *m_origMatrix;
    bool m_root_is_layered;
#else
    cairo_t *m_cr;
#endif
    double m_dpi, m_tmp_dpi;
    bool m_rtl;
};

#endif // UIDRAWNINGENGINE_H
