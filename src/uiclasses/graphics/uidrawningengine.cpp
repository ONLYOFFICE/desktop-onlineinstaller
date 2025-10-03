#include "uidrawningengine.h"
#include "uidrawingsurface.h"
#include "uiapplication.h"
#include "uipalette.h"
#include "uimetrics.h"
#ifdef _WIN32
#else
# include <glib.h>
# include <pango/pangocairo.h>
# include <librsvg-2.0/librsvg/rsvg.h>
# include <cmath>
# define GetRValue(rgb) ((double)((BYTE)(rgb))/255)
# define GetGValue(rgb) ((double)((BYTE)(((WORD)(rgb)) >> 8))/255)
# define GetBValue(rgb) ((double)((BYTE)((rgb) >> 16))/255)
#endif


#ifdef _WIN32
static Gdiplus::Color ColorFromColorRef(COLORREF rgb)
{
    Gdiplus::Color color;
    color.SetFromCOLORREF(rgb);
    return color;
}

static void RoundedPath(Gdiplus::GraphicsPath &ph, unsigned char corner, float x, float y, float width, float height, float rad)
{
    float d = rad * 2;
    if (d > width) d = width;
    if (d > height) d = height;

    // ph.StartFigure();
    if (rad != 0 && corner & UIDrawingEngine::CornerLTop)
        ph.AddArc(x, y, d, d, 180, 90);
    else ph.AddLine(x, y, x + rad, y);

    if (rad != 0 && corner & UIDrawingEngine::CornerRTop)
        ph.AddArc(x + width - d, y, d, d, 270, 90);
    else ph.AddLine(x + width - rad, y, x + width, y);

    if (rad != 0 && corner & UIDrawingEngine::CornerRBottom)
        ph.AddArc(x + width - d, y + height - d, d, d, 0, 90);
    else ph.AddLine(x + width, y + height - rad, x + width, y + height);

    if (rad != 0 && corner & UIDrawingEngine::CornerLBottom)
        ph.AddArc(x, y + height - d, d, d, 90, 90);
    else ph.AddLine(x + rad, y + height, x, y + height);

    ph.CloseFigure();
}

static BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
    SendMessage(hwndChild, WM_PAINT_LAYERED_CHILD, 0, lParam);
    return TRUE;
}
#else
static void RoundedPath(cairo_t *cr, unsigned char corner, double x, double y, double w, double h, double rad)
{
    if (rad > w/2) rad = w/2;
    if (rad > h/2) rad = h/2;

    if (corner & UIDrawingEngine::CornerLTop)
        cairo_arc(cr, x + rad, y + rad, rad, G_PI, -G_PI_2);
    else cairo_move_to(cr, x, y);

    if (corner & UIDrawingEngine::CornerRTop)
        cairo_arc(cr, x + w - rad, y + rad, rad, -G_PI_2, 0);
    else cairo_line_to(cr, x + w, y);

    if (corner & UIDrawingEngine::CornerRBottom)
        cairo_arc(cr, x + w - rad, y + h - rad, rad, 0, G_PI_2);
    else cairo_line_to(cr, x + w, y + h);

    if (corner & UIDrawingEngine::CornerLBottom)
        cairo_arc(cr, x + rad, y + h - rad, rad, G_PI_2, G_PI);
    else cairo_line_to(cr, x, y + h);

    cairo_close_path(cr);
}
#endif

static inline void GetIconMargins(Margins &mrg, const Metrics *metrics, double dpi, bool rtl) noexcept
{
#ifdef _WIN32
    mrg.left = metrics->value(Metrics::IconMarginLeft) * dpi;
    mrg.right = metrics->value(Metrics::IconMarginRight) * dpi;
#else
    mrg.left = (rtl ? metrics->value(Metrics::IconMarginRight) : metrics->value(Metrics::IconMarginLeft)) * dpi;
    mrg.right = (rtl ? metrics->value(Metrics::IconMarginLeft) : metrics->value(Metrics::IconMarginRight)) * dpi;
#endif
    mrg.top = metrics->value(Metrics::IconMarginTop) * dpi;
    mrg.bottom = metrics->value(Metrics::IconMarginBottom) * dpi;
}

static inline void GetIconPlacement(Rect &rc, const Rect &src_rc, const Margins &mrg, const Metrics *metrics, double dpi, bool rtl) noexcept
{
    rc.x = 0;
    rc.y = 0;
    rc.width = metrics->value(Metrics::IconWidth) * dpi;
    rc.height = metrics->value(Metrics::IconHeight) * dpi;
    int algn = metrics->value(Metrics::IconAlignment);
#ifdef _WIN32
    if (algn & Metrics::AlignHLeft)
#else
    if (algn & (rtl ? Metrics::AlignHRight : Metrics::AlignHLeft))
#endif
        rc.x = mrg.left + src_rc.x;
    if (algn & Metrics::AlignHCenter)
        rc.x = mrg.left + src_rc.x + (src_rc.width - rc.width) / 2;
#ifdef _WIN32
    if (algn & Metrics::AlignHRight)
#else
    if (algn & (rtl ? Metrics::AlignHLeft : Metrics::AlignHRight))
#endif
        rc.x = src_rc.x + src_rc.width - mrg.right - rc.width;
    if (algn & Metrics::AlignVTop)
        rc.y = mrg.top + src_rc.y;
    if (algn & Metrics::AlignVCenter)
        rc.y = mrg.top + src_rc.y + (src_rc.height - rc.height) / 2;
    if (algn & Metrics::AlignVBottom)
        rc.y = src_rc.y + src_rc.height - mrg.bottom - rc.height;
}

UIDrawingEngine::UIDrawingEngine()
{}

UIDrawingEngine::UIDrawingEngine(bool rtl) :
    m_ds(nullptr)
#ifdef _WIN32
  , m_rc(nullptr),
    m_hwnd(nullptr),
    m_hdc(nullptr),
    m_memDC(nullptr),
    m_memBmp(nullptr),
    m_oldBmp(nullptr),
    m_bmp(nullptr),
    m_graphics(nullptr),
    m_origMatrix(nullptr),
    m_root_is_layered(false)
#else
  , m_rc(nullptr),
    m_cr(nullptr)
#endif
  , m_dpi(1.0),
    m_rtl(rtl)
{}

