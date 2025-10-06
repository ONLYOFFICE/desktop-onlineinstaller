#include "uifontmetrics.h"
#include "uiwidget.h"
#ifdef _WIN32
# include <gdiplus.h>
#endif

UIFontMetrics::UIFontMetrics()
{

}

UIFontMetrics::~UIFontMetrics()
{

}

void UIFontMetrics::textSize(UIWidget *wgt, PlatformFont hFont, const tstring &text, int &width, int &height) const
{
    PlatformWindow hWnd = wgt->platformWindow();
#ifdef _WIN32
    HDC hdc = GetDC(hWnd);
    Gdiplus::Graphics gr(hdc);
    LOGFONTW logFont = {0};
    GetObject(hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(hdc, &logFont);
    Gdiplus::RectF lutRc, boxRc;
    gr.MeasureString(text.c_str(), text.length(), &font, lutRc, &boxRc);
    ReleaseDC(hWnd, hdc);
    width = boxRc.Width;
    height = boxRc.Height;
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(hWnd, text.c_str());
    pango_layout_set_font_description(lut, hFont);
    pango_layout_get_size(lut, &width, &height);
    width /= PANGO_SCALE;
    height /= PANGO_SCALE;
    g_object_unref(lut);
#endif
}