UIDrawingEngine* UIDrawingEngine::instance()
{
    static UIDrawingEngine inst(UIApplication::instance()->layoutDirection() == UIApplication::RightToLeft);
    return &inst;
}

UIDrawingEngine::~UIDrawingEngine()
{

}

UIDrawningSurface *UIDrawingEngine::surface()
{
    return m_ds;
}

void UIDrawingEngine::DrawFlatRect(bool clearBkg) const noexcept
{
    const Metrics *metrics = m_ds->metrics();
    const Palette *palette = m_ds->palette();
    float brd_w = roundf(metrics->value(Metrics::BorderWidth) * m_dpi);
#ifdef _WIN32
    if (clearBkg) {
        m_graphics->Clear(ColorFromColorRef(palette->color(Palette::Background)));
    } else {
        Gdiplus::SolidBrush brush(ColorFromColorRef(palette->color(Palette::Background)));
        m_graphics->FillRectangle(&brush, (int)m_rc->left, (int)m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top);
    }

    if (brd_w != 0) {
        float d = brd_w / 2.0f;
        Gdiplus::Pen pen(ColorFromColorRef(palette->color(Palette::Border)), brd_w);
        m_graphics->DrawRectangle(&pen, m_rc->left + d - 0.5f, m_rc->top + d, m_rc->right - m_rc->left - brd_w, m_rc->bottom - m_rc->top - brd_w);
    }
#else
    COLORREF rgb = palette->color(Palette::Background);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_paint(m_cr);
    if (brd_w != 0) {
        COLORREF rgb = palette->color(Palette::Border);
        cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
        cairo_set_line_width(m_cr, brd_w);
        cairo_rectangle(m_cr, 0, 0, m_rc->width, m_rc->height);
        cairo_stroke(m_cr);
    }
#endif
}

void UIDrawingEngine::DrawRoundedRect(unsigned char corner, int offset, bool clearBkg) const noexcept
{
    const Metrics *metrics = m_ds->metrics();
    const Palette *palette = m_ds->palette();
    float brd_w = roundf(metrics->value(Metrics::BorderWidth) * m_dpi);
    float rad = metrics->value(Metrics::BorderRadius) * m_dpi;
#ifdef _WIN32
    float x = m_rc->left + offset;
    float y = m_rc->top + offset;
    float width = m_rc->right - m_rc->left - offset * 2;
    float height = m_rc->bottom - m_rc->top - offset * 2;

    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    if (clearBkg)
        m_graphics->Clear(ColorFromColorRef(palette->color(Palette::Base)));

    Gdiplus::GraphicsPath ph;
    RoundedPath(ph, corner, x, y, width, height, rad);
    Gdiplus::SolidBrush brush(ColorFromColorRef(palette->color(Palette::Background)));
    m_graphics->FillPath(&brush, &ph);
    if (brd_w != 0) {
        float d = brd_w / 2.0;
        ph.Reset();
        RoundedPath(ph, corner, x + d, y + d, width - 2*d, height - 2*d, rad - d);
        Gdiplus::Pen pen(ColorFromColorRef(palette->color(Palette::Border)), brd_w);
        m_graphics->DrawPath(&pen, &ph);
    }
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
#else
    int x = m_rc->x + offset;
    int y = m_rc->y + offset;
    int width = m_rc->width - offset * 2;
    int height = m_rc->height - offset * 2;

    COLORREF rgb = palette->color(Palette::Background);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    RoundedPath(m_cr, corner, x, y, width, height, metrics->value(Metrics::BorderRadius));
    cairo_fill(m_cr);
    if (brd_w != 0) {
        rgb = palette->color(Palette::Border);
        cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
        cairo_set_line_width(m_cr, brd_w);
        double d = brd_w / 2.0;
        RoundedPath(m_cr, corner, x + d, y + d, width - 2*d, height - 2*d, rad - d);
        cairo_stroke(m_cr);
    }
#endif
}

void UIDrawingEngine::DrawStockCloseIcon() const
{
    const Metrics *metrics = m_ds->metrics();
    int half_height = metrics->value(Metrics::IconHeight) * m_dpi / 2;
#ifdef _WIN32
    int x = m_rc->left + (m_rc->right - m_rc->left) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top) / 2;

    Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Primitive)), metrics->value(Metrics::PrimitiveWidth) * m_dpi);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeNone);
    m_graphics->DrawLine(&pen, x, y, x + half_height, y + half_height);
    m_graphics->DrawLine(&pen, x, y, x + half_height, y - half_height);
    m_graphics->DrawLine(&pen, x, y, x - half_height, y + half_height);
    m_graphics->DrawLine(&pen, x, y, x - half_height, y - half_height);
#else
    int x = m_rc->x + m_rc->width / 2;
    int y = m_rc->y + m_rc->height / 2;

    COLORREF rgb = 0;
#ifdef ALTERNATE_LINUX_STOCK_ICON
    rgb = m_ds->palette()->color(Palette::Base);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_arc(m_cr, x, y, 2 * half_height, 0, 2 * G_PI);
    cairo_fill(m_cr);
#endif
    rgb = m_ds->palette()->color(Palette::Primitive);
    cairo_set_line_width(m_cr, metrics->value(Metrics::PrimitiveWidth));
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_NONE);
    cairo_move_to(m_cr, x, y);
    cairo_line_to(m_cr, x + half_height + 0.1, y + half_height + 0.1);
    cairo_move_to(m_cr, x, y);
    cairo_line_to(m_cr, x + half_height + 0.1, y - half_height - 0.1);
    cairo_move_to(m_cr, x, y);
    cairo_line_to(m_cr, x - half_height, y + half_height);
    cairo_move_to(m_cr, x, y);
    cairo_line_to(m_cr, x - half_height, y - half_height);
    cairo_stroke(m_cr);
    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_DEFAULT);
#endif
}

void UIDrawingEngine::DrawStockMinimizeIcon() const
{
    const Metrics *metrics = m_ds->metrics();
#ifdef _WIN32
    int width = metrics->value(Metrics::IconWidth) * m_dpi;
    int height = metrics->value(Metrics::IconHeight) * m_dpi;
    int x = m_rc->left + (m_rc->right - m_rc->left - width) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - height) / 2;
    int yMid = y + height / 2;
    Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Primitive)), metrics->value(Metrics::PrimitiveWidth) * m_dpi);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeNone);
    m_graphics->DrawLine(&pen, x, yMid, x + width, yMid);
#else
    int x = m_rc->x + (m_rc->width - metrics->value(Metrics::IconWidth)) / 2;
    int y = m_rc->y + (m_rc->height - metrics->value(Metrics::IconHeight)) / 2;
    int half_height = metrics->value(Metrics::IconHeight) / 2;

    COLORREF rgb = 0;
#ifdef ALTERNATE_LINUX_STOCK_ICON
    rgb = m_ds->palette()->color(Palette::Base);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_arc(m_cr, m_rc->x + (m_rc->width) / 2, m_rc->y + (m_rc->height) / 2, 2 * half_height, 0, 2 * G_PI);
    cairo_fill(m_cr);
#endif
    rgb = m_ds->palette()->color(Palette::Primitive);
    cairo_set_line_width(m_cr, metrics->value(Metrics::PrimitiveWidth));
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_NONE);
    cairo_move_to(m_cr, x, y + half_height);
    cairo_line_to(m_cr, x + metrics->value(Metrics::IconWidth), y + half_height);
    cairo_stroke(m_cr);
    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_DEFAULT);
#endif
}

void UIDrawingEngine::DrawStockMaximizeIcon() const
{
    const Metrics *metrics = m_ds->metrics();
#ifdef _WIN32
    int height = metrics->value(Metrics::IconHeight) * m_dpi;
    int x = m_rc->left + (m_rc->right - m_rc->left - height) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - height) / 2;
    int quarterh = height / 4;
    int resth = height - quarterh;

    Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Primitive)), metrics->value(Metrics::PrimitiveWidth) * m_dpi);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeNone);
    m_graphics->DrawLine(&pen, x, y + quarterh, x + resth, y + quarterh);
    m_graphics->DrawLine(&pen, x + resth, y + quarterh, x + resth, y + height);
    m_graphics->DrawLine(&pen, x + resth, y + height, x, y + height);
    m_graphics->DrawLine(&pen, x, y + height, x, y + quarterh);

    m_graphics->DrawLine(&pen, x + quarterh, y + quarterh, x + quarterh, y);
    m_graphics->DrawLine(&pen, x + quarterh, y, x + height, y);
    m_graphics->DrawLine(&pen, x + height, y, x + height, y + resth);
    m_graphics->DrawLine(&pen, x + height, y + resth, x + resth, y + resth);
#else
    int width = metrics->value(Metrics::IconWidth);
    int height = metrics->value(Metrics::IconHeight);
    int x = m_rc->x + (m_rc->width - width) / 2;
    int y = m_rc->y + (m_rc->height - height) / 2;
    int quarterw = width / 4;
    int restw = width - quarterw;
    int quarterh = height / 4;
    int resth = height - quarterh;

    COLORREF rgb = 0;
#ifdef ALTERNATE_LINUX_STOCK_ICON
    int half_height = height / 2;
    rgb = m_ds->palette()->color(Palette::Base);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_arc(m_cr, m_rc->x + (m_rc->width) / 2, m_rc->y + (m_rc->height) / 2, 2 * half_height, 0, 2 * G_PI);
    cairo_fill(m_cr);
#endif
    rgb = m_ds->palette()->color(Palette::Primitive);
    cairo_set_line_width(m_cr, metrics->value(Metrics::PrimitiveWidth));
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_NONE);
    cairo_move_to(m_cr, x, y + quarterh);
    cairo_line_to(m_cr, x + restw, y + quarterh);
    cairo_line_to(m_cr, x + restw, y + height);
    cairo_line_to(m_cr, x, y + height);
    cairo_line_to(m_cr, x, y + quarterh);
    cairo_move_to(m_cr, x + quarterw, y + quarterh);
    cairo_line_to(m_cr, x + quarterw, y);
    cairo_line_to(m_cr, x + width, y);
    cairo_line_to(m_cr, x + width, y + resth);
    cairo_line_to(m_cr, x + restw, y + resth);
    cairo_stroke(m_cr);
    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_DEFAULT);
#endif
}

void UIDrawingEngine::DrawStockRestoreIcon() const
{
    const Metrics *metrics = m_ds->metrics();
#ifdef _WIN32
    int height = metrics->value(Metrics::IconHeight) * m_dpi;
    int x = m_rc->left + (m_rc->right - m_rc->left - height) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - height) / 2;

    Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Primitive)), metrics->value(Metrics::PrimitiveWidth) * m_dpi);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeNone);
    m_graphics->DrawLine(&pen, x, y, x + height, y);
    m_graphics->DrawLine(&pen, x + height, y, x + height, y + height);
    m_graphics->DrawLine(&pen, x + height, y + height, x, y + height);
    m_graphics->DrawLine(&pen, x, y + height, x, y);
#else
    int x = m_rc->x + (m_rc->width - metrics->value(Metrics::IconWidth)) / 2;
    int y = m_rc->y + (m_rc->height - metrics->value(Metrics::IconHeight)) / 2;

    COLORREF rgb = 0;
#ifdef ALTERNATE_LINUX_STOCK_ICON
    int half_height = metrics->value(Metrics::IconHeight) / 2;
    rgb = m_ds->palette()->color(Palette::Base);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_arc(m_cr, m_rc->x + (m_rc->width) / 2, m_rc->y + (m_rc->height) / 2, 2 * half_height, 0, 2 * G_PI);
    cairo_fill(m_cr);
#endif
    rgb = m_ds->palette()->color(Palette::Primitive);
    cairo_set_line_width(m_cr, metrics->value(Metrics::PrimitiveWidth));
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_NONE);
    cairo_move_to(m_cr, x, y);
    cairo_line_to(m_cr, x + metrics->value(Metrics::IconWidth), y);
    cairo_line_to(m_cr, x + metrics->value(Metrics::IconWidth), y + metrics->value(Metrics::IconHeight));
    cairo_line_to(m_cr, x, y + metrics->value(Metrics::IconHeight));
    cairo_close_path(m_cr);
    cairo_stroke(m_cr);
    cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_DEFAULT);
#endif
}

void UIDrawingEngine::DrawCheckBox(const tstring &text, PlatformFont hFont, RECT &check_rc, bool checked) noexcept
{
    const Metrics *metrics = m_ds->metrics();
    const Palette *palette = m_ds->palette();
    int icon_width = metrics->value(Metrics::IconWidth) * m_dpi;
    int icon_height = metrics->value(Metrics::IconHeight) * m_dpi;
#ifdef _WIN32
    int x = m_rc->left + 1;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - icon_height) / 2;

    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    Gdiplus::Pen pen(ColorFromColorRef(palette->color(Palette::Primitive)), metrics->value(Metrics::PrimitiveWidth) * m_dpi);
    Gdiplus::Rect rc(x, y, icon_width - 1, icon_height - 1);

    check_rc.left = x;
    check_rc.top = y;
    check_rc.right = x + rc.Width;
    check_rc.bottom = y + rc.Height;

    Gdiplus::GraphicsPath ph;
    RoundedPath(ph, CornerAll, rc.X, rc.Y, rc.Width, rc.Height, metrics->value(Metrics::PrimitiveRadius) * m_dpi);
    m_graphics->DrawPath(&pen, &ph);
    if (checked) {
        pen.SetWidth(metrics->value(Metrics::AlternatePrimitiveWidth) * m_dpi);
        pen.SetColor(ColorFromColorRef(palette->color(Palette::AlternatePrimitive)));
        Gdiplus::PointF pts[3] = {
            Gdiplus::PointF(float(x + (m_rtl ? icon_width - 3 * m_dpi : 2 * m_dpi)), float(y + icon_height/2 - 1 * m_dpi)),
            Gdiplus::PointF(float(x + icon_width/2 + (m_rtl ? 1 * m_dpi : - 2 * m_dpi)), float(y + icon_height - 5 * m_dpi)),
            Gdiplus::PointF(float(x + (m_rtl ? 2 * m_dpi : icon_width - 3 * m_dpi)), float(y + 4 * m_dpi))
        };
        m_graphics->DrawLines(&pen, pts, 3);
    }

    if (!text.empty()) {
        RECT rc;
        int offset = (m_rtl) ? icon_width : 0;
        SetRect(&rc, m_rc->left + icon_width - offset, m_rc->top, m_rc->right - offset, m_rc->bottom);
        DrawString(rc, text, hFont);
    }
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
#else
    double x = m_rtl ? m_rc->width - icon_height - 1.5 : m_rc->x + 1.5;
    double y = m_rc->y + (m_rc->height - icon_height) / 2 + 0.5;

    COLORREF rgb = palette->color(Palette::Primitive);
    cairo_set_line_width(m_cr, metrics->value(Metrics::PrimitiveWidth));
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    check_rc = Rect(x, y, icon_width - 1, icon_height - 1);
    RoundedPath(m_cr, CornerAll, x, y, check_rc.width, check_rc.height, metrics->value(Metrics::PrimitiveRadius));
    cairo_stroke(m_cr);
    if (checked) {
        rgb = palette->color(Palette::AlternatePrimitive);
        cairo_set_line_width(m_cr, metrics->value(Metrics::AlternatePrimitiveWidth));
        cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

        cairo_move_to(m_cr, x + 2, y + icon_height/2 - 1);
        cairo_line_to(m_cr, x + icon_width/2 - 2, y + icon_height - 5);
        cairo_line_to(m_cr, x + icon_width - 3, y + 4);
        cairo_stroke(m_cr);
    }
    if (!text.empty()) {
        int offset = (m_rtl) ? icon_width : 0;
        Rect rc(m_rc->x + icon_width - offset, m_rc->y, m_rc->width - offset, m_rc->height);
        DrawString(rc, text, hFont);
    }
#endif
}

void UIDrawingEngine::DrawRadioButton(const tstring &text, PlatformFont hFont, RECT &check_rc, bool checked) noexcept
{
    const Metrics *metrics = m_ds->metrics();
    const Palette *palette = m_ds->palette();
    int icon_width = metrics->value(Metrics::IconWidth) * m_dpi;
    int icon_height = metrics->value(Metrics::IconHeight) * m_dpi;
#ifdef _WIN32
    int x = m_rc->left + 1;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - icon_height) / 2;

    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    check_rc.left = x;
    check_rc.top = y;
    check_rc.right = x + icon_width - 1;
    check_rc.bottom = y + icon_height - 1;

    Gdiplus::Pen pen(ColorFromColorRef(palette->color(Palette::Primitive)), metrics->value(Metrics::PrimitiveWidth) * m_dpi);
    m_graphics->DrawEllipse(&pen, x, y, icon_height - 1, icon_height - 1);
    if (checked) {
        Gdiplus::SolidBrush chunkBrush(ColorFromColorRef(palette->color(Palette::AlternatePrimitive)));
        m_graphics->FillEllipse(&chunkBrush, float(x) + float(2.7f * m_dpi), float(y) + float(2.7f * m_dpi), float(icon_height) - 5.4f * m_dpi - 1.0f, float(icon_height) - 5.4f * m_dpi - 1.0f);
    }

    if (!text.empty()) {
        RECT rc;
        int offset = (m_rtl) ? icon_width : 0;
        SetRect(&rc, m_rc->left + icon_width - offset, m_rc->top, m_rc->right - offset, m_rc->bottom);
        DrawString(rc, text, hFont);
    }
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
#else
    double x = m_rtl ? m_rc->width - icon_height - 1.5 : m_rc->x + 1.5;
    double y = m_rc->y + (m_rc->height - icon_height) / 2 + 0.5;

    COLORREF rgb = palette->color(Palette::Primitive);
    cairo_set_line_width(m_cr, metrics->value(Metrics::PrimitiveWidth));
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    check_rc = Rect(x, y, icon_width, icon_height);
    cairo_arc(m_cr, double(x) + double(icon_height)/2, double(y) + double(icon_height)/2, double(icon_height)/2, 0, 2 * G_PI);
    cairo_stroke(m_cr);
    if (checked) {
        rgb = palette->color(Palette::AlternatePrimitive);
        cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
        cairo_arc(m_cr, double(x) + double(icon_height)/2, double(y) + double(icon_height)/2, double(icon_height)/2 - 2.7, 0, 2 * G_PI);
        cairo_fill(m_cr);
    }
    if (!text.empty()) {
        int offset = (m_rtl) ? icon_width : 0;
        Rect rc(m_rc->x + icon_width - offset, m_rc->y, m_rc->width - offset, m_rc->height);
        DrawString(rc, text, hFont);
    }
#endif
}

void UIDrawingEngine::DrawToggleButton(const tstring &text, PlatformFont hFont, RECT &check_rc, bool checked) noexcept
{
    const Metrics *metrics = m_ds->metrics();
    const Palette *palette = m_ds->palette();
    int icon_width = metrics->value(Metrics::IconWidth) * m_dpi;
    int icon_height = metrics->value(Metrics::IconHeight) * m_dpi;
#ifdef _WIN32
    int x = m_rc->left + 1;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - icon_height) / 2;

    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->Clear(ColorFromColorRef(palette->color(Palette::Background)));

    Gdiplus::SolidBrush tglgBrush(ColorFromColorRef(palette->color(checked ? Palette::AlternateBase : Palette::Base)));
    Gdiplus::Rect rc(x, y, icon_width - 1, icon_height - 1);

    check_rc.left = x;
    check_rc.top = y;
    check_rc.right = x + icon_width - 1;
    check_rc.bottom = y + icon_height - 1;

    Gdiplus::GraphicsPath ph;
    RoundedPath(ph, CornerAll, rc.X, rc.Y, rc.Width, rc.Height, metrics->value(Metrics::BorderRadius) * m_dpi);
    m_graphics->FillPath(&tglgBrush, &ph);

    float _x = checked ? float(icon_width - icon_height) : 0.0f;
    float diam = float(icon_height) - 2.8f * m_dpi;
    Gdiplus::SolidBrush chunkBrush(ColorFromColorRef(palette->color(Palette::AlternatePrimitive)));
    m_graphics->FillEllipse(&chunkBrush, _x + float(x) + float(1.4f * m_dpi), float(y) + float(1.4f * m_dpi), diam - 1.0f, diam - 1.0f);

    if (!text.empty()) {
        RECT rc;
        int offset = (m_rtl) ? icon_width : 0;
        SetRect(&rc, m_rc->left + icon_width - offset, m_rc->top, m_rc->right - offset, m_rc->bottom);
        DrawString(rc, text, hFont);
    }
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
#else
    int x = m_rtl ? m_rc->width - icon_width - 1 : m_rc->x + 1;
    int y = m_rc->y + (m_rc->height - icon_height) / 2;

    COLORREF rgb = palette->color(checked ? Palette::AlternateBase : Palette::Base);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

    check_rc = Rect(x, y, icon_width, icon_height);

    RoundedPath(m_cr, CornerAll, check_rc.x, check_rc.y, check_rc.width, check_rc.height, metrics->value(Metrics::BorderRadius) * m_dpi);
    cairo_fill(m_cr);

    float _x = checked ? float(icon_width - icon_height) : 0.0f;
    float rad = (float(icon_height) - 2.8f * m_dpi) / 2;
    rgb = palette->color(Palette::AlternatePrimitive);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_arc(m_cr, _x + float(x) + float(1.4f * m_dpi) + rad, float(y) + float(1.4f * m_dpi) + rad, rad, 0, 2 * G_PI);
    cairo_fill(m_cr);
    if (!text.empty()) {
        int offset = (m_rtl) ? icon_width : 0;
        Rect rc(m_rc->x + icon_width - offset, m_rc->y, m_rc->width - offset, m_rc->height);
        DrawString(rc, text, hFont);
    }
#endif
}

void UIDrawingEngine::DrawProgressBar(int progress, int pulse_pos) const noexcept
{
    const Metrics *metrics = m_ds->metrics();
    const Palette *palette = m_ds->palette();
    Margins mrg;
    GetIconMargins(mrg, metrics, m_dpi, m_rtl);
    double brd_w = round(metrics->value(Metrics::BorderWidth) * m_dpi);
#ifdef _WIN32
    int x = m_rc->left + brd_w + mrg.left;
    int y = m_rc->top + brd_w + mrg.top;
    int width = m_rc->right - m_rc->left - brd_w * 2 - mrg.right - mrg.left - 1;
    int height = m_rc->bottom - m_rc->top - brd_w * 2 - mrg.bottom - mrg.top - 1;
    int rad = metrics->value(Metrics::BorderRadius) * m_dpi;

    if (rad != 0)
        m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->Clear(ColorFromColorRef(palette->color(Palette::Background)));

    Gdiplus::GraphicsPath ph;
    RoundedPath(ph, CornerAll, x, y, width, height, rad);

    Gdiplus::SolidBrush prgBrush(ColorFromColorRef(palette->color(Palette::Base)));
    m_graphics->FillPath(&prgBrush, &ph);
    {
        int _x = x, _width;
        if (pulse_pos != -1) {
            _width = width/5;
            _x = x + (int)round(double((width - _width) * pulse_pos)/100);
        } else {
            if (progress < 0)
                progress = 0;
            else
            if (progress > 100)
                progress = 100;
            _width = (int)round(double(width * progress)/100);
        }
        if (_width != 0 && height != 0) {
            Gdiplus::GraphicsPath _ph;
            RoundedPath(_ph, CornerAll, _x, y, _width, height, rad);
            Gdiplus::SolidBrush chunkBrush(ColorFromColorRef(palette->color(Palette::AlternateBase)));
            m_graphics->FillPath(&chunkBrush, &_ph);
        }
    }

    if (brd_w != 0) {
        Gdiplus::Pen pen(ColorFromColorRef(palette->color(Palette::Border)), brd_w);
        m_graphics->DrawPath(&pen, &ph);
    }
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
#else
    int x = m_rc->x + brd_w + mrg.left;
    int y = m_rc->y + brd_w + mrg.top;
    int width = m_rc->width - brd_w * 2 - mrg.right - mrg.left;
    int height = m_rc->height - brd_w * 2 - mrg.bottom - mrg.top;
    int rad = metrics->value(Metrics::BorderRadius) * m_dpi;

    RoundedPath(m_cr, CornerAll, x, y, width, height, rad);

    COLORREF rgb = palette->color(Palette::Base);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    cairo_fill(m_cr);
    {
        int _x = x, _width;
        if (pulse_pos != -1) {
            _width = width/5;
            _x += m_rtl ? (int)round(double((width - _width) * (100 - pulse_pos)) / 100) : (int)round(double((width - _width) * pulse_pos) / 100);
        } else {
            if (progress < 0)
                progress = 0;
            else
            if (progress > 100)
                progress = 100;
            _width = (int)round(double(width * progress)/100);
            if (m_rtl)
                _x += width - _width;
        }
        if (_width != 0 && height != 0) {
            RoundedPath(m_cr, CornerAll, _x, y, _width, height, rad);
            rgb = palette->color(Palette::AlternateBase);
            cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
            cairo_fill(m_cr);
        }
    }

    if (brd_w != 0) {
        rgb = palette->color(Palette::Border);
        cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
        cairo_set_line_width(m_cr, brd_w);
        double d = brd_w / 2.0;
        RoundedPath(m_cr, CornerAll, x + d, y + d, width - 2*d, height - 2*d, metrics->value(Metrics::BorderRadius) - d);
        cairo_stroke(m_cr);
    }
#endif
}

void UIDrawingEngine::DrawString(const RECT &rc, const tstring &text, PlatformFont hFont, bool multiline, RECT *bounds)
{
    const Metrics *metrics = m_ds->metrics();
    int mrg_top = metrics->value(Metrics::TextMarginTop) * m_dpi;
    int mrg_left = (m_rtl ? metrics->value(Metrics::TextMarginRight) : metrics->value(Metrics::TextMarginLeft)) * m_dpi;
    int mrg_right = (m_rtl ? metrics->value(Metrics::TextMarginLeft) : metrics->value(Metrics::TextMarginRight)) * m_dpi;
    int mrg_bottom = metrics->value(Metrics::TextMarginBottom) * m_dpi;
#ifdef _WIN32
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    LOGFONTW logFont = {0};
    GetObject(hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(m_hdc, &logFont);
    Gdiplus::RectF rcF(rc.left + mrg_left, rc.top + mrg_top, rc.right - rc.left - mrg_right - mrg_left, rc.bottom - rc.top - mrg_bottom - mrg_top);
    Gdiplus::StringAlignment h_algn, v_algn;
    UINT algn = metrics->value(Metrics::TextAlignment);
    if (algn & Metrics::AlignHLeft)
        h_algn = Gdiplus::StringAlignmentNear;
    if (algn & Metrics::AlignHCenter)
        h_algn = Gdiplus::StringAlignmentCenter;
    if (algn & Metrics::AlignHRight)
        h_algn = Gdiplus::StringAlignmentFar;
    if (algn & Metrics::AlignVTop)
        v_algn = Gdiplus::StringAlignmentNear;
    if (algn & Metrics::AlignVCenter)
        v_algn = Gdiplus::StringAlignmentCenter;
    if (algn & Metrics::AlignVBottom)
        v_algn = Gdiplus::StringAlignmentFar;
    Gdiplus::StringFormat strFmt;
    strFmt.SetAlignment(h_algn);
    strFmt.SetLineAlignment(v_algn);
    INT flags = multiline ? Gdiplus::StringFormatFlagsLineLimit : Gdiplus::StringFormatFlagsNoWrap;
    if (m_rtl) {
        flags |= Gdiplus::StringFormatFlagsDirectionRightToLeft;
        if (!m_root_is_layered) {
            m_origMatrix = new Gdiplus::Matrix;
            m_graphics->GetTransform(m_origMatrix);
            Gdiplus::Matrix rtlMatrix(-1.0f, 0.0f, 0.0f, 1.0f, float(m_rc->right + m_rc->left - 1), 0.0f);
            m_graphics->SetTransform(&rtlMatrix);
        }
    }
    strFmt.SetFormatFlags(flags);
    Gdiplus::SolidBrush brush(ColorFromColorRef(m_ds->palette()->color(Palette::Text)));
    m_graphics->DrawString(text.c_str(), -1, &font, rcF, &strFmt, &brush);
    if (bounds) {
        Gdiplus::CharacterRange range(0, text.length());
        strFmt.SetFormatFlags(strFmt.GetFormatFlags() | Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
        strFmt.SetMeasurableCharacterRanges(1, &range);
        Gdiplus::Region rgn;
        m_graphics->MeasureCharacterRanges(text.c_str(), text.length(), &font, rcF, &strFmt, 1, &rgn);
        Gdiplus::RectF bnds;
        rgn.GetBounds(&bnds, m_graphics);
        bounds->left = bnds.X;
        bounds->top = bnds.Y;
        bounds->right = round(bnds.X + bnds.Width);
        bounds->bottom = round(bnds.Y + bnds.Height);
    }
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
    if (m_rtl && m_origMatrix) {
        m_graphics->SetTransform(m_origMatrix);
        delete m_origMatrix;
        m_origMatrix = nullptr;
    }
#else
    Rect _rc(rc.x + mrg_left, rc.y + mrg_top, rc.width - mrg_right - mrg_left, rc.height - mrg_bottom - mrg_top);

    COLORREF rgb = m_ds->palette()->color(Palette::Text);
    cairo_set_source_rgb(m_cr, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    PangoLayout *lut = pango_cairo_create_layout(m_cr);
    pango_layout_set_text(lut, text.c_str(), -1);
    pango_layout_set_font_description(lut, hFont);
    pango_layout_set_wrap(lut, PANGO_WRAP_WORD);
    pango_layout_set_width(lut, multiline ? _rc.width * PANGO_SCALE : -1);
    pango_layout_set_height(lut, _rc.height * PANGO_SCALE);

    int txt_w, txt_h;
    pango_layout_get_size(lut, &txt_w, &txt_h);
    txt_w /= PANGO_SCALE;
    txt_h /= PANGO_SCALE;

    int text_x = _rc.x;
    int text_y = _rc.y;
    PangoAlignment h_algn = PANGO_ALIGN_LEFT;
    int algn = metrics->value(Metrics::TextAlignment);
    if (algn & (m_rtl ? Metrics::AlignHRight : Metrics::AlignHLeft)) {
        h_algn = PANGO_ALIGN_LEFT;
    }
    if (algn & Metrics::AlignHCenter) {
        h_algn = PANGO_ALIGN_CENTER;
        if (!multiline)
            text_x += round((_rc.width - txt_w) / 2.0);
    }
    if (algn & (m_rtl ? Metrics::AlignHLeft : Metrics::AlignHRight)) {
        h_algn = PANGO_ALIGN_RIGHT;
        if (!multiline)
            text_x += _rc.width - txt_w;
    }
    if (algn & Metrics::AlignVTop)
        text_y += 0;
    if (algn & Metrics::AlignVCenter)
        text_y += round((_rc.height - txt_h) / 2.0);
    if (algn & Metrics::AlignVBottom)
        text_y += _rc.height - txt_h;

    pango_layout_set_alignment(lut, h_algn);
    cairo_save(m_cr);
    cairo_rectangle(m_cr, text_x, text_y, _rc.width, _rc.height);
    cairo_clip(m_cr);
    cairo_move_to(m_cr, text_x, text_y);
    pango_cairo_show_layout(m_cr, lut);
    cairo_restore(m_cr);
    g_object_unref(lut);
    if (bounds) {
        bounds->x = text_x;
        bounds->y = text_y;
        bounds->width = txt_w;
        bounds->height = txt_h;
    }
#endif
}

void UIDrawingEngine::DrawIcon(PlatformIcon hIcon) const
{
    const Metrics *metrics = m_ds->metrics();
    Margins mrg;
    GetIconMargins(mrg, metrics, m_dpi, m_rtl);
    Rect dst_rc;
#ifdef _WIN32
    GetIconPlacement(dst_rc, Rect(m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top), mrg, metrics, m_dpi, m_rtl);
    DrawIconEx(m_memDC, dst_rc.x, dst_rc.y, hIcon, dst_rc.width, dst_rc.height, 0, NULL, DI_NORMAL);
#else
    GetIconPlacement(dst_rc, *m_rc, mrg, metrics, m_dpi, m_rtl);
    int pb_width = gdk_pixbuf_get_width(hIcon);
    int pb_height = gdk_pixbuf_get_height(hIcon);
    double sx = (double)dst_rc.width / pb_width;
    double sy = (double)dst_rc.height / pb_height;
    cairo_save(m_cr);
    // cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_NONE);
    cairo_translate(m_cr, dst_rc.x, dst_rc.y);
    cairo_scale(m_cr, sx, sy);
    gdk_cairo_set_source_pixbuf(m_cr, hIcon, 0, 0);
    cairo_paint(m_cr);
    cairo_restore(m_cr);
#endif
}

void UIDrawingEngine::DrawShadow() const noexcept
{
    const Metrics *metrics = m_ds->metrics();
    constexpr int SHADOW_TRANSPATENCY = 0x26;
    int shadowWidth = metrics->value(Metrics::ShadowWidth) * m_dpi;
    int rad = metrics->value(Metrics::ShadowRadius) * m_dpi;
#ifdef _WIN32
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    for (int i = 0; i < shadowWidth; i++) {
        int x = m_rc->left + i + 1;
        int y = m_rc->top + i + 1;
        int width = m_rc->right - m_rc->left - i * 2 - 2;
        int height = m_rc->bottom - m_rc->top - i * 2 - 2;

        Gdiplus::GraphicsPath ph;
        RoundedPath(ph, CornerAll, x, y, width, height, (2.0 - i / double(shadowWidth - 1)) * rad);
        int alpha = shadowWidth > 1 ? SHADOW_TRANSPATENCY * (i * i) / ((shadowWidth - 1) * (shadowWidth - 1)) : SHADOW_TRANSPATENCY;
        Gdiplus::Pen pen(Gdiplus::Color(alpha, 0, 0, 0), 1);
        m_graphics->DrawPath(&pen, &ph);
    }
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
#else
    shadowWidth += m_dpi;
    for (int i = 0; i < shadowWidth; i++) {
        int x = m_rc->x + i;
        int y = m_rc->y + i;
        int width = m_rc->width - i * 2;
        int height = m_rc->height - i * 2;

        RoundedPath(m_cr, CornerAll, x, y, width, height, (2.0 - i / double(shadowWidth - 1)) * rad);
        int alpha = shadowWidth > 1 ? SHADOW_TRANSPATENCY * (i * i) / ((shadowWidth - 1) * (shadowWidth - 1)) : SHADOW_TRANSPATENCY;
        cairo_set_source_rgba(m_cr, 0, 0, 0, (double)alpha / 255);
        cairo_set_line_width(m_cr, 1);
        cairo_stroke(m_cr);
    }
#endif
}

#ifdef _WIN32
void UIDrawingEngine::Begin(UIDrawningSurface *ds, HWND hwnd, RECT *rc, double dpi)
{
    if (m_ds) {
        printf("Engine is buisy...\n");
        fflush(stdout);
        return;
    }
    m_ds = ds;
    m_rc = rc;
    m_dpi = dpi;
    m_hwnd = hwnd;
    m_hdc = BeginPaint(hwnd, &m_ps);

    void *pixels = NULL;
    BITMAPINFO bm = {};
    bm.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bm.bmiHeader.biWidth = m_rc->right - m_rc->left;
    bm.bmiHeader.biHeight = m_rc->top - m_rc->bottom;
    bm.bmiHeader.biPlanes = 1;
    bm.bmiHeader.biBitCount = 32;
    bm.bmiHeader.biCompression = BI_RGB;

    m_memDC = CreateCompatibleDC(m_hdc);
    m_memBmp = CreateDIBSection(m_hdc, &bm, DIB_RGB_COLORS, &pixels, NULL, 0);
    m_oldBmp = (HBITMAP)SelectObject(m_memDC, m_memBmp);

    m_graphics = new Gdiplus::Graphics(m_memDC);
}

void UIDrawingEngine::DrawTopBorder(int brdWidth, COLORREF brdColor) const
{
    HPEN pen = CreatePen(PS_SOLID, brdWidth, brdColor);
    HPEN oldPen = (HPEN)SelectObject(m_memDC, pen);
    MoveToEx(m_memDC, m_rc->left - 1, m_rc->top + brdWidth - 1, NULL);
    LineTo(m_memDC, m_rc->right, m_rc->top + brdWidth - 1);
    SelectObject(m_memDC, oldPen);
    DeleteObject(pen);
}

void UIDrawingEngine::DrawEmfIcon(Gdiplus::Metafile *hEmf) noexcept
{
    const Metrics *metrics = m_ds->metrics();
    Margins mrg;
    GetIconMargins(mrg, metrics, m_dpi, m_rtl);
    Rect dst_rc;
    GetIconPlacement(dst_rc, Rect(m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top), mrg, metrics, m_dpi, m_rtl);
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    if (m_rtl) {
        if (!m_root_is_layered) {
            m_origMatrix = new Gdiplus::Matrix;
            m_graphics->GetTransform(m_origMatrix);
            Gdiplus::Matrix rtlMatrix(-1.0f, 0.0f, 0.0f, 1.0f, float(m_rc->right + m_rc->left - 1), 0.0f);
            m_graphics->SetTransform(&rtlMatrix);
        }
    }
    m_graphics->DrawImage(hEmf, dst_rc.x, dst_rc.y, dst_rc.width, dst_rc.height);
    if (m_rtl && m_origMatrix) {
        m_graphics->SetTransform(m_origMatrix);
        delete m_origMatrix;
        m_origMatrix = nullptr;
    }
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
}

void UIDrawingEngine::DrawImage(Gdiplus::Bitmap *hBmp) const noexcept
{
    const Metrics *metrics = m_ds->metrics();    
    Margins mrg;
    GetIconMargins(mrg, metrics, m_dpi, m_rtl);
    Rect dst_rc;
    GetIconPlacement(dst_rc, Rect(m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top), mrg, metrics, m_dpi, m_rtl);
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeLowQuality);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
    // m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->DrawImage(hBmp, dst_rc.x, dst_rc.y, dst_rc.width, dst_rc.height);
    m_graphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    m_graphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeDefault);
}

void UIDrawingEngine::End() noexcept
{
    BitBlt(m_hdc, m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, m_memDC, 0, 0, SRCCOPY);

    delete m_graphics;
    m_graphics = nullptr;

    SelectObject(m_memDC, m_oldBmp);
    m_oldBmp = nullptr;
    DeleteObject(m_memBmp);
    m_memBmp = nullptr;
    DeleteDC(m_memDC);
    m_memDC = nullptr;

    EndPaint(m_hwnd, &m_ps);
    m_hdc = nullptr;
    m_hwnd = nullptr;
    m_rc = nullptr;
    m_ds = nullptr;
}

void UIDrawingEngine::LayeredBegin(UIDrawningSurface *ds, HWND hwnd, RECT *rc, double dpi)
{
    if (m_ds) {
        printf("Engine is buisy....\n");
        fflush(stdout);
        return;
    }
    m_ds = ds;
    m_rc = rc;
    m_dpi = dpi;
    m_hwnd = hwnd;
    m_hdc = GetDC(m_hwnd);
    m_bmp = new Gdiplus::Bitmap(rc->right - rc->left, rc->bottom - rc->top, PixelFormat32bppARGB);
    m_graphics = new Gdiplus::Graphics(m_bmp);
    m_graphics->Clear(Gdiplus::Color(0,0,0,0));
    m_root_is_layered = true;
}

void UIDrawingEngine::LayeredChildBegin(UIDrawningSurface *ds, HWND hwnd, RECT *rc, double dpi)
{
    // if (m_ds) {
    //     printf("Engine is buisy...\n");
    //     fflush(stdout);
    //     return;
    // }
    m_tmp_ds = m_ds;
    m_tmp_rc = m_rc;
    m_tmp_dpi = m_dpi;
    m_tmp_hwnd = m_hwnd;
    m_tmp_hdc = m_hdc;

    POINT pt = {0,0};
    ClientToScreen(hwnd, &pt);
    if (HWND rootHwnd = GetAncestor(hwnd, GA_ROOT)) {
        ScreenToClient(rootHwnd, &pt);
        if (m_rtl) {
            RECT rootRc;
            GetClientRect(rootHwnd, &rootRc);
            pt.x = rootRc.right - pt.x - (rc->right - rc->left);
        }
    }
    OffsetRect(rc, pt.x, pt.y);

    m_ds = ds;
    m_rc = rc;
    m_dpi = dpi;
    m_hwnd = hwnd;
    m_hdc = GetDC(m_hwnd);
}

void UIDrawingEngine::LayeredUpdate(BYTE alpha) const
{
    EnumChildWindows(m_hwnd, EnumChildProc, 0);

    HDC memDC = CreateCompatibleDC(m_hdc);
    HBITMAP memBmp;
    m_bmp->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &memBmp);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    RECT wrc;
    GetWindowRect(m_hwnd, &wrc);
    POINT ptSrc = {0, 0};
    POINT ptDst = {wrc.left, wrc.top};
    SIZE szDst = {wrc.right - wrc.left, wrc.bottom - wrc.top};
    BLENDFUNCTION bf;
    bf.AlphaFormat = AC_SRC_ALPHA;
    bf.BlendFlags = 0;
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = alpha;
    UpdateLayeredWindow(m_hwnd, m_hdc, &ptDst, &szDst, memDC, &ptSrc, 0, &bf, ULW_ALPHA);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

void UIDrawingEngine::LayeredEnd() noexcept
{
    delete m_graphics;
    m_graphics = nullptr;
    delete m_bmp;
    m_bmp = nullptr;
    ReleaseDC(m_hwnd, m_hdc);
    m_hdc = nullptr;
    m_hwnd = nullptr;
    m_rc = nullptr;
    m_ds = nullptr;
    m_root_is_layered = false;
}

void UIDrawingEngine::LayeredChildEnd() noexcept
{
    ReleaseDC(m_hwnd, m_hdc);
    m_hdc = m_tmp_hdc;
    m_hwnd = m_tmp_hwnd;
    m_rc = m_tmp_rc;
    m_ds = m_tmp_ds;
    m_dpi = m_tmp_dpi;
}
#else
void UIDrawingEngine::Begin(UIDrawningSurface *ds, cairo_t *cr, Rect *rc) noexcept
{
    if (m_ds) {
        printf("Engine is buisy....\n");
        fflush(stdout);
        return;
    }
    m_ds = ds;
    m_cr = cr;
    m_rc = rc;
}

void UIDrawingEngine::DrawSvgIcon(_RsvgHandle *hSvg) const noexcept
{
    const Metrics *metrics = m_ds->metrics();
    Margins mrg;
    GetIconMargins(mrg, metrics, m_dpi, m_rtl);
    Rect dst_rc;
    GetIconPlacement(dst_rc, *m_rc, mrg, metrics, m_dpi, m_rtl);

    RsvgDimensionData dm;
    rsvg_handle_get_dimensions(hSvg, &dm);

    cairo_save(m_cr);
    cairo_translate(m_cr, dst_rc.x, dst_rc.y);
    cairo_scale(m_cr, (double)dst_rc.width / dm.width, (double)dst_rc.height / dm.height);
    rsvg_handle_render_cairo(hSvg, m_cr);
    cairo_restore(m_cr);
}

void UIDrawingEngine::End() noexcept
{
    m_rc = nullptr;
    m_cr = nullptr;
    m_ds = nullptr;
}
#endif
